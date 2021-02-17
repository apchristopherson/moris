#include <string>
#include <catch.hpp>
#include "assert.hpp"

#define protected public
#define private   public
//FEM//INT//src
#include "cl_FEM_Field_Interpolator_Manager.hpp"
#include "cl_FEM_IWG.hpp"
#include "cl_FEM_Set.hpp"
#undef protected
#undef private

//MTK/src
#include "cl_MTK_Enums.hpp"
//FEM/INT/src
#include "cl_FEM_Enums.hpp"
#include "cl_FEM_IWG_Factory.hpp"
#include "cl_FEM_CM_Factory.hpp"
#include "cl_FEM_SP_Factory.hpp"
#include "FEM_Test_Proxy/cl_FEM_Inputs_for_NS_Incompressible_UT.cpp"
//LINALG
#include "op_equal_equal.hpp"
#include "fn_norm.hpp"

using namespace moris;
using namespace fem;
/*
This UT tests the pressure ghost IWG for incompressible NS
for QUAD, HEX geometry type
for LINEAR, QUADRATIC and CUBIC interpolation order
*/
TEST_CASE( "IWG_Incompressible_NS_Pressure_Ghost", "[IWG_Incompressible_NS_Pressure_Ghost]" )
{
    // define an epsilon environment
    real tEpsilon = 1E-6;

    // define a perturbation relative size
    real tPerturbation = 1E-6;

    // init geometry inputs
    //------------------------------------------------------------------------------
    // create geometry type
    mtk::Geometry_Type tGeometryType = mtk::Geometry_Type::UNDEFINED;

    // create space coeff xHat
    Matrix< DDRMat > tXHat;

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
    moris::Cell< MSI::Dof_Type > tVelDofTypes  = { MSI::Dof_Type::VX };
    moris::Cell< MSI::Dof_Type > tPDofTypes    = { MSI::Dof_Type::P };
    moris::Cell< moris::Cell< MSI::Dof_Type > > tDofTypes = { tVelDofTypes, tPDofTypes };

    // init IWG
    //------------------------------------------------------------------------------
    // create the properties
    std::shared_ptr< fem::Property > tPropMasterViscosity = std::make_shared< fem::Property >();
    tPropMasterViscosity->set_parameters( { {{ 1.0 }} } );
    tPropMasterViscosity->set_val_function( tConstValFunc );
//    tPropMasterViscosity->set_dof_type_list( { tVelDofTypes } );
//    tPropMasterViscosity->set_val_function( tVXFIValFunc );
//    tPropMasterViscosity->set_dof_derivative_functions( { tVXFIDerFunc } );

    std::shared_ptr< fem::Property > tPropMasterDensity = std::make_shared< fem::Property >();
    tPropMasterDensity->set_parameters( { {{ 1.0 }} } );
    tPropMasterDensity->set_val_function( tConstValFunc );
//    tPropMasterDensity->set_dof_type_list( { tVelDofTypes } );
//    tPropMasterDensity->set_val_function( tVXFIValFunc );
//    tPropMasterDensity->set_dof_derivative_functions( { tVXFIDerFunc } );

    // define stabilization parameters
    fem::SP_Factory tSPFactory;

    std::shared_ptr< fem::Stabilization_Parameter > tSPPressureGhost =
            tSPFactory.create_SP( fem::Stabilization_Type::PRESSURE_GHOST );
    tSPPressureGhost->set_dof_type_list( { tVelDofTypes }, mtk::Master_Slave::MASTER );
    tSPPressureGhost->set_parameters( { {{ 1.0 }}, {{ 1.0 }} });
    tSPPressureGhost->set_property( tPropMasterViscosity, "Viscosity", mtk::Master_Slave::MASTER );
    tSPPressureGhost->set_property( tPropMasterDensity, "Density", mtk::Master_Slave::MASTER );

    // define the IWGs
    fem::IWG_Factory tIWGFactory;

    std::shared_ptr< fem::IWG > tIWG =
            tIWGFactory.create_IWG( fem::IWG_Type::INCOMPRESSIBLE_NS_PRESSURE_GHOST );
    tIWG->set_residual_dof_type( tPDofTypes );
    tIWG->set_dof_type_list( tDofTypes, mtk::Master_Slave::MASTER );
    tIWG->set_dof_type_list( tDofTypes, mtk::Master_Slave::SLAVE );
    tIWG->set_stabilization_parameter( tSPPressureGhost, "PressureGhost" );

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
    tIWG->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) ) = 0;
    tIWG->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::P ) )  = 1;

    // set size and populate the set master dof type map
    tIWG->mSet->mMasterDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWG->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) ) = 0;
    tIWG->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::P ) )  = 1;

    // set size and populate the set slave dof type map
    tIWG->mSet->mSlaveDofTypeMap .set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWG->mSet->mSlaveDofTypeMap ( static_cast< int >( MSI::Dof_Type::VX ) ) = 0;
    tIWG->mSet->mSlaveDofTypeMap ( static_cast< int >( MSI::Dof_Type::P ) )  = 1;

    // loop on the space dimension
    for( uint iSpaceDim = 2; iSpaceDim < 4; iSpaceDim++ )
    {
        // set geometry inputs
        //------------------------------------------------------------------------------

        // create and set normal
        Matrix< DDRMat > tNormal( iSpaceDim, 1, 0.5 );
        tNormal = tNormal / norm( tNormal );
        tIWG->set_normal( tNormal );

        // switch on space dimension
        switch( iSpaceDim )
        {
            case 2 :
            {
                // set geometry type
                tGeometryType = mtk::Geometry_Type::QUAD;

                // fill space coeff xHat
                tXHat = {{ 0.0, 0.0 },
                         { 1.0, 0.0 },
                         { 1.0, 1.0 },
                         { 0.0, 1.0 }};

               // set velocity dof types
               tVelDofTypes = { MSI::Dof_Type::VX, MSI::Dof_Type::VY };
               break;
            }
            case 3 :
            {
                // set geometry type
                tGeometryType = mtk::Geometry_Type::HEX;

                // fill space coeff xHat
                tXHat = {{ 0.0, 0.0, 0.0 },
                         { 1.0, 0.0, 0.0 },
                         { 1.0, 1.0, 0.0 },
                         { 0.0, 1.0, 0.0 },
                         { 0.0, 0.0, 1.0 },
                         { 1.0, 0.0, 1.0 },
                         { 1.0, 1.0, 1.0 },
                         { 0.0, 1.0, 1.0 }};

                // set velocity dof types
                tVelDofTypes = { MSI::Dof_Type::VX, MSI::Dof_Type::VY, MSI::Dof_Type::VZ };
                break;
            }
            default:
            {
                MORIS_ERROR( false, " QUAD or HEX only." );
                break;
            }
        }

        // space and time geometry interpolators
        //------------------------------------------------------------------------------
        // create a space geometry interpolation rule
        mtk::Interpolation_Rule tGIRule( tGeometryType,
                                    mtk::Interpolation_Type::LAGRANGE,
                                    mtk::Interpolation_Order::LINEAR,
                                    mtk::Interpolation_Type::LAGRANGE,
                                    mtk::Interpolation_Order::LINEAR );

        // create a space time geometry interpolator
        Geometry_Interpolator tGI = Geometry_Interpolator( tGIRule );

        // create time coeff tHat
        Matrix< DDRMat > tTHat = {{ 0.0 }, { 1.0 }};

        // set the coefficients xHat, tHat
        tGI.set_coeff( tXHat, tTHat );

        // set space dimension to CM, SP
        tSPPressureGhost->set_space_dim( iSpaceDim );

        // loop on the interpolation order
        for( uint iInterpOrder = 1; iInterpOrder < 4; iInterpOrder++ )
        {
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
            int tNumDofVel  = tNumCoeff * iSpaceDim;
            int tNumDofP    = tNumCoeff;

            //create a space time interpolation rule
            mtk::Interpolation_Rule tFIRule ( tGeometryType,
                                         mtk::Interpolation_Type::LAGRANGE,
                                         tInterpolationOrder,
                                         mtk::Interpolation_Type::LAGRANGE,
                                         mtk::Interpolation_Order::LINEAR );

            // fill coefficients for master FI
            Matrix< DDRMat > tMasterDOFHatVel;;
            fill_uhat( tMasterDOFHatVel, iSpaceDim, iInterpOrder );
            tMasterDOFHatVel( 3, 1 ) = 1000 * tMasterDOFHatVel( 3, 1 );
            Matrix< DDRMat > tMasterDOFHatP;
            fill_phat( tMasterDOFHatP, iSpaceDim, iInterpOrder );

            // create a cell of field interpolators for IWG
            Cell< Field_Interpolator* > tMasterFIs( tDofTypes.size() );

            // create the field interpolator velocity
            tMasterFIs( 0 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tVelDofTypes );
            tMasterFIs( 0 )->set_coeff( tMasterDOFHatVel );

            // create the field interpolator pressure
            tMasterFIs( 1 ) = new Field_Interpolator( 1, tFIRule, &tGI, tPDofTypes );
            tMasterFIs( 1 )->set_coeff( tMasterDOFHatP );

            // fill random coefficients for slave FI
            Matrix< DDRMat > tSlaveDOFHatVel;
            fill_uhat( tSlaveDOFHatVel, iSpaceDim, iInterpOrder );
            Matrix< DDRMat > tSlaveDOFHatP;
            fill_phat( tSlaveDOFHatP, iSpaceDim, iInterpOrder );

            // create a cell of field interpolators for IWG
            Cell< Field_Interpolator* > tSlaveFIs( tDofTypes.size() );

            // create the field interpolator velocity
            tSlaveFIs( 0 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tVelDofTypes );
            tSlaveFIs( 0 )->set_coeff( tSlaveDOFHatVel );

            // create the field interpolator pressure
            tSlaveFIs( 1 ) = new Field_Interpolator( 1, tFIRule, &tGI, tPDofTypes );
            tSlaveFIs( 1 )->set_coeff( tSlaveDOFHatP );

            // set size and fill the set residual assembly map
            tIWG->mSet->mResDofAssemblyMap.resize( 4 );
            tIWG->mSet->mResDofAssemblyMap( 0 ) = { { 0, tNumDofVel-1 } };
            tIWG->mSet->mResDofAssemblyMap( 1 ) = { { tNumDofVel, tNumDofP + tNumDofVel - 1 } };
            tIWG->mSet->mResDofAssemblyMap( 2 ) = { { tNumDofP + tNumDofVel, ( 2 * tNumDofVel ) + tNumDofP - 1 } };
            tIWG->mSet->mResDofAssemblyMap( 3 ) = { { ( 2 * tNumDofVel ) + tNumDofP, ( 2 * ( tNumDofP + tNumDofVel ) ) - 1 } };

            // set size and fill the set jacobian assembly map
            Matrix< DDSMat > tJacAssembly = {
                    { 0, tNumDofVel-1 },
                    { tNumDofVel, tNumDofVel + tNumDofP - 1 },
                    { tNumDofVel + tNumDofP, ( 2 * tNumDofVel ) + tNumDofP - 1 },
                    { 2 * tNumDofVel + tNumDofP, 2 * ( tNumDofVel + tNumDofP ) - 1 } };
            tIWG->mSet->mJacDofAssemblyMap.resize( 4 );
            tIWG->mSet->mJacDofAssemblyMap( 0 ) = tJacAssembly;
            tIWG->mSet->mJacDofAssemblyMap( 1 ) = tJacAssembly;
            tIWG->mSet->mJacDofAssemblyMap( 2 ) = tJacAssembly;
            tIWG->mSet->mJacDofAssemblyMap( 3 ) = tJacAssembly;

            // set size and init the set residual and jacobian
            tIWG->mSet->mResidual.resize( 1 );
            tIWG->mSet->mResidual( 0 ).set_size(
                    2 * ( tNumDofVel + tNumDofP ),
                    1,
                    0.0 );
            tIWG->mSet->mJacobian.set_size(
                    2 * ( tNumDofVel + tNumDofP ),
                    2 * ( tNumDofVel + tNumDofP ),
                    0.0 );

            // build global dof type list
            tIWG->get_global_dof_type_list();

            // populate the requested master dof type
            tIWG->mRequestedMasterGlobalDofTypes = tDofTypes;
            tIWG->mRequestedSlaveGlobalDofTypes  = tDofTypes;

            // create a field interpolator manager
            moris::Cell< moris::Cell< enum PDV_Type > > tDummyDv;
            Field_Interpolator_Manager tMasterFIManager( tDofTypes, tDummyDv, tSet );
            Field_Interpolator_Manager tSlaveFIManager( tDofTypes, tDummyDv, tSet );

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

            // loop over inetgration points
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
}/*END_TEST_CASE*/