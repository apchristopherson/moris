
// added by christian: link to Google Perftools
#ifdef WITHGPERFTOOLS
#include <gperftools/profiler.h>
#endif

#include "cl_Stopwatch.hpp" //CHR/src

#include "MTK_Tools.hpp"
#include "cl_MTK_Enums.hpp"              //MTK/src
#include "cl_MTK_Mesh_Manager.hpp"       //MTK/src

#include "cl_FEM_Node_Base.hpp"          //FEM/INT/src
#include "cl_FEM_Node.hpp"               //FEM/INT/src
#include "cl_FEM_Enums.hpp"              //FEM/INT/src

#include "cl_MDL_Model.hpp"
#include "cl_FEM_IWG_Factory.hpp"
#include "cl_FEM_Element_Factory.hpp"
#include "cl_FEM_Set.hpp"
#include "cl_FEM_Property_User_Defined_Info.hpp"
#include "cl_FEM_IWG_User_Defined_Info.hpp"
#include "cl_FEM_Constitutive_User_Defined_Info.hpp"

#include "cl_DLA_Solver_Factory.hpp"
#include "cl_DLA_Solver_Interface.hpp"

#include "cl_NLA_Nonlinear_Solver_Factory.hpp"
#include "cl_NLA_Nonlinear_Solver.hpp"
#include "cl_NLA_Nonlinear_Problem.hpp"
#include "cl_MSI_Solver_Interface.hpp"
#include "cl_MSI_Equation_Object.hpp"
#include "cl_MSI_Model_Solver_Interface.hpp"
#include "cl_DLA_Linear_Solver_Aztec.hpp"
#include "cl_DLA_Linear_Solver.hpp"

#include "cl_TSA_Time_Solver_Factory.hpp"
#include "cl_TSA_Monolithic_Time_Solver.hpp"
#include "cl_TSA_Time_Solver.hpp"

#include "op_equal_equal.hpp"

// fixme: temporary
#include "cl_Map.hpp"
#include "fn_unique.hpp"
#include "fn_sum.hpp" // for check
#include "fn_iscol.hpp"
#include "fn_trans.hpp"

namespace moris
{
    namespace mdl
    {
//------------------------------------------------------------------------------

    Model::Model(       mtk::Mesh_Manager*                                                                 aMeshManager,
                  const uint                                                                               aBSplineIndex,
                  const moris::Cell< moris_index >                                                       & aSetList,
                  const moris::Cell< fem::Element_Type >                                                 & aSetTypeList,
                  const moris::Cell< moris::Cell< fem::IWG_User_Defined_Info > >                         & aIWGUserDefinedInfo,
                  const moris::Cell< moris::Cell< moris::Cell< fem::Property_User_Defined_Info > > >     & aPropertyUserDefinedInfo,
                  const moris::Cell< moris::Cell< moris::Cell< fem::Constitutive_User_Defined_Info > > > & aConstitutiveUserDefinedInfo,
                  const moris_index                                                                        aMeshPairIndex,
                  const bool                                                                               aUseMultigrid )
        : mMeshManager( aMeshManager ),
          mMeshPairIndex( aMeshPairIndex ),
          mUseMultigrid( aUseMultigrid )
        {
            // init the number of set
            uint tNumFemSets = aSetList.size();

            // number of groups of IWGs
            uint tNumIWGGroups = aIWGUserDefinedInfo.size();

            // number of groups of property types
            uint tNumPropGroups = aPropertyUserDefinedInfo.size();

            // number of groups of constitutive types
            uint tNumCMGroups = aConstitutiveUserDefinedInfo.size();

            // check input
            MORIS_ERROR( tNumIWGGroups == tNumFemSets, " Model::Model - wrong number of IWG groups. " );
            MORIS_ERROR( tNumPropGroups == tNumFemSets, " Model::Model - wrong number of property type groups. " );
            MORIS_ERROR( tNumCMGroups == tNumFemSets, " Model::Model - wrong number of constitutive type groups. " );

            // start timer
            tic tTimer1;


            mBSplineIndex = aBSplineIndex;

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 0: initialize
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Get pointers to interpolation and integration mesh
            moris::moris_index tMeshPairIndex = aMeshPairIndex;
            mtk::Interpolation_Mesh* tInterpolationMesh = nullptr;
            mtk::Integration_Mesh*   tIntegrationMesh   = nullptr;
            mMeshManager->get_mesh_pair( tMeshPairIndex, tInterpolationMesh, tIntegrationMesh );

//            MORIS_ERROR( !(mDofOrder == 0 && tInterpolationMesh->get_mesh_type() == MeshType::HMR), " HMR B-Spline order can't be 0");

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 1: create nodes
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            // ask mesh about number of nodes on proc
            luint tNumOfIPNodes = tInterpolationMesh->get_num_nodes();

            // create node objects
            mIPNodes.resize(  tNumOfIPNodes, nullptr );

            for( uint iNode = 0; iNode < tNumOfIPNodes; iNode++ )
            {
                mIPNodes( iNode ) = new fem::Node( &tInterpolationMesh->get_mtk_vertex( iNode ) );
            }

            if( par_rank() == 0)
            {
                // stop timer
                real tElapsedTime = tTimer1.toc<moris::chronos::milliseconds>().wall;

                // print output
                std::fprintf( stdout, "Model: created %u FEM IP nodes in %5.3f seconds.\n\n",
                        ( unsigned int ) tNumOfIPNodes,
                        ( double ) tElapsedTime / 1000 );
            }

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 1.5: create IWGs
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            // a factory to create the IWGs
            fem::IWG_Factory tIWGFactory;

            // create a cell of IWGs
            mIWGs.resize( tNumIWGGroups );

            // loop over the sets
            for( uint iSet = 0; iSet < tNumIWGGroups; iSet++ )
            {
                // number of IWG in set
                uint tNumIWG = aIWGUserDefinedInfo( iSet ).size();

                // set size for the cell of IWGs for the set
                mIWGs( iSet ).resize( tNumIWG, nullptr );

                // loop over the IWG types for the set
                for( uint iIWG = 0; iIWG < tNumIWG; iIWG++ )
                {
                    // create an IWG with the factory for the IWG type
                    mIWGs( iSet )( iIWG ) = tIWGFactory.create_IWGs( aIWGUserDefinedInfo( iSet )( iIWG ).get_IWG_type() );

                    // set residual dof type
                    mIWGs( iSet )( iIWG )->set_residual_dof_type( aIWGUserDefinedInfo( iSet )( iIWG ).get_residual_dof_type() );

                    // set active dof type
                    mIWGs( iSet )( iIWG )->set_dof_type_list( aIWGUserDefinedInfo( iSet )( iIWG ).get_dof_type_list() );

                    // set active property type
                    mIWGs( iSet )( iIWG )->set_property_type_list( aIWGUserDefinedInfo( iSet )( iIWG ).get_property_type_list() );

                    // set active constitutive type
                    mIWGs( iSet )( iIWG )->set_constitutive_type_list( aIWGUserDefinedInfo( iSet )( iIWG ).get_constitutive_type_list() );

                    if( aSetTypeList( iSet ) == fem::Element_Type::DOUBLE_SIDESET )
                    {
                        // set active dof type
                        mIWGs( iSet )( iIWG )->set_dof_type_list( aIWGUserDefinedInfo( iSet )( iIWG ).get_dof_type_list( mtk::Master_Slave::SLAVE ),
                                                                  mtk::Master_Slave::SLAVE );

                        // set active property type
                        mIWGs( iSet )( iIWG )->set_property_type_list( aIWGUserDefinedInfo( iSet )( iIWG ).get_property_type_list( mtk::Master_Slave::SLAVE ),
                                                                       mtk::Master_Slave::SLAVE );

                        // set active constitutive type
                        mIWGs( iSet )( iIWG )->set_constitutive_type_list( aIWGUserDefinedInfo( iSet )( iIWG ).get_constitutive_type_list( mtk::Master_Slave::SLAVE ),
                                                                           mtk::Master_Slave::SLAVE );
                    }
                }
            }

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 2: create elements
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            // start timer
            tic tTimer2;

            // a factory to create the elements
            fem::Element_Factory tElementFactory;

            // create equation objects
            mFemSets.resize( tNumFemSets, nullptr );

//            // get the number of element to create
//            luint tNumberOfEquationObjects = tIntegrationMesh->get_num_elems()
//                                           + tIntegrationMesh->get_sidesets_num_faces( aSidesetList );
//            mFemClusters.reserve( tNumberOfEquationObjects );
            mFemClusters.reserve( 100000 ); //FIXME

            //------------------------------------------------------------------------------
            // init the fem set counter
            moris::uint tFemSetCounter = 0;

            // loop over the used fem set
            for( luint iSet = 0; iSet < tNumFemSets; iSet++ )
            {
                // create a list of clusters
                moris::mtk::Set * tMeshSet;

                switch ( aSetTypeList( iSet ) )
                {
                    case( fem::Element_Type::BULK ):
                        tMeshSet = tIntegrationMesh->get_block_by_index( aSetList( iSet ) );
                        break;

                    case( fem::Element_Type::SIDESET ):
                        tMeshSet = tIntegrationMesh->get_side_set_by_index( aSetList( iSet ) );
                        break;

                    case( fem::Element_Type::DOUBLE_SIDESET ):
                        tMeshSet = tIntegrationMesh->get_double_side_set_by_index( aSetList( iSet ) );
                        break;

                    default:
                        MORIS_ERROR( false, " Model - unknown fem::element_Type " );
                }

                if ( tMeshSet->get_num_clusters_on_set() !=0 )
                {
                    // create new fem set
                    mFemSets( tFemSetCounter ) = new fem::Set( tMeshSet,
                                                               aSetTypeList( iSet ),
                                                               mIWGs( iSet ),
                                                               aPropertyUserDefinedInfo( iSet ),
                                                               aConstitutiveUserDefinedInfo( iSet ),
                                                               mIPNodes );
                }
                else
                {
                    mFemSets( tFemSetCounter ) = new fem::Set();
                }

                // collect equation objects associated with the block-set
                mFemClusters.append( mFemSets( tFemSetCounter )->get_equation_object_list() );

                // update fem set counter
                tFemSetCounter++;
            }
            mFemClusters.shrink_to_fit();

            if( par_rank() == 0)
            {
                // stop timer
                real tElapsedTime = tTimer2.toc<moris::chronos::milliseconds>().wall;

                // print output

                std::fprintf( stdout,"Model: created %u FEM elements in %5.3f seconds.\n\n",
                        ( unsigned int ) mFemClusters.size(),
                        ( double ) tElapsedTime / 1000 );
            }

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 3: create Model Solver Interface
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            // start timer
            tic tTimer3;

            //--------------------------FIXME------------------------------------
            // This part should not be needed anymore when MTK has all the functionalities
            Matrix< IdMat > tCommTable;
            moris::map< moris::moris_id, moris::moris_index > tIdToIndMap;
            moris::uint tMaxNumAdofs;

            if ( tInterpolationMesh->get_mesh_type() == MeshType::HMR )
            {
//                if ( mBSplineIndex == 0 )
//                {
//                    mBSplineIndex  = this->get_lagrange_order_from_mesh();   // FIXME this function is absolutely wrong
//                }

                // get map from mesh
                tInterpolationMesh->get_adof_map( mBSplineIndex, mCoefficientsMap );

                tCommTable   = tInterpolationMesh->get_communication_table();
                tIdToIndMap  = mCoefficientsMap;
                tMaxNumAdofs = tInterpolationMesh->get_num_coeffs( mBSplineIndex );
            }
            else
            {
                tCommTable.set_size( 1, 1, 0 );
                tMaxNumAdofs = 1000000;
            }
            //--------------------------END FIXME--------------------------------

            // Construct cell of equation sets (cast from child to base class of entire cell)
            moris::Cell< MSI::Equation_Set * > tEquationSets(mFemSets.size());
            for(moris::uint iSet = 0; iSet < mFemSets.size(); iSet++ )
            {
                tEquationSets( iSet ) = mFemSets( iSet );
            }


            mModelSolverInterface = new moris::MSI::Model_Solver_Interface( tEquationSets,
                                                                            tCommTable,
                                                                            tIdToIndMap,
                                                                            tMaxNumAdofs,
                                                                            tInterpolationMesh );

            if ( tInterpolationMesh->get_mesh_type() == MeshType::HMR )
            {
                mModelSolverInterface->set_param("L2")= (sint)mBSplineIndex;
                mModelSolverInterface->set_param("TEMP")= (sint)mBSplineIndex;
            }

            //------------------------------------------------------------------------------

            // finalize the fem sets
            for( luint Ik = 0; Ik < mFemSets.size(); ++Ik )
            {
                // finalize the fem sets
                mFemSets( Ik )->finalize( mModelSolverInterface );
            }

            mModelSolverInterface->finalize( mUseMultigrid );

            // calculate AdofMap
            mAdofMap = mModelSolverInterface->get_dof_manager()->get_adof_ind_map();

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // STEP 4: create Solver Interface
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            mSolverInterface =  new moris::MSI::MSI_Solver_Interface( mModelSolverInterface );

            if( par_rank() == 0)
            {
                // stop timer
                real tElapsedTime = tTimer2.toc<moris::chronos::milliseconds>().wall;

                // print output
                std::fprintf( stdout,"Model: created Model-Solver Interface in %5.3f seconds.\n\n",
                        ( double ) tElapsedTime / 1000 );
            }
        }

//------------------------------------------------------------------------------

        Model::~Model()
        {
            // delete SI
            delete mSolverInterface;

            // delete MSI
            delete mModelSolverInterface;

            // delete IWGs
            for( auto tIWGs : mIWGs )
            {
                for( auto tIWG : tIWGs )
                {
                    delete tIWG;
                }
                tIWGs.clear();
            }
            mIWGs.clear();

            // delete fem nodes
            for( auto tIPNodes : mIPNodes )
            {
                delete tIPNodes;
            }
            mIPNodes.clear();

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

        void Model::set_weak_bcs( const Matrix<DDRMat> & aWeakBCs )
        {
            // set weak BCs
            for( auto tElement : mFemClusters )
            {
                Matrix< DDRMat > & tNodalWeakBCs = tElement->get_weak_bcs();
                uint tNumberOfNodes = tElement->get_num_nodes();
                tNodalWeakBCs.set_size( tNumberOfNodes, 1 );

                for( uint k=0; k < tNumberOfNodes; ++k )
                {
                    // copy weakbc into element
                    tNodalWeakBCs( k ) = aWeakBCs( tElement->get_node_index( k ) );
                }
            }
        }

//------------------------------------------------------------------------------

        void Model::set_weak_bcs_from_nodal_field( moris_index aFieldIndex )
        {
            for( auto tElement : mFemClusters )
            {
                Matrix< DDRMat > & tNodalWeakBCs = tElement->get_weak_bcs();
                uint tNumberOfNodes = tElement->get_num_nodes();
                tNodalWeakBCs.set_size( tNumberOfNodes, 1 );

                for( uint k=0; k<tNumberOfNodes; ++k )
                {
                    // copy weakbc into element
                    tNodalWeakBCs( k ) = mMeshManager->get_interpolation_mesh( mMeshPairIndex )
                                                     ->get_value_of_scalar_field( aFieldIndex,
                                                                                  EntityRank::NODE,
                                                                                  tElement->get_node_index( k ) );
                }
            }
        }

//------------------------------------------------------------------------------

        real Model::compute_integration_error(
                real (*aFunction)( const Matrix< DDRMat > & aPoint ) )
        {
            real aError = 0.0;
            for( auto tElement : mFemClusters )
            {
                aError += tElement->compute_integration_error( aFunction );
            }
            return aError;
        }

//------------------------------------------------------------------------------

        uint Model::get_lagrange_order_from_mesh()
        {

            // set order of this model according to Lagrange order
            // of first element on mesh
           return mtk::interpolation_order_to_uint( mMeshManager->get_interpolation_mesh(0)
                                                                ->get_mtk_cell( 0 ).get_interpolation_order() );
        }

//------------------------------------------------------------------------------

        void Model::set_dof_order( const uint aBSplineIndex )
        {
            mBSplineIndex = aBSplineIndex;
//            MORIS_ASSERT( aOrder == mBSplineIndex,
//                    "Model: the functionality to change the order of the model has nor been implemented yet" );
        }

//------------------------------------------------------------------------------
        real Model::compute_element_average( const uint aElementIndex )
        {
            return mFemClusters( aElementIndex )->compute_element_average_of_scalar_field();
        }

//------------------------------------------------------------------------------

        void Model::output_solution( const std::string & aFilePath )
        {
            mSolHMR.set_size(mMeshManager->get_interpolation_mesh(0)->get_num_nodes(),1,0.0);

            mtk::Interpolation_Mesh* tInterpMesh = mMeshManager->get_interpolation_mesh(0);

            moris::Cell<std::string> tBlockSetsNames = tInterpMesh->get_set_names( EntityRank::ELEMENT);

            // iterate through fem blocks
            uint tNumFemSets = mFemSets.size();

            for(moris::uint iSet = 0; iSet<tNumFemSets; iSet++)
            {
                // access set
                fem::Set * tFemSet = mFemSets(iSet);

                // access the element type in the set
                enum fem::Element_Type tElementTypeInSet = tFemSet->get_element_type();

                // if we are a bulk element, then we output the solution
                if(tElementTypeInSet == fem::Element_Type::BULK)
                {
                    // get mtk cell clusters in set
                    moris::Cell< mtk::Cluster const* > tCellClustersInSet = tFemSet->get_clusters_on_set();

                    // get coressponding equation objects
                    moris::Cell< MSI::Equation_Object * > & tEquationObj = tFemSet->get_equation_object_list();

                    // iterate through clusters in set
                    for(moris::uint iCl = 0; iCl < tCellClustersInSet.size(); iCl++)
                    {
                        mtk::Cluster const * tCluster = tCellClustersInSet(iCl);

                        moris::mtk::Cell const & tInterpCell = tCluster->get_interpolation_cell();

                        uint tNumVert = tInterpCell.get_number_of_vertices();

                        Matrix< DDRMat > & tPDofVals = tEquationObj(iCl)->get_pdof_values();

                        moris::Cell<moris::mtk::Vertex *> tVertices = tInterpCell.get_vertex_pointers();

                        for( luint Jk=0; Jk < tNumVert; ++Jk )
                        {
                            mSolHMR(tVertices(Jk)->get_index()) = tPDofVals(Jk);
                        }
                    }
                }

            }
        }

        Matrix<DDRMat>
        Model::get_solution_for_integration_mesh_output( enum MSI::Dof_Type aDofType )
        {
            // number of vertices in integration mesh
            uint tNumVertsInIGMesh = mMeshManager->get_integration_mesh(mMeshPairIndex)->get_num_entities(EntityRank::NODE);

            //  initialize integration mesh solution
            Matrix<DDRMat> tSolutionOnInteg ( tNumVertsInIGMesh,1,0.0);

            // keep track of how many times I have added to a given node
            Matrix<DDRMat> tVertexCount ( tNumVertsInIGMesh,1,0.0);

//            if ( mMeshManager->get_interpolation_mesh(0)->get_mesh_type() == MeshType::HMR )
//            {

                // iterate through fem blocks
                uint tNumFemSets = mFemSets.size();

                for(moris::uint iSet = 0; iSet<tNumFemSets; iSet++)
                {
                    // access set
                    fem::Set * tFemSet = mFemSets(iSet);

                    // access the element type in the set
                    enum fem::Element_Type tElementTypeInSet = tFemSet->get_element_type();

                    // if we are a bulk element, then we output the solution
                    if(tElementTypeInSet == fem::Element_Type::BULK)
                    {
                        // access dof list
                        moris::Cell< enum MSI::Dof_Type > & tUniqueDofList = tFemSet->get_unique_dof_type_list();

                        // see if the dof of interest is in there
                        bool tDofInSet = dof_type_is_in_list(aDofType,tUniqueDofList);

                        // if the dof is in this set, we need to add solution information to output
                        if(tDofInSet)
                        {
                            // get mtk cell clusters in set
                            moris::Cell< mtk::Cluster const* > tCellClustersInSet = tFemSet->get_clusters_on_set();

                            // get coressponding equation objects
                            moris::Cell< MSI::Equation_Object * > & tEquationObj = tFemSet->get_equation_object_list();

                            enum MSI::Dof_Type tDofType = MSI::Dof_Type::UX;
                            if ( aDofType == MSI::Dof_Type::TEMP)
                            {
                                tDofType = MSI::Dof_Type::TEMP;
                            }

                            // get the field interpolator
                            fem::Field_Interpolator* tFieldInterp = tFemSet->get_dof_type_field_interpolators(tDofType);

                            // iterate through clusters in set
                            for(moris::uint iCl = 0; iCl < tCellClustersInSet.size(); iCl++)
                            {
                                // get the cluster
                                mtk::Cluster const * tCluster = tCellClustersInSet(iCl);

                                // get the pdof values for this cluster
                                Cell < Matrix< DDRMat > >  tPDofVals;
                                tEquationObj(iCl)->get_my_pdof_values( tFieldInterp->get_dof_type(), tPDofVals);

                                //---------------------------------------------------------------------------------
                                uint tCols = tPDofVals.size();
                                uint tRows = tPDofVals( 0 ).numel();

                                Matrix< DDRMat > tReshapedPdofValues( tRows, tCols );

                                for( uint Ik = 0; Ik < tCols; Ik++ )
                                {
                                    tReshapedPdofValues( { 0, tRows - 1 }, { Ik, Ik } ) = tPDofVals( Ik ).matrix_data();
                                }

                                moris_index tDofIndex = 0;

                                for( uint Ik = 0; Ik < tFieldInterp->get_dof_type().size(); Ik ++ )
                                {
                                    if( aDofType == tFieldInterp->get_dof_type()( Ik ) )
                                    {
                                        tDofIndex = Ik;
                                    }
                                }
                                //---------------------------------------------------------------------------------

                                // set coefficients in field interpolator
                                tFieldInterp->set_coeff(tReshapedPdofValues);



                                // check if its trivial
                                bool tTrivialCluster = tCluster->is_trivial();

                                // in the trivial case, the interp pdofs, correspond to the integration vertices in clusters
                                if(tTrivialCluster)
                                {
                                    // get integration cell
                                    moris::Cell<moris::mtk::Cell const *> const & tPrimaryCells = tCluster->get_primary_cells_in_cluster();

                                    MORIS_ASSERT(tPrimaryCells.size() , "There needs to be exactly 1 primary cell in cluster for the trivial case");

                                    // an assumption is made here that the vertices on the primary correspond to the index in pdof vector
                                    moris::Cell< moris::mtk::Vertex* > tVerticesOnPrimaryCell = tPrimaryCells(0)->get_vertex_pointers();

                                    // iterate through vertices and add their pdof value to integration mesh solution vector
                                    for(moris::uint iVert =0; iVert<tVerticesOnPrimaryCell.size(); iVert++)
                                    {
                                        // vertex index
                                         moris_index tVertIndex = tVerticesOnPrimaryCell(iVert)->get_index();

                                         // add to solution vector
                                         tSolutionOnInteg(tVertIndex) = tPDofVals( tDofIndex )(iVert);

                                         // update count that this vertex has been encountered
                                         tVertexCount(tVertIndex) = tVertexCount(tVertIndex) + 1.0;
                                    }

                                }
                                else
                                {
                                    // get the vertices in the cluster
                                    moris::Cell<moris::mtk::Vertex *> tVertices = tCluster->get_primary_vertices_in_cluster();

                                    // iterate through vertices in cluster
                                    for(moris::uint iVert =0; iVert<tVertices.size(); iVert++)
                                    {
                                        // get vert spatial parametric coords
                                        moris::Matrix<moris::DDRMat> tVertParamCoords = tCluster->get_vertex_local_coordinate_wrt_interp_cell(tVertices(iVert));

                                        // fixme: figure out how to not need this transpose
                                        if(isrow(tVertParamCoords))
                                        {
                                            tVertParamCoords = trans(tVertParamCoords);
                                        }

                                        // size
                                        uint tSpatialParamSize = tVertParamCoords.numel();

                                        // add a time parametric coord
                                        tVertParamCoords.resize(tSpatialParamSize+1,1);
                                        tVertParamCoords(tSpatialParamSize) = 0;

                                        // set in field interpolator
                                        tFieldInterp->set_space_time(tVertParamCoords);

                                        // evaluate field value at this point
                                        Matrix<DDRMat> tSolFieldAtIntegPoint = tFieldInterp->val();

                                        // vertex index
                                        moris_index tVertIndex = tVertices(iVert)->get_index();

                                        // add to solution vector
                                        tSolutionOnInteg(tVertIndex) = tSolFieldAtIntegPoint(tDofIndex);

                                        // update count that this vertex has been encountered
                                        tVertexCount(tVertIndex) = tVertexCount(tVertIndex) + 1.0;
                                    }

                                }

                            }

                        }
                    }
                }

                // iterate through and divide by the number of times a vertex has been added to solution field
//                for(moris::uint iVert =0; iVert<tNumVertsInIGMesh; iVert++)
//                {
//                    if(tVertexCount(iVert) > 0.0)
//                    {
//                        tSolutionOnInteg(iVert) = tSolutionOnInteg(iVert)/tVertexCount(iVert);
//                    }
//                }

//            }

            return tSolutionOnInteg;
        }

        bool Model::dof_type_is_in_list( enum MSI::Dof_Type aDofTypeToFind,
                                         moris::Cell< enum MSI::Dof_Type > & aDofList)
        {
            bool tDofInList = false;

            uint tNumDofsInFullList = aDofList.size();

            // iterate through
            for(moris::uint iDof = 0; iDof<tNumDofsInFullList; iDof++)
            {
                if(aDofList(iDof) == aDofTypeToFind)
                {
                    tDofInList  = true;
                    break;
                }
            }
            return tDofInList;
        }

    } /* namespace mdl */
} /* namespace moris */
