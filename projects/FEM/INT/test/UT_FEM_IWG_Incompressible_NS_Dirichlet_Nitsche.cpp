#include <string>
#include <catch.hpp>
#include <memory>
#include "assert.hpp"

#define protected public
#define private   public
//FEM//INT//src
#include "cl_FEM_Field_Interpolator_Manager.hpp"
#include "cl_FEM_IWG.hpp"
#include "cl_FEM_Set.hpp"
#undef protected
#undef private
//LINALG/src
#include "op_equal_equal.hpp"
#include "fn_norm.hpp"
//MTK/src
#include "cl_MTK_Enums.hpp"
//FEM//INT//src
#include "cl_FEM_Enums.hpp"
#include "cl_FEM_Field_Interpolator.hpp"
#include "cl_FEM_Property.hpp"
#include "cl_FEM_CM_Factory.hpp"
#include "cl_FEM_SP_Factory.hpp"
#include "cl_FEM_IWG_Factory.hpp"
#include "cl_FEM_Integrator.hpp"
#include "FEM_Test_Proxy/cl_FEM_Inputs_for_NS_Incompressible_UT.cpp"

using namespace moris;
using namespace fem;

TEST_CASE( "IWG_Incompressible_NS_Dirichlet_Velocity_Symmetric_Nitsche",
        "[IWG_Incompressible_NS_Dirichlet_Velocity_Symmetric_Nitsche]" )
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
    moris::Cell< fem::Integration_Order > tIntegrationOrders = {
            fem::Integration_Order::QUAD_2x2,
            fem::Integration_Order::HEX_2x2x2 };

    // create list with number of coeffs
    Matrix< DDRMat > tNumCoeffs = {{ 8, 18, 32 },{ 16, 54, 128 }};

    // dof type list
    moris::Cell< MSI::Dof_Type > tVelDofTypes  = { MSI::Dof_Type::VX };
    moris::Cell< MSI::Dof_Type > tPDofTypes    = { MSI::Dof_Type::P };
    moris::Cell< MSI::Dof_Type > tVisDofTypes  = { MSI::Dof_Type::VISCOSITY };
    moris::Cell< moris::Cell< MSI::Dof_Type > > tDofTypes = { tVelDofTypes, tPDofTypes, tVisDofTypes };

    // create the properties
    std::shared_ptr< fem::Property > tPropViscosity = std::make_shared< fem::Property >();
    tPropViscosity->set_parameters( { {{ 1.0 }} } );
    tPropViscosity->set_val_function( tConstValFunc );
    //tPropViscosity->set_dof_type_list( { tPDofTypes } );
    //tPropViscosity->set_val_function( tPFIValFunc );
    //tPropViscosity->set_dof_derivative_functions( { tPFIDerFunc } );

    std::shared_ptr< fem::Property > tPropDensity = std::make_shared< fem::Property >();
    tPropDensity->set_parameters( { {{ 1.0 }} } );
    tPropDensity->set_val_function( tConstValFunc );
    //tPropDensity->set_dof_type_list( { tPDofTypes } );
    //tPropDensity->set_val_function( tPFIValFunc );
    //tPropDensity->set_dof_derivative_functions( { tPFIDerFunc } );

    std::shared_ptr< fem::Property > tPropVelocity = std::make_shared< fem::Property >();
    tPropVelocity->set_val_function( tConstValFunc );

    // define constitutive models
    fem::CM_Factory tCMFactory;

    std::shared_ptr< fem::Constitutive_Model > tCMMasterIncFluid =
            tCMFactory.create_CM( fem::Constitutive_Type::FLUID_INCOMPRESSIBLE );
    tCMMasterIncFluid->set_dof_type_list( { tVelDofTypes, tPDofTypes } );
    tCMMasterIncFluid->set_property( tPropViscosity, "Viscosity" );
    tCMMasterIncFluid->set_property( tPropDensity, "Density" );

    std::shared_ptr< fem::Constitutive_Model > tCMMasterTurbulence =
            tCMFactory.create_CM( fem::Constitutive_Type::FLUID_TURBULENCE );
    tCMMasterTurbulence->set_dof_type_list( { tVelDofTypes, tVisDofTypes } );
    tCMMasterTurbulence->set_property( tPropViscosity, "Viscosity" );
    tCMMasterTurbulence->set_property( tPropDensity, "Density" );

    // define stabilization parameters
    fem::SP_Factory tSPFactory;

    std::shared_ptr< fem::Stabilization_Parameter > tSPNitsche =
            tSPFactory.create_SP( fem::Stabilization_Type::VELOCITY_DIRICHLET_NITSCHE );
    tSPNitsche->set_dof_type_list( { tVelDofTypes }, mtk::Master_Slave::MASTER );
    tSPNitsche->set_property( tPropDensity, "Density", mtk::Master_Slave::MASTER );
    tSPNitsche->set_property( tPropViscosity, "Viscosity", mtk::Master_Slave::MASTER );
    tSPNitsche->set_parameters( { {{ 1.0 }}, {{ 1.0 }} } );

    // define the IWGs
    fem::IWG_Factory tIWGFactory;

    std::shared_ptr< fem::IWG > tIWGVelocity =
            tIWGFactory.create_IWG( fem::IWG_Type::INCOMPRESSIBLE_NS_VELOCITY_DIRICHLET_SYMMETRIC_NITSCHE );
    tIWGVelocity->set_residual_dof_type( tVelDofTypes );
    tIWGVelocity->set_dof_type_list( { tVelDofTypes, tPDofTypes, tVisDofTypes }, mtk::Master_Slave::MASTER );
    tIWGVelocity->set_property( tPropVelocity, "Dirichlet" );
    tIWGVelocity->set_constitutive_model( tCMMasterIncFluid, "IncompressibleFluid" );
    tIWGVelocity->set_constitutive_model( tCMMasterTurbulence, "TurbulenceFluid" );
    tIWGVelocity->set_stabilization_parameter( tSPNitsche, "DirichletNitsche" );

    // set a fem set pointer
    MSI::Equation_Set * tSet = new fem::Set();
    tIWGVelocity->set_set_pointer( static_cast< fem::Set* >( tSet ) );

    // set size for the set EqnObjDofTypeList
    tIWGVelocity->mSet->mUniqueDofTypeList.resize( 100, MSI::Dof_Type::END_ENUM );

    // set size and populate the set dof type map
    tIWGVelocity->mSet->mUniqueDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWGVelocity->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) )        = 0;
    tIWGVelocity->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::P ) )         = 1;
    tIWGVelocity->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::VISCOSITY ) ) = 2;

    // set size and populate the set master dof type map
    tIWGVelocity->mSet->mMasterDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWGVelocity->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) )        = 0;
    tIWGVelocity->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::P ) )         = 1;
    tIWGVelocity->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::VISCOSITY ) ) = 2;

    // build global dof type list
    tCMMasterIncFluid->get_global_dof_type_list();
    tCMMasterTurbulence->get_global_dof_type_list();
    tSPNitsche->get_global_dof_type_list();
    tIWGVelocity->get_global_dof_type_list();

    // loop on the space dimension
    for( uint iSpaceDim = 2; iSpaceDim < 4; iSpaceDim++ )
    {
        // create the normal
        Matrix< DDRMat > tNormal( iSpaceDim, 1, 0.5 );
        tNormal = tNormal / norm( tNormal );

        // velocity
        Matrix< DDRMat > tVelocity( iSpaceDim, 1, 10.0 );

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
        Interpolation_Rule tGIRule( tGeometryType,
                Interpolation_Type::LAGRANGE,
                mtk::Interpolation_Order::LINEAR,
                Interpolation_Type::LAGRANGE,
                mtk::Interpolation_Order::LINEAR );

        // create a space time geometry interpolator
        Geometry_Interpolator tGI = Geometry_Interpolator( tGIRule );

        // create time coeff tHat
        Matrix< DDRMat > tTHat = {{ 0.0 }, { 1.0 }};

        // set the coefficients xHat, tHat
        tGI.set_coeff( tXHat, tTHat );

        // set space dimensions for property, CM and SP
        tPropVelocity->set_parameters( { tVelocity } );
        tCMMasterIncFluid->set_space_dim( iSpaceDim );
        tCMMasterTurbulence->set_space_dim( iSpaceDim );

        // set the normal
        tIWGVelocity->set_normal( tNormal );

        // loop on the interpolation order
        for( uint iInterpOrder = 1; iInterpOrder < 4; iInterpOrder++ )
        {
            // integration points
            //------------------------------------------------------------------------------
            // get an integration order
            fem::Integration_Order tIntegrationOrder = tIntegrationOrders( iSpaceDim - 2 );

            // create an integration rule
            fem::Integration_Rule tIntegrationRule(
                    tGeometryType,
                    Integration_Type::GAUSS,
                    tIntegrationOrder,
                    mtk::Geometry_Type::LINE,
                    Integration_Type::GAUSS,
                    fem::Integration_Order::BAR_1 );

            // create an integrator
            fem::Integrator tIntegrator( tIntegrationRule );

            // get integration points
            Matrix< DDRMat > tIntegPoints;
            tIntegrator.get_points( tIntegPoints );

            // field interpolators
            //------------------------------------------------------------------------------
            // create an interpolation order
            mtk::Interpolation_Order tInterpolationOrder = tInterpolationOrders( iInterpOrder - 1 );

            // number of dof for interpolation order
            uint tNumCoeff = tNumCoeffs( iSpaceDim - 2, iInterpOrder - 1 );

            // get number of dof
            int tNumDofVel = tNumCoeff * iSpaceDim;
            int tNumDofP   = tNumCoeff;
            int tNumDofVis = tNumCoeff;

            //create a space time interpolation rule
            Interpolation_Rule tFIRule (
                    tGeometryType,
                    Interpolation_Type::LAGRANGE,
                    tInterpolationOrder,
                    Interpolation_Type::LAGRANGE,
                    mtk::Interpolation_Order::LINEAR );

            // fill random coefficients for master FI
            Matrix< DDRMat > tMasterDOFHatVel;
            fill_uhat( tMasterDOFHatVel, iSpaceDim, iInterpOrder );
            Matrix< DDRMat > tMasterDOFHatP;
            fill_phat( tMasterDOFHatP, iSpaceDim, iInterpOrder );
            Matrix< DDRMat > tMasterDOFHatVis;
            fill_phat( tMasterDOFHatVis, iSpaceDim, iInterpOrder );

            // create a cell of field interpolators for IWG
            Cell< Field_Interpolator* > tMasterFIs( tDofTypes.size() );

            // create the field interpolator velocity
            tMasterFIs( 0 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tVelDofTypes );
            tMasterFIs( 0 )->set_coeff( tMasterDOFHatVel );

            // create the field interpolator pressure
            tMasterFIs( 1 ) = new Field_Interpolator( 1, tFIRule, &tGI, tPDofTypes );
            tMasterFIs( 1 )->set_coeff( tMasterDOFHatP );

            // create the field interpolator viscosity
            tMasterFIs( 2 ) = new Field_Interpolator( 1, tFIRule, &tGI, tVisDofTypes );
            tMasterFIs( 2 )->set_coeff( tMasterDOFHatVis );

            // set size and fill the set residual assembly map
            tIWGVelocity->mSet->mResDofAssemblyMap.resize( tDofTypes.size() );
            tIWGVelocity->mSet->mResDofAssemblyMap( 0 ) = { { 0, tNumDofVel-1 } };
            tIWGVelocity->mSet->mResDofAssemblyMap( 1 ) = { { tNumDofVel, tNumDofVel + tNumDofP - 1 } };
            tIWGVelocity->mSet->mResDofAssemblyMap( 2 ) = { { tNumDofVel + tNumDofP, tNumDofVel + tNumDofP + tNumDofVis - 1 } };

            // set size and fill the set jacobian assembly map
            Matrix< DDSMat > tJacAssembly = {
                    { 0, tNumDofVel - 1 },
                    { tNumDofVel, tNumDofVel + tNumDofP - 1 },
                    { tNumDofVel + tNumDofP, tNumDofVel + tNumDofP + tNumDofVis - 1 } };
            tIWGVelocity->mSet->mJacDofAssemblyMap.resize( tDofTypes.size() );
            tIWGVelocity->mSet->mJacDofAssemblyMap( 0 ) = tJacAssembly;
            tIWGVelocity->mSet->mJacDofAssemblyMap( 1 ) = tJacAssembly;
            tIWGVelocity->mSet->mJacDofAssemblyMap( 2 ) = tJacAssembly;

            // set size and init the set residual and jacobian
            tIWGVelocity->mSet->mResidual.resize( 1 );
            tIWGVelocity->mSet->mResidual( 0 ).set_size( tNumDofVel + tNumDofP + tNumDofVis, 1, 0.0 );
            tIWGVelocity->mSet->mJacobian.set_size( tNumDofVel + tNumDofP + tNumDofVis, tNumDofVel + tNumDofP + tNumDofVis, 0.0 );

            // populate the requested master dof type
            tIWGVelocity->mRequestedMasterGlobalDofTypes = tDofTypes;

            // create a field interpolator manager
            moris::Cell< moris::Cell< enum PDV_Type > > tDummyDv;
            Field_Interpolator_Manager tFIManager( tDofTypes, tDummyDv, tSet );

            // populate the field interpolator manager
            tFIManager.mFI = tMasterFIs;
            tFIManager.mIPGeometryInterpolator = &tGI;
            tFIManager.mIGGeometryInterpolator = &tGI;

            // set the interpolator manager to the set
            tIWGVelocity->mSet->mMasterFIManager = &tFIManager;

            // set IWG field interpolator manager
            tIWGVelocity->set_field_interpolator_manager( &tFIManager );

            uint tNumGPs = tIntegPoints.n_cols();
            for( uint iGP = 0; iGP < tNumGPs; iGP ++ )
            {
                // reset IWG evaluation flags
                tIWGVelocity->reset_eval_flags();

                // create evaluation point xi, tau
                Matrix< DDRMat > tParamPoint = tIntegPoints.get_column( iGP );

                // set integration point
                tIWGVelocity->mSet->mMasterFIManager->set_space_time( tParamPoint );

                // check evaluation of the residual for IWG
                //------------------------------------------------------------------------------
                // reset residual
                tIWGVelocity->mSet->mResidual( 0 ).fill( 0.0 );

                // compute residual
                tIWGVelocity->compute_residual( 1.0 );

                // check evaluation of the jacobian by FD
                //------------------------------------------------------------------------------
                // reset jacobian
                tIWGVelocity->mSet->mJacobian.fill( 0.0 );

                // init the jacobian for IWG and FD evaluation
                Matrix< DDRMat > tJacobian;
                Matrix< DDRMat > tJacobianFD;

                // check jacobian by FD
                bool tCheckJacobian = tIWGVelocity->check_jacobian(
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
        }
    }
}/*END_TEST_CASE*/

TEST_CASE( "IWG_Incompressible_NS_Dirichlet_Velocity_Unsymmetric_Nitsche", "[IWG_Incompressible_NS_Dirichlet_Velocity_Unsymmetric_Nitsche]" )
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
     moris::Cell< fem::Integration_Order > tIntegrationOrders = {
             fem::Integration_Order::QUAD_2x2,
             fem::Integration_Order::HEX_2x2x2 };

    // create list with number of coeffs
    Matrix< DDRMat > tNumCoeffs = {{ 8, 18, 32 },{ 16, 54, 128 }};

    // dof type list
    moris::Cell< MSI::Dof_Type > tVelDofTypes  = { MSI::Dof_Type::VX };
    moris::Cell< MSI::Dof_Type > tPDofTypes    = { MSI::Dof_Type::P };
    moris::Cell< MSI::Dof_Type > tVisDofTypes  = { MSI::Dof_Type::VISCOSITY };
    moris::Cell< moris::Cell< MSI::Dof_Type > > tDofTypes = { tVelDofTypes, tPDofTypes, tVisDofTypes };

    // create the properties
    std::shared_ptr< fem::Property > tPropViscosity = std::make_shared< fem::Property >();
    tPropViscosity->set_parameters( { {{ 1.0 }} } );
    tPropViscosity->set_val_function( tConstValFunc );
    //tPropViscosity->set_dof_type_list( { tPDofTypes } );
    //tPropViscosity->set_val_function( tPFIValFunc );
    //tPropViscosity->set_dof_derivative_functions( { tPFIDerFunc } );

    std::shared_ptr< fem::Property > tPropDensity = std::make_shared< fem::Property >();
    tPropDensity->set_parameters( { {{ 1.0 }} } );
    tPropDensity->set_val_function( tConstValFunc );
    //tPropDensity->set_dof_type_list( { tPDofTypes } );
    //tPropDensity->set_val_function( tPFIValFunc );
    //tPropDensity->set_dof_derivative_functions( { tPFIDerFunc } );

    std::shared_ptr< fem::Property > tPropVelocity = std::make_shared< fem::Property >();
    tPropVelocity->set_val_function( tConstValFunc );
    //tPropVelocity->set_dof_type_list( { tPDofTypes } );
    //tPropVelocity->set_val_function( tPFIValFunc );
    //tPropVelocity->set_dof_derivative_functions( { tPFIDerFunc } );

    // define constitutive models
    fem::CM_Factory tCMFactory;

    std::shared_ptr< fem::Constitutive_Model > tCMMasterIncFluid =
            tCMFactory.create_CM( fem::Constitutive_Type::FLUID_INCOMPRESSIBLE );
    tCMMasterIncFluid->set_dof_type_list( { tVelDofTypes, tPDofTypes } );
    tCMMasterIncFluid->set_property( tPropViscosity, "Viscosity" );
    tCMMasterIncFluid->set_property( tPropDensity, "Density" );

    std::shared_ptr< fem::Constitutive_Model > tCMMasterTurbulence =
            tCMFactory.create_CM( fem::Constitutive_Type::FLUID_TURBULENCE );
    tCMMasterTurbulence->set_dof_type_list( { tVelDofTypes, tVisDofTypes } );
    tCMMasterTurbulence->set_property( tPropViscosity, "Viscosity" );
    tCMMasterTurbulence->set_property( tPropDensity, "Density" );

    // define stabilization parameters
    fem::SP_Factory tSPFactory;

    std::shared_ptr< fem::Stabilization_Parameter > tSPNitsche =
            tSPFactory.create_SP( fem::Stabilization_Type::VELOCITY_DIRICHLET_NITSCHE );
    tSPNitsche->set_dof_type_list( { tVelDofTypes }, mtk::Master_Slave::MASTER );
    tSPNitsche->set_property( tPropDensity, "Density", mtk::Master_Slave::MASTER );
    tSPNitsche->set_property( tPropViscosity, "Viscosity", mtk::Master_Slave::MASTER );
    tSPNitsche->set_parameters( { {{ 1.0 }}, {{ 1.0 }} } );

    // define the IWGs
    fem::IWG_Factory tIWGFactory;

    std::shared_ptr< fem::IWG > tIWGVelocity =
            tIWGFactory.create_IWG( fem::IWG_Type::INCOMPRESSIBLE_NS_VELOCITY_DIRICHLET_UNSYMMETRIC_NITSCHE );
    tIWGVelocity->set_residual_dof_type( tVelDofTypes );
    tIWGVelocity->set_dof_type_list( { tVelDofTypes, tPDofTypes, tVisDofTypes }, mtk::Master_Slave::MASTER );
    tIWGVelocity->set_property( tPropVelocity, "Dirichlet" );
    tIWGVelocity->set_constitutive_model( tCMMasterIncFluid, "IncompressibleFluid" );
    tIWGVelocity->set_constitutive_model( tCMMasterTurbulence, "TurbulenceFluid" );
    tIWGVelocity->set_stabilization_parameter( tSPNitsche, "DirichletNitsche" );

    // set a fem set pointer
    MSI::Equation_Set * tSet = new fem::Set();
    tIWGVelocity->set_set_pointer( static_cast< fem::Set* >( tSet ) );

    // set size for the set EqnObjDofTypeList
    tIWGVelocity->mSet->mUniqueDofTypeList.resize( 100, MSI::Dof_Type::END_ENUM );

    // set size and populate the set dof type map
    tIWGVelocity->mSet->mUniqueDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWGVelocity->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) )        = 0;
    tIWGVelocity->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::P ) )         = 1;
    tIWGVelocity->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::VISCOSITY ) ) = 2;

    // set size and populate the set master dof type map
    tIWGVelocity->mSet->mMasterDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWGVelocity->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) )        = 0;
    tIWGVelocity->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::P ) )         = 1;
    tIWGVelocity->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::VISCOSITY ) ) = 2;

    // loop on the space dimension
    for( uint iSpaceDim = 2; iSpaceDim < 4; iSpaceDim++ )
    {
        // create the normal
        Matrix< DDRMat > tNormal( iSpaceDim, 1, 0.5 );
        tNormal = tNormal / norm( tNormal );

        // velocity
        Matrix< DDRMat > tVelocity( iSpaceDim, 1, 10.0 );

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
        Interpolation_Rule tGIRule( tGeometryType,
                Interpolation_Type::LAGRANGE,
                mtk::Interpolation_Order::LINEAR,
                Interpolation_Type::LAGRANGE,
                mtk::Interpolation_Order::LINEAR );

        // create a space time geometry interpolator
        Geometry_Interpolator tGI = Geometry_Interpolator( tGIRule );

        // create time coeff tHat
        Matrix< DDRMat > tTHat = {{ 0.0 }, { 1.0 }};

        // set the coefficients xHat, tHat
        tGI.set_coeff( tXHat, tTHat );

        // set space dimensions for property, CM and SP
        tPropVelocity->set_parameters( { tVelocity } );
        tCMMasterIncFluid->set_space_dim( iSpaceDim );
        tCMMasterTurbulence->set_space_dim( iSpaceDim );

        // set the normal
        tIWGVelocity->set_normal( tNormal );

        // loop on the interpolation order
        for( uint iInterpOrder = 1; iInterpOrder < 4; iInterpOrder++ )
        {
            // integration points
            //------------------------------------------------------------------------------
            // get an integration order
            fem::Integration_Order tIntegrationOrder   = tIntegrationOrders( iSpaceDim - 2 );

            // create an integration rule
            fem::Integration_Rule tIntegrationRule(
                    tGeometryType,
                    Integration_Type::GAUSS,
                    tIntegrationOrder,
                    mtk::Geometry_Type::LINE,
                    Integration_Type::GAUSS,
                    fem::Integration_Order::BAR_1 );

            // create an integrator
            fem::Integrator tIntegrator( tIntegrationRule );

            // get integration points
            Matrix< DDRMat > tIntegPoints;
            tIntegrator.get_points( tIntegPoints );

            // field interpolators
            //------------------------------------------------------------------------------
            // create an interpolation order
            mtk::Interpolation_Order tInterpolationOrder = tInterpolationOrders( iInterpOrder - 1 );

            // number of dof for interpolation order
            uint tNumCoeff = tNumCoeffs( iSpaceDim - 2, iInterpOrder - 1 );

            // get number of dof
            int tNumDofVel = tNumCoeff * iSpaceDim;
            int tNumDofP   = tNumCoeff;
            int tNumDofVis = tNumCoeff;

            //create a space time interpolation rule
            Interpolation_Rule tFIRule (
                    tGeometryType,
                    Interpolation_Type::LAGRANGE,
                    tInterpolationOrder,
                    Interpolation_Type::LAGRANGE,
                    mtk::Interpolation_Order::LINEAR );

            // fill random coefficients for master FI
            Matrix< DDRMat > tMasterDOFHatVel;
            fill_uhat( tMasterDOFHatVel, iSpaceDim, iInterpOrder );
            Matrix< DDRMat > tMasterDOFHatP;
            fill_phat( tMasterDOFHatP, iSpaceDim, iInterpOrder );
            Matrix< DDRMat > tMasterDOFHatVis;
            fill_phat( tMasterDOFHatVis, iSpaceDim, iInterpOrder );

            // create a cell of field interpolators for IWG
            Cell< Field_Interpolator* > tMasterFIs( tDofTypes.size() );

            // create the field interpolator velocity
            tMasterFIs( 0 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tVelDofTypes );
            tMasterFIs( 0 )->set_coeff( tMasterDOFHatVel );

            // create the field interpolator pressure
            tMasterFIs( 1 ) = new Field_Interpolator( 1, tFIRule, &tGI, tPDofTypes );
            tMasterFIs( 1 )->set_coeff( tMasterDOFHatP );

            // create the field interpolator viscosity
            tMasterFIs( 2 ) = new Field_Interpolator( 1, tFIRule, &tGI, tVisDofTypes );
            tMasterFIs( 2 )->set_coeff( tMasterDOFHatVis );

            // set size and fill the set residual assembly map
            tIWGVelocity->mSet->mResDofAssemblyMap.resize( tDofTypes.size() );
            tIWGVelocity->mSet->mResDofAssemblyMap( 0 ) = { { 0, tNumDofVel-1 } };
            tIWGVelocity->mSet->mResDofAssemblyMap( 1 ) = { { tNumDofVel, tNumDofVel + tNumDofP - 1 } };
            tIWGVelocity->mSet->mResDofAssemblyMap( 2 ) = { { tNumDofVel + tNumDofP, tNumDofVel + tNumDofP + tNumDofVis - 1 } };

            // set size and fill the set jacobian assembly map
            Matrix< DDSMat > tJacAssembly = {
                    { 0, tNumDofVel - 1 },
                    { tNumDofVel, tNumDofVel + tNumDofP - 1 },
                    { tNumDofVel + tNumDofP, tNumDofVel + tNumDofP + tNumDofVis - 1 } };
            tIWGVelocity->mSet->mJacDofAssemblyMap.resize( tDofTypes.size() );
            tIWGVelocity->mSet->mJacDofAssemblyMap( 0 ) = tJacAssembly;
            tIWGVelocity->mSet->mJacDofAssemblyMap( 1 ) = tJacAssembly;
            tIWGVelocity->mSet->mJacDofAssemblyMap( 2 ) = tJacAssembly;

            // set size and init the set residual and jacobian
            tIWGVelocity->mSet->mResidual.resize( 1 );
            tIWGVelocity->mSet->mResidual( 0 ).set_size( tNumDofVel + tNumDofP + tNumDofVis, 1, 0.0 );
            tIWGVelocity->mSet->mJacobian.set_size( tNumDofVel + tNumDofP + tNumDofVis, tNumDofVel + tNumDofP + tNumDofVis, 0.0 );

            // build global dof type list
            tIWGVelocity->get_global_dof_type_list();

            // populate the requested master dof type
            tIWGVelocity->mRequestedMasterGlobalDofTypes = tDofTypes;

            // create a field interpolator manager
            moris::Cell< moris::Cell< enum PDV_Type > > tDummyDv;
            Field_Interpolator_Manager tFIManager( tDofTypes, tDummyDv, tSet );

            // populate the field interpolator manager
            tFIManager.mFI = tMasterFIs;
            tFIManager.mIPGeometryInterpolator = &tGI;
            tFIManager.mIGGeometryInterpolator = &tGI;

            // set the interpolator manager to the set
            tIWGVelocity->mSet->mMasterFIManager = &tFIManager;

            // set IWG field interpolator manager
            tIWGVelocity->set_field_interpolator_manager( &tFIManager );

            uint tNumGPs = tIntegPoints.n_cols();
            for( uint iGP = 0; iGP < tNumGPs; iGP ++ )
            {
                // reset IWG evaluation flags
                tIWGVelocity->reset_eval_flags();

                // create evaluation point xi, tau
                Matrix< DDRMat > tParamPoint = tIntegPoints.get_column( iGP );

                // set integration point
                tIWGVelocity->mSet->mMasterFIManager->set_space_time( tParamPoint );

                // check evaluation of the residual for IWG
                //------------------------------------------------------------------------------
                // reset residual
                tIWGVelocity->mSet->mResidual( 0 ).fill( 0.0 );

                // compute residual
                tIWGVelocity->compute_residual( 1.0 );

                // check evaluation of the jacobian by FD
                //------------------------------------------------------------------------------
                // reset jacobian
                tIWGVelocity->mSet->mJacobian.fill( 0.0 );

                // init the jacobian for IWG and FD evaluation
                Matrix< DDRMat > tJacobian;
                Matrix< DDRMat > tJacobianFD;

                // check jacobian by FD
                bool tCheckJacobian = tIWGVelocity->check_jacobian(
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
        }
    }
}/*END_TEST_CASE*/

TEST_CASE( "IWG_Incompressible_NS_Dirichlet_Pressure_Symmetric_Nitsche", "[IWG_Incompressible_NS_Dirichlet_Pressure_Symmetric_Nitsche]" )
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
     moris::Cell< fem::Integration_Order > tIntegrationOrders = {
             fem::Integration_Order::QUAD_2x2,
             fem::Integration_Order::HEX_2x2x2 };

    // create list with number of coeffs
    Matrix< DDRMat > tNumCoeffs = {{ 8, 18, 32 },{ 16, 54, 128 }};

    // dof type list
    moris::Cell< MSI::Dof_Type > tVelDofTypes  = { MSI::Dof_Type::VX };
    moris::Cell< MSI::Dof_Type > tPDofTypes    = { MSI::Dof_Type::P };
    moris::Cell< MSI::Dof_Type > tVisDofTypes  = { MSI::Dof_Type::VISCOSITY };
    moris::Cell< moris::Cell< MSI::Dof_Type > > tDofTypes = { tVelDofTypes, tPDofTypes, tVisDofTypes };

    // create the properties
    std::shared_ptr< fem::Property > tPropViscosity = std::make_shared< fem::Property >();
    tPropViscosity->set_parameters( { {{ 1.0 }} } );
    tPropViscosity->set_val_function( tConstValFunc );
    //tPropViscosity->set_dof_type_list( { tPDofTypes } );
    //tPropViscosity->set_val_function( tPFIValFunc );
    //tPropViscosity->set_dof_derivative_functions( { tPFIDerFunc } );

    std::shared_ptr< fem::Property > tPropDensity = std::make_shared< fem::Property >();
    tPropDensity->set_parameters( { {{ 1.0 }} } );
    tPropDensity->set_val_function( tConstValFunc );
    //tPropDensity->set_dof_type_list( { tPDofTypes } );
    //tPropDensity->set_val_function( tPFIValFunc );
    //tPropDensity->set_dof_derivative_functions( { tPFIDerFunc } );

    std::shared_ptr< fem::Property > tPropVelocity = std::make_shared< fem::Property >();
    tPropVelocity->set_val_function( tConstValFunc );
    //tPropVelocity->set_dof_type_list( { tPDofTypes } );
    //tPropVelocity->set_val_function( tPFIValFunc );
    //tPropVelocity->set_dof_derivative_functions( { tPFIDerFunc } );

    // define constitutive models
    fem::CM_Factory tCMFactory;

    std::shared_ptr< fem::Constitutive_Model > tCMMasterIncFluid =
            tCMFactory.create_CM( fem::Constitutive_Type::FLUID_INCOMPRESSIBLE );
    tCMMasterIncFluid->set_dof_type_list( { tVelDofTypes, tPDofTypes } );
    tCMMasterIncFluid->set_property( tPropViscosity, "Viscosity" );
    tCMMasterIncFluid->set_property( tPropDensity, "Density" );

    // define the IWGs
    fem::IWG_Factory tIWGFactory;

    std::shared_ptr< fem::IWG > tIWGPressure =
            tIWGFactory.create_IWG( fem::IWG_Type::INCOMPRESSIBLE_NS_PRESSURE_DIRICHLET_SYMMETRIC_NITSCHE );
    tIWGPressure->set_residual_dof_type( tPDofTypes );
    tIWGPressure->set_dof_type_list( { tVelDofTypes, tPDofTypes, tVisDofTypes }, mtk::Master_Slave::MASTER );
    tIWGPressure->set_property( tPropVelocity, "Dirichlet" );
    tIWGPressure->set_constitutive_model( tCMMasterIncFluid, "IncompressibleFluid" );

    // set a fem set pointer
    MSI::Equation_Set * tSet = new fem::Set();
    tIWGPressure->set_set_pointer( static_cast< fem::Set* >( tSet ) );

    // set size for the set EqnObjDofTypeList
    tIWGPressure->mSet->mUniqueDofTypeList.resize( 100, MSI::Dof_Type::END_ENUM );

    // set size and populate the set dof type map
    tIWGPressure->mSet->mUniqueDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWGPressure->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) )        = 0;
    tIWGPressure->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::P ) )         = 1;
    tIWGPressure->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::VISCOSITY ) ) = 2;

    // set size and populate the set master dof type map
    tIWGPressure->mSet->mMasterDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWGPressure->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) )        = 0;
    tIWGPressure->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::P ) )         = 1;
    tIWGPressure->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::VISCOSITY ) ) = 2;

    // loop on the space dimension
    for( uint iSpaceDim = 2; iSpaceDim < 4; iSpaceDim++ )
    {
        // create the normal
        Matrix< DDRMat > tNormal( iSpaceDim, 1, 0.5 );
        tNormal = tNormal / norm( tNormal );

        // velocity
        Matrix< DDRMat > tVelocity( iSpaceDim, 1, 10.0 );

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
        Interpolation_Rule tGIRule( tGeometryType,
                Interpolation_Type::LAGRANGE,
                mtk::Interpolation_Order::LINEAR,
                Interpolation_Type::LAGRANGE,
                mtk::Interpolation_Order::LINEAR );

        // create a space time geometry interpolator
        Geometry_Interpolator tGI = Geometry_Interpolator( tGIRule );

        // create time coeff tHat
        Matrix< DDRMat > tTHat = {{ 0.0 }, { 1.0 }};

        // set the coefficients xHat, tHat
        tGI.set_coeff( tXHat, tTHat );

        // set space dimensions for property, CM and SP
        tPropVelocity->set_parameters( { tVelocity } );
        tCMMasterIncFluid->set_space_dim( iSpaceDim );

        // set the normal
        tIWGPressure->set_normal( tNormal );

        // loop on the interpolation order
        for( uint iInterpOrder = 1; iInterpOrder < 4; iInterpOrder++ )
        {
            // integration points
            //------------------------------------------------------------------------------
            // get an integration order
            fem::Integration_Order tIntegrationOrder   = tIntegrationOrders( iSpaceDim - 2 );

            // create an integration rule
            fem::Integration_Rule tIntegrationRule(
                    tGeometryType,
                    Integration_Type::GAUSS,
                    tIntegrationOrder,
                    mtk::Geometry_Type::LINE,
                    Integration_Type::GAUSS,
                    fem::Integration_Order::BAR_1 );

            // create an integrator
            fem::Integrator tIntegrator( tIntegrationRule );

            // get integration points
            Matrix< DDRMat > tIntegPoints;
            tIntegrator.get_points( tIntegPoints );

            // field interpolators
            //------------------------------------------------------------------------------
            // create an interpolation order
            mtk::Interpolation_Order tInterpolationOrder = tInterpolationOrders( iInterpOrder - 1 );

            // number of dof for interpolation order
            uint tNumCoeff = tNumCoeffs( iSpaceDim - 2, iInterpOrder - 1 );

            // get number of dof
            int tNumDofVel = tNumCoeff * iSpaceDim;
            int tNumDofP   = tNumCoeff;
            int tNumDofVis = tNumCoeff;

            //create a space time interpolation rule
            Interpolation_Rule tFIRule (
                    tGeometryType,
                    Interpolation_Type::LAGRANGE,
                    tInterpolationOrder,
                    Interpolation_Type::LAGRANGE,
                    mtk::Interpolation_Order::LINEAR );

            // fill random coefficients for master FI
            Matrix< DDRMat > tMasterDOFHatVel;
            fill_uhat( tMasterDOFHatVel, iSpaceDim, iInterpOrder );
            Matrix< DDRMat > tMasterDOFHatP;
            fill_phat( tMasterDOFHatP, iSpaceDim, iInterpOrder );
            Matrix< DDRMat > tMasterDOFHatVis;
            fill_phat( tMasterDOFHatVis, iSpaceDim, iInterpOrder );

            // create a cell of field interpolators for IWG
            Cell< Field_Interpolator* > tMasterFIs( tDofTypes.size() );

            // create the field interpolator velocity
            tMasterFIs( 0 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tVelDofTypes );
            tMasterFIs( 0 )->set_coeff( tMasterDOFHatVel );

            // create the field interpolator pressure
            tMasterFIs( 1 ) = new Field_Interpolator( 1, tFIRule, &tGI, tPDofTypes );
            tMasterFIs( 1 )->set_coeff( tMasterDOFHatP );

            // create the field interpolator viscosity
            tMasterFIs( 2 ) = new Field_Interpolator( 1, tFIRule, &tGI, tVisDofTypes );
            tMasterFIs( 2 )->set_coeff( tMasterDOFHatVis );

            // set size and fill the set residual assembly map
            tIWGPressure->mSet->mResDofAssemblyMap.resize( tDofTypes.size() );
            tIWGPressure->mSet->mResDofAssemblyMap( 0 ) = { { 0, tNumDofVel-1 } };
            tIWGPressure->mSet->mResDofAssemblyMap( 1 ) = { { tNumDofVel, tNumDofVel + tNumDofP - 1 } };
            tIWGPressure->mSet->mResDofAssemblyMap( 2 ) = { { tNumDofVel + tNumDofP, tNumDofVel + tNumDofP + tNumDofVis - 1 } };

            // set size and fill the set jacobian assembly map
            Matrix< DDSMat > tJacAssembly = {
                    { 0, tNumDofVel - 1 },
                    { tNumDofVel, tNumDofVel + tNumDofP - 1 },
                    { tNumDofVel + tNumDofP, tNumDofVel + tNumDofP + tNumDofVis - 1 } };
            tIWGPressure->mSet->mJacDofAssemblyMap.resize( tDofTypes.size() );
            tIWGPressure->mSet->mJacDofAssemblyMap( 0 ) = tJacAssembly;
            tIWGPressure->mSet->mJacDofAssemblyMap( 1 ) = tJacAssembly;
            tIWGPressure->mSet->mJacDofAssemblyMap( 2 ) = tJacAssembly;

            // set size and init the set residual and jacobian
            tIWGPressure->mSet->mResidual.resize( 1 );
            tIWGPressure->mSet->mResidual( 0 ).set_size( tNumDofVel + tNumDofP + tNumDofVis, 1, 0.0 );
            tIWGPressure->mSet->mJacobian.set_size( tNumDofVel + tNumDofP + tNumDofVis, tNumDofVel + tNumDofP + tNumDofVis, 0.0 );

            // build global dof type list
            tIWGPressure->get_global_dof_type_list();

            // populate the requested master dof type
            tIWGPressure->mRequestedMasterGlobalDofTypes = tDofTypes;

            // create a field interpolator manager
            moris::Cell< moris::Cell< enum PDV_Type > > tDummyDv;
            Field_Interpolator_Manager tFIManager( tDofTypes, tDummyDv, tSet );

            // populate the field interpolator manager
            tFIManager.mFI = tMasterFIs;
            tFIManager.mIPGeometryInterpolator = &tGI;
            tFIManager.mIGGeometryInterpolator = &tGI;

            // set the interpolator manager to the set
            tIWGPressure->mSet->mMasterFIManager = &tFIManager;

            // set IWG field interpolator manager
            tIWGPressure->set_field_interpolator_manager( &tFIManager );

            uint tNumGPs = tIntegPoints.n_cols();
            for( uint iGP = 0; iGP < tNumGPs; iGP ++ )
            {
                // reset IWG evaluation flags
                tIWGPressure->reset_eval_flags();

                // create evaluation point xi, tau
                Matrix< DDRMat > tParamPoint = tIntegPoints.get_column( iGP );

                // set integration point
                tIWGPressure->mSet->mMasterFIManager->set_space_time( tParamPoint );

                // check evaluation of the residual for IWG
                //------------------------------------------------------------------------------
                // reset residual
                tIWGPressure->mSet->mResidual( 0 ).fill( 0.0 );

                // compute residual
                tIWGPressure->compute_residual( 1.0 );

                // check evaluation of the jacobian by FD
                //------------------------------------------------------------------------------
                // reset jacobian
                tIWGPressure->mSet->mJacobian.fill( 0.0 );

                // init the jacobian for IWG and FD evaluation
                Matrix< DDRMat > tJacobian;
                Matrix< DDRMat > tJacobianFD;

                // check jacobian by FD
                bool tCheckJacobian = tIWGPressure->check_jacobian(
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
        }
    }
}/*END_TEST_CASE*/

TEST_CASE( "IWG_Incompressible_NS_Dirichlet_Pressure_Unsymmetric_Nitsche", "[IWG_Incompressible_NS_Dirichlet_Pressure_Unsymmetric_Nitsche]" )
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
     moris::Cell< fem::Integration_Order > tIntegrationOrders = {
             fem::Integration_Order::QUAD_2x2,
             fem::Integration_Order::HEX_2x2x2 };

    // create list with number of coeffs
    Matrix< DDRMat > tNumCoeffs = {{ 8, 18, 32 },{ 16, 54, 128 }};

    // dof type list
    moris::Cell< MSI::Dof_Type > tVelDofTypes  = { MSI::Dof_Type::VX };
    moris::Cell< MSI::Dof_Type > tPDofTypes    = { MSI::Dof_Type::P };
    moris::Cell< MSI::Dof_Type > tVisDofTypes  = { MSI::Dof_Type::VISCOSITY };
    moris::Cell< moris::Cell< MSI::Dof_Type > > tDofTypes = { tVelDofTypes, tPDofTypes, tVisDofTypes };

    // create the properties
    std::shared_ptr< fem::Property > tPropViscosity = std::make_shared< fem::Property >();
    tPropViscosity->set_parameters( { {{ 1.0 }} } );
    tPropViscosity->set_val_function( tConstValFunc );
    //tPropViscosity->set_dof_type_list( { tPDofTypes } );
    //tPropViscosity->set_val_function( tPFIValFunc );
    //tPropViscosity->set_dof_derivative_functions( { tPFIDerFunc } );

    std::shared_ptr< fem::Property > tPropDensity = std::make_shared< fem::Property >();
    tPropDensity->set_parameters( { {{ 1.0 }} } );
    tPropDensity->set_val_function( tConstValFunc );
    //tPropDensity->set_dof_type_list( { tPDofTypes } );
    //tPropDensity->set_val_function( tPFIValFunc );
    //tPropDensity->set_dof_derivative_functions( { tPFIDerFunc } );

    std::shared_ptr< fem::Property > tPropVelocity = std::make_shared< fem::Property >();
    tPropVelocity->set_val_function( tConstValFunc );
    //tPropVelocity->set_dof_type_list( { tPDofTypes } );
    //tPropVelocity->set_val_function( tPFIValFunc );
    //tPropVelocity->set_dof_derivative_functions( { tPFIDerFunc } );

    // define constitutive models
    fem::CM_Factory tCMFactory;

    std::shared_ptr< fem::Constitutive_Model > tCMMasterIncFluid =
            tCMFactory.create_CM( fem::Constitutive_Type::FLUID_INCOMPRESSIBLE );
    tCMMasterIncFluid->set_dof_type_list( { tVelDofTypes, tPDofTypes } );
    tCMMasterIncFluid->set_property( tPropViscosity, "Viscosity" );
    tCMMasterIncFluid->set_property( tPropDensity, "Density" );

    // define the IWGs
    fem::IWG_Factory tIWGFactory;

    std::shared_ptr< fem::IWG > tIWGPressure =
            tIWGFactory.create_IWG( fem::IWG_Type::INCOMPRESSIBLE_NS_PRESSURE_DIRICHLET_UNSYMMETRIC_NITSCHE );
    tIWGPressure->set_residual_dof_type( tPDofTypes );
    tIWGPressure->set_dof_type_list( { tVelDofTypes, tPDofTypes, tVisDofTypes }, mtk::Master_Slave::MASTER );
    tIWGPressure->set_property( tPropVelocity, "Dirichlet" );
    tIWGPressure->set_constitutive_model( tCMMasterIncFluid, "IncompressibleFluid" );

    // set a fem set pointer
    MSI::Equation_Set * tSet = new fem::Set();
    tIWGPressure->set_set_pointer( static_cast< fem::Set* >( tSet ) );

    // set size for the set EqnObjDofTypeList
    tIWGPressure->mSet->mUniqueDofTypeList.resize( 100, MSI::Dof_Type::END_ENUM );

    // set size and populate the set dof type map
    tIWGPressure->mSet->mUniqueDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWGPressure->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) )        = 0;
    tIWGPressure->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::P ) )         = 1;
    tIWGPressure->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::VISCOSITY ) ) = 2;

    // set size and populate the set master dof type map
    tIWGPressure->mSet->mMasterDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tIWGPressure->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) )        = 0;
    tIWGPressure->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::P ) )         = 1;
    tIWGPressure->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::VISCOSITY ) ) = 2;

    // loop on the space dimension
    for( uint iSpaceDim = 2; iSpaceDim < 4; iSpaceDim++ )
    {
        // create the normal
        Matrix< DDRMat > tNormal( iSpaceDim, 1, 0.5 );
        tNormal = tNormal / norm( tNormal );

        // velocity
        Matrix< DDRMat > tVelocity( iSpaceDim, 1, 10.0 );

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
        Interpolation_Rule tGIRule( tGeometryType,
                Interpolation_Type::LAGRANGE,
                mtk::Interpolation_Order::LINEAR,
                Interpolation_Type::LAGRANGE,
                mtk::Interpolation_Order::LINEAR );

        // create a space time geometry interpolator
        Geometry_Interpolator tGI = Geometry_Interpolator( tGIRule );

        // create time coeff tHat
        Matrix< DDRMat > tTHat = {{ 0.0 }, { 1.0 }};

        // set the coefficients xHat, tHat
        tGI.set_coeff( tXHat, tTHat );

        // set space dimensions for property, CM and SP
        tPropVelocity->set_parameters( { tVelocity } );
        tCMMasterIncFluid->set_space_dim( iSpaceDim );

        // set the normal
        tIWGPressure->set_normal( tNormal );

        // loop on the interpolation order
        for( uint iInterpOrder = 1; iInterpOrder < 4; iInterpOrder++ )
        {
            // integration points
            //------------------------------------------------------------------------------
            // get an integration order
            fem::Integration_Order tIntegrationOrder   = tIntegrationOrders( iSpaceDim - 2 );

            // create an integration rule
            fem::Integration_Rule tIntegrationRule(
                    tGeometryType,
                    Integration_Type::GAUSS,
                    tIntegrationOrder,
                    mtk::Geometry_Type::LINE,
                    Integration_Type::GAUSS,
                    fem::Integration_Order::BAR_1 );

            // create an integrator
            fem::Integrator tIntegrator( tIntegrationRule );

            // get integration points
            Matrix< DDRMat > tIntegPoints;
            tIntegrator.get_points( tIntegPoints );

            // field interpolators
            //------------------------------------------------------------------------------
            // get an interpolation order
            mtk::Interpolation_Order tInterpolationOrder = tInterpolationOrders( iInterpOrder - 1 );

            // number of dof for interpolation order
            uint tNumCoeff = tNumCoeffs( iSpaceDim - 2, iInterpOrder - 1 );

            // get number of dof
            int tNumDofVel = tNumCoeff * iSpaceDim;
            int tNumDofP   = tNumCoeff;
            int tNumDofVis = tNumCoeff;

            //create a space time interpolation rule
            Interpolation_Rule tFIRule (
                    tGeometryType,
                    Interpolation_Type::LAGRANGE,
                    tInterpolationOrder,
                    Interpolation_Type::LAGRANGE,
                    mtk::Interpolation_Order::LINEAR );

            // fill coefficients for master FI
            Matrix< DDRMat > tMasterDOFHatVel;
            fill_uhat( tMasterDOFHatVel, iSpaceDim, iInterpOrder );
            Matrix< DDRMat > tMasterDOFHatP;
            fill_phat( tMasterDOFHatP, iSpaceDim, iInterpOrder );
            Matrix< DDRMat > tMasterDOFHatVis;
            fill_phat( tMasterDOFHatVis, iSpaceDim, iInterpOrder );

            // create a cell of field interpolators for IWG
            Cell< Field_Interpolator* > tMasterFIs( tDofTypes.size() );

            // create the field interpolator velocity
            tMasterFIs( 0 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tVelDofTypes );
            tMasterFIs( 0 )->set_coeff( tMasterDOFHatVel );

            // create the field interpolator pressure
            tMasterFIs( 1 ) = new Field_Interpolator( 1, tFIRule, &tGI, tPDofTypes );
            tMasterFIs( 1 )->set_coeff( tMasterDOFHatP );

            // create the field interpolator viscosity
            tMasterFIs( 2 ) = new Field_Interpolator( 1, tFIRule, &tGI, tVisDofTypes );
            tMasterFIs( 2 )->set_coeff( tMasterDOFHatVis );

            // set size and fill the set residual assembly map
            tIWGPressure->mSet->mResDofAssemblyMap.resize( tDofTypes.size() );
            tIWGPressure->mSet->mResDofAssemblyMap( 0 ) = { { 0, tNumDofVel-1 } };
            tIWGPressure->mSet->mResDofAssemblyMap( 1 ) = { { tNumDofVel, tNumDofVel + tNumDofP - 1 } };
            tIWGPressure->mSet->mResDofAssemblyMap( 2 ) = { { tNumDofVel + tNumDofP, tNumDofVel + tNumDofP + tNumDofVis - 1 } };

            // set size and fill the set jacobian assembly map
            Matrix< DDSMat > tJacAssembly = {
                    { 0, tNumDofVel - 1 },
                    { tNumDofVel, tNumDofVel + tNumDofP - 1 },
                    { tNumDofVel + tNumDofP, tNumDofVel + tNumDofP + tNumDofVis - 1 } };
            tIWGPressure->mSet->mJacDofAssemblyMap.resize( tDofTypes.size() );
            tIWGPressure->mSet->mJacDofAssemblyMap( 0 ) = tJacAssembly;
            tIWGPressure->mSet->mJacDofAssemblyMap( 1 ) = tJacAssembly;
            tIWGPressure->mSet->mJacDofAssemblyMap( 2 ) = tJacAssembly;

            // set size and init the set residual and jacobian
            tIWGPressure->mSet->mResidual.resize( 1 );
            tIWGPressure->mSet->mResidual( 0 ).set_size( tNumDofVel + tNumDofP + tNumDofVis, 1, 0.0 );
            tIWGPressure->mSet->mJacobian.set_size( tNumDofVel + tNumDofP + tNumDofVis, tNumDofVel + tNumDofP + tNumDofVis, 0.0 );

            // build global dof type list
            tIWGPressure->get_global_dof_type_list();

            // populate the requested master dof type
            tIWGPressure->mRequestedMasterGlobalDofTypes = tDofTypes;

            // create a field interpolator manager
            moris::Cell< moris::Cell< enum PDV_Type > > tDummyDv;
            Field_Interpolator_Manager tFIManager( tDofTypes, tDummyDv, tSet );

            // populate the field interpolator manager
            tFIManager.mFI = tMasterFIs;
            tFIManager.mIPGeometryInterpolator = &tGI;
            tFIManager.mIGGeometryInterpolator = &tGI;

            // set the interpolator manager to the set
            tIWGPressure->mSet->mMasterFIManager = &tFIManager;

            // set IWG field interpolator manager
            tIWGPressure->set_field_interpolator_manager( &tFIManager );

            uint tNumGPs = tIntegPoints.n_cols();
            for( uint iGP = 0; iGP < tNumGPs; iGP ++ )
            {
                // reset IWG evaluation flags
                tIWGPressure->reset_eval_flags();

                // create evaluation point xi, tau
                Matrix< DDRMat > tParamPoint = tIntegPoints.get_column( iGP );

                // set integration point
                tIWGPressure->mSet->mMasterFIManager->set_space_time( tParamPoint );

                // check evaluation of the residual for IWG
                //------------------------------------------------------------------------------
                // reset residual
                tIWGPressure->mSet->mResidual( 0 ).fill( 0.0 );

                // compute residual
                tIWGPressure->compute_residual( 1.0 );

                // check evaluation of the jacobian by FD
                //------------------------------------------------------------------------------
                // reset jacobian
                tIWGPressure->mSet->mJacobian.fill( 0.0 );

                // init the jacobian for IWG and FD evaluation
                Matrix< DDRMat > tJacobian;
                Matrix< DDRMat > tJacobianFD;

                // check jacobian by FD
                bool tCheckJacobian = tIWGPressure->check_jacobian(
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
        }
    }
}/*END_TEST_CASE*/
