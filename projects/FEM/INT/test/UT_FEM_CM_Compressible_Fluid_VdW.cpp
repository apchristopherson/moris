
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
#include "cl_FEM_Integrator.hpp"
#include "cl_FEM_Property.hpp"
#include "cl_FEM_CM_Factory.hpp"
#include "fn_FEM_Check.hpp"
#include "FEM_Test_Proxy/cl_FEM_Inputs_for_NS_Compressible_UT.cpp"

using namespace moris;
using namespace fem;

TEST_CASE( "CM_Fluid_Compressible_VdW", "[CM_Fluid_Compressible_VdW]" )
{
    // define an epsilon environment
    real tEpsilon = 1.0E-6;
    real tEpsilonCubic = 3.0E-6;

    // define a perturbation relative size
    real tPerturbation = 5.0E-4;
    real tPerturbationCubic = 1.4E-3;

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
    //Matrix< DDRMat > tNumCoeffs = {{ 8, 18 },{ 16, 54 }};
    Matrix< DDRMat > tNumCoeffs = {{ 8, 18, 32 },{ 16, 54, 128 }};

    // dof type list
    moris::Cell< MSI::Dof_Type > tDensityDof  = { MSI::Dof_Type::RHO };
    moris::Cell< MSI::Dof_Type > tVelocityDof = { MSI::Dof_Type::VX };
    moris::Cell< MSI::Dof_Type > tTempDof     = { MSI::Dof_Type::TEMP };
    moris::Cell< moris::Cell< MSI::Dof_Type > > tDofTypes = { tDensityDof, tVelocityDof, tTempDof };

    //------------------------------------------------------------------------------
    // create the properties

    // isochoric heat capacity
    std::shared_ptr< fem::Property > tPropHeatCapacity = std::make_shared< fem::Property >();
    tPropHeatCapacity->set_parameters( { {{ 5.7 }} } );
    tPropHeatCapacity->set_val_function( tConstValFunc );

    // specific gas constant
    std::shared_ptr< fem::Property > tPropGasConstant = std::make_shared< fem::Property >();
    tPropGasConstant->set_parameters( { {{ 2.8 }} } );
    tPropGasConstant->set_val_function( tConstValFunc );

    // dynamic viscosity
    std::shared_ptr< fem::Property > tPropViscosity = std::make_shared< fem::Property >();
    tPropViscosity->set_parameters( { {{ 11.9 }} } );
    tPropViscosity->set_val_function( tConstValFunc );

    // thermal conductivity
    std::shared_ptr< fem::Property > tPropConductivity = std::make_shared< fem::Property >();
    tPropConductivity->set_parameters( { {{ 3.7 }} } );
    tPropConductivity->set_val_function( tConstValFunc );

    // Capillarity Coefficient
    std::shared_ptr< fem::Property > tPropCapillarity = std::make_shared< fem::Property >();
    tPropCapillarity->set_parameters( { {{ 2.1 }} } );
    tPropCapillarity->set_val_function( tConstValFunc );

    // First VdW constant
    std::shared_ptr< fem::Property > tPropFirstVdWconst = std::make_shared< fem::Property >();
    tPropFirstVdWconst->set_parameters( { {{ 9.1 }} } );
    tPropFirstVdWconst->set_val_function( tConstValFunc );

    // Second VdW constant
    std::shared_ptr< fem::Property > tPropSecondVdWconst = std::make_shared< fem::Property >();
    tPropSecondVdWconst->set_parameters( { {{ 6.9 }} } );
    tPropSecondVdWconst->set_val_function( tConstValFunc );


    // define constitutive model and assign properties
    fem::CM_Factory tCMFactory;

    std::shared_ptr< fem::Constitutive_Model > tCMMasterFluid =
            tCMFactory.create_CM( fem::Constitutive_Type::FLUID_COMPRESSIBLE_VDW );
    tCMMasterFluid->set_dof_type_list( {tDensityDof, tVelocityDof, tTempDof } );

    tCMMasterFluid->set_property( tPropHeatCapacity,   "IsochoricHeatCapacity" );
    tCMMasterFluid->set_property( tPropGasConstant,    "SpecificGasConstant" );
    tCMMasterFluid->set_property( tPropViscosity,      "DynamicViscosity" );
    tCMMasterFluid->set_property( tPropConductivity,   "ThermalConductivity" );
    tCMMasterFluid->set_property( tPropCapillarity,    "CapillarityCoefficient" );
    tCMMasterFluid->set_property( tPropFirstVdWconst,  "FirstVdWconstant" );
    tCMMasterFluid->set_property( tPropSecondVdWconst, "SecondVdWconstant" );

    // set a fem set pointer
    MSI::Equation_Set * tSet = new fem::Set();
    tCMMasterFluid->set_set_pointer( static_cast< fem::Set* >( tSet ) );

    // set size for the set EqnObjDofTypeList
    tCMMasterFluid->mSet->mUniqueDofTypeList.resize( 100, MSI::Dof_Type::END_ENUM );

    // set size and populate the set dof type map
    tCMMasterFluid->mSet->mUniqueDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tCMMasterFluid->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::RHO ) )   = 0;
    tCMMasterFluid->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) )    = 1;
    tCMMasterFluid->mSet->mUniqueDofTypeMap( static_cast< int >( MSI::Dof_Type::TEMP ) )  = 2;

    // set size and populate the set master dof type map
    tCMMasterFluid->mSet->mMasterDofTypeMap.set_size( static_cast< int >( MSI::Dof_Type::END_ENUM ) + 1, 1, -1 );
    tCMMasterFluid->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::RHO ) )   = 0;
    tCMMasterFluid->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::VX ) )    = 1;
    tCMMasterFluid->mSet->mMasterDofTypeMap( static_cast< int >( MSI::Dof_Type::TEMP ) )  = 2;

    // build global dof type list
    tCMMasterFluid->get_global_dof_type_list();

    // loop on the space dimension
    for( uint iSpaceDim = 2; iSpaceDim < 4; iSpaceDim++ )
    {
        // output for debugging
        //std::cout << "-------------------------------------------------------------------\n" << std::flush;
        //std::cout << "Performing Tests For Number of Spatial dimensions: " << iSpaceDim << "\n" << std::flush;
        //std::cout << "-------------------------------------------------------------------\n\n" << std::flush;

        // create and set normal
        Matrix< DDRMat > tNormal( iSpaceDim, 1, 0.5 );
        tNormal = tNormal / norm( tNormal );

        // create the jump
        Matrix< DDRMat > tJump( iSpaceDim, 1, 10.0 );

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
        Interpolation_Rule tGIRule(
                tGeometryType,
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
            //std::cout << "-------------------------------------------------------------------\n" << std::flush;
            //std::cout << "-------------------------------------------------------------------\n" << std::flush;
            //std::cout << "Performing Tests For Interpolation Order:" << iInterpOrder << "\n\n" << std::flush;

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
                    fem::Integration_Order::BAR_2 );

            // create an integrator
            fem::Integrator tIntegrator( tIntegrationRule );

            // get integration points
            Matrix< DDRMat > tIntegPoints;
            tIntegrator.get_points( tIntegPoints );

            // field interpolators
            //------------------------------------------------------------------------------
            // create an interpolation order
            mtk::Interpolation_Order tInterpolationOrder = tInterpolationOrders( iInterpOrder - 1 );

            //create a space time interpolation rule
            Interpolation_Rule tFIRule (
                    tGeometryType,
                    Interpolation_Type::LAGRANGE,
                    tInterpolationOrder,
                    Interpolation_Type::LAGRANGE,
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
            tCMMasterFluid->mSet->mMasterFIManager = &tFIManager;

            // set IWG field interpolator manager
            tCMMasterFluid->set_field_interpolator_manager( &tFIManager );

            uint tNumGPs = tIntegPoints.n_cols();
            for( uint iGP = 0; iGP < tNumGPs; iGP ++ )
            {
                // reset IWG evaluation flags
                tCMMasterFluid->reset_eval_flags();

                // create evaluation point xi, tau
                Matrix< DDRMat > tParamPoint = tIntegPoints.get_column( iGP );

                // set integration point
                tCMMasterFluid->mSet->mMasterFIManager->set_space_time( tParamPoint );

                // populate the requested master dof type for CM
                moris::Cell< moris::Cell< MSI::Dof_Type > > tRequestedMasterGlobalDofTypes =
                        tCMMasterFluid->get_global_dof_type_list();

                // populate the test master dof type for CM
                moris::Cell< moris::Cell< MSI::Dof_Type > > tMasterDofTypes =
                        tCMMasterFluid->get_dof_type_list();

                // loop over requested dof type
                for( uint iRequestedDof = 0; iRequestedDof < tRequestedMasterGlobalDofTypes.size(); iRequestedDof++ )
                {
                    // output for debugging
                    //std::cout << "-------------------------------------------------------------------\n" << std::flush;
                    //std::cout << "Performing test for DOF derivative wrt. (0-RHO, 1-VX, 2-TEMP): " << iRequestedDof << "\n\n" << std::flush;

                    // derivative dof type
                    Cell< MSI::Dof_Type > tDofDerivative = tRequestedMasterGlobalDofTypes( iRequestedDof );

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

                    //------------------------------------------------------------------------------
                    //  Pressure
                    //------------------------------------------------------------------------------
                    // evaluate dfluxdu
                    Matrix< DDRMat > tdPressuredDof = tCMMasterFluid->dFluxdDOF( tDofDerivative, CM_Function_Type::PRESSURE );

                    // evaluate dfluxdu by FD
                    Matrix< DDRMat > tdPressuredDofFD;
                    tCMMasterFluid->eval_dFluxdDOF_FD(
                            tDofDerivative,
                            tdPressuredDofFD,
                            tPerturbation,
                            FDScheme_Type::POINT_5,
                            CM_Function_Type::PRESSURE );

                    // check that analytical and FD match
                    bool tCheckPressure = fem::check( tdPressuredDof, tdPressuredDofFD, tEpsilon );
                    REQUIRE( tCheckPressure );

                }
            }
            // clean up
            tMasterFIs.clear();
        }
    }
}/*END_TEST_CASE*/

