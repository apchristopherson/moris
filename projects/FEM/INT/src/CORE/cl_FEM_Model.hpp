/*
 * cl_FEM_Model.hpp
 *
 *  Created on: Aug 22, 2018
 *      Author: schmidt
 */

#ifndef PROJECTS_FEM_MDL_SRC_CL_FEM_MODEL_HPP_
#define PROJECTS_FEM_MDL_SRC_CL_FEM_MODEL_HPP_

#include "typedefs.hpp"                       //MRS/COR/src
#include "cl_Cell.hpp"                        //MRS/CON/src

#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
#include "cl_MTK_Enums.hpp"
#include "fn_Parsing_Tools.hpp"
#include "cl_Communication_Tools.hpp"

#include "fn_PRM_FEM_Parameters.hpp" //FEM/INT/src
#include "cl_MSI_Dof_Type_Enums.hpp"
#include "cl_GEN_Pdv_Enums.hpp"

#include "cl_MSI_Equation_Model.hpp"
#include "cl_FEM_Phase_User_Info.hpp"
#include "cl_FEM_Set_User_Info.hpp"
#include "cl_Library_IO.hpp"

namespace moris
{

    //------------------------------------------------------------------------------
    namespace mtk
    {
        class Mesh_Manager;
        class Interpolation_Mesh;
        class Integration_Mesh;
        class Field;
    }

    namespace fem
    {
        class IWG;
        class Node_Base;
        class Set;
        class Field_Interpolator;
        class Property;
        class Constitutive_Model;
        class Stabilization_Parameter;
        class IWG;
        class IQI;
    }

    namespace MSI
    {
        class Model_Solver_Interface;
        class MSI_Solver_Interface;
        class Equation_Set;
        class Equation_Object;
        class Design_Variable_Interface;
        enum class Dof_Type;
    }

    namespace fem
    {
        //------------------------------------------------------------------------------

        class FEM_Model : public  MSI::Equation_Model
        {
                // pointer to reference mesh
                mtk::Mesh_Manager* mMeshManager = nullptr;
                moris_index        mMeshPairIndex;

                // list of IP node pointers
                moris::Cell< fem::Node_Base* > mIPNodes;

                // list of IG node pointers
                moris::Cell< fem::Node_Base* > mIGNodes;

                // list of QI values
                moris::Cell< moris::real > mQi;

                // parameter list to build the fem model
                moris::Cell< moris::Cell< ParameterList > > mParameterList;

                // unpacked fem inputs
                moris::Cell< fem::Set_User_Info > mSetInfo;

                // unpacked phase inputs
                moris::Cell< fem::Phase_User_Info > mPhaseInfo;
                std::map< std::string, uint >       mPhaseMap;

                // space dimension
                uint mSpaceDim;

                // fixme remove ?
                moris::Cell< std::shared_ptr< fem::Property > >                mProperties;
                moris::Cell< std::shared_ptr< mtk::Field > >                   mFields;
                moris::Cell< std::shared_ptr< fem::Constitutive_Model > >      mCMs;
                moris::Cell< std::shared_ptr< fem::Stabilization_Parameter > > mSPs;
                moris::Cell< std::shared_ptr< fem::IWG > >                     mIWGs;
                moris::Cell< std::shared_ptr< fem::IQI > >                     mIQIs;

                //! requested IQI Names
                moris::Cell< std::string > mRequestedIQINames;

                //------------------------------------------------------------------------------
            public:

                //! Gauss point information. Only used for output
                uint mBulkGaussPoints               = 0;
                uint mSideSetsGaussPoints           = 0;
                uint mDoubleSidedSitsetsGaussPoints = 0;

                //------------------------------------------------------------------------------
                /**
                 * constructor
                 * @param[ in ] aMesh          mesh for this problem
                 * @param[ in ] aMeshPairIndex mesh pair index
                 * @param[ in ] aSetInfo       cell of set user info
                 */
                FEM_Model(
                        mtk::Mesh_Manager                 * aMeshManager,
                        const moris_index                 & aMeshPairIndex,
                        moris::Cell< fem::Set_User_Info > & aSetInfo );

                //------------------------------------------------------------------------------
                /**
                 * constructor
                 * @param[ in ] aMesh          mesh for this problem
                 * @param[ in ] aMeshPairIndex mesh pair index
                 * @param[ in ] aSetInfo       cell of set user info
                 * @param[ in ] aDesignVariableInterface a design variable interface pointer
                 */
                FEM_Model(
                        mtk::Mesh_Manager                 * aMeshManager,
                        const moris_index                 & aMeshPairIndex,
                        moris::Cell< fem::Set_User_Info > & aSetInfo,
                        MSI::Design_Variable_Interface    * aDesignVariableInterface );

                //------------------------------------------------------------------------------
                /**
                 * constructor with fem input
                 * @param[ in ] aMesh          mesh for this problem
                 * @param[ in ] aMeshPairIndex mesh pair index
                 * @param[ in ] aParameterList a list of list of parameter lists
                 * @param[ in ] aLibrary       a file path for property functions
                 */
                FEM_Model(
                        mtk::Mesh_Manager                           * aMeshManager,
                        const moris_index                           & aMeshPairIndex,
                        moris::Cell< moris::Cell< ParameterList > >   aParameterList,
                        std::shared_ptr< Library_IO >                 aLibrary );

                //------------------------------------------------------------------------------
                /**
                 * constructor with fem input
                 * @param[ in ] aMesh          mesh for this problem
                 * @param[ in ] aMeshPairIndex mesh pair index
                 * @param[ in ] aParameterList a list of list of parameter lists
                 * @param[ in ] aLibrary       a file path for property functions
                 * @param[ in ] aDesignVariableInterface a design variable interface pointer
                 */
                FEM_Model(
                        mtk::Mesh_Manager                           * aMeshManager,
                        const moris_index                           & aMeshPairIndex,
                        moris::Cell< moris::Cell< ParameterList > >   aParameterList,
                        std::shared_ptr< Library_IO >                 aLibrary,
                        MSI::Design_Variable_Interface              * aDesignVariableInterface );

                //------------------------------------------------------------------------------
                /**
                 * trivial constructor
                 */
                FEM_Model(){};

                //------------------------------------------------------------------------------
                /**
                 * destructor
                 */
                ~FEM_Model();

                //------------------------------------------------------------------------------
                /**
                 * initialize the FEM model from parameter lists
                 * @param[ in ] aLibrary       a file path for property functions
                 */
                void initialize( std::shared_ptr< Library_IO > aLibrary );

                //------------------------------------------------------------------------------

                /**
                 * resets model member variables
                 */
                void reset()
                {
                    mBulkGaussPoints               = 0;
                    mSideSetsGaussPoints           = 0;
                    mDoubleSidedSitsetsGaussPoints = 0;
                };

                //------------------------------------------------------------------------------

                /**
                 * resets model member variables
                 */
                void report_on_assembly()
                {
                    MORIS_LOG_SPEC( "Number of Bulk Gauss Points", sum_all(mBulkGaussPoints) );
                    MORIS_LOG_SPEC( "Number of SideSet Gauss Points", sum_all(mSideSetsGaussPoints) );
                    MORIS_LOG_SPEC( "Number of DoubleSidedSiteset Gauss Points", sum_all(mDoubleSidedSitsetsGaussPoints) );
                };

                //------------------------------------------------------------------------------
                /**
                 * create interpolation nodes
                 * @param[ in ] aIPMesh interpolation mesh pointer
                 */
                void create_interpolation_nodes( mtk::Interpolation_Mesh * aIPMesh );

                //------------------------------------------------------------------------------
                /**
                 * create integration nodes
                 * @param[ in ] aIGMesh integration mesh pointer
                 */
                void create_integration_nodes( mtk::Integration_Mesh * aIGMesh );

                //------------------------------------------------------------------------------
                /**
                 * get integration xyz active flags
                 * @param[ in ] aNodeIndices list of node indices
                 * @param[ in ] aPdvTypes    list of pdv types
                 * @param[ in ] aIsActiveDv  matrix to fill with 0/1 when pdv is active
                 *                           ( tNumNodeIndices x tNumPdvTypes )
                 */
                void get_integration_xyz_active_flags(
                        const Matrix< IndexMat >      & aNodeIndices,
                        const moris::Cell< PDV_Type > & aPdvTypes,
                        Matrix< DDSMat >              & aIsActiveDv );

                //------------------------------------------------------------------------------
                /**
                 * get integration xyz pdv ids
                 * @param[ in ] aNodeIndices list of node indices
                 * @param[ in ] aPdvTypes    list of pdv types
                 * @param[ in ] aXYZPdvIds   matrix to fill with ids for pdv
                 *                           ( tNumNodeIndices x tNumPdvTypes )
                 */
                void get_integration_xyz_pdv_ids(
                        const Matrix< IndexMat >      & aNodeIndices,
                        const moris::Cell< PDV_Type > & aRequestedPdvTypes,
                        Matrix< DDSMat >              & aXYZPdvIds );

                //------------------------------------------------------------------------------
                /**
                 * get integration xyz pdv ids
                 * @param[ in ] aNodeIndices list of node indices
                 * @param[ in ] aPdvTypes    list of pdv types
                 * @param[ in ] aIsActiveDv  matrix to fill with 0/1 when pdv is active
                 *                           ( tNumNodeIndices x tNumPdvTypes )
                 * @param[ in ] aXYZPdvIds   matrix to fill with ids for pdv
                 *                           ( tNumNodeIndices x tNumPdvTypes )
                 */
                void get_integration_xyz_pdv_active_flags_and_ids(
                        const Matrix< IndexMat >      & aNodeIndices,
                        const moris::Cell< PDV_Type > & aRequestedPdvTypes,
                        Matrix< DDSMat >              & aIsActiveDv,
                        Matrix< DDSMat >              & aXYZPdvIds );

                //------------------------------------------------------------------------------
                /**
                 * get integration xyz pdv local cluster assembly indices
                 * @param[ in ] aNodeIndices           list of node indices
                 * @param[ in ] aPdvTypes              list of pdv types
                 * @param[ in ] aXYZPdvAssemblyIndices matrix to fill with assembly indices for pdv
                 *                           ( tNumNodeIndices x tNumPdvTypes )
                 */
                void get_integration_xyz_pdv_assembly_indices(
                        const Matrix< IndexMat >      & aNodeIndices,
                        const moris::Cell< PDV_Type > & aRequestedPdvTypes,
                        Matrix< DDSMat >              & aXYZPdvAssemblyIndices );

                //------------------------------------------------------------------------------
                /**
                 * reset integration xyz pdv local cluster assembly indices
                 * @param[ in ] aNodeIndices list of node indices to reset
                 */
                void reset_integration_xyz_pdv_assembly_indices(
                        const Matrix< IndexMat > & aNodeIndices );

                //------------------------------------------------------------------------------
                /**
                 * set integration xyz pdv local cluster assembly index
                 * @param[ in ] aNodeIndex           node index
                 * @param[ in ] aPdvType             enum for pdv type
                 * @param[ in ] aXYZPdvAssemblyIndex assembly index for pdv type to set
                 */
                void set_integration_xyz_pdv_assembly_index(
                        moris_index   aNodeIndex,
                        enum PDV_Type aPdvType,
                        moris_index   aXYZPdvAssemblyIndex );

                //------------------------------------------------------------------------------
                /**
                 * create fem sets
                 * @param[ in ] aIPMesh interpolation mesh pointer
                 * @param[ in ] aIGMesh integration mesh pointer
                 */
                void create_fem_sets(
                               mtk::Interpolation_Mesh * aIPMesh,
                               mtk::Integration_Mesh   * aIGMesh );

                //------------------------------------------------------------------------------
                /**
                 * create fem sets
                 * @param[ in ] aIPMesh  interpolation mesh pointer
                 * @param[ in ] aIGMesh  integration mesh pointer
                 * @param[ in ] aSetInfo cell of set user info
                 */
                void create_fem_sets(
                        mtk::Interpolation_Mesh           * aIPMesh,
                        mtk::Integration_Mesh             * aIGMesh,
                        moris::Cell< fem::Set_User_Info > & aSetInfo );

                //------------------------------------------------------------------------------
                /**
                 * set parameter list
                 * @param[ in ] aParameterList a list of parameter for the FEM model
                 */
                void set_parameter_list( moris::Cell< moris::Cell< ParameterList > > aParameterList )
                {
                    mParameterList = aParameterList;
                }

                //------------------------------------------------------------------------------
                /**
                 * set space dimension ( only for UT)
                 * @param[ in ] aSpaceDim int for space dimension
                 */
                void set_space_dim( uint aSpaceDim )
                {
                    mSpaceDim = aSpaceDim;
                }

                //------------------------------------------------------------------------------
                /**
                 * get equation sets for test
                 */
                moris::Cell< MSI::Equation_Set * > & get_equation_sets()
                {
                    return mFemSets;
                }

                //------------------------------------------------------------------------------
                /**
                 * get equation objects
                 */
                moris::Cell< MSI::Equation_Object * > & get_equation_objects()
                {
                    return mFemClusters;
                }

                //------------------------------------------------------------------------------
                /**
                 * MTK set to fem set index map
                 */
                map< std::tuple< moris_index, bool, bool >, moris_index > & get_mesh_set_to_fem_set_index_map()
                {
                    return mMeshSetToFemSetMap;
                }

                //------------------------------------------------------------------------------

                /**
                 * set requested IQI names
                 * @param[ in ] aRequestedIQINames List of requested IQI names
                 */
                void set_requested_IQI_names( const moris::Cell< std::string > & aRequestedIQINames )
                {
                    mRequestedIQINames = aRequestedIQINames;
                }

                //------------------------------------------------------------------------------
                /**
                 * get requested IQI names
                 */
                const
                moris::Cell< std::string > & get_requested_IQI_names()
                {
                    return mRequestedIQINames;
                }

                //------------------------------------------------------------------------------
                /**
                 * finalize the fem sets
                 */
                void finalize_equation_sets(
                        MSI::Model_Solver_Interface * aModelSolverInterface );

                //------------------------------------------------------------------------------
                /**
                 * create a list of property pointers
                 * @param[ in ] aProperties    a list of property pointers to fill
                 * @param[ in ] aMSIDofTypeMap a map from std::string to MSI::Dof_Type
                 * @param[ in ] aDvTypeMap     a map from std::string to PDV_Type
                 * @param[ in ] aLibrary       a file path for property functions
                 */
                void create_properties(
                        std::map< std::string, uint >            & aPropertyMap,
                        moris::map< std::string, MSI::Dof_Type > & aMSIDofTypeMap,
                        moris::map< std::string, PDV_Type >      & aDvTypeMap,
                        std::shared_ptr< Library_IO >              aLibrary );

                //------------------------------------------------------------------------------
                /**
                 * create a list of field pointers
                 * * @param[ in ] aFieldNameToIndexMap  Map which maps the field name to an index
                 */
                void create_fields(
                        std::map< std::string, uint > & aFieldMap );

                //------------------------------------------------------------------------------
                /**
                 * create a list of constitutive model pointers
                 * @param[ in ] aPropertyMap   a map from property name to property index
                 * @param[ in ] aMSIDofTypeMap a map from std::string to MSI::Dof_Type
                 * @param[ in ] aDvTypeMap     a map from std::string to PDV_Type
                 */
                void create_constitutive_models(
                        std::map< std::string, uint >            & aPropertyMap,
                        moris::map< std::string, MSI::Dof_Type > & aMSIDofTypeMap,
                        moris::map< std::string, PDV_Type >      & aDvTypeMap );

                //------------------------------------------------------------------------------
                /**
                 * create a list of stabilization parameter pointers
                 * @param[ in ] aSPMap         a map from SP name to index in mSPs to fill
                 * @param[ in ] aPropertyMap   a map from property name to index in mProperties
                 * @param[ in ] aMSIDofTypeMap a map from std::string to MSI::Dof_Type
                 * @param[ in ] aDvTypeMap     a map from std::string to PDV_Type
                 */
                void create_stabilization_parameters(
                        std::map< std::string, uint >            & aSPMap,
                        std::map< std::string, uint >            & aPropertyMap,
                        moris::map< std::string, MSI::Dof_Type > & aMSIDofTypeMap,
                        moris::map< std::string, PDV_Type >      & aDvTypeMap );

                //------------------------------------------------------------------------------
                /**
                 * create a list of IWG pointers
                 * @param[ in ] aPropertyMap   a map from property name to index in mProperties
                 * @param[ in ] aSPMap         a map from SP name to index in aSPs
                 * @param[ in ] aMSIDofTypeMap a map from std::string to MSI::Dof_Type
                 */
                void create_IWGs(
                        std::map< std::string, uint >            & aPropertyMap,
                        std::map< std::string, uint >            & aSPMap,
                        moris::map< std::string, MSI::Dof_Type > & aMSIDofTypeMap );

                //------------------------------------------------------------------------------
                /**
                 * create an IQI
                 * @param[ in ] aPropertyMap   a map from property name to index in mProperties
                 * @param[ in ] aSPMap         a map from SP name to index in aSPs
                 * @param[ in ] aMSIDofTypeMap a map from std::string to MSI::Dof_Type
                 */
                void create_IQIs(
                        std::map< std::string, uint >            & aPropertyMap,
                        std::map< std::string, uint >            & aSPMap,
                        moris::map< std::string, MSI::Dof_Type > & aMSIDofTypeMap );

                //------------------------------------------------------------------------------
                /**
                 * create fem set info
                 * @param[ in ] aWithPhase FIXME remove just there to overload
                 */
                void create_fem_set_info( bool aWithPhase );

                //------------------------------------------------------------------------------
                /**
                 * create phase info
                 */
                void create_phases();

                //------------------------------------------------------------------------------
                /**
                 * scale the IQIs according to user input.
                 */
                void normalize_IQIs();

                //------------------------------------------------------------------------------
                /**
                 * get mesh set name from input
                 * @param[ in ] aBulkType             enum for bulk type
                 *                                    (bulk, single sideset, double sideset, ...)
                 * @param[ in ] aMasterPhaseName      name for master phase
                 * @param[ in ] aSlavePhaseName       name for slave phase
                 * @param[ in ] aNeighborPhasesString string with neighboring phases for single sideset
                 * @param[ in ] aSideOrdinalsString   string with side ordinals for single sideset
                 * @param[ in ] aIsGhost              bool true if ghost IWG
                 * @param[ in ] aMeshSetNames         cell of mesh set names to fill
                 */
                void get_mesh_set_names(
                        fem::Element_Type               aBulkType,
                        std::string                     aMasterPhaseName,
                        std::string                     aSlavePhaseName,
                        std::string                     aNeighborPhasesString,
                        std::string                     aSideOrdinalsString,
                        bool                            aIsGhost,
                        moris::Cell< std::string >    & aMeshSetNames );

                //------------------------------------------------------------------------------
                // FIXME Old version of FEM inputs to be removed
                //------------------------------------------------------------------------------
                /**
                 * create a list of constitutive model pointers
                 * @param[ in ] aCMMap         a map from CM name to index in aCMs
                 * @param[ in ] aPropertyMap   a map from property name to property index
                 * @param[ in ] aMSIDofTypeMap a map from std::string to MSI::Dof_Type
                 * @param[ in ] aDvTypeMap     a map from std::string to PDV_Type
                 */
                void create_constitutive_models(
                        std::map< std::string, uint >            & aCMMap,
                        std::map< std::string, uint >            & aPropertyMap,
                        moris::map< std::string, MSI::Dof_Type > & aMSIDofTypeMap,
                        moris::map< std::string, PDV_Type >      & aDvTypeMap );

                //------------------------------------------------------------------------------
                /**
                 * create a list of stabilization parameter pointers
                 * @param[ in ] aPropertyMap   a map from property name to index in mProperties
                 * @param[ in ] aCMMap         a map from CM name to index in aCMs
                 * @param[ in ] aMSIDofTypeMap a map from std::string to MSI::Dof_Type
                 * @param[ in ] aDvTypeMap     a map from std::string to PDV_Type
                 */
                void create_stabilization_parameters(
                        std::map< std::string, uint >            & aSPMap,
                        std::map< std::string, uint >            & aPropertyMap,
                        std::map< std::string, uint >            & aCMMap,
                        moris::map< std::string, MSI::Dof_Type > & aMSIDofTypeMap,
                        moris::map< std::string, PDV_Type >      & aDvTypeMap );

                //------------------------------------------------------------------------------
                /**
                 * create a list of IWG pointers
                 * @param[ in ] aPropertyMap   a map from property name to property
                 *                             index in aProperties
                 * @param[ in ] aCMMap         a map from CM name to CM index in aCMs
                 * @param[ in ] aSPMap         a map from SP name to SP index in aSPs
                 * @param[ in ] aMSIDofTypeMap a map from std::string to MSI::Dof_Type
                 * @param[ in ] aDvTypeMap     a map from std::string to PDV_Type
                 */
                void create_IWGs(
                        std::map< std::string, uint >            & aIWGMap,
                        std::map< std::string, uint >            & aPropertyMap,
                        std::map< std::string, uint >            & aCMMap,
                        std::map< std::string, uint >            & aSPMap,
                        moris::map< std::string, MSI::Dof_Type > & aMSIDofTypeMap,
                        moris::map< std::string, PDV_Type >      & aDvTypeMap );

                //------------------------------------------------------------------------------
                /**
                 * create an IQI
                 * @param[ in ] aPropertyMap   a map from property name to index in mProperties
                 * @param[ in ] aCMMap         a map from CM name to index in aCMs
                 * @param[ in ] aSPMap         a map from SP name to index in aSPs
                 * @param[ in ] aMSIDofTypeMap a map from std::string to MSI::Dof_Type
                 * @param[ in ] aDvTypeMap     a map from std::string to PDV_Type
                 */
                void create_IQIs(
                        std::map< std::string, uint >            & aIQIMap,
                        std::map< std::string, uint >            & aPropertyMap,
                        std::map< std::string, uint >            & aCMMap,
                        std::map< std::string, uint >            & aSPMap,
                        moris::map< std::string, MSI::Dof_Type > & aMSIDofTypeMap,
                        moris::map< std::string, PDV_Type >      & aDvTypeMap );

                //------------------------------------------------------------------------------
                /**
                 * create fem set info
                 */
                void create_fem_set_info();

                //------------------------------------------------------------------------------

                //void populate_fields();
        };
        //------------------------------------------------------------------------------
    } /* namespace mdl */
} /* namespace moris */


#endif /* PROJECTS_FEM_MDL_SRC_CL_FEM_MODEL_HPP_ */