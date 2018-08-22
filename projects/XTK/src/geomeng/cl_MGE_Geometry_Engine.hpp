/*
 * cl_Geometry_Engine.hpp
 *
 *  Created on: Jun 21, 2017
 *      Author: ktdoble
 */

#ifndef SRC_GEOMENG_CL_MGE_GEOMETRY_ENGINE_HPP_
#define SRC_GEOMENG_CL_MGE_GEOMETRY_ENGINE_HPP_

// Standard library includes
#include <memory> // for shared_ptr
#include <math.h>

#include "linalg/cl_XTK_Matrix_Base_Utilities.hpp"
#include "linalg/cl_XTK_Matrix_Base.hpp"
#include "geometry/cl_Geometry.hpp"

// Geometry Engine Includes
#include "geomeng/cl_MGE_Enums.hpp"
#include "geomeng/cl_MGE_Geometry_Object.hpp"
#include "geomeng/cl_MGE_Geometry_Object_Manager.hpp"

// XTKL: General Includes
#include "assert/fn_xtk_assert.hpp"
#include "ios/cl_Logger.hpp"

// XTKL: Container includes
#include "containers/cl_XTK_Cell.hpp"

// XTKL: Tools includes
#include "tools/cl_Interpolaton.hpp"

//XTKL: Geometry Engine Includes
#include "xtk/cl_XTK_Phase_Table.hpp"
#include "xtk/cl_XTK_Pending_Node.hpp"

//XTKL: Topology
#include "topology/cl_XTK_Topology.hpp"
#include "topology/cl_XTK_Basis_Function.hpp"


namespace xtk
{


/*
 *
 * $\frac{\partial{\phi_A}}{\partial{p}}$ (change in phi with respect to a design variable
 * See for more detailed description of this function:
 *
 */
template<typename Real, typename Real_Matrix>
void compute_dx_dp_with_linear_basis(Mat<Real,Real_Matrix>  & aDPhiADp,
                                     Mat<Real,Real_Matrix>  & aDPhiBDp,
                                     Mat<Real,Real_Matrix>  & aEdgeCoordinates,
                                     Mat<Real,Real_Matrix>  & aEdgeNodePhi,
                                     Mat<Real,Real_Matrix>  & aDxDp)
{

    XTK_ASSERT(aDPhiADp.get_num_rows() != 0,"dPhi/dp not implemented in geometry would cause a seg fault here");
    XTK_ASSERT(aDPhiBDp.get_num_rows() != 0,"dPhi/dp not implemented in geometry would cause a seg fault here");
    Real const & tPhiA = aEdgeNodePhi(0,0);
    Real const & tPhiB = aEdgeNodePhi(1,0);

    // Initialize
    Mat<Real,Real_Matrix> tXa = aEdgeCoordinates.get_row(0);

    Mat<Real,Real_Matrix> tXb = aEdgeCoordinates.get_row(1);

    // Compute $\frac{\partial x_{\Gamma}}{\partial \phi}$
    Real_Matrix tDxgammaDphiA = -(tPhiB)/std::pow((tPhiA-tPhiB),2)*(tXb.matrix_data()-tXa.matrix_data());
    Real_Matrix tDxgammaDphiB =  (tPhiA)/std::pow((tPhiA-tPhiB),2)*(tXb.matrix_data()-tXa.matrix_data());

    Mat<Real,Real_Matrix> tDxgDphiAMat(tDxgammaDphiA);
    Mat<Real,Real_Matrix> tDxgDphiBMat(tDxgammaDphiB);

    // Compute dx/dp
    Real_Matrix tDxDp = aDPhiADp * transpose(tDxgDphiAMat) +  aDPhiBDp * transpose(tDxgDphiBMat);
    aDxDp = Mat<Real,Real_Matrix>(tDxDp);

}


template<typename Real, typename Integer, typename Real_Matrix, typename Integer_Matrix>
class Geometry_Engine
{

public:

    // Single geometry constructor
    Geometry_Engine(Geometry<Real, Integer, Real_Matrix, Integer_Matrix>  & aGeometry,
                    Phase_Table<Integer, Integer_Matrix> const & aPhaseTable) :
        mThresholdValue(0),
        mComputeDxDp(false),
        mActiveGeometryIndex(0),
        mGeometryObjects(),
        mPhaseTable(aPhaseTable)
    {
        mPerturbationValue = 0.0000005;
        mGeometry.push_back(&aGeometry);
    }

    Geometry_Engine(Cell<Geometry<Real, Integer, Real_Matrix, Integer_Matrix>*> const  & aGeometry,
                    Phase_Table<Integer, Integer_Matrix> const & aPhaseTable) :
        mThresholdValue(0),
        mComputeDxDp(false),
        mActiveGeometryIndex(0),
        mGeometry(aGeometry),
        mPhaseTable(aPhaseTable)
    {
        mPerturbationValue = 0.0000005;
    }


    Geometry_Engine()
    {
    }

    // Options which the user can change (all are given defaults)
    Real mThresholdValue;
    Real mPerturbationValue;
    bool mComputeDxDp; // Should be turned off if a sensitivity has not been implemented


    /*
     * Initiali allocation of geometry objects, this creates a geometry object for each node coordinate.
     * In this case, aNodeCoords needs to be ordered by proc indices.
     */
    void
    create_geometry_objects_for_background_mesh_nodes(Mat<Real,Real_Matrix> const & aNodeCoords)
    {
        // Allocate space
        Integer tNumNodes = aNodeCoords.get_num_rows();
        mNodePhaseVals = Mat<Real,Real_Matrix>(tNumNodes,mGeometry.size(),0);

        // Allocate geometry object
        Cell<Geometry_Object<Real, Integer, Real_Matrix,Integer_Matrix>> tGeometryObjects(tNumNodes);

        // Allocate sensitivity (even though these nodes have none)
        mDxDp = Cell<Mat<Real,Real_Matrix>>(tNumNodes,Mat<Real,Real_Matrix>(0,0,0.0));
        mNodeADVIndices = Cell<Mat<Integer,Integer_Matrix>>(tNumNodes,Mat<Integer,Integer_Matrix>(0,0,0));

        // Associate each geometry object with a row in phase val matrix (note phase val computed later)
        Mat<Integer, Integer_Matrix> tNodeIndex(1,tNumNodes);
        for(Integer i = 0; i<tNumNodes; i++)
        {
            tGeometryObjects(i).set_phase_val_row(i);
            tNodeIndex(0,i) = i;
        }

        // Place these in the geometry object manager
        mGeometryObjects.store_geometry_objects(tNodeIndex,tGeometryObjects);

        // Loop through each geometry and then each node and compute the level set field value
        // add value to phase value matrix
        for(Integer j = 0; j<get_num_geometries(); j++)
        {
            bool tAnalyticFlag = mGeometry(j)->is_analytic();

            // Analytic
            if(tAnalyticFlag)
            {
                for(Integer i = 0; i<tNumNodes; i++ )
                {
                    mNodePhaseVals(i,j) = mGeometry(j)->evaluate_field_value_with_coordinate(i,aNodeCoords);
                }
            }

            // Discrete
            else
            {
                for(Integer i = 0; i<tNumNodes; i++ )
                {
                    mNodePhaseVals(i,j) = mGeometry(j)->access_field_value_with_entity_index(i,EntityRank::NODE);
                }
            }
        }

    }

    /*
     * Creates a geometry object association for pending nodes
     * These ndoes have node indices and parent information
     */
    void
    associate_new_nodes_with_geometry_object(Cell<Pending_Node<Real, Integer,Real_Matrix, Integer_Matrix>> & aNewNodes,
                                             bool aInterfaceNodes)
    {

        // Allocate space
        Integer tNumNewNodes = aNewNodes.size();
        Integer tNumCurrNodes = mNodePhaseVals.get_num_rows();

        mNodePhaseVals.resize(tNumNewNodes+tNumCurrNodes,mGeometry.size());

        // Allocate sensitivity data
        mDxDp.resize(tNumNewNodes+tNumCurrNodes,Mat<Real,Real_Matrix>(0,0,0.0));
        mNodeADVIndices.resize(tNumNewNodes+tNumCurrNodes,Mat<Integer,Integer_Matrix>(0,0,0));

        Cell<Geometry_Object<Real, Integer, Real_Matrix,Integer_Matrix>> tGeometryObjects(tNumNewNodes);

        Mat<Integer, Integer_Matrix> tNodeIndex(1,tNumNewNodes);
        for(Integer i = 0; i<tNumNewNodes; i++)
        {
            tGeometryObjects(i).set_phase_val_row(i+tNumCurrNodes);
            tNodeIndex(0,i) = aNewNodes(i).get_node_index();
        }


        // If these are nodes on an interface and the sensitivity has been computed.
        // Add the dxdp information to the member variables
        if(aInterfaceNodes && mComputeDxDp)
        {
            for(Integer i = 0; i<tNumNewNodes; i++)
            {
                mDxDp(i+tNumCurrNodes) = aNewNodes(i).get_sensitivity_dx_dp();
                mNodeADVIndices(i+tNumCurrNodes) = aNewNodes(i).get_node_adv_indices();
            }
        }

        if(tNumNewNodes !=0)
        {
        mGeometryObjects.store_geometry_objects(tNodeIndex,tGeometryObjects);
        }

        for(Integer j = 0; j<get_num_geometries(); j++)
        {

            for(Integer i = 0; i<tNumNewNodes; i++ )
            {
                // Ask the pending node about its parent
                // This information is needed to know what to interpolate based on
                Mat<Real,Real_Matrix> const & tLocalCoordinate = aNewNodes(i).get_local_coordinate_relative_to_parent();
                Mat<Real,Real_Matrix> tLevelSetValues(1,1);
                Topology<Real, Integer, Real_Matrix, Integer_Matrix> const & tParentTopology = aNewNodes(i).get_parent_topology();

                // Interpolate all level set values to node
                this->interpolate_level_set_value_to_child_node_location(tParentTopology, j,tLocalCoordinate,tLevelSetValues);
                mNodePhaseVals(i+tNumCurrNodes,j) = tLevelSetValues(0,0);
            }

        }


    }


    /** is_intersected checks to see if an entity provided to it intersects a geometry field. Intersects in this context
     * means a geometry crosses a certain threshold (typically 0). For levelset fields, this can be thought of as a phase change
     *
     * @param[in] aNodeCoords       - Node coordinate
     * @param[in] aNodeToEntityConn - Connectivity between nodes and parent entity
     * @param[in] aCheckType        - Specifies what type of intersection check is to be performed
     *                                   0 - No information on interface required
     *                                   1 - information on interface required
     */
    void is_intersected(Mat<Real,Real_Matrix> const &                                      aNodeCoords,
                        Mat<Integer, Integer_Matrix> const &                               aNodetoEntityConn,
                        Integer                                                            aCheckType,
                        Cell<Geometry_Object<Real, Integer, Real_Matrix,Integer_Matrix>> & aGeometryObjects)
    {
        //Get information for loops
        Integer tNumEntities = aNodetoEntityConn.get_num_rows(); // Number of entities provided to the geometry engine
        Integer tNumConnected = aNodetoEntityConn.get_num_columns();

        //Initialize
        Integer tIntersectedCount = 0;    // Intersected element counter
        aGeometryObjects.resize(tNumEntities,Geometry_Object<Real,Integer, Real_Matrix,Integer_Matrix>());


        // Reserve space to host interface nodes
        if(aCheckType == 1)
        {
            mDxDp.reserve(tNumEntities);
        }

        //Loop over elements and determine if the element has an intersection
        for(Integer i = 0; i < tNumEntities; i++)
        {

            //Populate the intersection flag of this element with a bool
            Mat<Integer, Integer_Matrix> tRow = aNodetoEntityConn.get_row(i);
            Mat<Integer, Integer_Matrix> tNodeADVIndices;
            Cell<Mat<Real,Real_Matrix>>tIntersectionInfo = compute_intersection_info(tRow, aNodeCoords, aCheckType,tNodeADVIndices);

            if((tIntersectionInfo(0)(0, 0) == true))
            {

                aGeometryObjects(tIntersectedCount).set_parent_entity_index(i);

                if(aCheckType == 1)
                {
                    aGeometryObjects(tIntersectedCount).set_interface_loc_coord(tIntersectionInfo(0)(0, 1));
                    aGeometryObjects(tIntersectedCount).set_interface_glb_coord(tIntersectionInfo(1));

                    if(mComputeDxDp)
                    {
                        aGeometryObjects(tIntersectedCount).set_sensitivity_dx_dp(tIntersectionInfo(2));
                        aGeometryObjects(tIntersectedCount).set_node_adv_indices(tNodeADVIndices);
                    }
                }

                tIntersectedCount++;
                continue;
            }
        }

        // resize
        aGeometryObjects.resize(tIntersectedCount, Geometry_Object<Real, Integer, Real_Matrix,Integer_Matrix>());
    }


    /*
     * Computes the intersection of an isocountour with an entity and returning the local coordinate relative to the parent
     * and the global coordinate if needed
     */
    void
    get_intersection_location(Real const &                         aIsocontourThreshold,
                              Real const &                         aPerturbationThreshold,
                              Mat<Real,Real_Matrix> const &        aGlobalNodeCoordinates,
                              Mat<Real,Real_Matrix> const &        aEntityNodeVars,
                              Mat<Integer, Integer_Matrix> const & aEntityNodeIndices,
                              Mat<Real,Real_Matrix> &              aIntersectionLocalCoordinates,
                              Mat<Real,Real_Matrix> &              aIntersectionGlobalCoordinates,
                              bool                                 aCheckLocalCoordinate = true,
                              bool                                 aComputeGlobalCoordinate = false)
    {

            // compute the local coordinate where the intersection occurs
            Interpolation::linear_interpolation_value(aEntityNodeVars, aIsocontourThreshold, aIntersectionLocalCoordinates);


            // Perturb away from node if necessary
            if(aCheckLocalCoordinate)
            {
                if(aIntersectionLocalCoordinates(0, 0) >= 1-aPerturbationThreshold)
                {
                    aIntersectionLocalCoordinates(0, 0) = aIntersectionLocalCoordinates(0, 0) - aPerturbationThreshold;
                }

                if(aIntersectionLocalCoordinates(0, 0) <= -1+aPerturbationThreshold)
                {
                    aIntersectionLocalCoordinates(0, 0) = aIntersectionLocalCoordinates(0, 0) + aPerturbationThreshold;
                }
            }

            // Compute the global coordinate only if you plan to use it
            if(aComputeGlobalCoordinate)
            {
                // Place only the entity coordinates in a matrix
                Mat<Real,Real_Matrix> tEntityCoordinates(2,3);
                replace_row(aEntityNodeIndices(0,0), aGlobalNodeCoordinates,0,tEntityCoordinates);
                replace_row(aEntityNodeIndices(0,1), aGlobalNodeCoordinates,1,tEntityCoordinates);

                // compute the global coordinate
                Interpolation::linear_interpolation_location(tEntityCoordinates,aIntersectionLocalCoordinates,aIntersectionGlobalCoordinates);
            }
    }


    void
    compute_dx_dp_finite_difference(Real                                 const & aPerturbationVal,
                                    Mat<Real,Real_Matrix>                const & aGlobalNodeCoordinates,
                                    Mat<Real,Real_Matrix>                const & aEntityNodeCoordinates,
                                    Mat<Real,Real_Matrix>                const & aIntersectionLclCoordinate,
                                    Mat<Integer, Integer_Matrix>         const & aEntityNodeIndices,
                                    Mat<Real,Real_Matrix>       & aEntityNodeVars,
                                    Mat<Real,Real_Matrix>       & aDxDp)
    {

        Integer tNumNodeVars = aEntityNodeVars.get_num_rows();
        XTK_ASSERT(tNumNodeVars == 2,"Currently compute_dx_dp_finite_difference has only been tested on edges");
        aDxDp.resize(2,3);

        Real tPerturbationLen = 2*aPerturbationVal;
        Real tScale   = 1/tPerturbationLen;
        Cell<Real>  tPerturbationSign = {1,-1};

        Mat<Real,Real_Matrix>       tDxDp(1,3);
        Mat<Real,Real_Matrix>       tPerturbedLocalCoordinate(1,1);
        Cell<Mat<Real,Real_Matrix>> tPerturbedGlobCoordinates = {Mat<Real,Real_Matrix>(1,3),
                                                                 Mat<Real,Real_Matrix>(1,3)};
        // Loop over all the nodes and perturb up and down
        for(Integer i = 0; i<tNumNodeVars; i++)
        {
            // Perturb up and down
            for(Integer j = 0; j<2; j++)
            {

                Real tPerturb = tPerturbationSign(j)*aPerturbationVal;
                // Perturb
                aEntityNodeVars(i,0) = aEntityNodeVars(i,0) + tPerturb;

                // Locate perturbed interface
                get_intersection_location(mThresholdValue, aPerturbationVal,aGlobalNodeCoordinates,aEntityNodeVars, aEntityNodeIndices, tPerturbedLocalCoordinate, tPerturbedGlobCoordinates(j),false, true);

                // Reverse perturb
                aEntityNodeVars(i,0) = aEntityNodeVars(i,0) - tPerturb;

            }

            tDxDp.matrix_data() = tScale*(tPerturbedGlobCoordinates(1).matrix_data() - tPerturbedGlobCoordinates(0).matrix_data());

            replace_row(0,tDxDp,i,aDxDp);


        }
    }


    void
    compute_dx_dp_for_an_intersection(Mat<Integer, Integer_Matrix> const & aEntityNodeIndices,
                                      Mat<Real,Real_Matrix> const &       aGlobalNodeCoordinates,
                                      Mat<Real,Real_Matrix> const &       aIntersectionLclCoordinate,
                                      Mat<Real,Real_Matrix> &             aEntityNodeVars,
                                      Mat<Real,Real_Matrix> &             aDxDp,
                                      Mat<Integer, Integer_Matrix> & aADVIndices)
    {
        Real tPerturbationLength = 0.005;
        Integer tNumNodes = aEntityNodeIndices.get_num_columns();

        XTK_ASSERT(tNumNodes == 2,"Currently, this function is only supported on edges");

        // Initialize
        Cell<Mat<Real,Real_Matrix>> tDPhiiDp = {Mat<Real,Real_Matrix>(0,0),
                                                Mat<Real,Real_Matrix>(0,0)};

        Mat<Real,Real_Matrix> tEntityNodeCoordinates(2,3);

        // Assemble the entity local coordinates
        replace_row(aEntityNodeIndices(0,0), aGlobalNodeCoordinates,0,tEntityNodeCoordinates);
        replace_row(aEntityNodeIndices(0,1), aGlobalNodeCoordinates,1,tEntityNodeCoordinates);

        // Get information from a analytic geometry
        if(ActiveGeometry().is_analytic())
        {
            aADVIndices = get_node_adv_indices_analytic();
            for(Integer i = 0; i < tNumNodes; i++)
            {
                tDPhiiDp(i) = ActiveGeometry().evaluate_sensitivity_dphi_dp_with_coordinate(aEntityNodeIndices(0, i),aGlobalNodeCoordinates);
            }

            compute_dx_dp_with_linear_basis(tDPhiiDp(0), tDPhiiDp(1), tEntityNodeCoordinates, aEntityNodeVars, aDxDp);
        }

        // Get information from a discrete geometry
        else
        {
            aADVIndices = get_node_adv_indices_discrete(aEntityNodeIndices);
            compute_dx_dp_finite_difference(tPerturbationLength, aGlobalNodeCoordinates, tEntityNodeCoordinates, aIntersectionLclCoordinate,aEntityNodeIndices,aEntityNodeVars, aDxDp);
        }
    }

    /**
     * Returns a reference to the geometry object at the provided index
     */
    Geometry_Object<Real,Integer,Real_Matrix,Integer_Matrix> &
    get_geometry_object(Integer const & aNodeIndex)
    {
       return mGeometryObjects.get_geometry_object_from_manager(aNodeIndex);
    }

    /**
     * Returns a reference to the geometry object at the provided index
     */
    Geometry_Object<Real,Integer,Real_Matrix,Integer_Matrix> const &
    get_geometry_object(Integer const & aNodeIndex) const
    {
       return mGeometryObjects.get_geometry_object_from_manager(aNodeIndex);
    }

    Integer get_num_phases()
    {
        return mPhaseTable.get_num_phases();
    }


    /*
     * Get phase value for a given node and geometry index
     */
    Real
    get_entity_phase_val(Integer const & aNodeIndex,
                         Integer const & aGeomIndex)
    {
        Geometry_Object<Real,Integer,Real_Matrix,Integer_Matrix> & tNodesGeoObj = get_geometry_object(aNodeIndex);
        Integer tNodeRowIndex = tNodesGeoObj.get_phase_val_row();

        return mNodePhaseVals(tNodeRowIndex,aGeomIndex);
    }

    /*
     * Get dxdp for a node
     */
    Mat<Real,Real_Matrix> const &
    get_node_dx_dp(Integer const & aNodeIndex) const
    {
        Geometry_Object<Real,Integer,Real_Matrix,Integer_Matrix> const & tNodesGeoObj = get_geometry_object(aNodeIndex);
        Integer tNodeRowIndex = tNodesGeoObj.get_phase_val_row();
        return mDxDp(tNodeRowIndex);
    }

    Mat<Integer, Integer_Matrix> const &
    get_node_adv_indices(Integer const & aNodeIndex) const
    {
        Geometry_Object<Real,Integer,Real_Matrix,Integer_Matrix> const & tNodesGeoObj = get_geometry_object(aNodeIndex);
        Integer tNodeRowIndex = tNodesGeoObj.get_phase_val_row();
        return mNodeADVIndices(tNodeRowIndex);
    }



    /*
     * For a given, node index return the phase index relative to each geometry (i.e. inside/outside indicator)
     */
    void get_phase_index(Mat<Integer,Integer_Matrix> const & aNodeIndex,
                         Mat<Integer, Integer_Matrix> & aNodePhaseIndex)
    {
        // 0 for neg 1 for pos
        Real tNodePhaseValue = 0;
        Mat<Integer,Integer_Matrix> tPhaseOnOff(1,mGeometry.size());

        for(Integer i = 0; i<aNodeIndex.get_num_columns(); i++)
        {
            Geometry_Object<Real,Integer,Real_Matrix,Integer_Matrix> & tNodesGeoObj = get_geometry_object(aNodeIndex(0,i));
            Integer tNodeRowIndex = tNodesGeoObj.get_phase_val_row();

            for(Integer iG = 0; iG<mGeometry.size(); iG++)
            {
                tNodePhaseValue =  mNodePhaseVals(tNodeRowIndex,iG);

                // Negative
                if(tNodePhaseValue<mThresholdValue)
                {
                    tPhaseOnOff(0,iG) = 0;
                }

                else
                {
                    tPhaseOnOff(0,iG) = 1;
                }
            }
            aNodePhaseIndex(i,0) = mPhaseTable.get_phase_index(tPhaseOnOff);
        }

    }

    /*
     * Provided the inside and out phase values for an entity, return the phase index
     */
    Integer
    get_elem_phase_index(Mat<Integer,Integer_Matrix> const & aElemOnOff)
    {
        return mPhaseTable.get_phase_index(aElemOnOff);
    }

    /*
     * Returns whether a node is inside or outside wrt to a given geometry index
     */
    Integer
    get_node_phase_index_wrt_a_geometry(Integer aNodeIndex,
                                        Integer aGeometryIndex)
    {
        Geometry_Object<Real,Integer,Real_Matrix,Integer_Matrix> & tNodesGeoObj = get_geometry_object(aNodeIndex);
        Integer tNodeRowIndex = tNodesGeoObj.get_phase_val_row();

        Real tNodePhaseVal = mNodePhaseVals(tNodeRowIndex,aGeometryIndex);

        Integer tPhaseOnOff = 10000;

        if(tNodePhaseVal<mThresholdValue)
        {
            tPhaseOnOff = 0;
        }
        else
        {
            tPhaseOnOff = 1;
        }

        return tPhaseOnOff;
    }

    /*
     * Returns whether the active geometry is analytic
     */
    bool is_geometry_analytic()
    {
        return ActiveGeometry().is_analytic();
    }

    /*
     * Returns the number of geometries
     */
    Integer get_num_geometries()
    {
        return mGeometry.size();
    }

    /*
     * Returns the active geometry index
     */
    Integer get_active_geometry_index()
    {
        return mActiveGeometryIndex;
    }

    /*
     * Advance the active geometry index
     */
    void advance_geometry_index()
    {
        XTK_ASSERT(mActiveGeometryIndex<get_num_geometries(),"Trying to advance past the number of geometries in the geometry engine");
        mActiveGeometryIndex += 1;
    }

    Mat<Integer, Integer_Matrix>
    get_node_adv_indices_analytic()
    {
        Integer tNumDVS = get_num_design_vars_analytic();
        Mat<Integer, Integer_Matrix> tMatrix(1,tNumDVS);
        for(Integer i = 0; i<tNumDVS; i++)
        {
            tMatrix(0,i) = i;
        }
        return tMatrix;
    }


    /*
     * Returns the ADV indices of the provided nodes
     */
    Mat<Integer, Integer_Matrix>
    get_node_adv_indices_discrete(Mat<Integer,Integer_Matrix> const & aEntityNodes)
    {
        Mat<Integer, Integer_Matrix> tNodeADVIndices = ActiveGeometry().get_node_adv_indices(aEntityNodes);
        return tNodeADVIndices;
    }

    Integer
    get_num_design_vars_analytic()
    {
        Integer tNumRows = 0;
        Integer tNumCols = 0;
        Integer tNumDVs = 0;
        for(Integer i = 0; i<mGeometry.size(); i++)
        {
            mGeometry(i)->get_dphi_dp_size(tNumRows,tNumCols);
            tNumDVs += tNumCols;
        }

        return tNumDVs;
    }

    /*
     *
     */
    void
    store_dx_dp(Mat<Integer,Integer_Matrix> const & aDxDp,
                Mat<Integer,Integer_Matrix> const & aNodeADVIndices)
    {

        Integer tSizeDXDP = mDxDp.size();
        Integer tSizeADVs = mDxDp.size();

        XTK_ASSERT(tSizeADVs == tSizeDXDP,"Incorrect sizing in dxdp member vars");
        mDxDp.push_back(aDxDp);
        mNodeADVIndices.push_back(aNodeADVIndices);

    }





private:
    Integer mActiveGeometryIndex;
    Cell<Geometry<Real, Integer, Real_Matrix, Integer_Matrix>*> mGeometry;

    // Contains all the geometry objects
    Geometry_Object_Manager<Real,Integer,Real_Matrix,Integer_Matrix> mGeometryObjects;

    // Phase Table
    Phase_Table<Integer,Integer_Matrix> mPhaseTable;

    // Node Entity Phase Vals
    // Only analytic phase values are stored here to prevent duplicate storage of discrete geometries
    Mat<Real,Real_Matrix> mNodePhaseVals;

    // Sensitivity Data
    Cell<Mat<Real,Real_Matrix>> mDxDp;
    Cell<Mat<Integer,Integer_Matrix>> mNodeADVIndices;



private:
    Geometry<Real, Integer, Real_Matrix, Integer_Matrix> &
    ActiveGeometry() const
    {
        return (*mGeometry(mActiveGeometryIndex));
    }




    /**
     * compute_intersection_info, calculates the relevant intersection information placed in the geometry object
     * @param[in]  aEntityNodeInds - node to entity connectivity
     * @param[in]  aNodeVars      - node level set values
     * @param[in]  aCheckType     - if a entity local location is necessary 1, else 0.
     * @param[out] Returns an intersection flag and local coordinates if aCheckType 1 in cell 1 and node sensitivity information in cell 2 if intersection point located
     **/
    Cell<Mat<Real,Real_Matrix>>
    compute_intersection_info(Mat<Integer, Integer_Matrix> const & aEntityNodeInds,
                              Mat<Real,Real_Matrix> const &        aNodeCoords,
                              Integer const &                      aCheckType,
                              Mat<Integer, Integer_Matrix> &       aNodeADVIndices )
    {
        Cell<Mat<Real,Real_Matrix>> tIntersectionInfo(3);

        //Initialize
        Real tMax = 0;
        Real tMin = 0;
        Integer tNodeInd  = 0;
        Integer tNumNodes = aEntityNodeInds.get_num_columns();
        Mat<Real,Real_Matrix> tEntityNodeVars(tNumNodes, 1);
        Mat<Real,Real_Matrix> tInterpLocationCoords(1,1);


        // Loop through nodes and get levelset values from precomputed values in aNodeVars or in the levelset mesh
        for(Integer n = 0; n < tNumNodes; n++)
        {   //Get node id n
            tNodeInd = aEntityNodeInds(0, n);

            Geometry_Object<Real,Integer,Real_Matrix,Integer_Matrix> & tGeoObj = get_geometry_object(tNodeInd);
            Integer tPhaseValRowIndex = tGeoObj.get_phase_val_row();
            tEntityNodeVars(n, 0) = mNodePhaseVals(tPhaseValRowIndex, mActiveGeometryIndex);
        }

        //get the max and minimum levelset value for the entity
        tMax = tEntityNodeVars.get_max_value();
        tMin = tEntityNodeVars.get_min_value();

        //    If there is a sign change in element node variables return true, else return false

        //TODO: intersection flag should not be a real (needs to be a bool) split this function
        Mat<Real,Real_Matrix> tIntersection(1, 2, 0.0);// Initialize as false


        if(tMax == mThresholdValue)
        {
            tMax = mThresholdValue + mPerturbationValue;
        }

        if(tMin == mThresholdValue)
        {
            tMin = mThresholdValue - mPerturbationValue;
        }
        XTK_ASSERT(tMax != mThresholdValue && tMin != mThresholdValue, "Threshold levelset value at all nodes! There is no handling of this inside XTK currently.");

        if((tMax > mThresholdValue) && (tMin < mThresholdValue))
        {


            tIntersection(0, 0) = 1;

            if(aCheckType == 1)
            {


                Mat<Real,Real_Matrix> tIntersectLocalCoordinate(1,1);
                Mat<Real,Real_Matrix> tIntersectGlobalCoordinate(1,3);

                get_intersection_location(mThresholdValue,
                                          mPerturbationValue,
                                          aNodeCoords,
                                          tEntityNodeVars,
                                          aEntityNodeInds,
                                          tIntersectLocalCoordinate,
                                          tIntersectGlobalCoordinate,
                                          true,
                                          true);

                tIntersection(0, 1) = tIntersectLocalCoordinate(0, 0);

                // Global coordinate
                tIntersectionInfo(0) = tIntersectLocalCoordinate;
                tIntersectionInfo(1) = tIntersectGlobalCoordinate;

                if(mComputeDxDp)
                {
                    Mat<Real,Real_Matrix> tDxDp(1,1,100.0);
                    compute_dx_dp_for_an_intersection(aEntityNodeInds,aNodeCoords,tIntersectLocalCoordinate,tEntityNodeVars, tDxDp, aNodeADVIndices);

                    tIntersectionInfo(2) = tDxDp;
                }



            }
        }

        tIntersectionInfo(0) = tIntersection;

        return tIntersectionInfo;

    }

    void
     interpolate_level_set_value_to_child_node_location(Topology<Real, Integer, Real_Matrix, Integer_Matrix> const & aParentTopology,
                                                        Integer const &                                              aGeometryIndex,
                                                        Mat<Real,Real_Matrix> const &                                aNodeLocalCoordinate,
                                                        Mat<Real,Real_Matrix> &                                      aLevelSetValues)
     {

         // Get node indices attached to parent (These are indices relative to another mesh and may need to be mapped)
         Mat<Integer, Integer_Matrix> const & tNodesAttachedToParent = aParentTopology.get_node_indices();

         // Get number of nodes attached to parent
         Integer tNumNodesAttachedToParent = tNodesAttachedToParent.get_num_columns();
         Mat<Real,Real_Matrix> tNodesLevelSetValues(1, tNumNodesAttachedToParent);

         for(Integer i = 0; i < tNumNodesAttachedToParent; i++)
         {
             Geometry_Object<Real,Integer,Real_Matrix,Integer_Matrix> & tGeoObj = get_geometry_object(tNodesAttachedToParent(0,i));
             Integer tPhaseRow = tGeoObj.get_phase_val_row();

             tNodesLevelSetValues(0,i) = mNodePhaseVals(tPhaseRow,aGeometryIndex);
         }

         tNodesLevelSetValues = transpose(tNodesLevelSetValues);

         // Ask the topology how to interpolate
         Mat<Real,Real_Matrix> tBasisValues(1,1);
         Basis_Function<Real,Real_Matrix> const & tParentBasisFunctions = aParentTopology.get_basis_function();

         // Evaluate basis function
         tParentBasisFunctions.evaluate_basis_function(aNodeLocalCoordinate,tBasisValues);

         // Get references to underlying matrices
         Real_Matrix & tBasisValueMatrix          = tBasisValues.matrix_data();
         Real_Matrix & tLevelSetValueMatrix       = aLevelSetValues.matrix_data();

         // Compute \phi = Ni.\phi_i
         tLevelSetValueMatrix = tBasisValueMatrix*transpose(tNodesLevelSetValues);

     }


};
}

#endif /* SRC_GEOMENG_CL_MGE_GEOMETRY_ENGINE_HPP_ */
