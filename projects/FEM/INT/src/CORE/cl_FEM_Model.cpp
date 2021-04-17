
// added by christian: link to Google Perftools
#ifdef WITHGPERFTOOLS
#include <gperftools/profiler.h>
#endif

//LINALG/src
#include "cl_Map.hpp"
#include "cl_Matrix.hpp"
#include "fn_unique.hpp"
#include "fn_sum.hpp" // for check
#include "fn_iscol.hpp"
#include "fn_trans.hpp"
#include "op_equal_equal.hpp"
//MTK/src
#include "MTK_Tools.hpp"
#include "cl_MTK_Enums.hpp"
#include "cl_MTK_Mesh_Manager.hpp"
#include "cl_MTK_Integration_Mesh.hpp"
#include "cl_MTK_Interpolation_Mesh.hpp"
//FEM/INT/src
#include "cl_FEM_Node_Base.hpp"
#include "cl_FEM_Node.hpp"
#include "cl_FEM_Enums.hpp"
#include "cl_FEM_Model.hpp"
#include "cl_FEM_Set.hpp"
#include "cl_FEM_CM_Factory.hpp"
#include "cl_FEM_SP_Factory.hpp"
#include "cl_FEM_IWG_Factory.hpp"
#include "cl_FEM_IQI_Factory.hpp"
#include "cl_FEM_Field.hpp"
//FEM/MSI/src
#include "cl_MSI_Equation_Object.hpp"
#include "cl_MSI_Dof_Type_Enums.hpp"
//FEM/VIS/src
#include "cl_VIS_Output_Enums.hpp"
//GEN/src
#include "cl_GEN_Pdv_Enums.hpp"
// Logging package
#include "cl_Logger.hpp"
#include "cl_Tracer.hpp"

namespace moris
{
    namespace fem
    {
        // User-defined FEM function
        typedef void ( *FEM_Function ) (
                moris::Matrix< moris::DDRMat >                & aPropMatrix,
                moris::Cell< moris::Matrix< moris::DDRMat > > & aParameters,
                moris::fem::Field_Interpolator_Manager        * aFIManager );
        //------------------------------------------------------------------------------

        FEM_Model::FEM_Model(
                std::shared_ptr< mtk::Mesh_Manager > aMeshManager,
                const moris_index                 & aMeshPairIndex,
                moris::Cell< fem::Set_User_Info > & aSetInfo )
        : mMeshManager( aMeshManager ),
          mMeshPairIndex( aMeshPairIndex )
        {
            Tracer tTracer( "FEM", "FemModel", "Create" );

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 0: unpack mesh
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // get pointers to interpolation and integration meshes
            mtk::Interpolation_Mesh* tIPMesh = nullptr;
            mtk::Integration_Mesh*   tIGMesh = nullptr;

            mMeshManager->get_mesh_pair( mMeshPairIndex, tIPMesh, tIGMesh );

            // set the space dimension
            mSpaceDim = tIPMesh->get_spatial_dim();

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 1: create nodes
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            this->create_interpolation_nodes( tIPMesh );

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 2: create sets
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            this->create_fem_sets( tIPMesh, tIGMesh, aSetInfo );
        }

        //------------------------------------------------------------------------------

        FEM_Model::FEM_Model(
                std::shared_ptr< mtk::Mesh_Manager > aMeshManager,
                const moris_index                 & aMeshPairIndex,
                moris::Cell< fem::Set_User_Info > & aSetInfo,
                MSI::Design_Variable_Interface    * aDesignVariableInterface )
        : mMeshManager( aMeshManager ),
          mMeshPairIndex( aMeshPairIndex )
        {
            Tracer tTracer( "FEM", "FemModel", "Create" );

            this->set_design_variable_interface( aDesignVariableInterface );

            // if no design variables have been stipulated, skip
            if (aDesignVariableInterface == nullptr)
            {
                mFEMOnly = true;
                MORIS_LOG("Skipping GEN, FEM Only");
            }

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 0: unpack mesh
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // get pointers to interpolation and integration meshes
            mtk::Interpolation_Mesh* tIPMesh = nullptr;
            mtk::Integration_Mesh*   tIGMesh = nullptr;

            mMeshManager->get_mesh_pair( mMeshPairIndex, tIPMesh, tIGMesh );

            // set the space dimension
            mSpaceDim = tIPMesh->get_spatial_dim();

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 1: create interpolation nodes
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            this->create_interpolation_nodes( tIPMesh );

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 2: create integration nodes
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            this->create_integration_nodes( tIGMesh );

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 3: create fem sets
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            this->create_fem_sets( tIPMesh, tIGMesh, aSetInfo );
        }

        //------------------------------------------------------------------------------

        FEM_Model::FEM_Model(
                std::shared_ptr< mtk::Mesh_Manager >          aMeshManager,
                const moris_index                           & aMeshPairIndex,
                moris::Cell< moris::Cell< ParameterList > >   aParameterList,
                std::shared_ptr< Library_IO >                 aLibrary )
        : mMeshManager( aMeshManager ),
          mMeshPairIndex( aMeshPairIndex ),
          mParameterList( aParameterList )
        {
            Tracer tTracer( "FEM", "FemModel", "Create" );

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 0: unpack fem input and mesh
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            // get pointers to interpolation and integration meshes
            mtk::Interpolation_Mesh* tIPMesh = nullptr;
            mtk::Integration_Mesh*   tIGMesh = nullptr;
            mMeshManager->get_mesh_pair( mMeshPairIndex, tIPMesh, tIGMesh );

            // set the space dimension
            mSpaceDim = tIPMesh->get_spatial_dim();

            // unpack the FEM inputs
            this->initialize( aLibrary );

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 1: create IP nodes
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            this->create_interpolation_nodes( tIPMesh );

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 2: create fem sets
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            this->create_fem_sets( tIPMesh, tIGMesh );
        }

        //------------------------------------------------------------------------------

        FEM_Model::FEM_Model(
                std::shared_ptr< mtk::Mesh_Manager >          aMeshManager,
                const moris_index                           & aMeshPairIndex,
                moris::Cell< moris::Cell< ParameterList > >   aParameterList,
                std::shared_ptr< Library_IO >                 aLibrary,
                MSI::Design_Variable_Interface              * aDesignVariableInterface )
        : mMeshManager( aMeshManager ),
          mMeshPairIndex( aMeshPairIndex ),
          mParameterList( aParameterList )
        {
            Tracer tTracer( "FEM", "FemModel", "Create" );

            this->set_design_variable_interface( aDesignVariableInterface );

            // if no design variables have been stipulated, skip
            if (aDesignVariableInterface == nullptr)
            {
                mFEMOnly = true;
                MORIS_LOG("Skipping GEN, FEM Only");
            }

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 0: unpack fem input and mesh
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            // get pointers to interpolation and integration meshes
            mtk::Interpolation_Mesh* tIPMesh = nullptr;
            mtk::Integration_Mesh*   tIGMesh = nullptr;
            mMeshManager->get_mesh_pair( mMeshPairIndex, tIPMesh, tIGMesh );

            // set the space dimension
            mSpaceDim = tIPMesh->get_spatial_dim();

            // unpack the FEM inputs
            this->initialize( aLibrary );

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 1: create interpolation nodes
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            this->create_interpolation_nodes( tIPMesh );

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 2: create integration nodes
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            this->create_integration_nodes( tIGMesh );

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 3: create fem sets
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            this->create_fem_sets( tIPMesh, tIGMesh );
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_interpolation_nodes( mtk::Interpolation_Mesh* aIPMesh )
        {
            // ask mesh about number of IP nodes on proc
            luint tNumIPNodes = aIPMesh->get_num_nodes();

            // create IP node objects
            mIPNodes.resize( tNumIPNodes, nullptr );

            for( uint iNode = 0; iNode < tNumIPNodes; iNode++ )
            {
                mIPNodes( iNode ) = new fem::Node( &aIPMesh->get_mtk_vertex( iNode ) );
            }

            // print output
            MORIS_LOG_SPEC("IP nodes",sum_all(tNumIPNodes));
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_integration_nodes( mtk::Integration_Mesh * aIGMesh )
        {
            // ask IG mesh about number of IG vertices on proc
            luint tNumIGNodes = aIGMesh->get_num_nodes();

            // set size for list IG nodes
            mIGNodes.resize( tNumIGNodes, nullptr );

            moris::Cell < enum PDV_Type > tGeoPdvType;
            switch ( mSpaceDim )
            {
                case 1 :
                    tGeoPdvType = { PDV_Type::X_COORDINATE };
                    break;
                case 2 :
                    tGeoPdvType = { PDV_Type::X_COORDINATE, PDV_Type::Y_COORDINATE };
                    break;
                case 3 :
                    tGeoPdvType = { PDV_Type::X_COORDINATE, PDV_Type::Y_COORDINATE, PDV_Type::Z_COORDINATE };
                    break;
                default:
                    MORIS_ERROR( false,
                            "FEM_Model::create_integration_nodes - Space dimension can only be 1, 2 or 3D.");
            }

            // loop over IG mesh vertices
            for( uint iNode = 0; iNode < tNumIGNodes; iNode++ )
            {
                // create a new IG Node
                mIGNodes( iNode ) = new fem::Node( &aIGMesh->get_mtk_vertex( iNode ) );

                if (mFEMOnly == false)
                {
                    // get IG node index
                    uint tVertexIndex = mIGNodes( iNode )->get_index();

                    // get IG node coordinates
                    Matrix<DDRMat> tVertexCoordsFromMesh;
                    mIGNodes( iNode )->get_vertex_coords( tVertexCoordsFromMesh );

                    // get the pdv values from the MSI/GEN interface
                    Matrix<IndexMat> tVertexIndices( 1, 1, tVertexIndex );
                    moris::Cell<Matrix<DDRMat>> tVertexCoordsFromGen( mSpaceDim );
                    moris::Cell<Matrix<DDSMat>> tIsActiveDv;

                    this->get_design_variable_interface()->get_ig_pdv_value(
                        tVertexIndices,
                        tGeoPdvType,
                        tVertexCoordsFromGen,
                        tIsActiveDv );

                    // set active flags for xyz
                    mIGNodes(iNode)->set_vertex_xyz_active_flags( tIsActiveDv );

                    // reshape the XYZ values into a cell of vectors
                    for ( uint iSpaceDim = 0; iSpaceDim < mSpaceDim; iSpaceDim++ )
                    {
                        if (tIsActiveDv( iSpaceDim )(0))
                        {
                            MORIS_ERROR( equal_to(
                                            tVertexCoordsFromGen(iSpaceDim)(0),
                                            tVertexCoordsFromMesh(iSpaceDim), 1.0),
                                        "FEM_Model::create_integration_nodes - GE coordinate and MTK coordinate differ\n" );
                        }
                    }

                    // get the id associated to the pdv
                    moris::Cell<moris::Matrix<DDSMat>> tPdvIds;
                    this->get_design_variable_interface()->get_ig_dv_ids_for_type_and_ind(
                        tVertexIndices,
                        tGeoPdvType,
                        tPdvIds );

                    // set pdv ids for xyz
                    mIGNodes( iNode )->set_vertex_xyz_pdv_ids( tPdvIds );
                }
            }

            // print output
            MORIS_LOG_SPEC("IG nodes",sum_all(tNumIGNodes));
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_fem_sets(
                mtk::Interpolation_Mesh           * aIPMesh,
                mtk::Integration_Mesh             * aIGMesh,
                moris::Cell< fem::Set_User_Info > & aSetInfo )
        {
            // get the number of sets
            uint tNumFemSets = aSetInfo.size();

            // create equation sets
            mFemSets.resize( tNumFemSets, nullptr );

            // get number of IP cells
            uint tNumIPCells = aIPMesh->get_num_elems();

            // reserve size for list of equation objects
            mFemClusters.reserve( tNumIPCells );

            // loop over the used fem set
            for( luint iSet = 0; iSet < tNumFemSets; iSet++ )
            {
                // get the mesh set name
                std::string tMeshSetName = aSetInfo( iSet).get_mesh_set_name();

                moris_index tMeshSetIndex;
                if( tMeshSetName.size() > 0 )
                {
                    // get the mesh set index from its name
                    tMeshSetIndex = aIGMesh->get_set_index_by_name( tMeshSetName );
                }
                else
                {
                    tMeshSetIndex = aSetInfo( iSet ).get_mesh_index();
                }

                // fill the mesh set index to fem set index map
                mMeshSetToFemSetMap[ std::make_tuple(
                        tMeshSetIndex,
                        aSetInfo( iSet ).get_time_continuity(),
                        aSetInfo( iSet ).get_time_boundary() ) ] = iSet;

                // get the mesh set pointer
                moris::mtk::Set * tMeshSet = aIGMesh->get_set_by_index( tMeshSetIndex );

                // if non-empty mesh set
                if ( tMeshSet->get_num_clusters_on_set() !=0 )
                {
                    // create a fem set
                    mFemSets( iSet ) = new fem::Set( this, tMeshSet, aSetInfo( iSet ), mIPNodes );
                    mFemSets( iSet )->set_equation_model( this );
                }
                // if empty mesh set
                else
                {
                    // create an empty fem set
                    mFemSets( iSet ) = new fem::Set();
                    mFemSets( iSet )->set_equation_model( this );
                }

                // collect equation objects associated with the set
                mFemClusters.append( mFemSets( iSet )->get_equation_object_list() );
            }
            // shrink list to fit size
            mFemClusters.shrink_to_fit();

            uint tNumElements       = mFemClusters.size();
            uint tGlobalNumElements = sum_all(tNumElements);

            // print output
            MORIS_LOG_SPEC("IP elements",tGlobalNumElements);
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_fem_sets(
                mtk::Interpolation_Mesh * aIPMesh,
                mtk::Integration_Mesh   * aIGMesh )
        {
            // get number of fem sets
            uint tNumFemSets = mSetInfo.size();

            // set size for list of equation sets
            mFemSets.resize( tNumFemSets, nullptr );

            // get number of IP cells
            uint tNumIPCells = aIPMesh->get_num_elems();

            // reserve size for list of equation objects
            mFemClusters.reserve( tNumIPCells );

            // loop over the used fem set
            for( luint iSet = 0; iSet < tNumFemSets; iSet++ )
            {
                // get the mesh set name
                std::string tMeshSetName = mSetInfo( iSet ).get_mesh_set_name();

                // get the mesh set index from its name
                moris_index tMeshSetIndex = aIGMesh->get_set_index_by_name( tMeshSetName );

                // fill the mesh set index to fem set index map
                mMeshSetToFemSetMap[ std::make_tuple(
                        tMeshSetIndex,
                        mSetInfo( iSet ).get_time_continuity(),
                        mSetInfo( iSet ).get_time_boundary() ) ] = iSet;

                // get the mesh set pointer
                moris::mtk::Set * tMeshSet = aIGMesh->get_set_by_index( tMeshSetIndex );

                // if non-empty mesh set
                if ( tMeshSet->get_num_clusters_on_set() != 0 )
                {
                    // create new fem set
                    mFemSets( iSet ) = new fem::Set( this, tMeshSet, mSetInfo( iSet ), mIPNodes );

                    mFemSets( iSet )->set_equation_model( this );
                }
                // if empty mesh set
                else
                {
                    // create an empty fem set
                    mFemSets( iSet ) = new fem::Set();

                    mFemSets( iSet )->set_equation_model( this );
                }

                // collect equation objects associated with the set
                mFemClusters.append( mFemSets( iSet )->get_equation_object_list() );
            }

            // shrink to fit
            mFemClusters.shrink_to_fit();

            uint tNumElements       = mFemClusters.size();
            uint tGlobalNumElements = sum_all(tNumElements);

            // print output
            MORIS_LOG_SPEC("IP elements",tGlobalNumElements);
        }

        //------------------------------------------------------------------------------

        void FEM_Model::get_integration_xyz_active_flags(
                const Matrix< IndexMat >      & aNodeIndices,
                const moris::Cell< PDV_Type > & aRequestedPdvTypes,
                Matrix< DDSMat >              & aIsActiveDv )
        {
            // Get the number of node indices requested
            uint tNumIndices = aNodeIndices.length();

            // Get the number of dv types requested
            uint tNumTypes = aRequestedPdvTypes.size();

            // set size for is active dv types
            aIsActiveDv.set_size( tNumIndices, tNumTypes, 0 );

            // loop over the requested dv types
            for ( uint tNode = 0; tNode < tNumIndices; tNode++ )
            {
                // get node index
                uint tNodeIndex = aNodeIndices( tNode );

                // get flags from nodes
                Matrix< DDSMat > tIsActiveDvTemp;
                mIGNodes( tNodeIndex )->get_vertex_xyz_active_flags(
                        tIsActiveDvTemp,
                        aRequestedPdvTypes );

                aIsActiveDv.get_row( tNode ) = tIsActiveDvTemp.matrix_data();
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::get_integration_xyz_pdv_ids(
                const Matrix< IndexMat >      & aNodeIndices,
                const moris::Cell< PDV_Type > & aRequestedPdvTypes,
                Matrix< DDSMat >              & aXYZPdvIds )
        {
            // Get the number of node indices requested
            uint tNumIndices = aNodeIndices.length();

            // Get the number of dv types requested
            uint tNumTypes = aRequestedPdvTypes.size();

            // set size for is active dv types
            aXYZPdvIds.set_size( tNumIndices, tNumTypes, -1 );

            // loop over the requested dv types
            for ( uint tNode = 0; tNode < tNumIndices; tNode++ )
            {
                // get node index
                uint tNodeIndex = aNodeIndices( tNode );

                // get flags from nodes
                Matrix< DDSMat > tPdvIdsTemp;
                mIGNodes( tNodeIndex )->get_vertex_xyz_pdv_ids(
                        tPdvIdsTemp,
                        aRequestedPdvTypes );

                aXYZPdvIds.get_row( tNode ) = tPdvIdsTemp.matrix_data();
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::get_integration_xyz_pdv_active_flags_and_ids(
                const Matrix< IndexMat >      & aNodeIndices,
                const moris::Cell< PDV_Type > & aRequestedPdvTypes,
                Matrix< DDSMat >              & aIsActiveDv,
                Matrix< DDSMat >              & aXYZPdvIds )
        {
            // Get the number of node indices requested
            uint tNumIndices = aNodeIndices.length();

            // Get the number of dv types requested
            uint tNumTypes = aRequestedPdvTypes.size();

            // set size for is active dv types
            aIsActiveDv.set_size( tNumIndices, tNumTypes, 0 );

            // set size for is active dv types
            aXYZPdvIds.set_size( tNumIndices, tNumTypes, -1 );

            // loop over the requested dv types
            for ( uint tNode = 0; tNode < tNumIndices; tNode++ )
            {
                // get node index
                uint tNodeIndex = aNodeIndices( tNode );

                // get flags from nodes
                Matrix< DDSMat > tIsActiveDvTemp;
                mIGNodes( tNodeIndex )->get_vertex_xyz_active_flags(
                        tIsActiveDvTemp,
                        aRequestedPdvTypes );
                aIsActiveDv.get_row( tNode ) = tIsActiveDvTemp.matrix_data();

                // get flags from nodes
                Matrix< DDSMat > tPdvIdsTemp;
                mIGNodes( tNodeIndex )->get_vertex_xyz_pdv_ids(
                        tPdvIdsTemp,
                        aRequestedPdvTypes );
                aXYZPdvIds.get_row( tNode ) = tPdvIdsTemp.matrix_data();
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::reset_integration_xyz_pdv_assembly_indices(
                const Matrix< IndexMat > & aNodeIndices )
        {
            // Get the number of node indices requested
            uint tNumIndices = aNodeIndices.length();

            // loop over the requested dv types
            for ( uint tNode = 0; tNode < tNumIndices; tNode++ )
            {
                // get node index
                uint tNodeIndex = aNodeIndices( tNode );

                // get assembly index from nodes
                mIGNodes( tNodeIndex )->reset_local_xyz_pdv_assembly_index();
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::get_integration_xyz_pdv_assembly_indices(
                const Matrix< IndexMat >      & aNodeIndices,
                const moris::Cell< PDV_Type > & aRequestedPdvTypes,
                Matrix< DDSMat >              & aXYZPdvAssemblyIndices )
        {
            // Get the number of node indices requested
            uint tNumIndices = aNodeIndices.length();

            // Get the number of dv types requested
            uint tNumTypes = aRequestedPdvTypes.size();

            // set size for is active dv types
            aXYZPdvAssemblyIndices.set_size( tNumIndices, tNumTypes, -1 );

            // loop over the requested dv types
            for ( uint tNode = 0; tNode < tNumIndices; tNode++ )
            {
                // get node index
                uint tNodeIndex = aNodeIndices( tNode );

                // get assembly index from nodes
                Matrix< DDSMat > tPdvAssemblyIndicesTemp;
                mIGNodes( tNodeIndex )->get_local_xyz_pdv_assembly_indices(
                        tPdvAssemblyIndicesTemp,
                        aRequestedPdvTypes );

                aXYZPdvAssemblyIndices.get_row( tNode ) = tPdvAssemblyIndicesTemp.matrix_data();
            }
        }

        void FEM_Model::set_integration_xyz_pdv_assembly_index(
                moris_index aNodeIndex,
                enum PDV_Type aPdvType,
                moris_index aXYZPdvAssemblyIndex )
        {
            mIGNodes( aNodeIndex )->set_local_xyz_pdv_assembly_index( aXYZPdvAssemblyIndex, aPdvType );
        }

        //------------------------------------------------------------------------------

        void FEM_Model::initialize( std::shared_ptr< Library_IO > aLibrary )
        {
            // get msi string to dof type map
            moris::map< std::string, MSI::Dof_Type > tMSIDofTypeMap =
                    moris::MSI::get_msi_dof_type_map();

            // get string to dv type map
            moris::map< std::string, PDV_Type > tMSIDvTypeMap =
                    get_pdv_type_map();

            // get string to field type map
            moris::map< std::string, mtk::Field_Type > tFieldTypeMap =
                    mtk::get_field_type_map();

            switch( mParameterList.size() )
            {
                // without phase
                case 7:
                {
                    // create properties
                    std::map< std::string, uint > tPropertyMap;
                    this->create_properties( tPropertyMap, tMSIDofTypeMap, tMSIDvTypeMap, tFieldTypeMap, aLibrary );

                    // create fields
                    std::map< std::string, uint > tFieldMap;
                    this->create_fields( tFieldMap );

                    // create constitutive models
                    std::map< std::string, uint > tCMMap;
                    this->create_constitutive_models( tCMMap, tPropertyMap, tMSIDofTypeMap, tMSIDvTypeMap );

                    // create stabilization parameters
                    std::map< std::string, uint > tSPMap;
                    this->create_stabilization_parameters( tSPMap, tPropertyMap, tCMMap, tMSIDofTypeMap, tMSIDvTypeMap );

                    // create IWGs
                    std::map< std::string, uint > tIWGMap;
                    this->create_IWGs( tIWGMap, tPropertyMap, tCMMap, tSPMap, tFieldMap, tMSIDofTypeMap, tMSIDvTypeMap, tFieldTypeMap );

                    // create IQIs
                    std::map< std::string, uint > tIQIMap;
                    this->create_IQIs( tIQIMap, tPropertyMap, tCMMap, tSPMap, tFieldMap, tMSIDofTypeMap, tMSIDvTypeMap, tFieldTypeMap );

                    // create FEM set info
                    this->create_fem_set_info();

                    break;
                }
                // with phase
                case 8:
                {
                    // create phases
                    this->create_phases();

                    // create properties
                    std::map< std::string, uint > tPropertyMap;
                    this->create_properties( tPropertyMap, tMSIDofTypeMap, tMSIDvTypeMap, tFieldTypeMap, aLibrary );

                    // create fields
                    std::map< std::string, uint > tFieldMap;
                    this->create_fields( tFieldMap );

                    // create constitutive models
                    this->create_constitutive_models( tPropertyMap, tMSIDofTypeMap, tMSIDvTypeMap );

                    // create stabilization parameters
                    std::map< std::string, uint > tSPMap;
                    this->create_stabilization_parameters( tSPMap, tPropertyMap, tMSIDofTypeMap, tMSIDvTypeMap );

                    // create IWGs
                    this->create_IWGs( tPropertyMap, tSPMap, tMSIDofTypeMap );

                    // create IQIs
                    this->create_IQIs( tPropertyMap, tSPMap, tMSIDofTypeMap );

                    // create FEM set info
                    this->create_fem_set_info( true );

                    // get fem computation type parameter list
                    ParameterList tComputationParameterList = mParameterList( 5 )( 0 );

                    // get bool for printing physics model
                    bool tPrintPhysics =
                            tComputationParameterList.get< bool >( "print_physics_model" );

                    // if print FEM model
                    if( tPrintPhysics )
                    {
                        // phase info
                        std::cout<<"Phase info "<<std::endl;

                        // loop over phase info
                        for( uint iPhase = 0; iPhase < mPhaseInfo.size(); iPhase++ )
                        {
                            std::cout<<"%-------------------------------------------------"<<std::endl;
                            mPhaseInfo( iPhase ).print_names();
                            std::cout<<"%-------------------------------------------------"<<std::endl;
                        }

                        std::cout<<" "<<std::endl;

                        // set info
                        std::cout<<"Set info "<<std::endl;

                        // loop over set info
                        for( uint iSet = 0; iSet < mSetInfo.size(); iSet++ )
                        {
                            std::cout<<"%-------------------------------------------------"<<std::endl;
                            mSetInfo( iSet ).print_names();
                            std::cout<<"%-------------------------------------------------"<<std::endl;
                        }
                    }
                    break;
                }

                default:
                    MORIS_ERROR( false, "FEM_Model::initialize - wrong size for parameter list" );
            }
        }

        //------------------------------------------------------------------------------

        FEM_Model::~FEM_Model()
        {
            // delete fem nodes
            for( auto tIPNodes : mIPNodes )
            {
                delete tIPNodes;
            }
            mIPNodes.clear();

            // delete fem nodes
            for( auto tIGNodes : mIGNodes )
            {
                delete tIGNodes;
            }
            mIGNodes.clear();

            // delete the fem sets
            for( auto tFemSet : mFemSets )
            {
                delete tFemSet;
            }
            mFemSets.clear();

            // delete the fem cluster
            mFemClusters.clear();
        }

        //------------------------------------------------------------------------------

        void FEM_Model::finalize_equation_sets(
                MSI::Model_Solver_Interface * aModelSolverInterface )
        {
            // loop over the fem sets
            for( MSI::Equation_Set * tFemSet : mFemSets )
            {
                // finalize the fem set
                tFemSet->finalize( aModelSolverInterface );
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_properties(
                std::map< std::string, uint >              & aPropertyMap,
                moris::map< std::string, MSI::Dof_Type >   & aMSIDofTypeMap,
                moris::map< std::string, PDV_Type >        & aDvTypeMap,
                moris::map< std::string, mtk::Field_Type > & aFieldTypeMap,
                std::shared_ptr< Library_IO >                aLibrary )
        {
            // get the property parameter list
            moris::Cell< ParameterList > tPropParameterList = mParameterList( 0 );

            // get the number of properties
            uint tNumProps = tPropParameterList.size();

            // create a list of property pointers
            mProperties.resize( tNumProps, nullptr );

            // loop over the parameter lists
            for ( uint iProp = 0; iProp < tNumProps; iProp++ )
            {
                // get property parameter list
                ParameterList tPropParameter = tPropParameterList( iProp );

                // get property name from parameter list
                std::string tPropertyName = tPropParameter.get< std::string >( "property_name" );

                // create a property pointer
                mProperties( iProp ) = std::make_shared< fem::Property >();

                // set a name for the property
                mProperties( iProp )->set_name( tPropertyName );

                // fill property map
                aPropertyMap[ tPropertyName ] = iProp;

                // set dof dependencies
                moris::Cell< moris::Cell< moris::MSI::Dof_Type > > tDofTypes;
                string_to_cell_of_cell(
                        tPropParameter.get< std::string >( "dof_dependencies" ),
                        tDofTypes,
                        aMSIDofTypeMap );
                mProperties( iProp )->set_dof_type_list( tDofTypes );

                // set dv dependencies
                moris::Cell< moris::Cell< PDV_Type > > tDvTypes;
                string_to_cell_of_cell(
                        tPropParameter.get< std::string >( "dv_dependencies" ),
                        tDvTypes,
                        aDvTypeMap );
                mProperties( iProp )->set_dv_type_list( tDvTypes );

                // set field dependencies
                moris::Cell< moris::Cell< mtk::Field_Type > > tFieldTypes;
                string_to_cell_of_cell(
                        tPropParameter.get< std::string >( "field_dependencies" ),
                        tFieldTypes,
                        aFieldTypeMap );
                mProperties( iProp )->set_field_type_list( tFieldTypes );

                // set function parameters
                moris::Cell< moris::Matrix< DDRMat > > tFuncParameters;
                string_to_cell_mat_2(
                        tPropParameter.get< std::string >( "function_parameters" ),
                        tFuncParameters );
                mProperties( iProp )->set_parameters( tFuncParameters );

                // set value function for property
                std::string tValFuncName = tPropParameter.get< std::string >( "value_function" );
                FEM_Function tValFunction = nullptr;
                if ( tValFuncName.size() > 1 )
                {
                    tValFunction = aLibrary->load_function<FEM_Function>( tValFuncName );
                    mProperties( iProp )->set_val_function( tValFunction );
                }

                // set dof derivative function for property
                moris::Cell< std::string > tDofDerFuncNames;
                string_to_cell(
                        tPropParameter.get< std::string >( "dof_derivative_functions" ),
                        tDofDerFuncNames );
                uint tNumDofDerFuncs = tDofDerFuncNames.size();
                moris::Cell< fem::PropertyFunc > tDofDerFunctions( tNumDofDerFuncs, nullptr );
                for( uint iFunc = 0; iFunc < tNumDofDerFuncs; iFunc++ )
                {
                    if( tDofDerFuncNames( iFunc ).size() > 1 )
                    {
                        FEM_Function tValFunction =
                                aLibrary->load_function<FEM_Function>( tDofDerFuncNames( iFunc ) );
                        tDofDerFunctions( iFunc ) = tValFunction;
                    }
                }
                mProperties( iProp )->set_dof_derivative_functions( tDofDerFunctions );

                // set dv derivative function for property
                moris::Cell< std::string > tDvDerFuncNames;
                string_to_cell(
                        tPropParameter.get< std::string >( "dv_derivative_functions" ),
                        tDvDerFuncNames );
                uint tNumDvDerFuncs = tDvDerFuncNames.size();
                moris::Cell< fem::PropertyFunc > tDvDerFunctions( tNumDvDerFuncs, nullptr );
                for( uint iFunc = 0; iFunc < tNumDvDerFuncs; iFunc++ )
                {
                    if( tDvDerFuncNames( iFunc ).size() > 1 )
                    {
                        FEM_Function tValFunction =
                                aLibrary->load_function<FEM_Function>( tDvDerFuncNames( iFunc ) );
                        tDvDerFunctions( iFunc ) = tValFunction;
                    }
                }
                mProperties( iProp )->set_dv_derivative_functions( tDvDerFunctions );
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_fields(
                std::map< std::string, uint > & aFieldMap )
        {
            // get the property parameter list
            moris::Cell< ParameterList > tFieldParameterList = mParameterList( 6 );

            // get the number of properties
            sint tNumFields = tFieldParameterList.size();

            // create a list of property pointers
            mFields.resize( tNumFields, nullptr );

            // loop over the parameter lists
            for ( sint iFields = 0; iFields < tNumFields; iFields++ )
            {
                // get property parameter list
                ParameterList tFieldParameter = tFieldParameterList( iFields );

                // get property name from parameter list
                std::string tFieldName = tFieldParameter.get< std::string >( "field_name" );

                // create a property pointer
                std::shared_ptr< fem::Field> tField =  std::make_shared< fem::Field >(
                        mMeshManager->get_mesh_pair( mMeshPairIndex ) );

                // set a name for the property
                tField->set_label( tFieldName );

                // fill property map
                aFieldMap[ tFieldName ] = iFields;

                // set field type
                moris::map< std::string, mtk::Field_Type > tFieldTypeMap =
                        mtk::get_field_type_map();

                moris::Cell< mtk::Field_Type > tFieldTypes;
                string_to_cell(
                        tFieldParameter.get< std::string >( "field_type" ),
                        tFieldTypes,
                        tFieldTypeMap );

                // set field type
                tField->set_field_type( tFieldTypes );

                mFieldTypeMap.resize( std::max( static_cast< uint >( tFieldTypes( 0 ) ) + 1, ( uint )mFieldTypeMap.size() ), -1 );
                mFieldTypeMap( static_cast< uint >(tFieldTypes( 0 )) ) = iFields;

                MORIS_ERROR( (tFieldParameter.get< std::string >( "field_create_from_file" ).empty()) or
                        (tFieldParameter.get< std::string >( "IQI_Name" ).empty() ),
                        "FEM_Model::create_fields(); Field must be either created based on IQI or read from file.");

                MORIS_ERROR( not ((not tFieldParameter.get< std::string >( "field_create_from_file" ).empty()) and
                        (not tFieldParameter.get< std::string >( "IQI_Name" ).empty()  )),
                        "FEM_Model::create_fields(); Field must be either created based on IQI or read from file.");

                if( not tFieldParameter.get< std::string >( "field_create_from_file" ).empty() )
                {
                    tField->set_field_from_file( tFieldParameter.get< std::string >( "field_create_from_file" ) );
                }

                if( not tFieldParameter.get< std::string >( "IQI_Name" ).empty() )
                {
                    tField->set_IQI_name( tFieldParameter.get< std::string >( "IQI_Name" ) );
                }

                if( not tFieldParameter.get< std::string >( "field_output_to_file" ).empty() )
                {
                    tField->set_field_to_file( tFieldParameter.get< std::string >( "field_output_to_file" ) );
                }

                mFields( iFields ) = tField;
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_constitutive_models(
                std::map< std::string, uint >            & aPropertyMap,
                moris::map< std::string, MSI::Dof_Type > & aMSIDofTypeMap,
                moris::map< std::string, PDV_Type >      & aDvTypeMap )
        {
            // create a constitutive model factory
            CM_Factory tCMFactory;

            // get the CM parameter list
            moris::Cell< ParameterList > tCMParameterList = mParameterList( 1 );

            // get number of constitutive models
            uint tNumCMs = tCMParameterList.size();

            // loop over the parameter lists for CM
            for( uint iCM = 0; iCM < tNumCMs; iCM++ )
            {
                // get the treated CM parameter list
                ParameterList tCMParameter = tCMParameterList( iCM );

                // get the constitutive type from parameter list
                fem::Constitutive_Type tCMType =
                        static_cast< fem::Constitutive_Type >( tCMParameter.get< uint >( "constitutive_type" ) );

                // get the constitutive model name from parameter list
                std::string tCMName =
                        tCMParameter.get< std::string >( "constitutive_name" );

                // get the phase from parameter list
                std::string tPhaseName =
                        tCMParameter.get< std::string >( "phase_name" );

                // get the model type
                fem::Model_Type tCMModelType =
                        static_cast< fem::Model_Type >( tCMParameter.get< uint >( "model_type" ) );

                // create a constitutive model pointer
                std::shared_ptr< fem::Constitutive_Model > tCM =
                        tCMFactory.create_CM( tCMType );

                // set CM name
                tCM->set_name( tCMName );

                // set CM model type. must come before "set_space_dim"
                // fixme: currently cannot set a plane type and tensor type at the same time from an input file
                if( tCMModelType != fem::Model_Type::UNDEFINED )
                {
                    tCM->set_model_type( tCMModelType );
                }

                // set CM space dimension
                tCM->set_space_dim( mSpaceDim );

                // set CM dof dependencies
                moris::Cell< moris::Cell< moris::MSI::Dof_Type > > tDofTypes;
                string_to_cell_of_cell(
                        std::get< 0 >( tCMParameter.get< std::pair< std::string, std::string > >( "dof_dependencies" ) ),
                        tDofTypes,
                        aMSIDofTypeMap );
                moris::Cell< std::string > tDofTypeNames;
                string_to_cell(
                        std::get< 1 >( tCMParameter.get< std::pair< std::string, std::string > >( "dof_dependencies" ) ),
                        tDofTypeNames );
                tCM->set_dof_type_list( tDofTypes, tDofTypeNames );

                // set CM dv dependencies
                moris::Cell< moris::Cell< PDV_Type > > tDvTypes;
                string_to_cell_of_cell(
                        std::get< 0 >( tCMParameter.get< std::pair< std::string, std::string > >( "dv_dependencies" ) ),
                        tDvTypes,
                        aDvTypeMap );
                moris::Cell< std::string > tDvTypeNames;
                string_to_cell(
                        std::get< 1 >( tCMParameter.get< std::pair< std::string, std::string > >( "dv_dependencies" ) ),
                        tDvTypeNames );
                tCM->set_dv_type_list( tDvTypes, tDvTypeNames );

                // set CM properties
                moris::Cell< moris::Cell< std::string > > tPropertyNamesPair;
                string_to_cell_of_cell(
                        tCMParameter.get< std::string >( "properties" ),
                        tPropertyNamesPair );
                for( uint iProp = 0; iProp < tPropertyNamesPair.size(); iProp++ )
                {
                    // get the property name
                    std::string tPropertyName = tPropertyNamesPair( iProp )( 0 );

                    // check if property in the map
                    MORIS_ERROR( aPropertyMap.find( tPropertyName ) != aPropertyMap.end(),
                            "FEM_Model::create_CMs - Unknown aPropertyString : %s \n",
                            tPropertyName.c_str() );

                    // get property index
                    uint tPropertyIndex = aPropertyMap[ tPropertyName ];

                    // set property for CM
                    tCM->set_property(
                            mProperties( tPropertyIndex ),
                            tPropertyNamesPair( iProp )( 1 ) );
                }

                // set local properties
                tCM->set_local_properties();

                // check the phase exist
                MORIS_ERROR( mPhaseMap.find( tPhaseName ) != mPhaseMap.end(),
                        "FEM_Model::create_constitutive_models - Unknown tPhaseName : %s \n",
                        tPhaseName.c_str() );

                // set CM to corresponding phase
                mPhaseInfo( mPhaseMap[ tPhaseName ] ).set_CM( tCM );
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_stabilization_parameters(
                std::map< std::string, uint >            & aSPMap,
                std::map< std::string, uint >            & aPropertyMap,
                moris::map< std::string, MSI::Dof_Type > & aMSIDofTypeMap,
                moris::map< std::string, PDV_Type >      & aDvTypeMap )
        {
            // create a stabilization parameter factory
            SP_Factory tSPFactory;

            // get the SP parameter list
            moris::Cell< ParameterList > tSPParameterList = mParameterList( 2 );

            // get the number of stabilization parameters
            uint tNumSPs = tSPParameterList.size();

            // set size for the list of stabilization parameter pointer
            mSPs.resize( tNumSPs, nullptr );

            // loop over the parameter list
            for ( uint iSP = 0; iSP < tNumSPs; iSP++ )
            {
                // get the stabilization parameters
                ParameterList tSPParameter = tSPParameterList( iSP );

                // get the stabilization parameter name
                std::string tSPName = tSPParameter.get< std::string >( "stabilization_name" );

                // get the stabilization type from parameter list
                fem::Stabilization_Type tSPType =
                        static_cast< fem::Stabilization_Type >( tSPParameter.get< uint >( "stabilization_type" ) );

                // create a stabilization parameter pointer
                mSPs( iSP ) = tSPFactory.create_SP( tSPType );

                // set name
                mSPs( iSP )->set_name( tSPName );

                // set SP space dimension
                mSPs( iSP )->set_space_dim( mSpaceDim );

                // fill stabilization map
                aSPMap[ tSPName ] = iSP;

                // set parameters
                moris::Cell< moris::Matrix< DDRMat > > tFuncParameters;
                string_to_cell_mat_2(
                        tSPParameter.get< std::string >( "function_parameters" ),
                        tFuncParameters );
                mSPs( iSP )->set_parameters( tFuncParameters );

                // init string for master or slave
                std::string tIsMasterString = "master";
                mtk::Master_Slave tIsMaster = mtk::Master_Slave::MASTER;

                // loop on master and slave
                for( uint iMaster = 0; iMaster <= mSPs( iSP )->get_has_slave(); iMaster++ )
                {
                    // if slave
                    if( iMaster )
                    {
                        // reset string for slave
                        tIsMasterString = "slave";
                        tIsMaster = mtk::Master_Slave::SLAVE;
                    }

                    // get the treated IWG phase
                    std::string tPhaseName =
                            tSPParameter.get< std::string >( tIsMasterString + "_phase_name" );

                    // check for unknown phase
                    MORIS_ERROR( mPhaseMap.find( tPhaseName ) != mPhaseMap.end(),
                            "FEM_Model::create_stabilization_parameters - Unknown phase name: %s \n",
                            tPhaseName.c_str() );

                    // set dof dependencies
                    moris::Cell< moris::Cell< moris::MSI::Dof_Type > > tDofTypes;
                    string_to_cell_of_cell(
                            std::get< 0 >( tSPParameter.get< std::pair< std::string, std::string > >( tIsMasterString + "_dof_dependencies" ) ),
                            tDofTypes,
                            aMSIDofTypeMap );
                    moris::Cell< std::string > tDofTypeNames;
                    string_to_cell( std::get< 1 >(
                            tSPParameter.get< std::pair< std::string, std::string > >( tIsMasterString + "_dof_dependencies" ) ),
                            tDofTypeNames );
                    mSPs( iSP )->set_dof_type_list( tDofTypes, tDofTypeNames, tIsMaster );

                    // set dv dependencies
                    moris::Cell< moris::Cell< PDV_Type > > tDvTypes;
                    string_to_cell_of_cell(
                            std::get< 0 >( tSPParameter.get< std::pair< std::string, std::string > >( tIsMasterString + "_dv_dependencies" ) ),
                            tDvTypes,
                            aDvTypeMap );
                    moris::Cell< std::string > tDvTypeNames;
                    string_to_cell(
                            std::get< 1 >( tSPParameter.get< std::pair< std::string, std::string > >( tIsMasterString + "_dv_dependencies" ) ),
                            tDvTypeNames );
                    mSPs( iSP )->set_dv_type_list( tDvTypes, tDvTypeNames, tIsMaster );

                    // set master properties
                    moris::Cell< moris::Cell< std::string > > tPropertyNamesPair;
                    string_to_cell_of_cell(
                            tSPParameter.get< std::string >( tIsMasterString + "_properties" ),
                            tPropertyNamesPair );

                    for( uint iProp = 0; iProp < tPropertyNamesPair.size(); iProp++ )
                    {
                        // get the property name
                        std::string tPropertyName = tPropertyNamesPair( iProp )( 0 );

                        // check for unknown property
                        MORIS_ERROR( aPropertyMap.find( tPropertyName ) != aPropertyMap.end(),
                                "FEM_Model::create_stabilization_parameters - Unknown %s aPropertyString : %s \n",
                                tIsMasterString.c_str(),
                                tPropertyName.c_str() );

                        // get property index
                        uint tPropertyIndex = aPropertyMap[ tPropertyName ];

                        // set property for CM
                        mSPs( iSP )->set_property(
                                mProperties( tPropertyIndex ),
                                tPropertyNamesPair( iProp )( 1 ),
                                tIsMaster );
                    }

                    // set constitutive models
                    moris::Cell< moris::Cell< std::string > > tCMNamesPair;
                    string_to_cell_of_cell(
                            tSPParameter.get< std::string >( tIsMasterString + "_constitutive_models" ),
                            tCMNamesPair );

                    // loop over CM names
                    for( uint iCM = 0; iCM < tCMNamesPair.size(); iCM++ )
                    {
                        // get the CM name
                        std::string tCMName = tCMNamesPair( iCM )( 0 );

                        // get CM from phase
                        std::shared_ptr< fem::Constitutive_Model > tCM =
                                mPhaseInfo( mPhaseMap[ tPhaseName ] ).get_CM_by_name( tCMName );

                        // set CM for SP
                        mSPs( iSP )->set_constitutive_model(
                                tCM,
                                tCMNamesPair( iCM )( 1 ) );
                    }

                    // get the cluster measures specifications
                    moris::Cell< moris::Cell< std::string > > tClusterMeasureTypes;
                    string_to_cell_of_cell(
                            std::get< 0 >( tSPParameter.get< std::pair< std::string, std::string > >( "cluster_measures" ) ),
                            tClusterMeasureTypes );

                    // get the cluster measures names
                    moris::Cell< std::string > tClusterMeasureNames;
                    string_to_cell( std::get< 1 >(tSPParameter.get< std::pair< std::string, std::string > >( "cluster_measures" ) ),
                            tClusterMeasureNames );

                    // build a cell of tuples describing the cluster measures specifications
                    moris::Cell< std::tuple<
                    fem::Measure_Type,
                    mtk::Primary_Void,
                    mtk::Master_Slave > > tClusterMeasureTuples( tClusterMeasureNames.size() );

                    // get fem::Measure_Type, mtk::Primary_Void and mtk::Master_Slave map
                    // to convert string to enums
                    moris::map< std::string, fem::Measure_Type > tFemMeasureMap = fem::get_measure_type_map();
                    moris::map< std::string, mtk::Primary_Void > tMtkPrimaryMap = mtk::get_primary_type_map();
                    moris::map< std::string, mtk::Master_Slave > tMtkMasterMap = mtk::get_master_type_map();

                    // loop over cluster measures names
                    for( uint iCMEA = 0; iCMEA < tClusterMeasureNames.size(); iCMEA++ )
                    {
                        // check that measure type is member of map
                        MORIS_ERROR( tFemMeasureMap.key_exists( tClusterMeasureTypes( iCMEA )( 0 ) ),
                                "FEM_Model::create_stabilization_parameters - key does not exist: %s",
                                tClusterMeasureTypes( iCMEA )( 0 ).c_str() );

                        // get fem measure type from map
                        fem::Measure_Type tFemMeasureType = tFemMeasureMap.find( tClusterMeasureTypes( iCMEA )( 0 ) );

                        // check that primary type is member of map
                        MORIS_ERROR( tMtkPrimaryMap.key_exists( tClusterMeasureTypes( iCMEA )( 1 ) ),
                                "FEM_Model::create_stabilization_parameters - key does not exist: %s",
                                tClusterMeasureTypes( iCMEA )( 1 ).c_str() );

                        // get mtk primary type from map
                        mtk::Primary_Void tMtkPrimaryType = tMtkPrimaryMap.find( tClusterMeasureTypes( iCMEA )( 1 ) );

                        // check that master type is member of map
                        MORIS_ERROR( tMtkMasterMap.key_exists( tClusterMeasureTypes( iCMEA )( 2 ) ),
                                "FEM_Model::create_stabilization_parameters - key does not exist: %s",
                                tClusterMeasureTypes( iCMEA )( 2 ).c_str() );

                        // get mtk master type from map
                        mtk::Master_Slave tMtkMasterType = tMtkMasterMap.find( tClusterMeasureTypes( iCMEA )( 2 ) );

                        // build the cluster measure specification tuple and set it in cell of tuples
                        tClusterMeasureTuples( iCMEA ) = std::make_tuple( tFemMeasureType, tMtkPrimaryType, tMtkMasterType );
                    }

                    // set the cell of cluster measure specification tuples to the SP
                    mSPs( iSP )->set_cluster_measure_type_list(
                            tClusterMeasureTuples,
                            tClusterMeasureNames );
                }
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_IWGs(
                std::map< std::string, uint >              & aPropertyMap,
                std::map< std::string, uint >              & aSPMap,
                moris::map< std::string, MSI::Dof_Type >   & aMSIDofTypeMap )
        {
            // create an IWG factory
            IWG_Factory tIWGFactory;

            // get the IWG parameter list
            moris::Cell< ParameterList > tIWGParameterList = mParameterList( 3 );

            // get number of IWGs
            uint tNumIWGs = tIWGParameterList.size();

            // create a list of IWG pointers
            mIWGs.resize( tNumIWGs, nullptr );

            // loop over the parameter lists
            for( uint iIWG = 0; iIWG < tNumIWGs; iIWG++ )
            {
                // get the treated IWG parameter list
                ParameterList tIWGParameter = tIWGParameterList( iIWG );

                // get the treated IWG name
                std::string tIWGName = tIWGParameter.get< std::string >( "IWG_name" );

                // get the treated IWG type
                fem::IWG_Type tIWGType =
                        static_cast< fem::IWG_Type >( tIWGParameter.get< uint >( "IWG_type" ) );

                // get the ghost order from parameter list
                uint tGhostOrder = tIWGParameter.get< uint >( "ghost_order" );

                // get the treated IWG residual dof type
                moris::Cell< moris::MSI::Dof_Type > tResDofTypes;
                string_to_cell(
                        tIWGParameter.get< std::string >( "dof_residual" ),
                        tResDofTypes,
                        aMSIDofTypeMap );

                // get the treated IWG bulk type
                fem::Element_Type tIWGBulkType =
                        static_cast< fem::Element_Type >( tIWGParameter.get< uint >( "IWG_bulk_type" ) );

                //
                bool tMasterSlave = ( tIWGBulkType == fem::Element_Type::DOUBLE_SIDESET );

                // create an IWG pointer
                mIWGs( iIWG ) = tIWGFactory.create_IWG( tIWGType );

                // set name
                mIWGs( iIWG )->set_name( tIWGName );

                // set interpolation order
                mIWGs( iIWG )->set_interpolation_order( tGhostOrder );

                // set residual dof type
                mIWGs( iIWG )->set_residual_dof_type( tResDofTypes );

                // set bulk type
                mIWGs( iIWG )->set_bulk_type( tIWGBulkType );

                // init string for master or slave
                std::string tIsMasterString = "master";
                mtk::Master_Slave tIsMaster = mtk::Master_Slave::MASTER;

                // loop on master and slave
                for( uint iMaster = 0; iMaster <= tMasterSlave; iMaster++ )
                {
                    // if slave
                    if( iMaster )
                    {
                        // reset string for slave
                        tIsMasterString = "slave";
                        tIsMaster = mtk::Master_Slave::SLAVE;
                    }

                    // get the treated IWG phase
                    std::string tPhaseName =
                            tIWGParameter.get< std::string >( tIsMasterString + "_phase_name" );

                    // check for unknown phase
                    MORIS_ERROR( mPhaseMap.find( tPhaseName ) != mPhaseMap.end(),
                            "FEM_Model::create_IWGs - Unknown phase name: %s \n",
                            tPhaseName.c_str() );

                    // set phase name
                    mIWGs( iIWG )->set_phase_name( tPhaseName, tIsMaster );

                    // get phase index
                    uint tPhaseIndex = mPhaseMap[ tPhaseName ];

                    // get dof type list from phase
                    mPhaseInfo( tPhaseIndex ).add_dof_type_to_list( tResDofTypes );

                    // set properties
                    moris::Cell< moris::Cell< std::string > > tPropertyNamesPair;
                    string_to_cell_of_cell(
                            tIWGParameter.get< std::string >( tIsMasterString + "_properties" ),
                            tPropertyNamesPair );

                    for( uint iProp = 0; iProp < tPropertyNamesPair.size(); iProp++ )
                    {
                        // get property name
                        std::string tPropertyName = tPropertyNamesPair( iProp )( 0 );

                        // check for unknown property
                        MORIS_ERROR( aPropertyMap.find( tPropertyName ) != aPropertyMap.end() ,
                                "FEM_Model::create_IWGs - Unknown %s aPropertyString: %s \n",
                                tIsMasterString.c_str(),
                                tPropertyName.c_str() );

                        // get property index
                        uint tPropertyIndex = aPropertyMap[ tPropertyName ];

                        // set property for IWG
                        mIWGs( iIWG )->set_property(
                                mProperties( tPropertyIndex ),
                                tPropertyNamesPair( iProp )( 1 ),
                                tIsMaster );
                    }

                    // set constitutive models
                    moris::Cell< moris::Cell< std::string > > tCMNamesPair;
                    string_to_cell_of_cell(
                            tIWGParameter.get< std::string >( tIsMasterString + "_constitutive_models" ),
                            tCMNamesPair );

                    // loop over constitutive models
                    for( uint iCM = 0; iCM < tCMNamesPair.size(); iCM++ )
                    {
                        // get the CM name
                        std::string tCMName = tCMNamesPair( iCM )( 0 );

                        // get CM from phase
                        std::shared_ptr< fem::Constitutive_Model > tCM =
                                mPhaseInfo( tPhaseIndex ).get_CM_by_name( tCMName );

                        // set CM for IWG
                        mIWGs( iIWG )->set_constitutive_model(
                                tCM,
                                tCMNamesPair( iCM )( 1 ),
                                tIsMaster );
                    }
                }

                // set stabilization parameters
                moris::Cell< moris::Cell< std::string > > tSPNamesPair;
                string_to_cell_of_cell(
                        tIWGParameter.get< std::string >( "stabilization_parameters" ),
                        tSPNamesPair );

                // loop over SP names
                for( uint iSP = 0; iSP < tSPNamesPair.size(); iSP++ )
                {
                    // get the SP name
                    std::string tSPName = tSPNamesPair( iSP )( 0 );

                    // check for unknown SP
                    MORIS_ERROR( aSPMap.find( tSPName ) != aSPMap.end(),
                            "FEM_Model::create_IWGs - Unknown aSPString: %s \n",
                            tSPName.c_str() );

                    // get SP index
                    uint tSPIndex = aSPMap[ tSPNamesPair( iSP )( 0 ) ];

                    // set SP for IWG
                    mIWGs( iIWG )->set_stabilization_parameter(
                            mSPs( tSPIndex ),
                            tSPNamesPair( iSP )( 1 ) );
                }
            }

            // loop over the parameter lists to set dof dependencies
            for( uint iIWG = 0; iIWG < tNumIWGs; iIWG++ )
            {
                // get the treated IWG parameter list
                ParameterList tIWGParameter = tIWGParameterList( iIWG );

                // get the IWG bulk type
                fem::Element_Type tIWGBulkType = mIWGs( iIWG )->get_bulk_type();

                // get the IWG master phase name
                std::string tPhaseName =
                        mIWGs( iIWG )->get_phase_name( mtk::Master_Slave::MASTER );

                // check for unknown phase
                MORIS_ERROR( mPhaseMap.find( tPhaseName ) != mPhaseMap.end(),
                        "FEM_Model::create_IWGs - Unknown phase name: %s \n",
                        tPhaseName.c_str() );

                // get the phase index
                uint tPhaseIndex = mPhaseMap[ tPhaseName ];

                // get dof type list from master phase
                const moris::Cell< moris::Cell< MSI::Dof_Type > > & tMasterDofTypes =
                        mPhaseInfo( tPhaseIndex ).get_dof_type_list();

                // get dof type list from master phase
                const moris::Cell< moris::Cell< PDV_Type > > & tMasterPdvTypes =
                        mPhaseInfo( tPhaseIndex ).get_dv_type_list();

                // set master dof dependencies
                mIWGs( iIWG )->set_dof_type_list( tMasterDofTypes, mtk::Master_Slave::MASTER );

                // set master dv dependencies
                mIWGs( iIWG )->set_dv_type_list( tMasterPdvTypes, mtk::Master_Slave::MASTER );

                if( tIWGBulkType == fem::Element_Type::DOUBLE_SIDESET )
                {
                    // get the IWG slave phase name
                    std::string tSlavePhaseName =
                            mIWGs( iIWG )->get_phase_name( mtk::Master_Slave::SLAVE );

                    // check for unknown phase
                    MORIS_ERROR( mPhaseMap.find( tSlavePhaseName ) != mPhaseMap.end() ,
                            "FEM_Model::create_IWGs - Unknown phase name: %s \n",
                            tSlavePhaseName.c_str() );

                    // get CM index
                    uint tSlavePhaseIndex = mPhaseMap[ tSlavePhaseName ];

                    // get dof type list from phase
                    const moris::Cell< moris::Cell< MSI::Dof_Type > > & tSlaveDofTypes =
                            mPhaseInfo( tSlavePhaseIndex ).get_dof_type_list();

                    // get pdv type list from phase
                    const moris::Cell< moris::Cell< PDV_Type > > & tSlavePdvTypes =
                            mPhaseInfo( tSlavePhaseIndex ).get_dv_type_list();

                    // set slave dof dependencies
                    mIWGs( iIWG )->set_dof_type_list( tSlaveDofTypes, mtk::Master_Slave::SLAVE );

                    // set slave dv dependencies
                    mIWGs( iIWG )->set_dv_type_list( tSlavePdvTypes, mtk::Master_Slave::SLAVE );
                }
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_IQIs(
                std::map< std::string, uint >            & aPropertyMap,
                std::map< std::string, uint >            & aSPMap,
                moris::map< std::string, MSI::Dof_Type > & aMSIDofTypeMap )
        {
            // create an IQI factory
            IQI_Factory tIQIFactory;

            // get the IQI parameter list
            moris::Cell< ParameterList > tIQIParameterList = mParameterList( 4 );

            // get number of IQIs
            uint tNumIQIs = tIQIParameterList.size();

            // set size for list of IQI pointers
            mIQIs.resize( tNumIQIs, nullptr );

            // loop over the parameter lists
            for( uint iIQI = 0; iIQI < tNumIQIs; iIQI++ )
            {
                // get the treated IQI parameter list
                ParameterList tIQIParameter = tIQIParameterList( iIQI );

                // get the treated IQI name from parameter list
                std::string tIQIName =
                        tIQIParameter.get< std::string >( "IQI_name" );

                // get the IQI type from parameter list
                fem::IQI_Type tIQIType =
                        static_cast< fem::IQI_Type >( tIQIParameter.get< uint >( "IQI_type" ) );

                // get the quantity dof type from parameter list
                moris::Cell< moris::MSI::Dof_Type > tQuantityDofTypes;
                string_to_cell(
                        tIQIParameter.get< std::string >( "dof_quantity" ),
                        tQuantityDofTypes,
                        aMSIDofTypeMap );

                // get the field index from parameter list
                sint tIQIFieldIndex =
                        tIQIParameter.get< moris::sint >( "vectorial_field_index" );

                // set function parameters
                moris::Cell< moris::Matrix< DDRMat > > tFuncParameters;
                string_to_cell_mat_2(
                        tIQIParameter.get< std::string >( "function_parameters" ),
                        tFuncParameters );

                // get the treated IQI bulk type
                fem::Element_Type tIQIBulkType =
                        static_cast< fem::Element_Type >( tIQIParameter.get< uint >( "IQI_bulk_type" ) );

                // set bool to true if double sideset
                bool tMasterSlave = ( tIQIBulkType == fem::Element_Type::DOUBLE_SIDESET );

                // create an IQI pointer
                mIQIs( iIQI ) = tIQIFactory.create_IQI( tIQIType );

                // set name
                mIQIs( iIQI )->set_name( tIQIName );

                // set quantity dof type
                mIQIs( iIQI )->set_quantity_dof_type( tQuantityDofTypes );

                // set index for vectorial field
                mIQIs( iIQI )->set_output_type_index( tIQIFieldIndex );

                // set bulk type
                mIQIs( iIQI )->set_bulk_type( tIQIBulkType );

                // set constant parameters
                mIQIs( iIQI )->set_parameters( tFuncParameters );

                // init string for master or slave
                std::string tIsMasterString = "master";
                mtk::Master_Slave tIsMaster = mtk::Master_Slave::MASTER;

                // loop on master and slave
                for( uint iMaster = 0; iMaster <= tMasterSlave; iMaster++ )
                {
                    // if slave
                    if( iMaster )
                    {
                        // reset string for slave
                        tIsMasterString = "slave";
                        tIsMaster = mtk::Master_Slave::SLAVE;
                    }

                    // get the treated IWG phase
                    std::string tPhaseName = tIQIParameter.get< std::string >( tIsMasterString + "_phase_name" );

                    // check for unknown phase
                    MORIS_ERROR( mPhaseMap.find( tPhaseName ) != mPhaseMap.end() ,
                            "FEM_Model::create_IQIs - Unknown phase name: %s \n",
                            tPhaseName.c_str() );

                    // set phase name
                    mIQIs( iIQI )->set_phase_name( tPhaseName, tIsMaster );

                    // get the phase index
                    uint tPhaseIndex = mPhaseMap[ tPhaseName ];

                    // get dof type list from phase
                    const moris::Cell< moris::Cell< MSI::Dof_Type > > & tDofTypes =
                            mPhaseInfo( tPhaseIndex ).get_dof_type_list();

                    // get dof type list from phase
                    const moris::Cell< moris::Cell< PDV_Type > > & tDvTypes =
                            mPhaseInfo( tPhaseIndex ).get_dv_type_list();

                    // set master dof dependencies
                    mIQIs( iIQI )->set_dof_type_list( tDofTypes );

                    // set master dv dependencies
                    mIQIs( iIQI )->set_dv_type_list( tDvTypes );

                    // set master properties
                    moris::Cell< moris::Cell< std::string > > tPropertyNamesPair;
                    string_to_cell_of_cell(
                            tIQIParameter.get< std::string >( tIsMasterString + "_properties" ),
                            tPropertyNamesPair );

                    for( uint iProp = 0; iProp < tPropertyNamesPair.size(); iProp++ )
                    {
                        // get the property name
                        std::string tPropertyName = tPropertyNamesPair( iProp )( 0 );

                        // check for unknown property
                        MORIS_ERROR( aPropertyMap.find( tPropertyName ) != aPropertyMap.end() ,
                                "FEM_Model::create_IQIs - Unknown %s aPropertyString: %s \n",
                                tIsMasterString.c_str(),
                                tPropertyName.c_str() );

                        // get property index
                        uint tPropertyIndex = aPropertyMap[ tPropertyName ];

                        // set property for IWG
                        mIQIs( iIQI )->set_property(
                                mProperties( tPropertyIndex ),
                                tPropertyNamesPair( iProp )( 1 ),
                                tIsMaster );
                    }

                    // set master constitutive models
                    moris::Cell< moris::Cell< std::string > > tCMNamesPair;
                    string_to_cell_of_cell(
                            tIQIParameter.get< std::string >( tIsMasterString + "_constitutive_models" ),
                            tCMNamesPair );

                    for( uint iCM = 0; iCM < tCMNamesPair.size(); iCM++ )
                    {
                        // get CM name
                        std::string tCMName = tCMNamesPair( iCM )( 0 );

                        // get CM from phase
                        std::shared_ptr< fem::Constitutive_Model > tCM =
                                mPhaseInfo( tPhaseIndex ).get_CM_by_name( tCMName );

                        // set CM for IQI
                        mIQIs( iIQI )->set_constitutive_model(
                                tCM,
                                tCMNamesPair( iCM )( 1 ),
                                tIsMaster );
                    }
                }

                // set stabilization parameters
                moris::Cell< moris::Cell< std::string > > tSPNamesPair;
                string_to_cell_of_cell(
                        tIQIParameter.get< std::string >( "stabilization_parameters" ),
                        tSPNamesPair );

                for( uint iSP = 0; iSP < tSPNamesPair.size(); iSP++ )
                {
                    // get the SP name
                    std::string tSPName = tSPNamesPair( iSP )( 0 );

                    // check for unknown SP
                    MORIS_ERROR( aSPMap.find( tSPName ) != aSPMap.end(),
                            "FEM_Model::create_IQIs - Unknown aSPString: %s \n",
                            tSPName.c_str() );

                    // get SP index
                    uint tSPIndex = aSPMap[ tSPName ];

                    // set SP for IWG
                    mIQIs( iIQI )->set_stabilization_parameter(
                            mSPs( tSPIndex ),
                            tSPNamesPair( iSP )( 1 ) );
                }
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_phases()
        {
            // get the phase parameter list
            moris::Cell< ParameterList > tPhaseParameterList = mParameterList( 7 );

            // get number of phases
            uint tNumPhases = tPhaseParameterList.size();

            // resize the list of phase user info
            mPhaseInfo.resize( tNumPhases );

            // loop over the parameter lists
            for( uint iPhase = 0; iPhase < tNumPhases; iPhase++ )
            {
                // get the treated phase parameter list
                ParameterList tPhaseParameter = tPhaseParameterList( iPhase );

                // get the phase name from parameter list
                std::string tPhaseName =
                        tPhaseParameter.get< std::string >( "phase_name" );

                // set phase name to phase
                mPhaseInfo( iPhase ).set_phase_name( tPhaseName );

                // get the phase index from parameter list
                moris::Matrix< moris::IndexMat > tPhaseIndices;
                string_to_mat( tPhaseParameter.get< std::string >( "phase_indices" ), tPhaseIndices );

                // set phase mesh indices to phase
                mPhaseInfo( iPhase ).set_phase_indices( tPhaseIndices );

                // fill phase map
                mPhaseMap[ tPhaseName ] = iPhase;
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_fem_set_info( bool aWithPhase )
        {
            // init number of fem sets to be created
            uint tNumFEMSets = 0;

            // get the IWG and IQI parameter lists
            moris::Cell< ParameterList > tIWGParameterList = mParameterList( 3 );
            moris::Cell< ParameterList > tIQIParameterList = mParameterList( 4 );

            // get fem computation type parameter list
            ParameterList tComputationParameterList = mParameterList( 5 )( 0 );

            // bool true for analytical forward analysis, false for finite difference
            // decide if dRdu and dQIdu are computed by A/FD
            bool tIsAnalyticalFA =
                    tComputationParameterList.get< bool >( "is_analytical_forward" );

            // get enum for FD scheme for forward analysis
            fem::FDScheme_Type tFDSchemeForFA = static_cast< fem::FDScheme_Type >(
                    tComputationParameterList.get< uint >( "finite_difference_scheme_forward" ) );

            // get perturbation size for FD for forward analysis
            real tFDPerturbationFA = tComputationParameterList.get< real >(
                    "finite_difference_perturbation_size_forward" );

            // get bool for analytical/finite difference for sensitivity analysis
            // decide if dRdp and dQIdp are computed by A/FD
            bool tIsAnalyticalSA =
                    tComputationParameterList.get< bool >( "is_analytical_sensitivity" );

            // get enum for FD scheme for sensitivity analysis
            fem::FDScheme_Type tFDSchemeForSA = static_cast< fem::FDScheme_Type >(
                    tComputationParameterList.get< uint >( "finite_difference_scheme" ) );

            // get perturbation size for FD for sensitivity analysis
            real tFDPerturbationSA = tComputationParameterList.get< real >(
                    "finite_difference_perturbation_size" );

            // get enum for perturbation strategy for finite difference
            fem::Perturbation_Type tPerturbationStrategy = static_cast< fem::Perturbation_Type >(
                    tComputationParameterList.get< uint >( "finite_difference_perturbation_strategy" ) );

            // create a map of the set
            std::map< std::tuple< std::string, bool, bool >, uint > tMeshtoFemSet;

            // loop over the IWGs
            for( uint iIWG = 0; iIWG < tIWGParameterList.size(); iIWG++ )
            {
                // get the treated IWG parameter list
                ParameterList tIWGParameter = tIWGParameterList( iIWG );

                // get the IWG bulk type
                fem::Element_Type tIWGBulkType = mIWGs( iIWG )->get_bulk_type();

                // get time continuity flag
                bool tTimeContinuity = mIWGs( iIWG )->get_time_continuity();

                // get time boundary flag
                bool tTimeBoundary = mIWGs( iIWG )->get_time_boundary();

                // get bool for ghost
                bool tIsGhost = mIWGs( iIWG )->get_ghost_flag();

                // get the IWG master phase name
                std::string tMasterPhaseName =
                        mIWGs( iIWG )->get_phase_name( mtk::Master_Slave::MASTER );

                // get the IWG slave phase name
                std::string tSlavePhaseName =
                        mIWGs( iIWG )->get_phase_name( mtk::Master_Slave::SLAVE );

                // get slave phase string from IWG input
                std::string tSlavePhaseString =
                        tIWGParameter.get< std::string >( "neighbor_phases" );

                // get ordinal string from IWG input
                std::string tOrdinalString =
                        tIWGParameter.get< std::string >( "side_ordinals" );

                // get mesh set names for IWG
                moris::Cell< std::string > tMeshSetNames;
                this->get_mesh_set_names(
                        tIWGBulkType,
                        tMasterPhaseName,
                        tSlavePhaseName,
                        tSlavePhaseString,
                        tOrdinalString,
                        tIsGhost,
                        tMeshSetNames );

                // loop over the mesh set names
                for( uint iSetName = 0; iSetName < tMeshSetNames.size(); iSetName++ )
                {
                    // check if the mesh set name already in map
                    if( tMeshtoFemSet.find( std::make_tuple(
                            tMeshSetNames( iSetName ),
                            tTimeContinuity,
                            tTimeBoundary ) ) == tMeshtoFemSet.end() )
                    {
                        // add the mesh set name map
                        tMeshtoFemSet[ std::make_tuple(
                                tMeshSetNames( iSetName ),
                                tTimeContinuity,
                                tTimeBoundary ) ] = tNumFEMSets++;

                        // create a fem set info for the mesh set
                        Set_User_Info aSetUserInfo;

                        // set its mesh set name
                        aSetUserInfo.set_mesh_set_name( tMeshSetNames( iSetName ) );

                        // set its time continuity flag
                        aSetUserInfo.set_time_continuity( tTimeContinuity );

                        // set its time boundary flag
                        aSetUserInfo.set_time_boundary( tTimeBoundary );

                        // set its forward analysis type flag
                        aSetUserInfo.set_is_analytical_forward_analysis( tIsAnalyticalFA );

                        // set its FD scheme for forward analysis
                        aSetUserInfo.set_finite_difference_scheme_for_forward_analysis( tFDSchemeForFA );

                        // set its FD perturbation size for forward analysis
                        aSetUserInfo.set_finite_difference_perturbation_size_for_forward_analysis( tFDPerturbationFA );

                        // set its sensitivity analysis type flag
                        aSetUserInfo.set_is_analytical_sensitivity_analysis( tIsAnalyticalSA );

                        // set its FD scheme for sensitivity analysis
                        aSetUserInfo.set_finite_difference_scheme_for_sensitivity_analysis( tFDSchemeForSA );

                        // set its FD perturbation size for sensitivity analysis
                        aSetUserInfo.set_finite_difference_perturbation_size( tFDPerturbationSA );

                        // set its perturbation strategy for finite difference
                        aSetUserInfo.set_perturbation_strategy( tPerturbationStrategy );

                        // set the IWG
                        aSetUserInfo.set_IWG( mIWGs( iIWG ) );

                        // add it to the list of fem set info
                        mSetInfo.push_back( aSetUserInfo );
                    }
                    else
                    {
                        // set the IWG
                        mSetInfo( tMeshtoFemSet[ std::make_tuple(
                                tMeshSetNames( iSetName ),
                                tTimeContinuity,
                                tTimeBoundary ) ] ).set_IWG( mIWGs( iIWG ) );
                    }
                }
            }

            // loop over the IQIs
            for( uint iIQI = 0; iIQI < tIQIParameterList.size(); iIQI++ )
            {
                // get the treated IWG parameter list
                ParameterList tIQIParameter = tIQIParameterList( iIQI );

                // get the IWG bulk type
                fem::Element_Type tIQIBulkType = mIQIs( iIQI )->get_bulk_type();

                // get time continuity flag
                bool tTimeContinuity = mIQIs( iIQI )->get_time_continuity();

                // get time boundary flag
                bool tTimeBoundary = mIQIs( iIQI )->get_time_boundary();

                // get the IWG master phase name
                std::string tMasterPhaseName =
                        mIQIs( iIQI )->get_phase_name( mtk::Master_Slave::MASTER );

                // get the IWG slave phase name
                std::string tSlavePhaseName =
                        mIQIs( iIQI )->get_phase_name( mtk::Master_Slave::SLAVE );

                // get slave phase string from IQI input
                std::string tSlavePhaseString =
                        tIQIParameter.get< std::string >( "neighbor_phases" );

                // get ordinal string from IQI input
                std::string tOrdinalString =
                        tIQIParameter.get< std::string >( "side_ordinals" );

                // get mesh set names for IWG
                moris::Cell< std::string > tMeshSetNames;
                this->get_mesh_set_names(
                        tIQIBulkType,
                        tMasterPhaseName,
                        tSlavePhaseName,
                        tSlavePhaseString,
                        tOrdinalString,
                        false,
                        tMeshSetNames );

                // loop over the mesh set names
                for( uint iSetName = 0; iSetName < tMeshSetNames.size(); iSetName++ )
                {
                    // if the mesh set name not in map
                    if( tMeshtoFemSet.find( std::make_tuple(
                            tMeshSetNames( iSetName ),
                            tTimeContinuity,
                            tTimeBoundary ) ) == tMeshtoFemSet.end() )
                    {
                        // add the mesh set name map
                        tMeshtoFemSet[ std::make_tuple(
                                tMeshSetNames( iSetName ),
                                tTimeContinuity,
                                tTimeBoundary ) ] = tNumFEMSets++;

                        // create a fem set info for the mesh set
                        Set_User_Info aSetUserInfo;

                        // set its mesh set name
                        aSetUserInfo.set_mesh_set_name( tMeshSetNames( iSetName ) );

                        // set its time continuity flag
                        aSetUserInfo.set_time_continuity( tTimeContinuity );

                        // set its time boundary flag
                        aSetUserInfo.set_time_boundary( tTimeBoundary );

                        // set its forward analysis type flag
                        aSetUserInfo.set_is_analytical_forward_analysis( tIsAnalyticalFA );

                        // set its FD scheme for forward analysis
                        aSetUserInfo.set_finite_difference_scheme_for_forward_analysis( tFDSchemeForFA );

                        // set its FD perturbation size for forward analysis
                        aSetUserInfo.set_finite_difference_perturbation_size_for_forward_analysis( tFDPerturbationFA );

                        // set its sensitivity analysis type flag
                        aSetUserInfo.set_is_analytical_sensitivity_analysis( tIsAnalyticalSA );

                        // set its FD scheme for sensitivity analysis
                        aSetUserInfo.set_finite_difference_scheme_for_sensitivity_analysis( tFDSchemeForSA );

                        // set its FD perturbation size for sensitivity analysis
                        aSetUserInfo.set_finite_difference_perturbation_size( tFDPerturbationSA );

                        // set its perturbation strategy for finite difference
                        aSetUserInfo.set_perturbation_strategy( tPerturbationStrategy );

                        // set the IQI
                        aSetUserInfo.set_IQI( mIQIs( iIQI ) );

                        // add it to the list of fem set info
                        mSetInfo.push_back( aSetUserInfo );
                    }
                    else
                    {
                        // set the IQI
                        mSetInfo( tMeshtoFemSet[ std::make_tuple(
                                tMeshSetNames( iSetName ),
                                tTimeContinuity,
                                tTimeBoundary ) ] ).set_IQI( mIQIs( iIQI ) );
                    }
                }
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::get_mesh_set_names(
                fem::Element_Type               aIWGBulkType,
                std::string                     aMasterPhaseName,
                std::string                     aSlavePhaseName,
                std::string                     aSlavePhaseString,
                std::string                     aOrdinalString,
                bool                            aIsGhost,
                moris::Cell< std::string >    & aMeshSetNames )
        {
            // get the master phase mesh index
            moris::Matrix< moris::IndexMat > tMasterPhaseIndices =
                    mPhaseInfo( mPhaseMap[ aMasterPhaseName ] ).get_phase_indices();

            // get the number of master phase mesh indices
            uint tNumMasterIndices = tMasterPhaseIndices.numel();

            // switch on the element type
            switch ( aIWGBulkType )
            {
                case fem::Element_Type::BULK :
                {
                    // loop over phase mesh indices
                    for( uint iMeshIndex = 0; iMeshIndex < tNumMasterIndices; iMeshIndex++ )
                    {
                        //                        // FIXME ! get mesh set names from integration mesh for index
                        //                        mMeshManager->get_integration_mesh( 0 )->
                        //                                get_block_set_names_with_color( tMasterPhaseIndices( iMeshIndex ), aMeshSetNames );

                        // add mesh set name to list
                        aMeshSetNames.push_back(
                                "HMR_dummy_c_p" +
                                std::to_string( tMasterPhaseIndices( iMeshIndex ) ) );

                        // add mesh set name to list
                        aMeshSetNames.push_back(
                                "HMR_dummy_n_p" +
                                std::to_string( tMasterPhaseIndices( iMeshIndex ) ) );
                    }
                    break;
                }
                case fem::Element_Type::SIDESET :
                {
                    // get neighbor phase names from string
                    moris::Cell< std::string > tSlavePhaseNames;
                    string_to_cell( aSlavePhaseString, tSlavePhaseNames );

                    // get number of neighbor phase
                    uint tNumSingle = tSlavePhaseNames.size();

                    // get ordinals for boundary from string
                    Matrix< DDSMat > tOrdinals;
                    string_to_mat( aOrdinalString, tOrdinals );
                    uint tNumBoundary = tOrdinals.numel();

                    // loop over master phase mesh indices
                    for( uint iMasterMeshIndex = 0; iMasterMeshIndex < tNumMasterIndices; iMasterMeshIndex++ )
                    {
                        // get single sideset
                        for( uint iSingle = 0; iSingle < tNumSingle; iSingle++ )
                        {
                            // get the neighbor phase name
                            std::string tNeighborPhaseName = tSlavePhaseNames( iSingle );

                            // get the slave phase mesh index
                            moris::Matrix< moris::IndexMat > tSlavePhaseIndices =
                                    mPhaseInfo( mPhaseMap[ tNeighborPhaseName ] ).get_phase_indices();

                            // get number of neighbor phase mesh indices
                            uint tNumNeighborIndices = tSlavePhaseIndices.numel();

                            for( uint iNeighborMeshIndex = 0; iNeighborMeshIndex < tNumNeighborIndices; iNeighborMeshIndex++ )
                            {
                                // FIXME get this info from the mesh
                                // add mesh set name to list
                                aMeshSetNames.push_back(
                                        "iside_b0_" +
                                        std::to_string( tMasterPhaseIndices( iMasterMeshIndex ) ) +
                                        "_b1_" +
                                        std::to_string( tSlavePhaseIndices( iNeighborMeshIndex ) ) );
                            }
                        }

                        // get boundary sideset
                        for( uint iBoundary = 0; iBoundary < tNumBoundary; iBoundary++ )
                        {
                            // FIXME get this info from the mesh
                            // add mesh set name to list
                            aMeshSetNames.push_back(
                                    "SideSet_" +
                                    std::to_string( tOrdinals( iBoundary ) ) +
                                    "_c_p" +
                                    std::to_string( tMasterPhaseIndices( iMasterMeshIndex ) ) );

                            // FIXME get this info from the mesh
                            // add mesh set name to list
                            aMeshSetNames.push_back(
                                    "SideSet_" +
                                    std::to_string( tOrdinals( iBoundary ) ) +
                                    "_n_p" +
                                    std::to_string( tMasterPhaseIndices( iMasterMeshIndex ) ) );
                        }
                    }
                    break;
                }
                case fem::Element_Type::DOUBLE_SIDESET :
                {
                    // if ghost
                    if ( aIsGhost )
                    {
                        // loop over master phase mesh indices
                        for( uint iMasterMeshIndex = 0; iMasterMeshIndex < tNumMasterIndices; iMasterMeshIndex++ )
                        {
                            // FIXME get this info from the mesh
                            // add mesh set name to list
                            aMeshSetNames.push_back(
                                    "ghost_p" +
                                    std::to_string( tMasterPhaseIndices( iMasterMeshIndex ) ) );
                        }
                    }
                    // if interface
                    else
                    {
                        MORIS_ERROR( aMasterPhaseName != aSlavePhaseName,
                                "FEM_Model::get_mesh_set_names - Master and slave phases are the same, FIXME case not handled yet ");

                        // get the slave phase mesh index
                        moris::Matrix< moris::IndexMat > tSlavePhaseIndices =
                                mPhaseInfo( mPhaseMap[ aSlavePhaseName ] ).get_phase_indices();

                        // get number of slave phase mesh index
                        uint tNumSlaveIndices = tSlavePhaseIndices.numel();

                        // loop over master phase mesh indices
                        for( uint iMasterMeshIndex = 0; iMasterMeshIndex < tNumMasterIndices; iMasterMeshIndex++ )
                        {
                            // get master index
                            uint tMasterPhaseIndex = tMasterPhaseIndices( iMasterMeshIndex );

                            // loop over slave phase mesh indices
                            for( uint iSlaveMeshIndex = 0; iSlaveMeshIndex < tNumSlaveIndices; iSlaveMeshIndex++ )
                            {
                                // get slave index
                                uint tSlavePhaseIndex = tSlavePhaseIndices( iSlaveMeshIndex );

                                // if master and slave index are different
                                if( tMasterPhaseIndex != tSlavePhaseIndex )
                                {
                                    // FIXME get this info from the mesh
                                    // get interface name
                                    aMeshSetNames.push_back(
                                            "dbl_iside_p0_" +
                                            std::to_string( tMasterPhaseIndices( iMasterMeshIndex ) ) +
                                            "_p1_" +
                                            std::to_string( tSlavePhaseIndices( iSlaveMeshIndex ) ) );
                                }
                            }
                        }
                    }
                    break;
                }
                default :
                {
                    MORIS_ERROR( false, "FEM_Model::get_mesh_set_names - Unknown set type" );
                }
            }
        }

        //-------------------------------------------------------------------------------------------------

        void FEM_Model::normalize_IQIs()
        {
            for (uint tRequestedIQIIndex = 0; tRequestedIQIIndex < mRequestedIQINames.size(); tRequestedIQIIndex++)
            {
                // IQI index
                uint tIQIIndex = 0;
                while (tIQIIndex < mIQIs.size()
                        and (mIQIs(tIQIIndex)->get_name() not_eq mRequestedIQINames(tRequestedIQIIndex)))
                {
                    tIQIIndex++;
                }
                MORIS_ASSERT(tIQIIndex < mIQIs.size(),
                        ("IQI was not found with the requested name " + mRequestedIQINames(tRequestedIQIIndex)).c_str());

                // Set normalization
                std::string tNormalization = mParameterList(4)(tIQIIndex).get< std::string >("normalization");
                if (tNormalization == "none")
                {
                    // Do nothing
                }
                else if (tNormalization == "time")
                {
                    MORIS_ERROR(false, "Time normalization not implemented yet for IQIs yet, implementation should go here.");
                }
                else if (tNormalization == "design")
                {
                    mIQIs(tRequestedIQIIndex)->set_reference_value(mGlobalIQIVal(tRequestedIQIIndex)(0));
                    mGlobalIQIVal(tRequestedIQIIndex)(0) = 1.0;
                }
                else
                {
                    // Try to set reference values directly
                    try
                    {
                        mIQIs(tRequestedIQIIndex)->set_reference_value(string_to_mat<DDRMat>(tNormalization)(0));
                    }
                    catch (...)
                    {
                        // create error message
                        std::string tErrMsg =
                                "FEM_Model::normalize_IQIs() - Unknown normalization: " + tNormalization +
                                ". Must be 'none', 'time', 'design', or a reference value.";

                        // error
                        MORIS_ERROR( false , tErrMsg.c_str() );
                    }
                }
            }
        }

        //-------------------------------------------------------------------------------------------------
        // FEM INPUT - old version
        //-------------------------------------------------------------------------------------------------

        void FEM_Model::create_constitutive_models(
                std::map< std::string, uint >            & aCMMap,
                std::map< std::string, uint >            & aPropertyMap,
                moris::map< std::string, MSI::Dof_Type > & aMSIDofTypeMap,
                moris::map< std::string, PDV_Type >      & aDvTypeMap )
        {
            // create a constitutive model factory
            CM_Factory tCMFactory;

            // get the CM parameter list
            moris::Cell< ParameterList > tCMParameterList = mParameterList( 1 );

            // get number of constitutive models
            uint tNumCMs = tCMParameterList.size();

            // create a list of CMs
            mCMs.resize( tNumCMs, nullptr );

            // loop over the parameter lists for CM
            for( uint iCM = 0; iCM < tNumCMs; iCM++ )
            {
                // get the constitutive type from parameter list
                fem::Constitutive_Type tCMType =
                        static_cast< fem::Constitutive_Type >( tCMParameterList( iCM ).get< uint >( "constitutive_type" ) );

                // create a constitutive model pointer
                mCMs( iCM ) = tCMFactory.create_CM( tCMType );

                // set CM name
                mCMs( iCM )->set_name( tCMParameterList( iCM ).get< std::string >( "constitutive_name" ) );

                // fill CM map
                aCMMap[ tCMParameterList( iCM ).get< std::string >( "constitutive_name" ) ] = iCM;

                // set CM model type
                fem::Model_Type tCMModelType =
                        static_cast< fem::Model_Type >( tCMParameterList( iCM ).get< uint >( "model_type" ) );
                if( tCMModelType != fem::Model_Type::UNDEFINED )
                {
                    mCMs( iCM )->set_model_type( tCMModelType );
                }

                // set CM space dimension
                mCMs( iCM )->set_space_dim( mSpaceDim );

                // set CM dof dependencies
                moris::Cell< moris::Cell< moris::MSI::Dof_Type > > tDofTypes;
                string_to_cell_of_cell(
                        std::get< 0 >( tCMParameterList( iCM ).get< std::pair< std::string, std::string > >( "dof_dependencies" ) ),
                        tDofTypes,
                        aMSIDofTypeMap );
                moris::Cell< std::string > tDofTypeNames;
                string_to_cell(
                        std::get< 1 >( tCMParameterList( iCM ).get< std::pair< std::string, std::string > >( "dof_dependencies" ) ),
                        tDofTypeNames );
                mCMs( iCM )->set_dof_type_list( tDofTypes, tDofTypeNames );

                // set CM dv dependencies
                moris::Cell< moris::Cell< PDV_Type > > tDvTypes;
                string_to_cell_of_cell(
                        std::get< 0 >( tCMParameterList( iCM ).get< std::pair< std::string, std::string > >( "dv_dependencies" ) ),
                        tDvTypes,
                        aDvTypeMap );
                moris::Cell< std::string > tDvTypeNames;
                string_to_cell(
                        std::get< 1 >( tCMParameterList( iCM ).get< std::pair< std::string, std::string > >( "dv_dependencies" ) ),
                        tDvTypeNames );
                mCMs( iCM )->set_dv_type_list( tDvTypes, tDvTypeNames );

                // set CM properties
                moris::Cell< moris::Cell< std::string > > tPropertyNamesPair;
                string_to_cell_of_cell(
                        tCMParameterList( iCM ).get< std::string >( "properties" ),
                        tPropertyNamesPair );
                for( uint iProp = 0; iProp < tPropertyNamesPair.size(); iProp++ )
                {
                    // if property name is in the property map
                    if ( aPropertyMap.find( tPropertyNamesPair( iProp )( 0 ) ) != aPropertyMap.end() )
                    {
                        // get property index
                        uint tPropertyIndex = aPropertyMap[ tPropertyNamesPair( iProp )( 0 ) ];

                        // set property for CM
                        mCMs( iCM )->set_property(
                                mProperties( tPropertyIndex ),
                                tPropertyNamesPair( iProp )( 1 ) );
                    }
                    else
                    {
                        // error message for unknown property
                        MORIS_ERROR( false,
                                "FEM_Model::create_CMs - Unknown aPropertyString : %s \n",
                                tPropertyNamesPair( iProp )( 0 ).c_str() );
                    }
                }
                // set local properties
                mCMs( iCM )->set_local_properties();

            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_stabilization_parameters(
                std::map< std::string, uint >            & aSPMap,
                std::map< std::string, uint >            & aPropertyMap,
                std::map< std::string, uint >            & aCMMap,
                moris::map< std::string, MSI::Dof_Type > & aMSIDofTypeMap,
                moris::map< std::string, PDV_Type >      & aDvTypeMap )
        {
            // create a stabilization parameter factory
            SP_Factory tSPFactory;

            // get the SP parameter list
            moris::Cell< ParameterList > tSPParameterList = mParameterList( 2 );

            // get the number of stabilization parameters
            uint tNumSPs = tSPParameterList.size();

            // set size for the list of stabilization parameter pointer
            mSPs.resize( tNumSPs, nullptr );

            // loop over the parameter list
            for ( uint iSP = 0; iSP < tNumSPs; iSP++ )
            {
                // get the SP parameter
                ParameterList tSPParameter = tSPParameterList( iSP );

                // get the stabilization type from parameter list
                fem::Stabilization_Type tSPType =
                        static_cast< fem::Stabilization_Type >( tSPParameter.get< uint >( "stabilization_type" ) );

                // create a stabilization parameter pointer
                mSPs( iSP ) = tSPFactory.create_SP( tSPType );

                // set name
                mSPs( iSP )->set_name( tSPParameter.get< std::string >( "stabilization_name" ) );

                // set SP space dimension
                mSPs( iSP )->set_space_dim( mSpaceDim );

                // fill stabilization map
                aSPMap[ tSPParameter.get< std::string >( "stabilization_name" ) ] = iSP;

                // set parameters
                moris::Cell< moris::Matrix< DDRMat > > tFuncParameters;
                string_to_cell_mat_2(
                        tSPParameter.get< std::string >( "function_parameters" ),
                        tFuncParameters );
                mSPs( iSP )->set_parameters( tFuncParameters );

                // init string for master or slave
                std::string tIsMasterString = "master";
                mtk::Master_Slave tIsMaster = mtk::Master_Slave::MASTER;

                // loop on master and slave
                for( uint iMaster = 0; iMaster <= mSPs( iSP )->get_has_slave(); iMaster++ )
                {
                    // if slave
                    if( iMaster )
                    {
                        // reset string for slave
                        tIsMasterString = "slave";
                        tIsMaster = mtk::Master_Slave::SLAVE;
                    }

                    // set dof dependencies
                    moris::Cell< moris::Cell< moris::MSI::Dof_Type > > tDofTypes;
                    string_to_cell_of_cell(
                            std::get< 0 >( tSPParameter.get< std::pair< std::string, std::string > >( tIsMasterString + "_dof_dependencies" ) ),
                            tDofTypes,
                            aMSIDofTypeMap );
                    moris::Cell< std::string > tDofTypeNames;
                    string_to_cell( std::get< 1 >(
                            tSPParameter.get< std::pair< std::string, std::string > >( tIsMasterString + "_dof_dependencies" ) ),
                            tDofTypeNames );
                    mSPs( iSP )->set_dof_type_list( tDofTypes, tDofTypeNames, tIsMaster );

                    // set dv dependencies
                    moris::Cell< moris::Cell< PDV_Type > > tDvTypes;
                    string_to_cell_of_cell(
                            std::get< 0 >( tSPParameter.get< std::pair< std::string, std::string > >( tIsMasterString + "_dv_dependencies" ) ),
                            tDvTypes,
                            aDvTypeMap );
                    moris::Cell< std::string > tDvTypeNames;
                    string_to_cell(
                            std::get< 1 >( tSPParameter.get< std::pair< std::string, std::string > >( tIsMasterString + "_dv_dependencies" ) ),
                            tDvTypeNames );
                    mSPs( iSP )->set_dv_type_list( tDvTypes, tDvTypeNames, tIsMaster );

                    // set master properties
                    moris::Cell< moris::Cell< std::string > > tPropertyNamesPair;
                    string_to_cell_of_cell(
                            tSPParameter.get< std::string >( tIsMasterString + "_properties" ),
                            tPropertyNamesPair );

                    for( uint iProp = 0; iProp < tPropertyNamesPair.size(); iProp++ )
                    {
                        // get the property name
                        std::string tPropertyName = tPropertyNamesPair( iProp )( 0 );

                        // check for unknown property
                        MORIS_ERROR( aPropertyMap.find( tPropertyName ) != aPropertyMap.end() ,
                                "FEM_Model::create_stabilization_parameters - Unknown master aPropertyString : %s \n",
                                tPropertyName.c_str() );

                        // get property index
                        uint tPropertyIndex = aPropertyMap[ tPropertyName ];

                        // set property for CM
                        mSPs( iSP )->set_property(
                                mProperties( tPropertyIndex ),
                                tPropertyNamesPair( iProp )( 1 ),
                                tIsMaster );
                    }

                    // set constitutive models
                    moris::Cell< moris::Cell< std::string > > tCMNamesPair;
                    string_to_cell_of_cell(
                            tSPParameter.get< std::string >( tIsMasterString + "_constitutive_models" ),
                            tCMNamesPair );

                    // loop over the CM names
                    for( uint iCM = 0; iCM < tCMNamesPair.size(); iCM++ )
                    {
                        // get the CM name
                        std::string tCMName = tCMNamesPair( iCM )( 0 );

                        // check for unknown CM
                        MORIS_ERROR( aCMMap.find( tCMName ) != aCMMap.end() ,
                                "FEM_Model::create_stabilization_parameters - Unknown master aCMString: %s \n",
                                tCMName.c_str() );

                        // get CM index
                        uint tCMIndex = aCMMap[ tCMName ];

                        // set CM for SP
                        mSPs( iSP )->set_constitutive_model(
                                mCMs( tCMIndex ),
                                tCMNamesPair( iCM )( 1 ) );
                    }
                }
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_IWGs(
                std::map< std::string, uint >              & aIWGMap,
                std::map< std::string, uint >              & aPropertyMap,
                std::map< std::string, uint >              & aCMMap,
                std::map< std::string, uint >              & aSPMap,
                std::map< std::string, uint >              & aFieldMap,
                moris::map< std::string, MSI::Dof_Type >   & aMSIDofTypeMap,
                moris::map< std::string, PDV_Type >        & aDvTypeMap,
                moris::map< std::string, mtk::Field_Type > & aFieldTypeMap)
        {
            // create an IWG factory
            IWG_Factory tIWGFactory;

            // get the IWG parameter list
            moris::Cell< ParameterList > tIWGParameterList = mParameterList( 3 );

            // get number of IWGs
            uint tNumIWGs = tIWGParameterList.size();

            // create a list of IWG pointers
            mIWGs.resize( tNumIWGs, nullptr );

            // loop over the parameter lists
            for( uint iIWG = 0; iIWG < tNumIWGs; iIWG++ )
            {
                // get the treated IWG parameter list
                ParameterList tIWGParameter = tIWGParameterList( iIWG );

                // get the IWG type from parameter list
                fem::IWG_Type tIWGType =
                        static_cast< fem::IWG_Type >( tIWGParameter.get< uint >( "IWG_type" ) );

                // create an IWG pointer
                mIWGs( iIWG ) = tIWGFactory.create_IWG( tIWGType );

                // get the IWG name
                std::string tIWGName = tIWGParameter.get< std::string >( "IWG_name" );

                // set name
                mIWGs( iIWG )->set_name( tIWGName );

                // fill IWG map
                aIWGMap[ tIWGName ] = iIWG;

                // get the ghost order from parameter list
                uint tGhostOrder = tIWGParameter.get< uint >( "ghost_order" );
                mIWGs( iIWG )->set_interpolation_order( tGhostOrder );

                // set residual dof type
                moris::Cell< moris::MSI::Dof_Type > tResDofTypes;
                string_to_cell(
                        tIWGParameter.get< std::string >( "dof_residual" ),
                        tResDofTypes,
                        aMSIDofTypeMap );
                mIWGs( iIWG )->set_residual_dof_type( tResDofTypes );

                // init string for master or slave
                std::string tIsMasterString = "master";
                mtk::Master_Slave tIsMaster = mtk::Master_Slave::MASTER;

                // loop on master and slave
                for( uint iMaster = 0; iMaster <= 1; iMaster++ )
                {
                    // if slave
                    if( iMaster )
                    {
                        // reset string for slave
                        tIsMasterString = "slave";
                        tIsMaster = mtk::Master_Slave::SLAVE;
                    }

                    // set dof dependencies
                    moris::Cell< moris::Cell< moris::MSI::Dof_Type > > tDofTypes;
                    string_to_cell_of_cell(
                            tIWGParameter.get< std::string >( tIsMasterString + "_dof_dependencies" ),
                            tDofTypes,
                            aMSIDofTypeMap );
                    mIWGs( iIWG )->set_dof_type_list( tDofTypes, tIsMaster );

                    // set dv dependencies
                    moris::Cell< moris::Cell< PDV_Type > > tDvTypes;
                    string_to_cell_of_cell(
                            tIWGParameter.get< std::string >( tIsMasterString + "_dv_dependencies" ),
                            tDvTypes,
                            aDvTypeMap );
                    mIWGs( iIWG )->set_dv_type_list( tDvTypes, tIsMaster );

                    // set field types
                    moris::Cell< moris::Cell< moris::mtk::Field_Type > > tFieldTypes;
                    string_to_cell_of_cell(
                            tIWGParameter.get< std::string >( tIsMasterString + "_field_types" ),
                            tFieldTypes,
                            aFieldTypeMap );
                    mIWGs( iIWG )->set_field_type_list( tFieldTypes, tIsMaster );

                    // set properties
                    moris::Cell< moris::Cell< std::string > > tPropertyNamesPair;
                    string_to_cell_of_cell(
                            tIWGParameter.get< std::string >( tIsMasterString + "_properties" ),
                            tPropertyNamesPair );

                    for( uint iProp = 0; iProp < tPropertyNamesPair.size(); iProp++ )
                    {
                        // if property name is in the property map
                        if ( aPropertyMap.find( tPropertyNamesPair( iProp )( 0 ) ) != aPropertyMap.end() )
                        {
                            // get property index
                            uint tPropertyIndex = aPropertyMap[ tPropertyNamesPair( iProp )( 0 ) ];

                            // set property for IWG
                            mIWGs( iIWG )->set_property(
                                    mProperties( tPropertyIndex ),
                                    tPropertyNamesPair( iProp )( 1 ),
                                    tIsMaster );
                        }
                        else
                        {
                            // create error message unknown property
                            MORIS_ERROR( false ,
                                    "FEM_Model::create_IWGs - Unknown %s aPropertyString: %s \n",
                                    tIsMasterString.c_str(),
                                    tPropertyNamesPair( iProp )( 0 ).c_str() );
                        }
                    }

                    // set constitutive models
                    moris::Cell< moris::Cell< std::string > > tCMNamesPair;
                    string_to_cell_of_cell(
                            tIWGParameter.get< std::string >( tIsMasterString + "_constitutive_models" ),
                            tCMNamesPair );

                    for( uint iCM = 0; iCM < tCMNamesPair.size(); iCM++ )
                    {
                        // if CM name is in the CM map
                        if ( aCMMap.find( tCMNamesPair( iCM )( 0 ) ) != aCMMap.end() )
                        {
                            // get CM index
                            uint tCMIndex = aCMMap[ tCMNamesPair( iCM )( 0 ) ];

                            // set CM for IWG
                            mIWGs( iIWG )->set_constitutive_model(
                                    mCMs( tCMIndex ),
                                    tCMNamesPair( iCM )( 1 ),
                                    tIsMaster );
                        }
                        else
                        {
                            // error message unknown CM
                            MORIS_ERROR( false ,
                                    "FEM_Model::create_IWGs - Unknown %s aCMString: %s \n",
                                    tIsMasterString.c_str(),
                                    tCMNamesPair( iCM )( 0 ).c_str() );
                        }
                    }
                }

                // set stabilization parameters
                moris::Cell< moris::Cell< std::string > > tSPNamesPair;
                string_to_cell_of_cell(
                        tIWGParameter.get< std::string >( "stabilization_parameters" ),
                        tSPNamesPair );

                for( uint iSP = 0; iSP < tSPNamesPair.size(); iSP++ )
                {
                    // if CM name is in the CM map
                    if ( aSPMap.find( tSPNamesPair( iSP )( 0 ) ) != aSPMap.end() )
                    {
                        // get SP index
                        uint tSPIndex = aSPMap[ tSPNamesPair( iSP )( 0 ) ];

                        // set SP for IWG
                        mIWGs( iIWG )->set_stabilization_parameter(
                                mSPs( tSPIndex ),
                                tSPNamesPair( iSP )( 1 ) );
                    }
                    else
                    {
                        // error message unknown SP
                        MORIS_ERROR( false ,
                                "FEM_Model::create_IWGs - Unknown aSPString: %s \n",
                                tSPNamesPair( iSP )( 0 ).c_str() );
                    }
                }

                //                // debug
                //                mIWGs( iIWG )->print_names();
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_IQIs(
                std::map< std::string, uint >              & aIQIMap,
                std::map< std::string, uint >              & aPropertyMap,
                std::map< std::string, uint >              & aCMMap,
                std::map< std::string, uint >              & aSPMap,
                std::map< std::string, uint >              & aFieldMap,
                moris::map< std::string, MSI::Dof_Type >   & aMSIDofTypeMap,
                moris::map< std::string, PDV_Type >        & aDvTypeMap,
                moris::map< std::string, mtk::Field_Type > & aFieldTypeMap)
        {
            // create an IQI factory
            IQI_Factory tIQIFactory;

            // get the IQI parameter list
            moris::Cell< ParameterList > tIQIParameterList = mParameterList( 4 );

            // get number of IQIs
            uint tNumIQIs = tIQIParameterList.size();

            // set size for list of IQI pointers
            mIQIs.resize( tNumIQIs, nullptr );

            // loop over the parameter lists
            for( uint iIQI = 0; iIQI < tNumIQIs; iIQI++ )
            {
                // get the treated IQI parameter list
                ParameterList tIQIParameter = tIQIParameterList( iIQI );

                // get name from parameter list
                std::string tIQIName = tIQIParameter.get< std::string >( "IQI_name" );

                // get the IQI type from parameter list
                fem::IQI_Type tIQIType =
                        static_cast< fem::IQI_Type >( tIQIParameter.get< uint >( "IQI_type" ) );

                // create an IQI pointer
                mIQIs( iIQI ) = tIQIFactory.create_IQI( tIQIType );

                // set name
                mIQIs( iIQI )->set_name( tIQIName );

                // fill IQI map
                aIQIMap[ tIQIName ] = iIQI;

                // get the treated IQI quantity dof type
                moris::Cell< moris::MSI::Dof_Type > tQuantityDofTypes;
                string_to_cell(
                        tIQIParameter.get< std::string >( "dof_quantity" ),
                        tQuantityDofTypes,
                        aMSIDofTypeMap );
                mIQIs( iIQI )->set_quantity_dof_type( tQuantityDofTypes );

                // set index for vectorial field
                mIQIs( iIQI )->set_output_type_index(
                        tIQIParameter.get< moris::sint >( "vectorial_field_index" ) );

                // set function parameters
                moris::Cell< moris::Matrix< DDRMat > > tFuncParameters;
                string_to_cell_mat_2(
                        tIQIParameter.get< std::string >( "function_parameters" ),
                        tFuncParameters );
                mIQIs( iIQI )->set_parameters( tFuncParameters );

                // init string for master or slave
                std::string tIsMasterString = "master";
                mtk::Master_Slave tIsMaster = mtk::Master_Slave::MASTER;

                // loop on master and slave
                for( uint iMaster = 0; iMaster <= 1; iMaster++ )
                {
                    // if slave
                    if( iMaster )
                    {
                        // reset string for slave
                        tIsMasterString = "slave";
                        tIsMaster = mtk::Master_Slave::SLAVE;
                    }

                    // set dof dependencies
                    moris::Cell< moris::Cell< moris::MSI::Dof_Type > > tDofTypes;
                    string_to_cell_of_cell(
                            tIQIParameter.get< std::string >( tIsMasterString + "_dof_dependencies" ),
                            tDofTypes,
                            aMSIDofTypeMap );
                    mIQIs( iIQI )->set_dof_type_list( tDofTypes, tIsMaster );

                    // set dv dependencies
                    moris::Cell< moris::Cell< PDV_Type > > tDvTypes;
                    string_to_cell_of_cell(
                            tIQIParameter.get< std::string >( tIsMasterString + "_dv_dependencies" ),
                            tDvTypes,
                            aDvTypeMap );
                    mIQIs( iIQI )->set_dv_type_list( tDvTypes, tIsMaster );

                    // set field types
                    moris::Cell< moris::Cell< moris::mtk::Field_Type > > tFieldTypes;
                    string_to_cell_of_cell(
                            tIQIParameter.get< std::string >( tIsMasterString + "_field_types" ),
                            tFieldTypes,
                            aFieldTypeMap );
                    mIQIs( iIQI )->set_field_type_list( tFieldTypes, tIsMaster );

                    // set properties
                    moris::Cell< moris::Cell< std::string > > tPropertyNamesPair;
                    string_to_cell_of_cell(
                            tIQIParameter.get< std::string >( tIsMasterString + "_properties" ),
                            tPropertyNamesPair );

                    for( uint iProp = 0; iProp < tPropertyNamesPair.size(); iProp++ )
                    {
                        // if property name is in the property map
                        if ( aPropertyMap.find( tPropertyNamesPair( iProp )( 0 ) ) != aPropertyMap.end() )
                        {
                            // get property index
                            uint tPropertyIndex = aPropertyMap[ tPropertyNamesPair( iProp )( 0 ) ];

                            // set property for IWG
                            mIQIs( iIQI )->set_property(
                                    mProperties( tPropertyIndex ),
                                    tPropertyNamesPair( iProp )( 1 ),
                                    tIsMaster );
                        }
                        else
                        {
                            // error message unknown property
                            MORIS_ERROR( false ,
                                    "FEM_Model::create_IQIs - Unknown %s aPropertyString: %s \n",
                                    tIsMasterString.c_str(),
                                    tPropertyNamesPair( iProp )( 0 ).c_str() );
                        }
                    }

                    // set constitutive models
                    moris::Cell< moris::Cell< std::string > > tCMNamesPair;
                    string_to_cell_of_cell(
                            tIQIParameter.get< std::string >( tIsMasterString + "_constitutive_models" ),
                            tCMNamesPair );

                    for( uint iCM = 0; iCM < tCMNamesPair.size(); iCM++ )
                    {
                        // if CM name is in the CM map
                        if ( aCMMap.find( tCMNamesPair( iCM )( 0 ) ) != aCMMap.end() )
                        {
                            // get CM index
                            uint tCMIndex = aCMMap[ tCMNamesPair( iCM )( 0 ) ];

                            // set CM for IQI
                            mIQIs( iIQI )->set_constitutive_model(
                                    mCMs( tCMIndex ),
                                    tCMNamesPair( iCM )( 1 ),
                                    tIsMaster );
                        }
                        else
                        {
                            // error message unknown CM
                            MORIS_ERROR( false ,
                                    "FEM_Model::create_IQIs - Unknown %s aCMString: %s \n",
                                    tIsMasterString.c_str(),
                                    tCMNamesPair( iCM )( 0 ).c_str() );
                        }
                    }
                }

                // set stabilization parameters
                moris::Cell< moris::Cell< std::string > > tSPNamesPair;
                string_to_cell_of_cell(
                        tIQIParameter.get< std::string >( "stabilization_parameters" ),
                        tSPNamesPair );

                for( uint iSP = 0; iSP < tSPNamesPair.size(); iSP++ )
                {
                    // if SP name is in the SP map
                    if ( aSPMap.find( tSPNamesPair( iSP )( 0 ) ) != aSPMap.end() )
                    {
                        // get SP index
                        uint tSPIndex = aSPMap[ tSPNamesPair( iSP )( 0 ) ];

                        // set SP for IQI
                        mIQIs( iIQI )->set_stabilization_parameter(
                                mSPs( tSPIndex ),
                                tSPNamesPair( iSP )( 1 ) );
                    }
                    else
                    {
                        // error message unknown SP
                        MORIS_ERROR( false ,
                                "FEM_Model::create_IQIs - Unknown aSPString: %s \n",
                                tSPNamesPair( iSP )( 0 ).c_str() );
                    }
                }

                //                // debug
                //                mIQIs( iIQI )->print_names();
            }
        }

        //------------------------------------------------------------------------------

        void FEM_Model::create_fem_set_info()
        {
            // init number of fem sets to be created
            uint tNumFEMSets = 0;

            // get the IWG and IQI parameter lists
            moris::Cell< ParameterList > tIWGParameterList = mParameterList( 3 );
            moris::Cell< ParameterList > tIQIParameterList = mParameterList( 4 );

            // get fem computation type parameter list
            ParameterList tComputationParameterList = mParameterList( 5 )( 0 );

            // get bool for printing physics model
            bool tPrintPhysics =
                    tComputationParameterList.get< bool >( "print_physics_model" );

            // get bool for analytical/finite differenec for SA
            bool tIsAnalyticalSA =
                    tComputationParameterList.get< bool >( "is_analytical_sensitivity" );

            // get enum for FD scheme
            fem::FDScheme_Type tFDSchemeForSA = static_cast< fem::FDScheme_Type >(
                    tComputationParameterList.get< uint >( "finite_difference_scheme" ) );

            // get perturbation size for FD
            real tFDPerturbation = tComputationParameterList.get< real >(
                    "finite_difference_perturbation_size" );

            // create a map of the set
            std::map< std::tuple< std::string, bool, bool >, uint > tMeshtoFemSet;

            // loop over the IWGs
            for( uint iIWG = 0; iIWG < tIWGParameterList.size(); iIWG++ )
            {
                // get the mesh set names from the IWG parameter list
                moris::Cell< std::string > tMeshSetNames;
                string_to_cell( tIWGParameterList( iIWG ).get< std::string >( "mesh_set_names" ), tMeshSetNames );

                // get the time continuity flag from the IWG parameter list
                bool tTimeContinuity = tIWGParameterList( iIWG ).get< bool >( "time_continuity" );

                // get the time boundary flag from the IQI parameter list
                bool tTimeBoundary = tIWGParameterList( iIWG ).get< bool >( "time_boundary" );

                // loop over the mesh set names
                for( uint iSetName = 0; iSetName < tMeshSetNames.size(); iSetName++ )
                {
                    // check if the mesh set name already in map
                    if( tMeshtoFemSet.find( std::make_tuple(
                            tMeshSetNames( iSetName ),
                            tTimeContinuity,
                            tTimeBoundary ) ) == tMeshtoFemSet.end() )
                    {
                        // add the mesh set name map
                        tMeshtoFemSet[ std::make_tuple(
                                tMeshSetNames( iSetName ),
                                tTimeContinuity,
                                tTimeBoundary ) ] = tNumFEMSets++;

                        // create a fem set info for the mesh set
                        Set_User_Info aSetUserInfo;

                        // set its mesh set name
                        aSetUserInfo.set_mesh_set_name( tMeshSetNames( iSetName ) );

                        // set its time continuity flag
                        aSetUserInfo.set_time_continuity( tTimeContinuity );

                        // set its time boundary flag
                        aSetUserInfo.set_time_boundary( tTimeBoundary );

                        // set its sensitivity analysis type flag
                        aSetUserInfo.set_is_analytical_sensitivity_analysis( tIsAnalyticalSA );

                        // set its FD scheme for sensitivity analysis
                        aSetUserInfo.set_finite_difference_scheme_for_sensitivity_analysis( tFDSchemeForSA );

                        // set its FD perturbation size for sensitivity analysis
                        aSetUserInfo.set_finite_difference_perturbation_size( tFDPerturbation );

                        // set the IWG
                        aSetUserInfo.set_IWG( mIWGs( iIWG ) );

                        // add it to the list of fem set info
                        mSetInfo.push_back( aSetUserInfo );
                    }
                    else
                    {
                        // set the IWG
                        mSetInfo( tMeshtoFemSet[ std::make_tuple(
                                tMeshSetNames( iSetName ),
                                tTimeContinuity,
                                tTimeBoundary ) ] ).set_IWG( mIWGs( iIWG ) );
                    }
                }
            }

            // loop over the IQIs
            for( uint iIQI = 0; iIQI < tIQIParameterList.size(); iIQI++ )
            {
                // get the mesh set names from the IQI parameter list
                moris::Cell< std::string > tMeshSetNames;
                string_to_cell( tIQIParameterList( iIQI ).get< std::string >( "mesh_set_names" ), tMeshSetNames );

                // get the time continuity flag from the IQI parameter list
                bool tTimeContinuity = tIQIParameterList( iIQI ).get< bool >( "time_continuity" );

                // get the time boundary flag from the IQI parameter list
                bool tTimeBoundary = tIQIParameterList( iIQI ).get< bool >( "time_boundary" );

                // loop over the mesh set names
                for( uint iSetName = 0; iSetName < tMeshSetNames.size(); iSetName++ )
                {
                    // if the mesh set name not in map
                    if( tMeshtoFemSet.find( std::make_tuple(
                            tMeshSetNames( iSetName ),
                            tTimeContinuity,
                            tTimeBoundary ) ) == tMeshtoFemSet.end() )
                    {
                        // add the mesh set name map
                        tMeshtoFemSet[ std::make_tuple(
                                tMeshSetNames( iSetName ),
                                tTimeContinuity,
                                tTimeBoundary ) ] = tNumFEMSets++;

                        // create a fem set info for the mesh set
                        Set_User_Info aSetUserInfo;

                        // set its mesh set name
                        aSetUserInfo.set_mesh_set_name( tMeshSetNames( iSetName ) );

                        // set its time continuity flag
                        aSetUserInfo.set_time_continuity( tTimeContinuity );

                        // set its time boundary flag
                        aSetUserInfo.set_time_boundary( tTimeBoundary );

                        // set its sensitivity analysis type flag
                        aSetUserInfo.set_is_analytical_sensitivity_analysis( tIsAnalyticalSA );

                        // set its FD scheme for sensitivity analysis
                        aSetUserInfo.set_finite_difference_scheme_for_sensitivity_analysis( tFDSchemeForSA );

                        // set its FD perturbation size for sensitivity analysis
                        aSetUserInfo.set_finite_difference_perturbation_size( tFDPerturbation );

                        // set the IQI
                        aSetUserInfo.set_IQI( mIQIs( iIQI ) );

                        // add it to the list of fem set info
                        mSetInfo.push_back( aSetUserInfo );
                    }
                    else
                    {
                        // set the IQI
                        mSetInfo( tMeshtoFemSet[ std::make_tuple(
                                tMeshSetNames( iSetName ),
                                tTimeContinuity,
                                tTimeBoundary ) ] ).set_IQI( mIQIs( iIQI ) );
                    }
                }
            }

            // debug print
            if( tPrintPhysics )
            {
                for( uint iSet = 0; iSet < mSetInfo.size(); iSet++ )
                {
                    std::cout<<"%-------------------------------------------------"<<std::endl;
                    mSetInfo( iSet ).print_names();
                    std::cout<<"%-------------------------------------------------"<<std::endl;
                }
            }
        }

        //-------------------------------------------------------------------------------------------------

        const std::shared_ptr< fem::Field > & FEM_Model::get_field( mtk::Field_Type tFieldType )
        {
            size_t tIndex = mFieldTypeMap( static_cast< sint >( tFieldType ) );
            return mFields( tIndex );
        }
        //-------------------------------------------------------------------------------------------------

        void FEM_Model::populate_fields()
        {
            Cell< std::shared_ptr< fem::Field > > tFieldToPopulate;
            Cell< std::string >  tFieldIQINames;
            tFieldToPopulate.reserve(mFields.size());
            tFieldIQINames  .reserve(mFields.size());

            for( uint Ik = 0; Ik < mFields.size(); Ik ++ )
            {
                if( mFields( Ik )->get_populate_field_with_IQI() )
                {
                    tFieldToPopulate.push_back( mFields( Ik ) );
                    tFieldIQINames  .push_back( mFields( Ik )->get_IQI_name() );
                }
            }

            for( uint Ik = 0; Ik < mFemSets.size(); Ik ++ )
            {
                if( mFemSets( Ik )->get_element_type() == Element_Type::BULK )
                {
                    mFemSets( Ik )->populate_fields( tFieldToPopulate, tFieldIQINames );
                }
            }

            // output fields to file. FIXME better should be done somewhere else.
            for( uint Ik = 0; Ik < mFields.size(); Ik ++ )
            {
                mFields( Ik )->output_field_to_file();

                //mFields( Ik )->save_field_to_exodus( "FEM_Field.Exo" );
            }
        }

        //-------------------------------------------------------------------------------------------------

    } /* namespace mdl */
} /* namespace moris */
