/*
 * cl_GEN_Geometry_Engine.hpp
 *
 *  Created on: Jun 21, 2017
 *      Author: ktdoble
 */

#ifndef PROJECTS_GEN_GEN_MAIN_SRC_GEOMENG_CL_GEN_GEOMETRY_ENGINE_HPP_
#define PROJECTS_GEN_GEN_MAIN_SRC_GEOMENG_CL_GEN_GEOMETRY_ENGINE_HPP_

// Standard library includes
#include <memory> // for shared_ptr
#include <math.h>

#include "cl_Cell.hpp"
#include "cl_Logger.hpp"

// Linear algebra includes
#include "cl_Matrix.hpp"
#include "fn_trans.hpp"
#include "op_equal_equal.hpp"
#include "op_times.hpp"
#include "linalg_typedefs.hpp"

// GE
#include "cl_GEN_Analytic_Geometry.hpp"
#include "cl_GEN_Basis_Function.hpp"
#include "cl_GEN_Field.hpp"
#include "cl_GEN_Geom_Field.hpp"
#include "cl_GEN_Interpolaton.hpp"
#include "cl_GEN_Pending_Node.hpp"
#include "cl_GEN_Phase_Table.hpp"
#include "fn_GEN_approximate.hpp"

#include "cl_GEN_Geometry_Object.hpp"
#include "cl_GEN_Geometry_Object_Manager.hpp"
#include "cl_GEN_Pdv_Host.hpp"
#include "cl_GEN_Pdv_Host_Manager.hpp"

#include "cl_GEN_Cylinder_With_End_Caps.hpp"
#include "cl_GEN_Geometry.hpp"

#include "cl_GEN_Property.hpp"

#include "cl_GEN_Dv_Enums.hpp"
// MTK
#include "cl_MTK_Cluster.hpp"
#include "cl_MTK_Mesh_Manager.hpp"
#include "cl_Mesh_Enums.hpp"

// Parsing tools for parameter list
#include "fn_Exec_load_user_library.hpp"
#include "fn_Parsing_Tools.hpp"


namespace moris
{
namespace hmr
{
    class HMR;
    class Mesh;
}
namespace ge
{
/*
 * $\frac{\partial{\phi_A}}{\partial{p}}$ (change in phi with respect to a design variable
 * See for more detailed description of this function:
 */
inline
void compute_dx_dp_with_linear_basis( moris::Matrix< moris::DDRMat >  & aDPhiADp,
                                      moris::Matrix< moris::DDRMat >  & aDPhiBDp,
                                      moris::Matrix< moris::DDRMat >  & aEdgeCoordinates,
                                      moris::Matrix< moris::DDRMat >  & aEdgeNodePhi,
                                      moris::Matrix< moris::DDRMat >  & aDxDp )
{

  MORIS_ASSERT(aDPhiADp.n_rows() != 0,"dPhi/dp not implemented in geometry would cause a seg fault here");
  MORIS_ASSERT(aDPhiBDp.n_rows() != 0,"dPhi/dp not implemented in geometry would cause a seg fault here");
  moris::real const & tPhiA = aEdgeNodePhi(0,0);
  moris::real const & tPhiB = aEdgeNodePhi(1,0);

  // Initialize
  moris::Matrix< moris::DDRMat > tXa = aEdgeCoordinates.get_row(0);

  moris::Matrix< moris::DDRMat > tXb = aEdgeCoordinates.get_row(1);

  // ------- Compute $\frac{\partial x_{\Gamma}}{\partial \phi}$ -------
  moris::DDRMat tDxgammaDphiA = -(tPhiB)/std::pow((tPhiA-tPhiB),2)*(tXb.matrix_data()-tXa.matrix_data());
  moris::DDRMat tDxgammaDphiB =  (tPhiA)/std::pow((tPhiA-tPhiB),2)*(tXb.matrix_data()-tXa.matrix_data());

  moris::Matrix< moris::DDRMat > tDxgDphiAMat(tDxgammaDphiA);
  moris::Matrix< moris::DDRMat > tDxgDphiBMat(tDxgammaDphiB);
  // ------------------------------ end --------------------------------

  // Compute dx/dp
  moris::DDRMat tDxDp = aDPhiADp * moris::trans(tDxgDphiAMat) +  aDPhiBDp * moris::trans(tDxgDphiBMat);
  aDxDp = moris::Matrix< moris::DDRMat >(tDxDp);

}
//------------------------------------------------------------------------------

class GEN_Geometry_Engine
{
public:     // ----------- member data ----------
    // Options which the user can change (all are given defaults)
    moris::real mThresholdValue;
    moris::real mPerturbationValue;
    bool        mComputeDxDp;           // <- should be turned off if a sensitivity has not been implemented
    moris::uint mSpatialDim;

private:    // ----------- member data ----------
    moris::size_t mActiveGeometryIndex;
    Cell< GEN_Geometry* > mGeometry;

    // Contains all the geometry objects
    Geometry_Object_Manager mGeometryObjectManager;

    // Contains all the pdv hosts
    Pdv_Host_Manager mPdvHostManager;

    // Phase Table
    moris::ge::GEN_Phase_Table mPhaseTable;

    // Node Entity Phase Vals - only analytic phase values are stored here to prevent duplicate storage of discrete geometries
    moris::Matrix< moris::DDRMat > mNodePhaseVals;

    mtk::Mesh_Manager* mMesh;

    moris::Cell< std::shared_ptr< moris::hmr::HMR > > mHMRPerformer;

    moris::Cell< std::shared_ptr< moris::hmr::Mesh > > mMesh_HMR; //FIXME needs to be more general to only have a mesh manager as this member

    bool mTypesSet      = false;

    moris::Cell< moris::moris_index > mIntegNodeIndices;

    ParameterList mParameterList;

    std::shared_ptr< Library_IO > mLibrary = nullptr;

public:

//------------------------------------------------------------------------------
    /**
     * single geometry constructor
     * @param[ in ] aGeometry
     * @param[ in ] aPhaseTable
     * @param[ in ] aSpatialDim
     */
    GEN_Geometry_Engine( moris::ge::GEN_Geometry          & aGeometry,
                         moris::ge::GEN_Phase_Table const & aPhaseTable,
                         moris::uint                        aSpatialDim = 3 );

//------------------------------------------------------------------------------
    /**
     * multiple geometries constructor
     * @param[ in ] aGeometry
     * @param[ in ] aPhaseTable
     * @param[ in ] aSpatialDim
     */
    GEN_Geometry_Engine( Cell< GEN_Geometry* >      const & aGeometry,
                         moris::ge::GEN_Phase_Table const & aPhaseTable,
                         moris::uint                        aSpatialDim = 3 );

//------------------------------------------------------------------------------
    /**
     * trivial constructor
     */
    GEN_Geometry_Engine(){}

//------------------------------------------------------------------------------
    /**
     * input file constructor
     * @param[ in ] aParameterList a parameter list
     */
    GEN_Geometry_Engine( ParameterList aParameterList )
    : mParameterList(aParameterList)
    {}

//------------------------------------------------------------------------------

    ~GEN_Geometry_Engine()
    {
//        for( auto tGeometry : mGeometry )
//        {
//            delete tGeometry;
//        }
    }

//------------------------------------------------------------------------------
    /**
     * this function initializes the geometry engine with the provided analytic functions from the input file
     *
     * @param[ in ] aLibrary a pointer to library for reading inputs
     */
    void initialize( std::shared_ptr< Library_IO > aLibrary );

    void initialize();

    void initialize_geometries_and_phase_table();

//------------------------------------------------------------------------------
    /**
     * @brief Initial allocation of geometry objects,
     * this creates a geometry object for each node coordinate.
     * In this case, aNodeCoords needs to be ordered by proc indices.
     * @param[ in ] aNumNodes number of nodes
     */
    void initialize_geometry_objects_for_background_mesh_nodes( moris::size_t const & aNumNodes );

//------------------------------------------------------------------------------
    /**
     * ???
     * @param[ in ] aNodeCoords node coordinates
     */
    void initialize_geometry_object_phase_values( moris::Matrix< moris::DDRMat > const & aNodeCoords );

//------------------------------------------------------------------------------
    /**
     * @brief Creates a geometry object association for pending nodes
     * These nodes have node indices and parent information
     */
    void associate_new_nodes_with_geometry_object( moris::Cell< Pending_Node > & aNewNodes,
                                                   bool                          aInterfaceNodes );

//------------------------------------------------------------------------------
    /**
     * create new node geometry objects
     * @param[ in ] aNodeCoords node coordinates
     */
    void create_new_node_geometry_objects(Cell< moris_index >  const & aNewNodeIndices,
                                          bool                         aStoreParentTopo,
                                          Cell<xtk::Topology*> const & aParentTopo,
                                          Cell<Matrix<DDRMat>> const & aParamCoordRelativeToParent,
                                          Cell<Matrix<DDRMat>> const & aGlobalNodeCoord);

//------------------------------------------------------------------------------
    /**
     * @brief Links new nodes with an existing geometry object. This is used for unzipped interfaces
     * where more than one node is at the same location
     * @param[in] aNodesIndicesWithGeomObj - Node indices which already have a geometry object
     * @param[in] aNodesIndicesToLink - Node indices to link to the corresponding nodes in aNodesIndicesWithGeomObj
     */
    void link_new_nodes_to_existing_geometry_objects( Matrix< IndexMat > const & aNodesIndicesWithGeomObj,
                                                      Matrix< IndexMat > const & aNodesIndicesToLink );

//------------------------------------------------------------------------------
    /**
     * @brief is_intersected checks to see if an entity provided to it intersects a geometry field. Intersects in this context
     * means a geometry crosses a certain threshold (typically 0). For levelset fields, this can be thought of as a phase change
     *
     * @param[in] aNodeCoords       - Node coordinate
     * @param[in] aNodeToEntityConn - Connectivity between nodes and parent entity
     * @param[in] aCheckType        - Specifies what type of intersection check is to be performed
     *                                   0 - No information on interface required
     *                                   1 - information on interface required
     */
    void is_intersected( moris::Matrix< moris::DDRMat > const &   aNodeCoords,
                         moris::Matrix< moris::IndexMat > const & aNodetoEntityConn,
                         moris::size_t                            aCheckType,
                         Cell<GEN_Geometry_Object> &              aGeometryObjects );

//------------------------------------------------------------------------------
    /*!
     * @brief Computes the interface sensitivity of the provided node indices. After this call,
     * the sensitivity information of these interface nodes can be accessed through the interface
     * nodes respective geometry object.
     * @param[in] aInterfaceNodeIndices - Interface Node Indices (should be interface nodes wrt geometry index provided)
     * @param[in] aNodeCoords -  Node coordinates with location corresponding to indices of aIntefaceNodeIndices.
     * @param[in] aGeomIndex - Geometry Index
     * @param[in] aGlbCoord  - bool to calculate the global coordinate of the intersection point
     */
    void compute_interface_sensitivity( Matrix< IndexMat > const & aInterfaceNodeIndices,
                                        Matrix< DDRMat >   const & aNodeCoords,
                                        moris_index                aGeomIndex,
                                        bool               const   aGlbCoord = false );

//------------------------------------------------------------------------------
    /*
     * @brief Computes the intersection of an isocountour with an entity and returning the local coordinate relative to the parent
     * and the global coordinate if needed
     */
    void get_intersection_location( moris::real const &                      aIsocontourThreshold,
                                    moris::real const &                      aPerturbationThreshold,
                                    moris::Matrix< moris::DDRMat > const &   aGlobalNodeCoordinates,
                                    moris::Matrix< moris::DDRMat > const &   aEntityNodeVars,
                                    moris::Matrix< moris::IndexMat > const & aEntityNodeIndices,
                                    moris::Matrix< moris::DDRMat > &         aIntersectionLocalCoordinates,
                                    moris::Matrix< moris::DDRMat > &         aIntersectionGlobalCoordinates,
                                    bool                                     aCheckLocalCoordinate = true,
                                    bool                                     aComputeGlobalCoordinate = false );

//------------------------------------------------------------------------------
    void compute_dx_dp_finite_difference( moris::real                      const & aPerturbationVal,
                                          moris::Matrix< moris::DDRMat >   const & aGlobalNodeCoordinates,
                                          moris::Matrix< moris::DDRMat >   const & aEntityNodeCoordinates,
                                          moris::Matrix< moris::DDRMat >   const & aIntersectionLclCoordinate,
                                          moris::Matrix< moris::IndexMat > const & aEntityNodeIndices,
                                          moris::Matrix< moris::DDRMat >         & aEntityNodeVars,
                                          moris::Matrix< moris::DDRMat >         & aDxDp );

//------------------------------------------------------------------------------
    void compute_dx_dp_for_an_intersection( moris::Matrix< moris::IndexMat > const & aEntityNodeIndices,
                                            moris::Matrix< moris::DDRMat >   const & aGlobalNodeCoordinates,
                                            moris::Matrix< moris::DDRMat >   const & aIntersectionLclCoordinate,
                                            moris::Matrix< moris::DDRMat >         & aEntityNodeVars,
                                            moris::Matrix< moris::DDRMat >         & aDxDp,
                                            moris::Matrix< moris::IndexMat >       & aADVIndices );

//------------------------------------------------------------------------------
    /**
     * @brief Returns a reference to the geometry object at the provided index
     * @param[ in ] aNodeIndex a node index
     */
    GEN_Geometry_Object &
    get_geometry_object( moris::size_t const & aNodeIndex );

//------------------------------------------------------------------------------
    /**
     * @brief Returns a reference to the geometry object at the provided index
     */
    GEN_Geometry_Object const &
    get_geometry_object(moris::size_t const & aNodeIndex) const;

//------------------------------------------------------------------------------
    /*
     * @brief Get the total number of phases in the phase table
     */
    moris::size_t get_num_phases();

//------------------------------------------------------------------------------
    /*
     * @brief Get the 0 or 1 value associated with a given phase and geometry index
     */
    moris::moris_index
    get_phase_sign_of_given_phase_and_geometry( moris::moris_index aPhaseIndex,
                                                moris::moris_index aGeometryIndex );

//------------------------------------------------------------------------------
    /*
     * @brief Get phase value for a given node and geometry index
     */
    moris::real
    get_entity_phase_val( moris::size_t const & aNodeIndex,
                          moris::size_t const & aGeomIndex );

//------------------------------------------------------------------------------
    /*
     * @brief Get dxdp for a node
     */
    moris::Matrix< moris::DDRMat > const &
    get_node_dx_dp(moris::size_t const & aNodeIndex) const;

//------------------------------------------------------------------------------
    /*
     * @brief get adv indices for a node
     */
    moris::Matrix< moris::IndexMat > const &
    get_node_adv_indices( moris::size_t const & aNodeIndex ) const;

//------------------------------------------------------------------------------
    /*
     * @brief For a given node index, return the phase index relative to each geometry (i.e. inside/outside indicator)
     */
    void get_phase_index( moris::Matrix< moris::DDSTMat > const & aNodeIndex,
                          moris::Matrix< moris::DDSTMat > & aNodePhaseIndex );

//------------------------------------------------------------------------------
    /*
      * @brief For a given node index, return the phase index relative to each geometry (i.e. inside/outside indicator)
      */
     void get_phase_index( moris::moris_index const & aNodeIndex,
                           moris::size_t & aNodePhaseIndex );

//------------------------------------------------------------------------------
    /*
     * @brief Provided the inside and out phase values for an entity, return the phase index
     */
    moris::moris_index
    get_elem_phase_index(moris::Matrix< moris::IndexMat > const & aElemOnOff)
    {
        return mPhaseTable.get_phase_index(aElemOnOff);
    }

//------------------------------------------------------------------------------
    /*
     * @brief Returns whether a node is inside or outside wrt to a given geometry index
     */
    moris::size_t
    get_node_phase_index_wrt_a_geometry(moris::size_t aNodeIndex,
                                        moris::size_t aGeometryIndex);

//------------------------------------------------------------------------------
    /*
     * @brief Returns whether the active geometry is analytic
     */
    bool is_geometry_analytic();

//------------------------------------------------------------------------------
    /*
     * @brief Returns the number of geometries
     */
    moris::size_t get_num_geometries();

//------------------------------------------------------------------------------
    /*
     * @brief Returns the number of phases
     */
    moris::size_t get_num_bulk_phase();

//------------------------------------------------------------------------------
    /*
     * @brief Returns the active geometry index
     */
    moris::size_t get_active_geometry_index();

//------------------------------------------------------------------------------
    /*
     * @brief Advance the active geometry index
     */
    void advance_geometry_index();

//------------------------------------------------------------------------------
    /*
     * this function need to be deleted as they are not used in the current PDV interface implementation !!!
     */
    moris::Matrix< moris::IndexMat > get_node_adv_indices_analytic();

//------------------------------------------------------------------------------
    /*
     * this function need to be deleted as they are not used in the current PDV interface implementation !!!
     */
    moris::uint get_num_design_variables() const;

//------------------------------------------------------------------------------
    /*
     * @brief Returns the ADV indices of the provided nodes
     * this function need to be deleted as they are not used in the current PDV interface implementation !!!
     */
    moris::Matrix< moris::IndexMat > get_node_adv_indices_discrete
    ( moris::Matrix< moris::IndexMat > const & aEntityNodes );

//------------------------------------------------------------------------------
    /**
     * this function need to be deleted as they are not used in the current PDV interface implementation !!!
     */
    moris::size_t get_num_design_vars_analytic();

//------------------------------------------------------------------------------
    /*
     * returns a pointer to the PDV host manager ( FEM <-> GEN interface needs this manager )
     */
    Pdv_Host_Manager* get_pdv_host_manager();

//------------------------------------------------------------------------------
    /*
     * returns a pointer to the geometry object manager
     */
    Geometry_Object_Manager* get_all_geom_obj();

//------------------------------------------------------------------------------
    /*
     * @brief register a mesh to be used for later computation(s)
     */
    void register_mesh( mtk::Mesh_Manager* aMesh );

    moris_index register_mesh( std::shared_ptr< moris::hmr::Mesh > aMesh ); //FIXME: this needs to be deleted and the GE should only be able to register an mtk mesh pair

    void set_performer( std::shared_ptr< hmr::HMR > aMesh );

    void set_library( std::shared_ptr< Library_IO > aLibrary );

    void perform( );

    void perform_refinement( );
//------------------------------------------------------------------------------
    /*
     * @brief function specific to fiber problem
     */
    Matrix< DDRMat > get_cylinder_vals( moris_index aWhichMesh,
                                        GEN_CylinderWithEndCaps* aFiber,
                                        uint aNumberOfFibers );             //FIXME this is currently only setup to work with an HMR member mesh

//------------------------------------------------------------------------------
    /*
     * @brief gives the maximum level-set values at all nodes in the mesh
     */
    void get_max_field_values_for_all_geometries( Matrix< DDRMat > & aAllFieldVals,
                                                  moris_index        aWhichMesh = 0 )
    {
////        MORIS_ERROR( false, "GEN_Geometry_Engine::get_field_values_for_all_geometries() - this function is not implemented yet" );
//        // TODO: implement this function and make it work for the case of a mesh manager...
//        uint tNumVertices = mMesh_HMR( aWhichMesh )->get_num_nodes();
//
//        aAllFieldVals.set_size( tNumVertices, 1, - MORIS_REAL_MAX );
//
//        for( uint iVert = 0; iVert <tNumVertices; iVert++)
//        {
//            Matrix< DDRMat > tCoord = mMesh_HMR( aWhichMesh )->get_mtk_vertex( iVert ).get_coords();
//
//            moris::real tVal = - MORIS_REAL_MAX;
//            for ( auto tGeometry : mGeometry )
//            {
//                Cell< moris::real > tTempConstCell = {{0}};
//                moris::real tTempVal = 0.0;
//
//                tGeometry->eval( tTempVal, tCoord, tTempConstCell );
//
//                tVal = std::max( tVal, tTempVal );
//            }
//
//            // FIXME will not work in parallel. Ind are not consistent because of aura
//            aAllFieldVals( 0 )( iVert ) = tVal;
//        }
    }

//------------------------------------------------------------------------------
    /*
     * @brief fills a cell of MORIS matrices with the level-set values corresponding to each geometry
     */
    void get_field_values_for_all_geometries( moris::Cell< Matrix< DDRMat > > & aAllFieldVals,
                                              const moris_index                 aWhichMesh = 0 );



    //------------------------------------------------------------------------------
    // TODO: move these functions over to the .cpp file
    //------------------------------------------------------------------------------

//------------------------------------------------------------------------------
    /*
     * add geometry dv type to dv type list
     * @param[ in ] aPdvType          list of dv types (material only)
     * @param[ in ] aUsingGeometryDvs bool true if geometry dv types used
     */
    void set_pdv_types( Cell< enum GEN_DV > aPdvType )
    {
        // set the set dv type flag to true
        mTypesSet = true;

        // set the dv type list for the pdv host manager
        mPdvHostManager.set_ip_pdv_types( aPdvType );
    }

//------------------------------------------------------------------------------
    /*
     * initialize interpolation pdv host list
     * @param[ in ] aWhichMesh a mesh index
     */
    void initialize_interp_pdv_host_list( moris_index aWhichMesh = 0 )
    {
        // check if the dv type list was set
        MORIS_ASSERT( mTypesSet, "GEN_Geometry_Engine::initialize_interp_pdv_host_list() - set_pdv_types() must be called before this function." );

        // get number of vertices on the IP mesh
        uint tTotalNumVertices = mMesh->get_interpolation_mesh(aWhichMesh)->get_num_nodes();

        // ask pdv host manager to init host
        mPdvHostManager.initialize_ip_hosts( tTotalNumVertices );
    }

//------------------------------------------------------------------------------
    /*
     * @brief assign the pdv type and property for each pdv host in a given set via a GEN_Field class
     */
    void assign_ip_hosts_by_set_name( std::string                  aSetName,
                                      std::shared_ptr< GEN_Field > aFieldPointer,
                                      enum GEN_DV                  aPdvType,
                                      moris_index                  aWhichMesh = 0 )
    {
        // get the mesh set from name
        moris::mtk::Set* tSetPointer = mMesh->get_integration_mesh( aWhichMesh )->get_set_by_name( aSetName );

        // get the list of cluster on mesh set
        moris::Cell< mtk::Cluster const * > tClusterPointers = tSetPointer->get_clusters_on_set();

        // get number of clusters on mesh set
        uint tNumClusters = tClusterPointers.size();

        // loop over the clusters on mesh set
        for(uint iClust=0; iClust<tNumClusters; iClust++)
        {
            // get the IP cell from cluster
            moris::mtk::Cell const & tIPCell = tClusterPointers(iClust)->get_interpolation_cell();

            // get the vertices from IP cell
            moris::Cell< moris::mtk::Vertex * > tVertices = tIPCell.get_vertex_pointers();

            // get the number of vertices on IP cell
            uint tNumVerts = tVertices.size();

            // loop over vertices on IP cell
            for(uint iVert=0; iVert<tNumVerts; iVert++)
            {
                // get the vertex index
                moris_index tVertIndex = tVertices(iVert)->get_index();

                // ask pdv host manager to assign to vertex a pdv type and a property
                mPdvHostManager.assign_field_to_pdv_type_by_vertex_index( aFieldPointer, aPdvType, tVertIndex );
            }
        }

        // ask pdv host manager to update local to global dv type map
        mPdvHostManager.update_ip_local_to_global_dv_type_map();

        // mark this DV type as unchanging
        mPdvHostManager.mark_ip_pdv_as_unchanging( aPdvType );
    }
//------------------------------------------------------------------------------
    /*
     * @brief assign the pdv type and property for each pdv host in a given set
     */
    void assign_ip_hosts_by_set_name( std::string                     aSetName,
                                      std::shared_ptr< GEN_Property > aPropertyPointer,
                                      enum GEN_DV                     aPdvType,
                                      moris_index                     aWhichMesh = 0 )
    {
        // get the mesh set from name
        moris::mtk::Set* tSetPointer = mMesh->get_integration_mesh( aWhichMesh )->get_set_by_name( aSetName );

        // get the list of cluster on mesh set
        moris::Cell< mtk::Cluster const * > tClusterPointers = tSetPointer->get_clusters_on_set();

        // get number of clusters on mesh set
        uint tNumClusters = tClusterPointers.size();

        // loop over the clusters on mesh set
        for(uint iClust=0; iClust<tNumClusters; iClust++)
        {
            // get the IP cell from cluster
            moris::mtk::Cell const & tIPCell = tClusterPointers(iClust)->get_interpolation_cell();

            // get the vertices from IP cell
            moris::Cell< moris::mtk::Vertex * > tVertices = tIPCell.get_vertex_pointers();

            // get the number of vertices on IP cell
            uint tNumVerts = tVertices.size();

            // loop over vertices on IP cell
            for(uint iVert=0; iVert<tNumVerts; iVert++)
            {
                // get the vertex index
                moris_index tVertIndex = tVertices(iVert)->get_index();

                // ask pdv host manager to assign to vertex a pdv type and a property
                mPdvHostManager.assign_property_to_pdv_type_by_vertex_index( aPropertyPointer, aPdvType, tVertIndex );
            }
        }

        // ask pdv host manager to update local to global dv type map
        mPdvHostManager.update_ip_local_to_global_dv_type_map();
    }

//------------------------------------------------------------------------------
    /*
     * @brief assign the pdv type and property for each pdv host in a given set via a GEN Field
     */
    void assign_ip_hosts_by_set_index( moris_index                  aSetIndex,
                                       std::shared_ptr< GEN_Field > aFieldPointer,
                                       enum GEN_DV                  aPdvType,
                                       moris_index                  aWhichMesh = 0 )
    {
        // get the mesh set from index
        moris::mtk::Set* tSetPointer = mMesh->get_integration_mesh( aWhichMesh )->get_set_by_index( aSetIndex );

        // get the list of cluster on mesh set
        moris::Cell< mtk::Cluster const * > tClusterPointers = tSetPointer->get_clusters_on_set();

        // get number of clusters on mesh set
        uint tNumClusters = tClusterPointers.size();

        // loop over the clusters on mesh set
        for(uint iClust=0; iClust<tNumClusters; iClust++)
        {
            // get the IP cell from cluster
            moris::mtk::Cell const & tIPCell = tClusterPointers(iClust)->get_interpolation_cell();

            // get the vertices from IP cell
            moris::Cell< moris::mtk::Vertex * > tVertices = tIPCell.get_vertex_pointers();

            // get the number of vertices on IP cell
            uint tNumVerts = tVertices.size();

            // loop over vertices on IP cell
            for(uint iVert=0; iVert<tNumVerts; iVert++)
            {
                // get the vertex index
                moris_index tVertIndex = tVertices(iVert)->get_index();

                // ask pdv host manager to assign to vertex a pdv type and a property
                mPdvHostManager.assign_field_to_pdv_type_by_vertex_index( aFieldPointer, aPdvType, tVertIndex );
            }
        }

        // ask pdv host manager to update local to global dv type map
        mPdvHostManager.update_ip_local_to_global_dv_type_map();

        // mark this DV type as unchanging
        mPdvHostManager.mark_ip_pdv_as_unchanging( aPdvType );
    }
//------------------------------------------------------------------------------
    /*
     * @brief assign the pdv type and property for each pdv host in a given set
     */
    void assign_ip_hosts_by_set_index( moris_index                     aSetIndex,
                                       std::shared_ptr< GEN_Property > aPropertyPointer,
                                       enum GEN_DV                     aPdvType,
                                       moris_index                     aWhichMesh = 0 )
    {
        // get the mesh set from index
        moris::mtk::Set* tSetPointer = mMesh->get_integration_mesh( aWhichMesh )->get_set_by_index( aSetIndex );

        // get the list of cluster on mesh set
        moris::Cell< mtk::Cluster const * > tClusterPointers = tSetPointer->get_clusters_on_set();

        // get number of clusters on mesh set
        uint tNumClusters = tClusterPointers.size();

        // loop over the clusters on mesh set
        for(uint iClust=0; iClust<tNumClusters; iClust++)
        {
            // get the IP cell from cluster
            moris::mtk::Cell const & tIPCell = tClusterPointers(iClust)->get_interpolation_cell();

            // get the vertices from IP cell
            moris::Cell< moris::mtk::Vertex * > tVertices = tIPCell.get_vertex_pointers();

            // get the number of vertices on IP cell
            uint tNumVerts = tVertices.size();

            // loop over vertices on IP cell
            for(uint iVert=0; iVert<tNumVerts; iVert++)
            {
                // get the vertex index
                moris_index tVertIndex = tVertices(iVert)->get_index();

                // ask pdv host manager to assign to vertex a pdv type and a property
                mPdvHostManager.assign_property_to_pdv_type_by_vertex_index( aPropertyPointer, aPdvType, tVertIndex );
            }
        }

        // ask pdv host manager to update local to global dv type map
        mPdvHostManager.update_ip_local_to_global_dv_type_map();
    }
//------------------------------------------------------------------------------
    /*
     * @brief create pdv objects for the spatial dimensions
     *        - if the hosts have already been initialized via the interpolation mesh, add new PDV objects to them
     *        - if not, initialize the hosts and add PDV objects to them
     */
    void initialize_integ_pdv_host_list( moris_index aWhichMesh = 0 )
    {
        moris::Cell< enum GEN_DV > tDimDvList(mSpatialDim);

        switch(mSpatialDim)
        {
            case(2):
                {
                    tDimDvList(0) = GEN_DV::XCOORD;
                    tDimDvList(1) = GEN_DV::YCOORD;
                    break;
                }
            case(3):
                {
                    tDimDvList(0) = GEN_DV::XCOORD;
                    tDimDvList(1) = GEN_DV::YCOORD;
                    tDimDvList(2) = GEN_DV::ZCOORD;
                    break;
                }
            default:
                {
                    MORIS_ERROR( false,"GEN_Geometry_Engine::initialize_integ_pdv_host_list() - Geometry Engine only works for 2D and 3D models." );
                }
        }

        mPdvHostManager.set_ig_pdv_types( tDimDvList );

        // get number of vertices on the IG mesh
        uint tTotalNumVertices = mMesh->get_integration_mesh(aWhichMesh)->get_num_nodes();

        // ask pdv host manager to initialize hosts
        mPdvHostManager.initialize_ig_hosts( tTotalNumVertices );

        // check to make sure there are IG nodes to put hosts on
        MORIS_ASSERT( mIntegNodeIndices.size() != 0, "GEN_Geometry_Engine::initialize_integ_pdv_host_list() - no integration node indices stored, has the XTK model performed decomposition already?" );

        uint tNumIndices = mIntegNodeIndices.size();

        mPdvHostManager.update_ig_local_to_global_dv_type_map();

        for( uint iInd=0; iInd<tNumIndices; iInd++ )
        {
            mPdvHostManager.create_ig_pdv_host( mSpatialDim, mIntegNodeIndices(iInd) );

            mPdvHostManager.get_ig_pdv_host( mIntegNodeIndices(iInd) )->update_pdv_list( mSpatialDim );

            Matrix< DDRMat > tTempCoords = mMesh->get_integration_mesh( aWhichMesh )->get_node_coordinate( mIntegNodeIndices(iInd) );

            for(uint iDim=0; iDim<mSpatialDim; iDim++)
            {
                mPdvHostManager.get_ig_pdv_host( mIntegNodeIndices(iInd) )->create_pdv( tTempCoords(iDim), tDimDvList(iDim), mPdvHostManager.get_ig_global_map() );
            }

        }
    }
//------------------------------------------------------------------------------
    void mark_ig_dv_type_as_unchanging( enum GEN_DV aPdvType )
    {
        mPdvHostManager.mark_ig_pdv_as_unchanging( aPdvType );
    }
//------------------------------------------------------------------------------















private:
    GEN_Geometry & ActiveGeometry() const;
    //------------------------------------------------------------------------------
    /**
     * @brief compute_intersection_info, calculates the relevant intersection information placed in the geometry object
     * @param[in]  aEntityNodeInds - node to entity connectivity
     * @param[in]  aNodeVars       - node level set values
     * @param[in]  aCheckType      - if a entity local location is necessary 1, else 0.
     * @param[out] Returns an intersection flag and local coordinates if aCheckType 1 in cell 1 and node sensitivity information in cell 2 if intersection point located
     **/
    bool compute_intersection_info( moris::moris_index               const & aEntityIndex,
                                    moris::Matrix< moris::IndexMat > const & aEntityNodeInds,
                                    moris::Matrix< moris::DDRMat >   const & aNodeCoords,
                                    moris::size_t                    const & aCheckType,
                                    moris::Matrix< moris::IndexMat >       & aNodeADVIndices,
                                    GEN_Geometry_Object                    & aGeometryObject );
    //------------------------------------------------------------------------------
    void interpolate_level_set_value_to_child_node_location( xtk::Topology                  const & aParentTopology,
                                                             moris::size_t                  const & aGeometryIndex,
                                                             moris::Matrix< moris::DDRMat > const & aNodeLocalCoordinate,
                                                             moris::Matrix< moris::DDRMat >       & aLevelSetValues );
    //------------------------------------------------------------------------------
};
}
}

#endif /* PROJECTS_GEN_GEN_MAIN_SRC_GEOMENG_CL_GEN_GEOMETRY_ENGINE_HPP_ */