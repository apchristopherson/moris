#include "cl_WRK_Performer_Manager.hpp"
#include "cl_WRK_Workflow_HMR_XTK.hpp"
#include "fn_WRK_perform_refinement.hpp"

#include "cl_HMR.hpp"
#include "cl_MTK_Mesh_Manager.hpp"
#include "cl_GEN_Geometry_Engine.hpp"
#include "cl_XTK_Model.hpp"
#include "cl_MDL_Model.hpp"

#include "cl_Logger.hpp"
#include "cl_Tracer.hpp"

#include "cl_Stopwatch.hpp"

#include "fn_norm.hpp"

namespace moris
{
    namespace wrk
    {

        //------------------------------------------------------------------------------

        // Parameter function
        typedef void ( *Parameter_Function ) ( moris::Cell< moris::Cell< moris::ParameterList > > & aParameterList );
        
        //--------------------------------------------------------------------------------------------------------------

        Workflow_HMR_XTK::Workflow_HMR_XTK( wrk::Performer_Manager * aPerformerManager )
        : Workflow( aPerformerManager )
        {

            // Performer set for this workflow
            mPerformerManager->mHMRPerformer.resize( 1 );
            mPerformerManager->mGENPerformer.resize( 1 );
            mPerformerManager->mXTKPerformer.resize( 1 );
            mPerformerManager->mMTKPerformer.resize( 2 );
            mPerformerManager->mMDLPerformer.resize( 1 );

            // load the HMR parameter list
            std::string tHMRString = "HMRParameterList";
            Parameter_Function tHMRParameterListFunc = mPerformerManager->mLibrary->load_function<Parameter_Function>( tHMRString );
            moris::Cell< moris::Cell< ParameterList > > tHMRParameterList;
            tHMRParameterListFunc( tHMRParameterList );

            std::string tGENString = "GENParameterList";
            Parameter_Function tGENParameterListFunc = mPerformerManager->mLibrary->load_function<Parameter_Function>( tGENString );
            moris::Cell< moris::Cell< ParameterList > > tGENParameterList;
            tGENParameterListFunc( tGENParameterList );

            // create HMR performer
            mPerformerManager->mHMRPerformer( 0 ) = std::make_shared< hmr::HMR >( tHMRParameterList( 0 )( 0 ), mPerformerManager->mLibrary );

            // create MTK performer - will be used for HMR mesh
            mPerformerManager->mMTKPerformer( 0 ) =std::make_shared< mtk::Mesh_Manager >();

            // Create GE performer
            mPerformerManager->mGENPerformer( 0 ) = std::make_shared< ge::Geometry_Engine >( tGENParameterList, mPerformerManager->mLibrary );

            // create MTK performer - will be used for XTK mesh
            mPerformerManager->mMTKPerformer( 1 ) = std::make_shared< mtk::Mesh_Manager >();

            // create MDL performer
            mPerformerManager->mMDLPerformer( 0 ) = std::make_shared< mdl::Model >( mPerformerManager->mLibrary, 0 );
            
            // Set performer to HMR
            mPerformerManager->mHMRPerformer( 0 )->set_performer( mPerformerManager->mMTKPerformer( 0 ) );

            // Set performer to MDL
            mPerformerManager->mMDLPerformer( 0 )->set_performer( mPerformerManager->mMTKPerformer( 1 ) );
        }

        //--------------------------------------------------------------------------------------------------------------

        void Workflow_HMR_XTK::initialize(
                Matrix<DDRMat>& aADVs,
                Matrix<DDRMat>& aLowerBounds,
                Matrix<DDRMat>& aUpperBounds)
        {
            // Stage 1: HMR refinement -------------------------------------------------------------------
            {
                // Trace HMR
                Tracer tTracer( "HMR", "HMRmesh", "Create" );

                mPerformerManager->mHMRPerformer( 0 )->reset_HMR();

                // uniform initial refinement
                mPerformerManager->mHMRPerformer( 0 )->perform_initial_refinement();

                // HMR refined by GE
                perform_refinement(mPerformerManager->mHMRPerformer( 0 ), {mPerformerManager->mGENPerformer( 0 )});

                // HMR finalize
                mPerformerManager->mHMRPerformer( 0 )->perform();
            }

            // Stage 2: Initialize Level set field in GEN -----------------------------------------------
            {
                // Trace GEN
                Tracer tTracer( "GEN", "Levelset", "InitializeADVs" );

                mPerformerManager->mGENPerformer( 0 )->distribute_advs(
                        mPerformerManager->mMTKPerformer( 0 ) );

                // Get ADVs
                aADVs        = mPerformerManager->mGENPerformer( 0 )->get_advs();
                aLowerBounds = mPerformerManager->mGENPerformer( 0 )->get_lower_bounds();
                aUpperBounds = mPerformerManager->mGENPerformer( 0 )->get_upper_bounds();
            }   

        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<DDRMat> Workflow_HMR_XTK::perform(const Matrix<DDRMat> & aNewADVs)
        {
            // Set new advs in GE
            mPerformerManager->mGENPerformer( 0 )->set_advs(aNewADVs);

            // Stage 1: HMR refinement

            // Stage 2: XTK -----------------------------------------------------------------------------
            this->create_xtk();

            // Compute level set data in GEN
            mPerformerManager->mGENPerformer( 0 )->distribute_advs(
                    mPerformerManager->mMTKPerformer( 0 ) );

            // Output GEN fields, if requested
            mPerformerManager->mGENPerformer( 0 )->output_fields(
                    mPerformerManager->mMTKPerformer( 0 )->get_interpolation_mesh( 0 ));

            // XTK perform - decompose - enrich - ghost - multigrid
            mPerformerManager->mXTKPerformer( 0 )->perform();

            // Assign PDVs
            mPerformerManager->mGENPerformer( 0 )->create_pdvs( mPerformerManager->mMTKPerformer( 1 ) );

            // Stage 3: MDL perform ---------------------------------------------------------------------

            mPerformerManager->mMDLPerformer( 0 )->set_design_variable_interface(
                                mPerformerManager->mGENPerformer( 0 )->get_design_variable_interface() );

            mPerformerManager->mMDLPerformer( 0 )->initialize();

            mPerformerManager->mGENPerformer( 0 )->communicate_requested_IQIs();

            // Build MDL components and solve
            mPerformerManager->mMDLPerformer( 0 )->perform();

            moris::Cell< moris::Matrix< DDRMat > > tVal = mPerformerManager->mMDLPerformer( 0 )->get_IQI_values();

            // Communicate IQIs
            for( uint iIQIIndex = 0; iIQIIndex < tVal.size(); iIQIIndex++ )
            {
                tVal( iIQIIndex )( 0 ) = sum_all( tVal( iIQIIndex )( 0 ) );
            }

            moris::Matrix< DDRMat > tMat( tVal.size(), 1, 0.0 );

            for( uint Ik = 0; Ik < tVal.size(); Ik ++ )
            {
                tMat( Ik ) = tVal( Ik )( 0 );
            }

            return tMat;
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<DDRMat> Workflow_HMR_XTK::compute_dcriteria_dadv()
        {
            mPerformerManager->mGENPerformer( 0 )->communicate_requested_IQIs();

            mPerformerManager->mMDLPerformer( 0 )->perform( 1 );

            Matrix<DDRMat> tDCriteriaDAdv = mPerformerManager->mGENPerformer( 0 )->get_dcriteria_dadv();

            if (par_rank() == 0)
            {
                MORIS_LOG_INFO ( "--------------------------------------------------------------------------------");
                MORIS_LOG_INFO ( "Gradients of design criteria wrt ADVs:");

                for (uint i=0;i<tDCriteriaDAdv.n_rows();++i)
                {
                    Matrix<DDRMat> tDIQIDAdv = tDCriteriaDAdv.get_row(i);

                    auto tItrMin = std::min_element(tDIQIDAdv.data(),tDIQIDAdv.data()+tDIQIDAdv.numel());
                    auto tIndMin = std::distance(tDIQIDAdv.data(),tItrMin);

                    auto tItrMax = std::max_element(tDIQIDAdv.data(),tDIQIDAdv.data()+tDIQIDAdv.numel());
                    auto tIndMax = std::distance(tDIQIDAdv.data(),tItrMax);

                    MORIS_LOG_INFO ( "Criteria(%i): norm = %e   min = %e  (index = %i)   max = %e  (index = %i)",
                                     i, norm(tDIQIDAdv),tDIQIDAdv.min(),tIndMin,tDIQIDAdv.max(),tIndMax);
                }

                MORIS_LOG_INFO ( "--------------------------------------------------------------------------------");
            }

            return tDCriteriaDAdv;
        }

        void Workflow_HMR_XTK::create_xtk()
        {
            // Read parameter list from shared object
            Parameter_Function tXTKParameterListFunc = mPerformerManager->mLibrary->load_function<Parameter_Function>( "XTKParameterList" );
            moris::Cell< moris::Cell< ParameterList > > tXTKParameterList;
            tXTKParameterListFunc( tXTKParameterList );

            // Create XTK
            mPerformerManager->mXTKPerformer( 0 ) = std::make_shared< xtk::Model >( tXTKParameterList( 0 )( 0 ) );

            // Reset output MTK performer
            mPerformerManager->mMTKPerformer( 1 ) = std::make_shared< mtk::Mesh_Manager >();
            mPerformerManager->mMDLPerformer( 0 )->set_performer( mPerformerManager->mMTKPerformer( 1 ) );

            // Set performers
            mPerformerManager->mXTKPerformer( 0 )->set_geometry_engine( mPerformerManager->mGENPerformer( 0 ).get() );
            mPerformerManager->mXTKPerformer( 0 )->set_input_performer( mPerformerManager->mMTKPerformer( 0 ) );
            mPerformerManager->mXTKPerformer( 0 )->set_output_performer( mPerformerManager->mMTKPerformer( 1 ) );
        }

        //--------------------------------------------------------------------------------------------------------------
    } /* namespace mdl */
} /* namespace moris */
