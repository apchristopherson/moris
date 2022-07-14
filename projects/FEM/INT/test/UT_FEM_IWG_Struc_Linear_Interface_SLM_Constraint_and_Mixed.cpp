#include <string>
#include <catch.hpp>
#include "assert.hpp"

#define protected public
#define private   public
//FEM//INT//src
#include "cl_FEM_Field_Interpolator_Manager.hpp"
#include "cl_FEM_IWG.hpp"
#include "cl_FEM_Set.hpp"
#include "cl_FEM_Cluster.hpp"
#undef protected
#undef private
//MTK/src
#include "cl_MTK_Enums.hpp"
//FEM//INT/src
#include "cl_FEM_Enums.hpp"
#include "cl_FEM_IWG_Factory.hpp"
#include "cl_FEM_CM_Factory.hpp"
#include "cl_FEM_SP_Factory.hpp"
#include "FEM_Test_Proxy/cl_FEM_Inputs_for_Elasticity_UT.cpp"
//LINALG/src
#include "op_equal_equal.hpp"
#include "fn_norm.hpp"

using namespace moris;
using namespace fem;

TEST_CASE( "IWG_Interface_SLM_Constraint", "[moris],[fem],[IWG_Interface_SLM_Constraint]" )
{
    // define an epsilon environment
    // FIXME
    real tEpsilon = 1E-5;

    // define a perturbation relative size
    real tPerturbation = 1E-6;

    // init geometry inputs
    //------------------------------------------------------------------------------
    // create geometry type
    mtk::Geometry_Type tGeometryType = mtk::Geometry_Type::UNDEFINED;

    // create list of interpolation orders
    moris::Cell< mtk::Interpolation_Order > tInterpolationOrders = {
            mtk::Interpolation_Order::LINEAR,
            mtk::Interpolation_Order::QUADRATIC,
            mtk::Interpolation_Order::CUBIC };

    // create list of integration orders
    moris::Cell< mtk::Integration_Order > tIntegrationOrders = {
            mtk::Integration_Order::QUAD_2x2,
            mtk::Integration_Order::HEX_2x2x2 };

    // create list with number of coeffs
    Matrix< DDRMat > tNumCoeffs = {{ 8, 18, 32 },{ 16, 54, 128 }};

    // dof type list
    moris::Cell< moris::Cell< MSI::Dof_Type > > tDispDofTypes = { { MSI::Dof_Type::UX } };
    moris::Cell< moris::Cell< MSI::Dof_Type > > tLMDofTypes = { { MSI::Dof_Type::VX } };
    moris::Cell< moris::Cell< MSI::Dof_Type > > tAllDofTypes = { tDispDofTypes( 0 ), tLMDofTypes( 0 ) };

    // init IWG
    //------------------------------------------------------------------------------
    // create the properties

    // define the IWGs
    fem::IWG_Factory tIWGFactory;

    std::shared_ptr< fem::IWG > tIWG =
            tIWGFactory.create_IWG( fem::IWG_Type::Struc_Linear_Interface_SLM_Constraint );
    tIWG->set_residual_dof_type( tDispDofTypes );
    tIWG->set_dof_type_list( tAllDofTypes, mtk::Master_Slave::MASTER );
    tIWG->set_dof_type_list( tAllDofTypes, mtk::Master_Slave::SLAVE );

    // init set info
    //------------------------------------------------------------------------------
    // set a fem set pointer
    MSI::Equation_Set * tSet = new fem::Set();
    static_cast<fem::Set*>(tSet)->set_set_type( fem::Element_Type::DOUBLE_SIDESET );
    tIWG->set_set_pointer( static_cast< fem::Set* >( tSet ) );

    // set size for the set EqnObjDofTypeList
    tIWG->mSet->mUniqueDofTypeList.resize( 100, MSI::Dof_Type::END_ENUM );

    // set size and populate the set dof type map
    tIWG->mSet->mUniqueDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWG->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::UX ) ) = 0;
    tIWG->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) ) = 1;

    // set size and populate the set master dof type map
    tIWG->mSet->mMasterDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWG->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::UX ) ) = 0;
    tIWG->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) ) = 1;

    // set size and populate the set slave dof type map
    tIWG->mSet->mSlaveDofTypeMap .set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWG->mSet->mSlaveDofTypeMap( static_cast< int >( MSI::Dof_Type::UX ) ) = 0;
    tIWG->mSet->mSlaveDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) ) = 1;

    // loop on the space dimension
    for( uint iSpaceDim = 2; iSpaceDim < 3; iSpaceDim++ )
    {
        // create and set normal
        Matrix< DDRMat > tNormal( iSpaceDim, 1, 0.5 );
        tNormal = tNormal / norm( tNormal );
        tIWG->set_normal( tNormal );

        // set geometry inputs
        //------------------------------------------------------------------------------
        // switch on space dimension
        switch( iSpaceDim )
        {
            case 2 :
            {
                // set geometry type
                tGeometryType = mtk::Geometry_Type::QUAD;
                break;
            }
            case 3 :
            {
                // set geometry type
                tGeometryType = mtk::Geometry_Type::HEX;
                break;
            }
            default:
            {
                MORIS_ERROR( false, " QUAD or HEX only." );
                break;
            }
        }

        // loop on the interpolation order
        for( uint iInterpOrder = 1; iInterpOrder < 4; iInterpOrder++ )
        {
            // create an interpolation order
            mtk::Interpolation_Order tGIInterpolationOrder = tInterpolationOrders( iInterpOrder - 1 );

            // space and time geometry interpolators
            //------------------------------------------------------------------------------
            // create a space geometry interpolation rule
            mtk::Interpolation_Rule tGIRule( tGeometryType,
                    mtk::Interpolation_Type::LAGRANGE,
                    tGIInterpolationOrder,
                    mtk::Interpolation_Type::LAGRANGE,
                    mtk::Interpolation_Order::LINEAR );

            // create a space time geometry interpolator
            Geometry_Interpolator tGI = Geometry_Interpolator( tGIRule );

            // create time coeff tHat
            Matrix< DDRMat > tTHat = {{ 0.0 }, { 1.0 }};

            Matrix< DDRMat > tXHat;
            fill_xhat_Elast( tXHat, iSpaceDim, iInterpOrder );

            // set the coefficients xHat, tHat
            tGI.set_coeff( tXHat, tTHat );

            // integration points
            //------------------------------------------------------------------------------
            // get an integration order
            mtk::Integration_Order tIntegrationOrder = tIntegrationOrders( iSpaceDim - 2 );

            // create an integration rule
            mtk::Integration_Rule tIntegrationRule(
                    tGeometryType,
                    mtk::Integration_Type::GAUSS,
                    tIntegrationOrder,
                    mtk::Geometry_Type::LINE,
                    mtk::Integration_Type::GAUSS,
                    mtk::Integration_Order::BAR_1 );

            // create an integrator
            mtk::Integrator tIntegrator( tIntegrationRule );

            // get integration points
            Matrix< DDRMat > tIntegPoints;
            tIntegrator.get_points( tIntegPoints );

            // field interpolators
            //------------------------------------------------------------------------------
            // create an interpolation order
            mtk::Interpolation_Order tInterpolationOrder = tInterpolationOrders( iInterpOrder - 1 );

            // number of dof for interpolation order
            uint tNumCoeff = tNumCoeffs( iSpaceDim - 2, iInterpOrder - 1 );

            // get number of dof per type
            int tNumDofDisp  = tNumCoeff * iSpaceDim;
            int tNumDofLM  = tNumCoeff * iSpaceDim;
            int tNumTotalDofs = 2 * tNumDofDisp + 2 * tNumDofLM;

            //create a space time interpolation rule
            mtk::Interpolation_Rule tFIRule ( tGeometryType,
                    mtk::Interpolation_Type::LAGRANGE,
                    tInterpolationOrder,
                    mtk::Interpolation_Type::LAGRANGE,
                    mtk::Interpolation_Order::LINEAR );

            // fill coefficients for master FI
            Matrix< DDRMat > tMasterDOFHatDisp;
            fill_uhat_Elast( tMasterDOFHatDisp, iSpaceDim, iInterpOrder );

            Matrix< DDRMat > tMasterDofHatLM;
            fill_uhat_Elast( tMasterDofHatLM, iSpaceDim, iInterpOrder );

            // create a cell of field interpolators for IWG
            Cell< Field_Interpolator* > tMasterFIs( tAllDofTypes.size() );

            // create the field interpolator displacement
            tMasterFIs( 0 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tDispDofTypes( 0 ) );
            tMasterFIs( 0 )->set_coeff( tMasterDOFHatDisp );

            tMasterFIs( 1 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tLMDofTypes( 0 ) );
            tMasterFIs( 1 )->set_coeff( tMasterDofHatLM );

            // fill random coefficients for slave FI
            Matrix< DDRMat > tSlaveDOFHatDisp;
            fill_uhat_Elast( tSlaveDOFHatDisp, iSpaceDim, iInterpOrder );

            Matrix< DDRMat > tSlaveDOFHatLM;
            fill_uhat_Elast( tSlaveDOFHatLM, iSpaceDim, iInterpOrder );

            // create a cell of field interpolators for IWG
            Cell< Field_Interpolator* > tSlaveFIs( tAllDofTypes.size() );

            // create the field interpolator displacement
            tSlaveFIs( 0 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tDispDofTypes( 0 ) );
            tSlaveFIs( 0 )->set_coeff( tSlaveDOFHatDisp );

            tSlaveFIs( 1 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tLMDofTypes( 0 ) );
            tSlaveFIs( 1 )->set_coeff( tSlaveDOFHatLM );


            // set size and fill the set residual assembly map
            tIWG->mSet->mResDofAssemblyMap.resize( 2 * tAllDofTypes.size() );
            tIWG->mSet->mResDofAssemblyMap( 0 ) = { { 0, tNumDofDisp - 1 } };
            tIWG->mSet->mResDofAssemblyMap( 1 ) = { { tNumDofDisp, tNumDofDisp + tNumDofLM - 1 } };
            tIWG->mSet->mResDofAssemblyMap( 2 ) = { { tNumDofDisp + tNumDofLM, 2 * tNumDofDisp + tNumDofLM - 1 } };
            tIWG->mSet->mResDofAssemblyMap( 3 ) = { { 2 * tNumDofDisp + tNumDofLM, 2 * tNumDofDisp + 2 * tNumDofLM - 1 } };

            // set size and fill the set jacobian assembly map
            Matrix< DDSMat > tJacAssembly = {
            		{ 0, tNumDofDisp - 1 },
            		{ tNumDofDisp, tNumDofDisp + tNumDofLM - 1 },
            		{ tNumDofDisp + tNumDofLM, 2 * tNumDofDisp + tNumDofLM - 1 },
            		{ 2 * tNumDofDisp + tNumDofLM, 2 * tNumDofDisp + 2 * tNumDofLM - 1 } };

            tIWG->mSet->mJacDofAssemblyMap.resize( 2 * tAllDofTypes.size() );
            tIWG->mSet->mJacDofAssemblyMap( 0 ) = tJacAssembly;
            tIWG->mSet->mJacDofAssemblyMap( 1 ) = tJacAssembly;
            tIWG->mSet->mJacDofAssemblyMap( 2 ) = tJacAssembly;
            tIWG->mSet->mJacDofAssemblyMap( 3 ) = tJacAssembly;

            // set size and init the set residual and jacobian
            tIWG->mSet->mResidual.resize( 1 );
            tIWG->mSet->mResidual( 0 ).set_size( tNumTotalDofs, 1, 0.0 );
            tIWG->mSet->mJacobian.set_size( tNumTotalDofs, tNumTotalDofs, 0.0 );

            // build global dof type list
            tIWG->get_global_dof_type_list();

            // populate the requested master dof type
            tIWG->mRequestedMasterGlobalDofTypes = tAllDofTypes;
            tIWG->mRequestedSlaveGlobalDofTypes  = tAllDofTypes;

            // create a field interpolator manager
            moris::Cell< moris::Cell< enum PDV_Type > > tDummyDv;
            moris::Cell< moris::Cell< enum mtk::Field_Type > > tDummyField;
            Field_Interpolator_Manager tMasterFIManager( tAllDofTypes, tDummyDv, tDummyField, tSet );
            Field_Interpolator_Manager tSlaveFIManager( tAllDofTypes, tDummyDv, tDummyField, tSet );

            // populate the field interpolator manager
            tMasterFIManager.mFI = tMasterFIs;
            tMasterFIManager.mIPGeometryInterpolator = &tGI;
            tMasterFIManager.mIGGeometryInterpolator = &tGI;
            tSlaveFIManager.mFI = tSlaveFIs;
            tSlaveFIManager.mIPGeometryInterpolator = &tGI;
            tSlaveFIManager.mIGGeometryInterpolator = &tGI;

            // set the interpolator manager to the set
            tIWG->mSet->mMasterFIManager = &tMasterFIManager;
            tIWG->mSet->mSlaveFIManager = &tSlaveFIManager;

            // set IWG field interpolator manager
            tIWG->set_field_interpolator_manager( &tMasterFIManager, mtk::Master_Slave::MASTER );
            tIWG->set_field_interpolator_manager( &tSlaveFIManager, mtk::Master_Slave::SLAVE );

            // loop over integration points
            uint tNumGPs = tIntegPoints.n_cols();
            for( uint iGP = 0; iGP < tNumGPs; iGP ++ )
            {
                // reset IWG evaluation flags
                tIWG->reset_eval_flags();

                // create evaluation point xi, tau
                Matrix< DDRMat > tParamPoint = tIntegPoints.get_column( iGP );

                // set integration point
                tIWG->mSet->mMasterFIManager->set_space_time( tParamPoint );
                tIWG->mSet->mSlaveFIManager->set_space_time( tParamPoint );

                // check evaluation of the residual for IWG
                //------------------------------------------------------------------------------
                // reset residual
                tIWG->mSet->mResidual( 0 ).fill( 0.0 );

                // compute residual
                tIWG->compute_residual( 1.0 );

                // check evaluation of the jacobian by FD
                //------------------------------------------------------------------------------
                // reset jacobian
                tIWG->mSet->mJacobian.fill( 0.0 );

                // init the jacobian for IWG and FD evaluation
                Matrix< DDRMat > tJacobian;
                Matrix< DDRMat > tJacobianFD;

//tIWG->compute_jacobian( 1.0 );
//print( tIWG->mSet->mResidual( 0 ), "Residual" );
//print( tIWG->mSet->mJacobian, "Jacobian" );
//MORIS_ERROR( false, "Stop here by intention" );

                // check jacobian by FD
                bool tCheckJacobian = tIWG->check_jacobian(
                        tPerturbation,
                        tEpsilon,
                        1.0,
                        tJacobian,
                        tJacobianFD,
                        true );

                // print for debug
                if( !tCheckJacobian )
                {
                    std::cout<<"Case: Geometry "<<iSpaceDim<<" Order "<<iInterpOrder<<" iGP "<<iGP<<std::endl;
                }

                // require check is true
                REQUIRE( tCheckJacobian );
            }

            // clean up
            tMasterFIs.clear();
            tSlaveFIs.clear();
        }
    }
}/* END_TEST_CASE */

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

TEST_CASE( "IWG_Interface_SLM_Mixed", "[moris],[fem],[IWG_Interface_SLM_Mixed]" )
{
    // define an epsilon environment
    // FIXME
    real tEpsilon = 1E-5;

    // define a perturbation relative size
    real tPerturbation = 1E-6;

    // init geometry inputs
    //------------------------------------------------------------------------------
    // create geometry type
    mtk::Geometry_Type tGeometryType = mtk::Geometry_Type::UNDEFINED;

    // create list of interpolation orders
    moris::Cell< mtk::Interpolation_Order > tInterpolationOrders = {
            mtk::Interpolation_Order::LINEAR,
            mtk::Interpolation_Order::QUADRATIC,
            mtk::Interpolation_Order::CUBIC };

    // create list of integration orders
    moris::Cell< mtk::Integration_Order > tIntegrationOrders = {
            mtk::Integration_Order::QUAD_2x2,
            mtk::Integration_Order::HEX_2x2x2 };

    // create list with number of coeffs
    Matrix< DDRMat > tNumCoeffs = {{ 8, 18, 32 },{ 16, 54, 128 }};

    // dof type list
    moris::Cell< moris::Cell< MSI::Dof_Type > > tDispDofTypes = { { MSI::Dof_Type::UX } };
    moris::Cell< moris::Cell< MSI::Dof_Type > > tLMDofTypes = { { MSI::Dof_Type::VX } };
    moris::Cell< moris::Cell< MSI::Dof_Type > > tAllDofTypes = { tDispDofTypes( 0 ), tLMDofTypes( 0 ) };

    // init IWG
    //------------------------------------------------------------------------------
    // create the properties

    // define the IWGs
    fem::IWG_Factory tIWGFactory;

    std::shared_ptr< fem::IWG > tIWG =
            tIWGFactory.create_IWG( fem::IWG_Type::Struc_Linear_Interface_SLM_Mixed );
    tIWG->set_residual_dof_type( tLMDofTypes );
    tIWG->set_dof_type_list( tAllDofTypes, mtk::Master_Slave::MASTER );
    tIWG->set_dof_type_list( tAllDofTypes, mtk::Master_Slave::SLAVE );

    // init set info
    //------------------------------------------------------------------------------
    // set a fem set pointer
    MSI::Equation_Set * tSet = new fem::Set();
    static_cast<fem::Set*>(tSet)->set_set_type( fem::Element_Type::DOUBLE_SIDESET );
    tIWG->set_set_pointer( static_cast< fem::Set* >( tSet ) );

    // set size for the set EqnObjDofTypeList
    tIWG->mSet->mUniqueDofTypeList.resize( 100, MSI::Dof_Type::END_ENUM );

    // set size and populate the set dof type map
    tIWG->mSet->mUniqueDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWG->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::UX ) ) = 0;
    tIWG->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) ) = 1;

    // set size and populate the set master dof type map
    tIWG->mSet->mMasterDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWG->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::UX ) ) = 0;
    tIWG->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) ) = 1;

    // set size and populate the set slave dof type map
    tIWG->mSet->mSlaveDofTypeMap .set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWG->mSet->mSlaveDofTypeMap( static_cast< int >( MSI::Dof_Type::UX ) ) = 0;
    tIWG->mSet->mSlaveDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) ) = 1;

    // loop on the space dimension
    for( uint iSpaceDim = 2; iSpaceDim < 3; iSpaceDim++ )
    {
        // create and set normal
        Matrix< DDRMat > tNormal( iSpaceDim, 1, 0.5 );
        tNormal = tNormal / norm( tNormal );
        tIWG->set_normal( tNormal );

        // set geometry inputs
        //------------------------------------------------------------------------------
        // switch on space dimension
        switch( iSpaceDim )
        {
            case 2 :
            {
                // set geometry type
                tGeometryType = mtk::Geometry_Type::QUAD;
                break;
            }
            case 3 :
            {
                // set geometry type
                tGeometryType = mtk::Geometry_Type::HEX;
                break;
            }
            default:
            {
                MORIS_ERROR( false, " QUAD or HEX only." );
                break;
            }
        }

        // loop on the interpolation order
        for( uint iInterpOrder = 1; iInterpOrder < 4; iInterpOrder++ )
        {
            // create an interpolation order
            mtk::Interpolation_Order tGIInterpolationOrder = tInterpolationOrders( iInterpOrder - 1 );

            // space and time geometry interpolators
            //------------------------------------------------------------------------------
            // create a space geometry interpolation rule
            mtk::Interpolation_Rule tGIRule( tGeometryType,
                    mtk::Interpolation_Type::LAGRANGE,
                    tGIInterpolationOrder,
                    mtk::Interpolation_Type::LAGRANGE,
                    mtk::Interpolation_Order::LINEAR );

            // create a space time geometry interpolator
            Geometry_Interpolator tGI = Geometry_Interpolator( tGIRule );

            // create time coeff tHat
            Matrix< DDRMat > tTHat = {{ 0.0 }, { 1.0 }};

            Matrix< DDRMat > tXHat;
            fill_xhat_Elast( tXHat, iSpaceDim, iInterpOrder );

            // set the coefficients xHat, tHat
            tGI.set_coeff( tXHat, tTHat );

            // integration points
            //------------------------------------------------------------------------------
            // get an integration order
            mtk::Integration_Order tIntegrationOrder = tIntegrationOrders( iSpaceDim - 2 );

            // create an integration rule
            mtk::Integration_Rule tIntegrationRule(
                    tGeometryType,
                    mtk::Integration_Type::GAUSS,
                    tIntegrationOrder,
                    mtk::Geometry_Type::LINE,
                    mtk::Integration_Type::GAUSS,
                    mtk::Integration_Order::BAR_1 );

            // create an integrator
            mtk::Integrator tIntegrator( tIntegrationRule );

            // get integration points
            Matrix< DDRMat > tIntegPoints;
            tIntegrator.get_points( tIntegPoints );

            // field interpolators
            //------------------------------------------------------------------------------
            // create an interpolation order
            mtk::Interpolation_Order tInterpolationOrder = tInterpolationOrders( iInterpOrder - 1 );

            // number of dof for interpolation order
            uint tNumCoeff = tNumCoeffs( iSpaceDim - 2, iInterpOrder - 1 );

            // get number of dof per type
            int tNumDofDisp  = tNumCoeff * iSpaceDim;
            int tNumDofLM  = tNumCoeff * iSpaceDim;
            int tNumTotalDofs = 2 * tNumDofDisp + 2 * tNumDofLM;

            //create a space time interpolation rule
            mtk::Interpolation_Rule tFIRule ( tGeometryType,
                    mtk::Interpolation_Type::LAGRANGE,
                    tInterpolationOrder,
                    mtk::Interpolation_Type::LAGRANGE,
                    mtk::Interpolation_Order::LINEAR );

            // fill coefficients for master FI
            Matrix< DDRMat > tMasterDOFHatDisp;
            fill_uhat_Elast( tMasterDOFHatDisp, iSpaceDim, iInterpOrder );

            Matrix< DDRMat > tMasterDofHatLM;
            fill_uhat_Elast( tMasterDofHatLM, iSpaceDim, iInterpOrder );

            // create a cell of field interpolators for IWG
            Cell< Field_Interpolator* > tMasterFIs( tAllDofTypes.size() );

            // create the field interpolator displacement
            tMasterFIs( 0 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tDispDofTypes( 0 ) );
            tMasterFIs( 0 )->set_coeff( tMasterDOFHatDisp );

            tMasterFIs( 1 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tLMDofTypes( 0 ) );
            tMasterFIs( 1 )->set_coeff( tMasterDofHatLM );

            // fill random coefficients for slave FI
            Matrix< DDRMat > tSlaveDOFHatDisp;
            fill_uhat_Elast( tSlaveDOFHatDisp, iSpaceDim, iInterpOrder );

            Matrix< DDRMat > tSlaveDOFHatLM;
            fill_uhat_Elast( tSlaveDOFHatLM, iSpaceDim, iInterpOrder );

            // create a cell of field interpolators for IWG
            Cell< Field_Interpolator* > tSlaveFIs( tAllDofTypes.size() );

            // create the field interpolator displacement
            tSlaveFIs( 0 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tDispDofTypes( 0 ) );
            tSlaveFIs( 0 )->set_coeff( tSlaveDOFHatDisp );

            tSlaveFIs( 1 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tLMDofTypes( 0 ) );
            tSlaveFIs( 1 )->set_coeff( tSlaveDOFHatLM );


            // set size and fill the set residual assembly map
            tIWG->mSet->mResDofAssemblyMap.resize( 2 * tAllDofTypes.size() );
            tIWG->mSet->mResDofAssemblyMap( 0 ) = { { 0, tNumDofDisp - 1 } };
            tIWG->mSet->mResDofAssemblyMap( 1 ) = { { tNumDofDisp, tNumDofDisp + tNumDofLM - 1 } };
            tIWG->mSet->mResDofAssemblyMap( 2 ) = { { tNumDofDisp + tNumDofLM, 2 * tNumDofDisp + tNumDofLM - 1 } };
            tIWG->mSet->mResDofAssemblyMap( 3 ) = { { 2 * tNumDofDisp + tNumDofLM, 2 * tNumDofDisp + 2 * tNumDofLM - 1 } };

            // set size and fill the set jacobian assembly map
            Matrix< DDSMat > tJacAssembly = {
            		{ 0, tNumDofDisp - 1 },
            		{ tNumDofDisp, tNumDofDisp + tNumDofLM - 1 },
            		{ tNumDofDisp + tNumDofLM, 2 * tNumDofDisp + tNumDofLM - 1 },
            		{ 2 * tNumDofDisp + tNumDofLM, 2 * tNumDofDisp + 2 * tNumDofLM - 1 } };

            tIWG->mSet->mJacDofAssemblyMap.resize( 2 * tAllDofTypes.size() );
            tIWG->mSet->mJacDofAssemblyMap( 0 ) = tJacAssembly;
            tIWG->mSet->mJacDofAssemblyMap( 1 ) = tJacAssembly;
            tIWG->mSet->mJacDofAssemblyMap( 2 ) = tJacAssembly;
            tIWG->mSet->mJacDofAssemblyMap( 3 ) = tJacAssembly;

            // set size and init the set residual and jacobian
            tIWG->mSet->mResidual.resize( 1 );
            tIWG->mSet->mResidual( 0 ).set_size( tNumTotalDofs, 1, 0.0 );
            tIWG->mSet->mJacobian.set_size( tNumTotalDofs, tNumTotalDofs, 0.0 );

            // build global dof type list
            tIWG->get_global_dof_type_list();

            // populate the requested master dof type
            tIWG->mRequestedMasterGlobalDofTypes = tAllDofTypes;
            tIWG->mRequestedSlaveGlobalDofTypes  = tAllDofTypes;

            // create a field interpolator manager
            moris::Cell< moris::Cell< enum PDV_Type > > tDummyDv;
            moris::Cell< moris::Cell< enum mtk::Field_Type > > tDummyField;
            Field_Interpolator_Manager tMasterFIManager( tAllDofTypes, tDummyDv, tDummyField, tSet );
            Field_Interpolator_Manager tSlaveFIManager( tAllDofTypes, tDummyDv, tDummyField, tSet );

            // populate the field interpolator manager
            tMasterFIManager.mFI = tMasterFIs;
            tMasterFIManager.mIPGeometryInterpolator = &tGI;
            tMasterFIManager.mIGGeometryInterpolator = &tGI;
            tSlaveFIManager.mFI = tSlaveFIs;
            tSlaveFIManager.mIPGeometryInterpolator = &tGI;
            tSlaveFIManager.mIGGeometryInterpolator = &tGI;

            // set the interpolator manager to the set
            tIWG->mSet->mMasterFIManager = &tMasterFIManager;
            tIWG->mSet->mSlaveFIManager = &tSlaveFIManager;

            // set IWG field interpolator manager
            tIWG->set_field_interpolator_manager( &tMasterFIManager, mtk::Master_Slave::MASTER );
            tIWG->set_field_interpolator_manager( &tSlaveFIManager, mtk::Master_Slave::SLAVE );

            // loop over integration points
            uint tNumGPs = tIntegPoints.n_cols();
            for( uint iGP = 0; iGP < tNumGPs; iGP ++ )
            {
                // reset IWG evaluation flags
                tIWG->reset_eval_flags();

                // create evaluation point xi, tau
                Matrix< DDRMat > tParamPoint = tIntegPoints.get_column( iGP );

                // set integration point
                tIWG->mSet->mMasterFIManager->set_space_time( tParamPoint );
                tIWG->mSet->mSlaveFIManager->set_space_time( tParamPoint );

                // check evaluation of the residual for IWG
                //------------------------------------------------------------------------------
                // reset residual
                tIWG->mSet->mResidual( 0 ).fill( 0.0 );

                // compute residual
                tIWG->compute_residual( 1.0 );

                // check evaluation of the jacobian by FD
                //------------------------------------------------------------------------------
                // reset jacobian
                tIWG->mSet->mJacobian.fill( 0.0 );

                // init the jacobian for IWG and FD evaluation
                Matrix< DDRMat > tJacobian;
                Matrix< DDRMat > tJacobianFD;

//tIWG->compute_jacobian( 1.0 );
//print( tIWG->mSet->mResidual( 0 ), "Residual" );
//print( tIWG->mSet->mJacobian, "Jacobian" );
//MORIS_ERROR( false, "Stop here by intention" );

                // check jacobian by FD
                bool tCheckJacobian = tIWG->check_jacobian(
                        tPerturbation,
                        tEpsilon,
                        1.0,
                        tJacobian,
                        tJacobianFD,
                        true );

                // print for debug
                if( !tCheckJacobian )
                {
                    std::cout<<"Case: Geometry "<<iSpaceDim<<" Order "<<iInterpOrder<<" iGP "<<iGP<<std::endl;
                }

                // require check is true
                REQUIRE( tCheckJacobian );
            }

            // clean up
            tMasterFIs.clear();
            tSlaveFIs.clear();
        }
    }
}/* END_TEST_CASE */