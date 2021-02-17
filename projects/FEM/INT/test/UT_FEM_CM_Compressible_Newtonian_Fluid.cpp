
#include "catch.hpp"

#define protected public
#define private   public
//FEM/INT/src
#include "cl_FEM_Field_Interpolator_Manager.hpp"
#include "cl_FEM_Constitutive_Model.hpp"
#include "cl_FEM_Set.hpp"
#undef protected
#undef private

//LINALG/src
#include "fn_equal_to.hpp"
#include "fn_norm.hpp"
//FEM/INT/src
#include "cl_FEM_Field_Interpolator.hpp"
#include "IG/cl_MTK_Integrator.hpp"
#include "cl_FEM_Property.hpp"
#include "cl_FEM_MM_Factory.hpp"
#include "cl_FEM_CM_Factory.hpp"
#include "fn_FEM_Check.hpp"
#include "FEM_Test_Proxy/cl_FEM_Inputs_for_NS_Compressible_UT.cpp"

using namespace moris;
using namespace fem;

TEST_CASE( "CM_Compressible_Newtonian_Fluid_Density_Primitive", "[CM_Compressible_Newtonian_Fluid_Density_Primitive]" )
{
    // define an epsilon environment
    real tEpsilon = 1.0E-6;
    real tEpsilonCubic = 1.0E-5;

    // define a perturbation relative size
    real tPerturbation = 2.0E-4;
    real tPerturbationCubic = 3.0E-4;

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
    //Matrix< DDRMat > tNumCoeffs = {{ 8, 18 },{ 16, 54 }};
    Matrix< DDRMat > tNumCoeffs = {{ 8, 18, 32 },{ 16, 54, 128 }};

    // dof type list
    moris::Cell< MSI::Dof_Type > tDensityDof  = { MSI::Dof_Type::RHO };
    moris::Cell< MSI::Dof_Type > tVelocityDof = { MSI::Dof_Type::VX };
    moris::Cell< MSI::Dof_Type > tTempDof     = { MSI::Dof_Type::TEMP };
    moris::Cell< moris::Cell< MSI::Dof_Type > > tDofTypes = { tDensityDof, tVelocityDof, tTempDof };

    //------------------------------------------------------------------------------
    // create the properties

    // dynamic viscosity
    std::shared_ptr< fem::Property > tPropViscosity = std::make_shared< fem::Property >();
    tPropViscosity->set_parameters( { {{ 11.9 }} } );
    tPropViscosity->set_val_function( tConstValFunc );

    // isochoric heat capacity
    std::shared_ptr< fem::Property > tPropHeatCapacity = std::make_shared< fem::Property >();
    tPropHeatCapacity->set_parameters( { {{ 5.7 }} } );
    tPropHeatCapacity->set_val_function( tConstValFunc );

    // specific gas constant
    std::shared_ptr< fem::Property > tPropGasConstant = std::make_shared< fem::Property >();
    tPropGasConstant->set_parameters( { {{ 2.8 }} } );
    tPropGasConstant->set_val_function( tConstValFunc );

    // thermal conductivity
    std::shared_ptr< fem::Property > tPropConductivity = std::make_shared< fem::Property >();
    tPropConductivity->set_parameters( { {{ 3.7 }} } );
    tPropConductivity->set_val_function( tConstValFunc );

    // define material model and assign properties
    fem::MM_Factory tMMFactory;

    std::shared_ptr< fem::Material_Model > tMMFluid =
            tMMFactory.create_CM( fem::Material_Type::PERFECT_GAS );
    tMMFluid->set_dof_type_list( {tDensityDof, tTempDof } );
    tMMFluid->set_property( tPropHeatCapacity, "IsochoricHeatCapacity" );
    tMMFluid->set_property( tPropGasConstant,  "SpecificGasConstant" );

    // define constitutive model and assign properties
    fem::CM_Factory tCMFactory;

    std::shared_ptr< fem::Constitutive_Model > tCMMasterFluid =
            tCMFactory.create_CM( fem::Constitutive_Type::FLUID_COMPRESSIBLE_NEWTONIAN );
    tCMMasterFluid->set_dof_type_list( {tDensityDof, tVelocityDof, tTempDof } );
    tCMMasterFluid->set_property( tPropViscosity,    "DynamicViscosity" );
    tCMMasterFluid->set_property( tPropConductivity, "ThermalConductivity" );
    tCMMasterFluid->set_material_model( tMMFluid, "ThermodynamicMaterialModel" );

    // set a fem set pointer
    MSI::Equation_Set * tSet = new fem::Set();
    tMMFluid->set_set_pointer( static_cast< fem::Set* >( tSet ) );
    tCMMasterFluid->set_set_pointer( static_cast< fem::Set* >( tSet ) );

    // set size for the set EqnObjDofTypeList
    tMMFluid->mSet->mUniqueDofTypeList.resize( 100, MSI::Dof_Type::END_ENUM );
    tCMMasterFluid->mSet->mUniqueDofTypeList.resize( 100, MSI::Dof_Type::END_ENUM );

    // set size and populate the set dof type map
    tMMFluid->mSet->mUniqueDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tMMFluid->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::RHO ) )   = 0;
    tMMFluid->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::TEMP ) )  = 1;

    tCMMasterFluid->mSet->mUniqueDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tCMMasterFluid->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::RHO ) )   = 0;
    tCMMasterFluid->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) )    = 1;
    tCMMasterFluid->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::TEMP ) )  = 2;

    // set size and populate the set master dof type map
    tMMFluid->mSet->mMasterDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tMMFluid->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::RHO ) )   = 0;
    tMMFluid->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::TEMP ) )  = 1;

    tCMMasterFluid->mSet->mMasterDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tCMMasterFluid->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::RHO ) )   = 0;
    tCMMasterFluid->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) )    = 1;
    tCMMasterFluid->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::TEMP ) )  = 2;    

    // build global dof type list
    tMMFluid->get_global_dof_type_list();
    tCMMasterFluid->get_global_dof_type_list();

    // loop on the space dimension
    for( uint iSpaceDim = 2; iSpaceDim < 4; iSpaceDim++ )
    {
        // output for debugging
        // std::cout << "-------------------------------------------------------------------\n" << std::flush;
        // std::cout << "Performing Tests For Number of Spatial dimensions: " << iSpaceDim << "\n" << std::flush;
        // std::cout << "-------------------------------------------------------------------\n\n" << std::flush;

        // create normal for IWG
        Matrix< DDRMat > tNormal( iSpaceDim, 1, 3.8 );

        // create the Jumps
        Matrix< DDRMat > tTempJump( 1, 1, 9.1 );
        Matrix< DDRMat > tVelocityJump( iSpaceDim, 1, 9.1 );

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
                tVelocityDof = { MSI::Dof_Type::VX, MSI::Dof_Type::VY };

                // set normal
                tNormal( 1 ) = -2.6;

                // set velocity jump
                tVelocityJump( 1 ) = -1.7;

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
                tVelocityDof = { MSI::Dof_Type::VX, MSI::Dof_Type::VY, MSI::Dof_Type::VZ };

                // set normal
                tNormal( 1 ) = -2.6;
                tNormal( 2 ) =  3.4;

                // set velocity jump
                tVelocityJump( 1 ) = -1.7;
                tVelocityJump( 2 ) = -5.2;

                break;
            }
            default:
            {
                MORIS_ERROR( false, " QUAD or HEX only." );
                break;
            }
        }

        // normalize normal vector
        tNormal = tNormal / norm( tNormal );

        // space and time geometry interpolators
        //------------------------------------------------------------------------------
        // create a space geometry interpolation rule
        mtk::Interpolation_Rule tGIRule(
                tGeometryType,
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

        // set space dimensions for property, CM and SP
        tMMFluid->set_space_dim( iSpaceDim );
        tCMMasterFluid->set_space_dim( iSpaceDim );

        // loop on the interpolation order
        for( uint iInterpOrder = 1; iInterpOrder < tInterpolationOrders.size() + 1; iInterpOrder++ )
        {
            // tune finite differencing for cubic shape functions
            if ( iInterpOrder == 3 )
            {
                tEpsilon = tEpsilonCubic;
                tPerturbation = tPerturbationCubic;
            }

            // output for debugging
            // std::cout << "-------------------------------------------------------------------\n" << std::flush;
            // std::cout << "-------------------------------------------------------------------\n" << std::flush;
            // std::cout << "Performing Tests For Interpolation Order:" << iInterpOrder << "\n\n" << std::flush;

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
                    mtk::Integration_Order::BAR_2 );

            // create an integrator
            mtk::Integrator tIntegrator( tIntegrationRule );

            // get integration points
            Matrix< DDRMat > tIntegPoints;
            tIntegrator.get_points( tIntegPoints );

            // field interpolators
            //------------------------------------------------------------------------------
            // create an interpolation order
            mtk::Interpolation_Order tInterpolationOrder = tInterpolationOrders( iInterpOrder - 1 );

            //create a space time interpolation rule
            mtk::Interpolation_Rule tFIRule (
                    tGeometryType,
                    mtk::Interpolation_Type::LAGRANGE,
                    tInterpolationOrder,
                    mtk::Interpolation_Type::LAGRANGE,
                    mtk::Interpolation_Order::LINEAR );

            // fill coefficients for master FI
            Matrix< DDRMat > tMasterDOFHatRho;
            fill_RhoHat( tMasterDOFHatRho, iSpaceDim, iInterpOrder );
            Matrix< DDRMat > tMasterDOFHatVel;
            fill_UHat( tMasterDOFHatVel, iSpaceDim, iInterpOrder );
            Matrix< DDRMat > tMasterDOFHatTemp;
            fill_TempHat( tMasterDOFHatTemp, iSpaceDim, iInterpOrder );

            // create a cell of field interpolators for IWG
            Cell< Field_Interpolator* > tMasterFIs( tDofTypes.size() );

            // create the field interpolator density
            tMasterFIs( 0 ) = new Field_Interpolator( 1, tFIRule, &tGI, tDensityDof );
            tMasterFIs( 0 )->set_coeff( tMasterDOFHatRho );

            // create the field interpolator velocity
            tMasterFIs( 1 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tVelocityDof );
            tMasterFIs( 1 )->set_coeff( tMasterDOFHatVel );

            // create the field interpolator pressure
            tMasterFIs( 2 ) = new Field_Interpolator( 1, tFIRule, &tGI, tTempDof );
            tMasterFIs( 2 )->set_coeff( tMasterDOFHatTemp );

            // create a field interpolator manager
            moris::Cell< moris::Cell< enum PDV_Type > > tDummyDv;
            Field_Interpolator_Manager tFIManager( tDofTypes, tDummyDv, tSet );

            // populate the field interpolator manager
            tFIManager.mFI = tMasterFIs;
            tFIManager.mIPGeometryInterpolator = &tGI;
            tFIManager.mIGGeometryInterpolator = &tGI;

            // set the interpolator manager to the set
            tMMFluid->mSet->mMasterFIManager = &tFIManager;
            tCMMasterFluid->mSet->mMasterFIManager = &tFIManager;

            // set IWG field interpolator manager
            tMMFluid->set_field_interpolator_manager( &tFIManager );
            tCMMasterFluid->set_field_interpolator_manager( &tFIManager );

            uint tNumGPs = tIntegPoints.n_cols();
            for( uint iGP = 0; iGP < tNumGPs; iGP ++ )
            {
                // reset IWG evaluation flags
                tMMFluid->reset_eval_flags();
                tCMMasterFluid->reset_eval_flags();             

                // create evaluation point xi, tau
                Matrix< DDRMat > tParamPoint = tIntegPoints.get_column( iGP );

                // set integration point
                tMMFluid->mSet->mMasterFIManager->set_space_time( tParamPoint );
                tCMMasterFluid->mSet->mMasterFIManager->set_space_time( tParamPoint );                

                // populate the requested master dof type for CM
                moris::Cell< moris::Cell< MSI::Dof_Type > > tRequestedMasterGlobalDofTypes =
                        tCMMasterFluid->get_global_dof_type_list();

                // populate the test master dof type for CM
                moris::Cell< moris::Cell< MSI::Dof_Type > > tMasterDofTypes =
                        tCMMasterFluid->get_dof_type_list();                                              

                // loop over requested dof type
                for( uint jRequestedDof = 0; jRequestedDof < tRequestedMasterGlobalDofTypes.size(); jRequestedDof++ )
                {
                    // output for debugging
                    // std::cout << "-------------------------------------------------------------------\n" << std::flush;
                    // std::cout << "Performing test for jacobian DOF derivative wrt. (0-RHO, 1-VX, 2-TEMP): " << jRequestedDof << "\n\n" << std::flush;

                    // derivative dof type
                    Cell< MSI::Dof_Type > tDofDerivative = tRequestedMasterGlobalDofTypes( jRequestedDof );

                    if ( jRequestedDof != 1 )
                    {
                        //------------------------------------------------------------------------------
                        //  Density - Material Model
                        //------------------------------------------------------------------------------
                        // evaluate DoF deriv
                        Matrix< DDRMat > tdDensitydDof = tMMFluid->DensityDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdDensitydDofFD;
                        tMMFluid->eval_TDvarDOF_FD(
                                tDofDerivative,
                                tdDensitydDofFD,
                                tPerturbation,
                                MSI::Dof_Type::RHO,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckDensity = fem::check( tdDensitydDof, tdDensitydDofFD, tEpsilon );
                        REQUIRE( tCheckDensity );

                        //------------------------------------------------------------------------------
                        // evaluate DoF deriv
                        Matrix< DDRMat > tdDensityDotdDof = tMMFluid->DensityDotDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdDensityDotdDofFD;
                        tMMFluid->eval_TDvarDotDOF_FD(
                                tDofDerivative,
                                tdDensityDotdDofFD,
                                tPerturbation,
                                MSI::Dof_Type::RHO,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckDensityDot = fem::check( tdDensityDotdDof, tdDensityDotdDofFD, tEpsilon );
                        REQUIRE( tCheckDensityDot );

                        //------------------------------------------------------------------------------
                        // evaluate DoF deriv
                        Matrix< DDRMat > tdDensitydxdDof = tMMFluid->dnDensitydxnDOF( tDofDerivative, 1 );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdDensitydxdDofFD;
                        tMMFluid->eval_dnTDvardxnDOF_FD(
                                tDofDerivative,
                                tdDensitydxdDofFD,
                                tPerturbation,
                                MSI::Dof_Type::RHO,
                                1,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckdDensitydx = fem::check( tdDensitydxdDof, tdDensitydxdDofFD, tEpsilon );
                        REQUIRE( tCheckdDensitydx );

                        //------------------------------------------------------------------------------
                        //  Pressure - Material Model
                        //------------------------------------------------------------------------------
                        // evaluate dof deriv
                        Matrix< DDRMat > tdPressuredDof = tMMFluid->PressureDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdPressuredDofFD;
                        tMMFluid->eval_TDvarDOF_FD(
                                tDofDerivative,
                                tdPressuredDofFD,
                                tPerturbation,
                                MSI::Dof_Type::P,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckPressure = fem::check( tdPressuredDof, tdPressuredDofFD, tEpsilon );
                        REQUIRE( tCheckPressure );

                        //------------------------------------------------------------------------------
                        // evaluate dof deriv
                        Matrix< DDRMat > tdPressureDotdDof = tMMFluid->PressureDotDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdPressureDotdDofFD;
                        tMMFluid->eval_TDvarDotDOF_FD(
                                tDofDerivative,
                                tdPressureDotdDofFD,
                                tPerturbation,
                                MSI::Dof_Type::P,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckPressureDot = fem::check( tdPressureDotdDof, tdPressureDotdDofFD, tEpsilon );
                        REQUIRE( tCheckPressureDot );

                        //------------------------------------------------------------------------------
                        // evaluate DoF deriv
                        Matrix< DDRMat > tdPressuredxdDof = tMMFluid->dnPressuredxnDOF( tDofDerivative, 1 );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdPressuredxdDofFD;
                        tMMFluid->eval_dnTDvardxnDOF_FD(
                                tDofDerivative,
                                tdPressuredxdDofFD,
                                tPerturbation,
                                MSI::Dof_Type::P,
                                1,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckdPressuredx = fem::check( tdPressuredxdDof, tdPressuredxdDofFD, tEpsilon );
                        REQUIRE( tCheckdPressuredx );

                        //------------------------------------------------------------------------------
                        //  Temperature - Material Model
                        //------------------------------------------------------------------------------
                        // evaluate dof deriv
                        Matrix< DDRMat > tdTemperaturedDof = tMMFluid->TemperatureDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdTemperaturedDofFD;
                        tMMFluid->eval_TDvarDOF_FD(
                                tDofDerivative,
                                tdTemperaturedDofFD,
                                tPerturbation,
                                MSI::Dof_Type::TEMP,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckTemperature = fem::check( tdTemperaturedDof, tdTemperaturedDofFD, tEpsilon );
                        REQUIRE( tCheckTemperature );

                        //------------------------------------------------------------------------------
                        // evaluate dof deriv
                        Matrix< DDRMat > tdTemperatureDotdDof = tMMFluid->TemperatureDotDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdTemperatureDotdDofFD;
                        tMMFluid->eval_TDvarDotDOF_FD(
                                tDofDerivative,
                                tdTemperatureDotdDofFD,
                                tPerturbation,
                                MSI::Dof_Type::TEMP,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckTemperatureDot = fem::check( tdTemperatureDotdDof, tdTemperatureDotdDofFD, tEpsilon );
                        REQUIRE( tCheckTemperatureDot );

                        //------------------------------------------------------------------------------
                        // evaluate DoF deriv
                        Matrix< DDRMat > tdTemperaturedxdDof = tMMFluid->dnTemperaturedxnDOF( tDofDerivative, 1 );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdTemperaturedxdDofFD;
                        tMMFluid->eval_dnTDvardxnDOF_FD(
                                tDofDerivative,
                                tdTemperaturedxdDofFD,
                                tPerturbation,
                                MSI::Dof_Type::TEMP,
                                1,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckdTemperaturedx = fem::check( tdTemperaturedxdDof, tdTemperaturedxdDofFD, tEpsilon );
                        REQUIRE( tCheckdTemperaturedx );

                        //------------------------------------------------------------------------------
                        //  Internal Energy - Material Model
                        //------------------------------------------------------------------------------
                        // evaluate dEnergydu
                        Matrix< DDRMat > tdEintdDof = tMMFluid->EintDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdEintdDofFD;
                        tMMFluid->eval_EintDOF_FD(
                                tDofDerivative,
                                tdEintdDofFD,
                                tPerturbation,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckEint = fem::check( tdEintdDof, tdEintdDofFD, tEpsilon );
                        REQUIRE( tCheckEint );   

                        //------------------------------------------------------------------------------
                        // evaluate dEnergyDotdu
                        Matrix< DDRMat > tdEintDotdDof = tMMFluid->EintDotDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdEintDotdDofFD;
                        tMMFluid->eval_EintDotDOF_FD(
                                tDofDerivative,
                                tdEintDotdDofFD,
                                tPerturbation,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckEintDot = fem::check( tdEintDotdDof, tdEintDotdDofFD, tEpsilon );
                        REQUIRE( tCheckEintDot );
                    }                 

                    //------------------------------------------------------------------------------
                    //  Energy
                    //------------------------------------------------------------------------------
                    // evaluate dEnergydu
                    Matrix< DDRMat > tdEnergydDof = tCMMasterFluid->dEnergydDOF( tDofDerivative );

                    // evaluate dfluxdu by FD
                    Matrix< DDRMat > tdEnergydDofFD;
                    tCMMasterFluid->eval_dEnergydDOF_FD(
                            tDofDerivative,
                            tdEnergydDofFD,
                            tPerturbation,
                            FDScheme_Type::POINT_5 );

                    // check that analytical and FD match
                    bool tCheckEnergy = fem::check( tdEnergydDof, tdEnergydDofFD, tEpsilon );
                    REQUIRE( tCheckEnergy );

                    //------------------------------------------------------------------------------
                    //  Energy Dot
                    //------------------------------------------------------------------------------
                    // evaluate dEnergydu
                    Matrix< DDRMat > tdEnergyDotdDof = tCMMasterFluid->dEnergyDotdDOF( tDofDerivative );

                    // evaluate dfluxdu by FD
                    Matrix< DDRMat > tdEnergyDotdDofFD;
                    tCMMasterFluid->eval_dEnergyDotdDOF_FD(
                            tDofDerivative,
                            tdEnergyDotdDofFD,
                            tPerturbation,
                            FDScheme_Type::POINT_5 );

                    // check that analytical and FD match
                    bool tCheckEnergyDot = fem::check( tdEnergyDotdDof, tdEnergyDotdDofFD, tEpsilon );
                    REQUIRE( tCheckEnergyDot );

                    //------------------------------------------------------------------------------
                    //  Thermal Flux
                    //------------------------------------------------------------------------------
                    // evaluate dfluxdu
                    Matrix< DDRMat > tdThermalFluxdu = tCMMasterFluid->dFluxdDOF( tDofDerivative, CM_Function_Type::THERMAL );

                    // evaluate dfluxdu by FD
                    Matrix< DDRMat > tdThermalFluxduFD;
                    tCMMasterFluid->eval_dFluxdDOF_FD(
                            tDofDerivative,
                            tdThermalFluxduFD,
                            tPerturbation,
                            FDScheme_Type::POINT_5,
                            CM_Function_Type::THERMAL );

                    // check that analytical and FD match
                    bool tCheckThermalFlux = fem::check( tdThermalFluxdu, tdThermalFluxduFD, tEpsilon );
                    REQUIRE( tCheckThermalFlux );

                    //------------------------------------------------------------------------------
                    //  Stress (Mechanical Flux)
                    //------------------------------------------------------------------------------
                    // evaluate dfluxdu
                    Matrix< DDRMat > tdStressdDof = tCMMasterFluid->dFluxdDOF( tDofDerivative, CM_Function_Type::MECHANICAL );

                    // evaluate dfluxdu by FD
                    Matrix< DDRMat > tdStressdDofFD;
                    tCMMasterFluid->eval_dFluxdDOF_FD(
                            tDofDerivative,
                            tdStressdDofFD,
                            tPerturbation,
                            FDScheme_Type::POINT_5,
                            CM_Function_Type::MECHANICAL );

                    // check that analytical and FD match
                    bool tCheckStress = fem::check( tdStressdDof, tdStressdDofFD, tEpsilon );
                    REQUIRE( tCheckStress );

                    //------------------------------------------------------------------------------
                    //  Energy Flux
                    //------------------------------------------------------------------------------
                    // evaluate dfluxdu
                    Matrix< DDRMat > tdEnergyFluxdu = tCMMasterFluid->dFluxdDOF( tDofDerivative, CM_Function_Type::ENERGY );

                    // evaluate dfluxdu by FD
                    Matrix< DDRMat > tdEnergyFluxduFD;
                    tCMMasterFluid->eval_dFluxdDOF_FD(
                            tDofDerivative,
                            tdEnergyFluxduFD,
                            tPerturbation,
                            FDScheme_Type::POINT_5,
                            CM_Function_Type::ENERGY );

                    // check that analytical and FD match
                    bool tCheckEnergyFlux = fem::check( tdEnergyFluxdu, tdEnergyFluxduFD, tEpsilon );
                    REQUIRE( tCheckEnergyFlux );

                    //------------------------------------------------------------------------------
                    //  Work Flux
                    //------------------------------------------------------------------------------
                    // evaluate dfluxdu
                    Matrix< DDRMat > tdWorkFluxdu = tCMMasterFluid->dFluxdDOF( tDofDerivative, CM_Function_Type::WORK );

                    // evaluate dfluxdu by FD
                    Matrix< DDRMat > tdWorkFluxduFD;
                    tCMMasterFluid->eval_dFluxdDOF_FD(
                            tDofDerivative,
                            tdWorkFluxduFD,
                            tPerturbation,
                            FDScheme_Type::POINT_5,
                            CM_Function_Type::WORK );

                    // check that analytical and FD match
                    bool tCheckWorkFlux = fem::check( tdWorkFluxdu, tdWorkFluxduFD, tEpsilon );
                    REQUIRE( tCheckWorkFlux );

                    //------------------------------------------------------------------------------
                    // Mechanical Strain Rate
                    //------------------------------------------------------------------------------
                    // evaluate dstraindu
                    Matrix< DDRMat > tdstraindu = tCMMasterFluid->dStraindDOF( tDofDerivative );

                    // evaluate dstraindu by FD
                    Matrix< DDRMat > tdstrainduFD;
                    tCMMasterFluid->eval_dStraindDOF_FD( tDofDerivative, tdstrainduFD, tPerturbation );

                    // check that analytical and FD match
                    bool tCheckStrainFluid = fem::check( tdstraindu, tdstrainduFD, tEpsilon );
                    REQUIRE( tCheckStrainFluid );

                    // loop over requested dof type
                //     for( uint iTestDof = 0; iTestDof < tRequestedMasterGlobalDofTypes.size(); iTestDof++ )
                //     {
                //         // output for debugging
                //         std::cout << "-------------------------------------------------------------------\n" << std::flush;
                //         std::cout << "Checking test-tractions for test DOF type (0-RHO, 1-VX, 2-TEMP): " << iTestDof << "\n\n" << std::flush;

                //         // derivative dof type
                //         Cell< MSI::Dof_Type > tTestDof = tRequestedMasterGlobalDofTypes( iTestDof );

                //         //------------------------------------------------------------------------------
                //         //  Thermal Test Traction
                //         //------------------------------------------------------------------------------
                //         // evaluate dTestTractiondDOF
                //         Matrix< DDRMat > tdThermalTestTractiondDOF =
                //                 tCMMasterFluid->dTestTractiondDOF(
                //                         tDofDerivative,
                //                         tNormal,
                //                         tTempJump,
                //                         tTestDof,
                //                         CM_Function_Type::THERMAL );

                //         //  evaluate dTestTractiondDOF by FD
                //         Matrix< DDRMat > tdThermalTestTractiondDofFD;
                //         tCMMasterFluid->eval_dtesttractiondu_FD(
                //                 tDofDerivative,
                //                 tTestDof,
                //                 tdThermalTestTractiondDofFD,
                //                 tPerturbation,
                //                 tNormal,
                //                 tTempJump,
                //                 FDScheme_Type::POINT_5,
                //                 CM_Function_Type::THERMAL );

                //         // check that analytical and FD match
                //         bool tCheckThermalTestTraction = fem::check( tdThermalTestTractiondDOF, tdThermalTestTractiondDofFD, tEpsilon );
                //         REQUIRE( tCheckThermalTestTraction );

                //         //------------------------------------------------------------------------------
                //         //  Mechanical Test Traction
                //         //------------------------------------------------------------------------------
                //         // evaluate dTestTractiondDOF
                //         Matrix< DDRMat > tdMechanicalTestTractiondDOF =
                //                 tCMMasterFluid->dTestTractiondDOF(
                //                         tDofDerivative,
                //                         tNormal,
                //                         tVelocityJump,
                //                         tTestDof,
                //                         CM_Function_Type::MECHANICAL );

                //         //  evaluate dTestTractiondDOF by FD
                //         Matrix< DDRMat > tdMechanicalTestTractiondDofFD;
                //         tCMMasterFluid->eval_dtesttractiondu_FD(
                //                 tDofDerivative,
                //                 tTestDof,
                //                 tdMechanicalTestTractiondDofFD,
                //                 tPerturbation,
                //                 tNormal,
                //                 tVelocityJump,
                //                 FDScheme_Type::POINT_5,
                //                 CM_Function_Type::MECHANICAL );

                //         // check that analytical and FD match
                //         bool tCheckMechanicalTestTraction = fem::check( tdMechanicalTestTractiondDOF, tdMechanicalTestTractiondDofFD, tEpsilon );
                //         REQUIRE( tCheckMechanicalTestTraction );
                //     }

                }
            }
            // clean up
            tMasterFIs.clear();
        }
    }
}/*END_TEST_CASE*/

//------------------------------------------------------------------------------

TEST_CASE( "CM_Compressible_Newtonian_Fluid_Pressure_Primitive", "[CM_Compressible_Newtonian_Fluid_Pressure_Primitive]" )
{
    // define an epsilon environment
    real tEpsilon = 2.0E-6;
    real tEpsilonCubic = 2.0E-5;

    // define a perturbation relative size
    real tPerturbation = 2.0E-6;
    real tPerturbationCubic = 4.0E-6;

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
    //Matrix< DDRMat > tNumCoeffs = {{ 8, 18 },{ 16, 54 }};
    Matrix< DDRMat > tNumCoeffs = {{ 8, 18, 32 },{ 16, 54, 128 }};

    // dof type list
    moris::Cell< MSI::Dof_Type > tPressureDof = { MSI::Dof_Type::P };
    moris::Cell< MSI::Dof_Type > tVelocityDof = { MSI::Dof_Type::VX };
    moris::Cell< MSI::Dof_Type > tTempDof     = { MSI::Dof_Type::TEMP };
    moris::Cell< moris::Cell< MSI::Dof_Type > > tDofTypes = { tPressureDof, tVelocityDof, tTempDof };

    //------------------------------------------------------------------------------
    // create the properties

    // dynamic viscosity
    std::shared_ptr< fem::Property > tPropViscosity = std::make_shared< fem::Property >();
    tPropViscosity->set_parameters( { {{ 11.9 }} } );
    tPropViscosity->set_val_function( tConstValFunc );

    // isochoric heat capacity
    std::shared_ptr< fem::Property > tPropHeatCapacity = std::make_shared< fem::Property >();
    tPropHeatCapacity->set_parameters( { {{ 5.7 }} } );
    tPropHeatCapacity->set_val_function( tConstValFunc );

    // specific gas constant
    std::shared_ptr< fem::Property > tPropGasConstant = std::make_shared< fem::Property >();
    tPropGasConstant->set_parameters( { {{ 2.8 }} } );
    tPropGasConstant->set_val_function( tConstValFunc );

    // thermal conductivity
    std::shared_ptr< fem::Property > tPropConductivity = std::make_shared< fem::Property >();
    tPropConductivity->set_parameters( { {{ 3.7 }} } );
    tPropConductivity->set_val_function( tConstValFunc );

    // define material model and assign properties
    fem::MM_Factory tMMFactory;

    std::shared_ptr< fem::Material_Model > tMMFluid =
            tMMFactory.create_CM( fem::Material_Type::PERFECT_GAS );
    tMMFluid->set_dof_type_list( {tPressureDof, tTempDof } );
    tMMFluid->set_property( tPropHeatCapacity, "IsochoricHeatCapacity" );
    tMMFluid->set_property( tPropGasConstant,  "SpecificGasConstant" );

    // define constitutive model and assign properties
    fem::CM_Factory tCMFactory;

    std::shared_ptr< fem::Constitutive_Model > tCMMasterFluid =
            tCMFactory.create_CM( fem::Constitutive_Type::FLUID_COMPRESSIBLE_NEWTONIAN );
    tCMMasterFluid->set_dof_type_list( {tPressureDof, tVelocityDof, tTempDof } );
    tCMMasterFluid->set_property( tPropViscosity,    "DynamicViscosity" );
    tCMMasterFluid->set_property( tPropConductivity, "ThermalConductivity" );
    tCMMasterFluid->set_material_model( tMMFluid, "ThermodynamicMaterialModel" );

    // set a fem set pointer
    MSI::Equation_Set * tSet = new fem::Set();
    tMMFluid->set_set_pointer( static_cast< fem::Set* >( tSet ) );
    tCMMasterFluid->set_set_pointer( static_cast< fem::Set* >( tSet ) );

    // set size for the set EqnObjDofTypeList
    tMMFluid->mSet->mUniqueDofTypeList.resize( 100, MSI::Dof_Type::END_ENUM );
    tCMMasterFluid->mSet->mUniqueDofTypeList.resize( 100, MSI::Dof_Type::END_ENUM );

    // set size and populate the set dof type map
    tMMFluid->mSet->mUniqueDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tMMFluid->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::P ) )   = 0;
    tMMFluid->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::TEMP ) )  = 1;

    tCMMasterFluid->mSet->mUniqueDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tCMMasterFluid->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::P ) )   = 0;
    tCMMasterFluid->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) )    = 1;
    tCMMasterFluid->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::TEMP ) )  = 2;

    // set size and populate the set master dof type map
    tMMFluid->mSet->mMasterDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tMMFluid->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::P ) )   = 0;
    tMMFluid->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::TEMP ) )  = 1;

    tCMMasterFluid->mSet->mMasterDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tCMMasterFluid->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::P ) )   = 0;
    tCMMasterFluid->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) )    = 1;
    tCMMasterFluid->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::TEMP ) )  = 2;    

    // build global dof type list
    tMMFluid->get_global_dof_type_list();
    tCMMasterFluid->get_global_dof_type_list();

    // loop on the space dimension
    for( uint iSpaceDim = 2; iSpaceDim < 4; iSpaceDim++ )
    {
        // output for debugging
        // std::cout << "-------------------------------------------------------------------\n" << std::flush;
        // std::cout << "Performing Tests For Number of Spatial dimensions: " << iSpaceDim << "\n" << std::flush;
        // std::cout << "-------------------------------------------------------------------\n\n" << std::flush;

        // create normal for IWG
        Matrix< DDRMat > tNormal( iSpaceDim, 1, 3.8 );

        // create the Jumps
        Matrix< DDRMat > tTempJump( 1, 1, 9.1 );
        Matrix< DDRMat > tVelocityJump( iSpaceDim, 1, 9.1 );

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
                tVelocityDof = { MSI::Dof_Type::VX, MSI::Dof_Type::VY };

                // set normal
                tNormal( 1 ) = -2.6;

                // set velocity jump
                tVelocityJump( 1 ) = -1.7;

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
                tVelocityDof = { MSI::Dof_Type::VX, MSI::Dof_Type::VY, MSI::Dof_Type::VZ };

                // set normal
                tNormal( 1 ) = -2.6;
                tNormal( 2 ) =  3.4;

                // set velocity jump
                tVelocityJump( 1 ) = -1.7;
                tVelocityJump( 2 ) = -5.2;

                break;
            }
            default:
            {
                MORIS_ERROR( false, " QUAD or HEX only." );
                break;
            }
        }

        // normalize normal vector
        tNormal = tNormal / norm( tNormal );

        // space and time geometry interpolators
        //------------------------------------------------------------------------------
        // create a space geometry interpolation rule
        mtk::Interpolation_Rule tGIRule(
                tGeometryType,
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

        // set space dimensions for property, CM and SP
        tMMFluid->set_space_dim( iSpaceDim );
        tCMMasterFluid->set_space_dim( iSpaceDim );

        // loop on the interpolation order
        for( uint iInterpOrder = 1; iInterpOrder < tInterpolationOrders.size() + 1; iInterpOrder++ )
        {
            // tune finite differencing for cubic shape functions
            if ( iInterpOrder == 3 )
            {
                tEpsilon = tEpsilonCubic;
                tPerturbation = tPerturbationCubic;
            }

            // output for debugging
            // std::cout << "-------------------------------------------------------------------\n" << std::flush;
            // std::cout << "-------------------------------------------------------------------\n" << std::flush;
            // std::cout << "Performing Tests For Interpolation Order:" << iInterpOrder << "\n\n" << std::flush;

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
                    mtk::Integration_Order::BAR_2 );

            // create an integrator
            mtk::Integrator tIntegrator( tIntegrationRule );

            // get integration points
            Matrix< DDRMat > tIntegPoints;
            tIntegrator.get_points( tIntegPoints );

            // field interpolators
            //------------------------------------------------------------------------------
            // create an interpolation order
            mtk::Interpolation_Order tInterpolationOrder = tInterpolationOrders( iInterpOrder - 1 );

            //create a space time interpolation rule
            mtk::Interpolation_Rule tFIRule (
                    tGeometryType,
                    mtk::Interpolation_Type::LAGRANGE,
                    tInterpolationOrder,
                    mtk::Interpolation_Type::LAGRANGE,
                    mtk::Interpolation_Order::LINEAR );

            // fill coefficients for master FI
            Matrix< DDRMat > tMasterDOFHatRho;
            fill_RhoHat( tMasterDOFHatRho, iSpaceDim, iInterpOrder );
            Matrix< DDRMat > tMasterDOFHatVel;
            fill_UHat( tMasterDOFHatVel, iSpaceDim, iInterpOrder );
            Matrix< DDRMat > tMasterDOFHatTemp;
            fill_TempHat( tMasterDOFHatTemp, iSpaceDim, iInterpOrder );

            // create a cell of field interpolators for IWG
            Cell< Field_Interpolator* > tMasterFIs( tDofTypes.size() );

            // create the field interpolator density
            tMasterFIs( 0 ) = new Field_Interpolator( 1, tFIRule, &tGI, tPressureDof );
            tMasterFIs( 0 )->set_coeff( tMasterDOFHatRho );

            // create the field interpolator velocity
            tMasterFIs( 1 ) = new Field_Interpolator( iSpaceDim, tFIRule, &tGI, tVelocityDof );
            tMasterFIs( 1 )->set_coeff( tMasterDOFHatVel );

            // create the field interpolator pressure
            tMasterFIs( 2 ) = new Field_Interpolator( 1, tFIRule, &tGI, tTempDof );
            tMasterFIs( 2 )->set_coeff( tMasterDOFHatTemp );

            // create a field interpolator manager
            moris::Cell< moris::Cell< enum PDV_Type > > tDummyDv;
            Field_Interpolator_Manager tFIManager( tDofTypes, tDummyDv, tSet );

            // populate the field interpolator manager
            tFIManager.mFI = tMasterFIs;
            tFIManager.mIPGeometryInterpolator = &tGI;
            tFIManager.mIGGeometryInterpolator = &tGI;

            // set the interpolator manager to the set
            tMMFluid->mSet->mMasterFIManager = &tFIManager;
            tCMMasterFluid->mSet->mMasterFIManager = &tFIManager;

            // set IWG field interpolator manager
            tMMFluid->set_field_interpolator_manager( &tFIManager );
            tCMMasterFluid->set_field_interpolator_manager( &tFIManager );

            uint tNumGPs = tIntegPoints.n_cols();
            for( uint iGP = 0; iGP < tNumGPs; iGP ++ )
            {
                // reset IWG evaluation flags
                tMMFluid->reset_eval_flags();
                tCMMasterFluid->reset_eval_flags();             

                // create evaluation point xi, tau
                Matrix< DDRMat > tParamPoint = tIntegPoints.get_column( iGP );

                // set integration point
                tMMFluid->mSet->mMasterFIManager->set_space_time( tParamPoint );
                tCMMasterFluid->mSet->mMasterFIManager->set_space_time( tParamPoint );                

                // populate the requested master dof type for CM
                moris::Cell< moris::Cell< MSI::Dof_Type > > tRequestedMasterGlobalDofTypes =
                        tCMMasterFluid->get_global_dof_type_list();

                // populate the test master dof type for CM
                moris::Cell< moris::Cell< MSI::Dof_Type > > tMasterDofTypes =
                        tCMMasterFluid->get_dof_type_list();                                              

                // loop over requested dof type
                for( uint jRequestedDof = 0; jRequestedDof < tRequestedMasterGlobalDofTypes.size(); jRequestedDof++ )
                {
                    // output for debugging
                    // std::cout << "-------------------------------------------------------------------\n" << std::flush;
                    // std::cout << "Performing test for jacobian DOF derivative wrt. (0-P, 1-VX, 2-TEMP): " << jRequestedDof << "\n\n" << std::flush;

                    // derivative dof type
                    Cell< MSI::Dof_Type > tDofDerivative = tRequestedMasterGlobalDofTypes( jRequestedDof );

                    if ( jRequestedDof != 1 )
                    {
                        //------------------------------------------------------------------------------
                        //  Density - Material Model
                        //------------------------------------------------------------------------------
                        // evaluate DoF deriv
                        Matrix< DDRMat > tdDensitydDof = tMMFluid->DensityDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdDensitydDofFD;
                        tMMFluid->eval_TDvarDOF_FD(
                                tDofDerivative,
                                tdDensitydDofFD,
                                tPerturbation,
                                MSI::Dof_Type::RHO,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckDensity = fem::check( tdDensitydDof, tdDensitydDofFD, tEpsilon );
                        REQUIRE( tCheckDensity );

                        //------------------------------------------------------------------------------
                        // evaluate DoF deriv
                        Matrix< DDRMat > tdDensityDotdDof = tMMFluid->DensityDotDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdDensityDotdDofFD;
                        tMMFluid->eval_TDvarDotDOF_FD(
                                tDofDerivative,
                                tdDensityDotdDofFD,
                                tPerturbation,
                                MSI::Dof_Type::RHO,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckDensityDot = fem::check( tdDensityDotdDof, tdDensityDotdDofFD, tEpsilon );
                        REQUIRE( tCheckDensityDot );

                        //------------------------------------------------------------------------------
                        // evaluate DoF deriv
                        Matrix< DDRMat > tdDensitydxdDof = tMMFluid->dnDensitydxnDOF( tDofDerivative, 1 );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdDensitydxdDofFD;
                        tMMFluid->eval_dnTDvardxnDOF_FD(
                                tDofDerivative,
                                tdDensitydxdDofFD,
                                tPerturbation,
                                MSI::Dof_Type::RHO,
                                1,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckdDensitydx = fem::check( tdDensitydxdDof, tdDensitydxdDofFD, tEpsilon );
                        REQUIRE( tCheckdDensitydx );

                        //------------------------------------------------------------------------------
                        //  Pressure - Material Model
                        //------------------------------------------------------------------------------
                        // evaluate dof deriv
                        Matrix< DDRMat > tdPressuredDof = tMMFluid->PressureDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdPressuredDofFD;
                        tMMFluid->eval_TDvarDOF_FD(
                                tDofDerivative,
                                tdPressuredDofFD,
                                tPerturbation,
                                MSI::Dof_Type::P,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckPressure = fem::check( tdPressuredDof, tdPressuredDofFD, tEpsilon );
                        REQUIRE( tCheckPressure );                        

                        //------------------------------------------------------------------------------
                        // evaluate dof deriv
                        Matrix< DDRMat > tdPressureDotdDof = tMMFluid->PressureDotDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdPressureDotdDofFD;
                        tMMFluid->eval_TDvarDotDOF_FD(
                                tDofDerivative,
                                tdPressureDotdDofFD,
                                tPerturbation,
                                MSI::Dof_Type::P,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckPressureDot = fem::check( tdPressureDotdDof, tdPressureDotdDofFD, tEpsilon );
                        REQUIRE( tCheckPressureDot );

                        //------------------------------------------------------------------------------
                        // evaluate DoF deriv
                        Matrix< DDRMat > tdPressuredxdDof = tMMFluid->dnPressuredxnDOF( tDofDerivative, 1 );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdPressuredxdDofFD;
                        tMMFluid->eval_dnTDvardxnDOF_FD(
                                tDofDerivative,
                                tdPressuredxdDofFD,
                                tPerturbation,
                                MSI::Dof_Type::P,
                                1,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckdPressuredx = fem::check( tdPressuredxdDof, tdPressuredxdDofFD, tEpsilon );
                        REQUIRE( tCheckdPressuredx );

                        //------------------------------------------------------------------------------
                        //  Temperature - Material Model
                        //------------------------------------------------------------------------------
                        // evaluate dof deriv
                        Matrix< DDRMat > tdTemperaturedDof = tMMFluid->TemperatureDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdTemperaturedDofFD;
                        tMMFluid->eval_TDvarDOF_FD(
                                tDofDerivative,
                                tdTemperaturedDofFD,
                                tPerturbation,
                                MSI::Dof_Type::TEMP,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckTemperature = fem::check( tdTemperaturedDof, tdTemperaturedDofFD, tEpsilon );
                        REQUIRE( tCheckTemperature );

                        //------------------------------------------------------------------------------
                        // evaluate dof deriv
                        Matrix< DDRMat > tdTemperatureDotdDof = tMMFluid->TemperatureDotDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdTemperatureDotdDofFD;
                        tMMFluid->eval_TDvarDotDOF_FD(
                                tDofDerivative,
                                tdTemperatureDotdDofFD,
                                tPerturbation,
                                MSI::Dof_Type::TEMP,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckTemperatureDot = fem::check( tdTemperatureDotdDof, tdTemperatureDotdDofFD, tEpsilon );
                        REQUIRE( tCheckTemperatureDot );

                        //------------------------------------------------------------------------------
                        // evaluate DoF deriv
                        Matrix< DDRMat > tdTemperaturedxdDof = tMMFluid->dnTemperaturedxnDOF( tDofDerivative, 1 );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdTemperaturedxdDofFD;
                        tMMFluid->eval_dnTDvardxnDOF_FD(
                                tDofDerivative,
                                tdTemperaturedxdDofFD,
                                tPerturbation,
                                MSI::Dof_Type::TEMP,
                                1,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckdTemperaturedx = fem::check( tdTemperaturedxdDof, tdTemperaturedxdDofFD, tEpsilon );
                        REQUIRE( tCheckdTemperaturedx );

                        //------------------------------------------------------------------------------
                        //  Internal Energy - Material Model
                        //------------------------------------------------------------------------------
                        // evaluate dEnergydu
                        Matrix< DDRMat > tdEintdDof = tMMFluid->EintDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdEintdDofFD;
                        tMMFluid->eval_EintDOF_FD(
                                tDofDerivative,
                                tdEintdDofFD,
                                tPerturbation,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckEint = fem::check( tdEintdDof, tdEintdDofFD, tEpsilon );
                        REQUIRE( tCheckEint );   

                        //------------------------------------------------------------------------------
                        // evaluate dEnergyDotdu
                        Matrix< DDRMat > tdEintDotdDof = tMMFluid->EintDotDOF( tDofDerivative );

                        // evaluate dfluxdu by FD
                        Matrix< DDRMat > tdEintDotdDofFD;
                        tMMFluid->eval_EintDotDOF_FD(
                                tDofDerivative,
                                tdEintDotdDofFD,
                                tPerturbation,
                                FDScheme_Type::POINT_5 );

                        // check that analytical and FD match
                        bool tCheckEintDot = fem::check( tdEintDotdDof, tdEintDotdDofFD, tEpsilon );
                        REQUIRE( tCheckEintDot );
                    }                 

                    //------------------------------------------------------------------------------
                    //  Energy
                    //------------------------------------------------------------------------------
                    // evaluate dEnergydu
                    Matrix< DDRMat > tdEnergydDof = tCMMasterFluid->dEnergydDOF( tDofDerivative );

                    // evaluate dfluxdu by FD
                    Matrix< DDRMat > tdEnergydDofFD;
                    tCMMasterFluid->eval_dEnergydDOF_FD(
                            tDofDerivative,
                            tdEnergydDofFD,
                            tPerturbation,
                            FDScheme_Type::POINT_5 );

                    // check that analytical and FD match
                    bool tCheckEnergy = fem::check( tdEnergydDof, tdEnergydDofFD, tEpsilon );
                    REQUIRE( tCheckEnergy );

                    //------------------------------------------------------------------------------
                    //  Energy Dot
                    //------------------------------------------------------------------------------
                    // evaluate dEnergydu
                    Matrix< DDRMat > tdEnergyDotdDof = tCMMasterFluid->dEnergyDotdDOF( tDofDerivative );

                    // evaluate dfluxdu by FD
                    Matrix< DDRMat > tdEnergyDotdDofFD;
                    tCMMasterFluid->eval_dEnergyDotdDOF_FD(
                            tDofDerivative,
                            tdEnergyDotdDofFD,
                            tPerturbation,
                            FDScheme_Type::POINT_5 );

                    // check that analytical and FD match
                    bool tCheckEnergyDot = fem::check( tdEnergyDotdDof, tdEnergyDotdDofFD, tEpsilon );
                    REQUIRE( tCheckEnergyDot );

                    //------------------------------------------------------------------------------
                    //  Thermal Flux
                    //------------------------------------------------------------------------------
                    // evaluate dfluxdu
                    Matrix< DDRMat > tdThermalFluxdu = tCMMasterFluid->dFluxdDOF( tDofDerivative, CM_Function_Type::THERMAL );

                    // evaluate dfluxdu by FD
                    Matrix< DDRMat > tdThermalFluxduFD;
                    tCMMasterFluid->eval_dFluxdDOF_FD(
                            tDofDerivative,
                            tdThermalFluxduFD,
                            tPerturbation,
                            FDScheme_Type::POINT_5,
                            CM_Function_Type::THERMAL );

                    // check that analytical and FD match
                    bool tCheckThermalFlux = fem::check( tdThermalFluxdu, tdThermalFluxduFD, tEpsilon );
                    REQUIRE( tCheckThermalFlux );

                    //------------------------------------------------------------------------------
                    //  Stress (Mechanical Flux)
                    //------------------------------------------------------------------------------
                    // evaluate dfluxdu
                    Matrix< DDRMat > tdStressdDof = tCMMasterFluid->dFluxdDOF( tDofDerivative, CM_Function_Type::MECHANICAL );

                    // evaluate dfluxdu by FD
                    Matrix< DDRMat > tdStressdDofFD;
                    tCMMasterFluid->eval_dFluxdDOF_FD(
                            tDofDerivative,
                            tdStressdDofFD,
                            tPerturbation,
                            FDScheme_Type::POINT_5,
                            CM_Function_Type::MECHANICAL );

                    // check that analytical and FD match
                    bool tCheckStress = fem::check( tdStressdDof, tdStressdDofFD, tEpsilon );
                    REQUIRE( tCheckStress );

                    //------------------------------------------------------------------------------
                    //  Energy Flux
                    //------------------------------------------------------------------------------
                    // evaluate dfluxdu
                    Matrix< DDRMat > tdEnergyFluxdu = tCMMasterFluid->dFluxdDOF( tDofDerivative, CM_Function_Type::ENERGY );

                    // evaluate dfluxdu by FD
                    Matrix< DDRMat > tdEnergyFluxduFD;
                    tCMMasterFluid->eval_dFluxdDOF_FD(
                            tDofDerivative,
                            tdEnergyFluxduFD,
                            tPerturbation,
                            FDScheme_Type::POINT_5,
                            CM_Function_Type::ENERGY );

                    // check that analytical and FD match
                    bool tCheckEnergyFlux = fem::check( tdEnergyFluxdu, tdEnergyFluxduFD, tEpsilon );
                    REQUIRE( tCheckEnergyFlux );

                    //------------------------------------------------------------------------------
                    //  Work Flux
                    //------------------------------------------------------------------------------
                    // evaluate dfluxdu
                    Matrix< DDRMat > tdWorkFluxdu = tCMMasterFluid->dFluxdDOF( tDofDerivative, CM_Function_Type::WORK );

                    // evaluate dfluxdu by FD
                    Matrix< DDRMat > tdWorkFluxduFD;
                    tCMMasterFluid->eval_dFluxdDOF_FD(
                            tDofDerivative,
                            tdWorkFluxduFD,
                            tPerturbation,
                            FDScheme_Type::POINT_5,
                            CM_Function_Type::WORK );

                    // check that analytical and FD match
                    bool tCheckWorkFlux = fem::check( tdWorkFluxdu, tdWorkFluxduFD, tEpsilon );
                    REQUIRE( tCheckWorkFlux );

                    //------------------------------------------------------------------------------
                    // Mechanical Strain Rate
                    //------------------------------------------------------------------------------
                    // evaluate dstraindu
                    Matrix< DDRMat > tdstraindu = tCMMasterFluid->dStraindDOF( tDofDerivative );

                    // evaluate dstraindu by FD
                    Matrix< DDRMat > tdstrainduFD;
                    tCMMasterFluid->eval_dStraindDOF_FD( tDofDerivative, tdstrainduFD, tPerturbation );

                    // check that analytical and FD match
                    bool tCheckStrainFluid = fem::check( tdstraindu, tdstrainduFD, tEpsilon );
                    REQUIRE( tCheckStrainFluid );

                    // loop over requested dof type
                //     for( uint iTestDof = 0; iTestDof < tRequestedMasterGlobalDofTypes.size(); iTestDof++ )
                //     {
                //         // output for debugging
                //         std::cout << "-------------------------------------------------------------------\n" << std::flush;
                //         std::cout << "Checking test-tractions for test DOF type (0-RHO, 1-VX, 2-TEMP): " << iTestDof << "\n\n" << std::flush;

                //         // derivative dof type
                //         Cell< MSI::Dof_Type > tTestDof = tRequestedMasterGlobalDofTypes( iTestDof );

                //         //------------------------------------------------------------------------------
                //         //  Thermal Test Traction
                //         //------------------------------------------------------------------------------
                //         // evaluate dTestTractiondDOF
                //         Matrix< DDRMat > tdThermalTestTractiondDOF =
                //                 tCMMasterFluid->dTestTractiondDOF(
                //                         tDofDerivative,
                //                         tNormal,
                //                         tTempJump,
                //                         tTestDof,
                //                         CM_Function_Type::THERMAL );

                //         //  evaluate dTestTractiondDOF by FD
                //         Matrix< DDRMat > tdThermalTestTractiondDofFD;
                //         tCMMasterFluid->eval_dtesttractiondu_FD(
                //                 tDofDerivative,
                //                 tTestDof,
                //                 tdThermalTestTractiondDofFD,
                //                 tPerturbation,
                //                 tNormal,
                //                 tTempJump,
                //                 FDScheme_Type::POINT_5,
                //                 CM_Function_Type::THERMAL );

                //         // check that analytical and FD match
                //         bool tCheckThermalTestTraction = fem::check( tdThermalTestTractiondDOF, tdThermalTestTractiondDofFD, tEpsilon );
                //         REQUIRE( tCheckThermalTestTraction );

                //         //------------------------------------------------------------------------------
                //         //  Mechanical Test Traction
                //         //------------------------------------------------------------------------------
                //         // evaluate dTestTractiondDOF
                //         Matrix< DDRMat > tdMechanicalTestTractiondDOF =
                //                 tCMMasterFluid->dTestTractiondDOF(
                //                         tDofDerivative,
                //                         tNormal,
                //                         tVelocityJump,
                //                         tTestDof,
                //                         CM_Function_Type::MECHANICAL );

                //         //  evaluate dTestTractiondDOF by FD
                //         Matrix< DDRMat > tdMechanicalTestTractiondDofFD;
                //         tCMMasterFluid->eval_dtesttractiondu_FD(
                //                 tDofDerivative,
                //                 tTestDof,
                //                 tdMechanicalTestTractiondDofFD,
                //                 tPerturbation,
                //                 tNormal,
                //                 tVelocityJump,
                //                 FDScheme_Type::POINT_5,
                //                 CM_Function_Type::MECHANICAL );

                //         // check that analytical and FD match
                //         bool tCheckMechanicalTestTraction = fem::check( tdMechanicalTestTractiondDOF, tdMechanicalTestTractiondDofFD, tEpsilon );
                //         REQUIRE( tCheckMechanicalTestTraction );
                //     }

                }
            }
            // clean up
            tMasterFIs.clear();
        }
    }
}/*END_TEST_CASE*/