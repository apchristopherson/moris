#include <string>
#include <iostream>
#include "typedefs.hpp"
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"
#include "cl_MSI_Equation_Object.hpp"
#include "cl_TSA_Time_Solver.hpp"
#include "cl_DLA_Solver_Interface.hpp"
#include "cl_DLA_Linear_Solver_Aztec.hpp"
#include "fn_PRM_FEM_Parameters.hpp"
#include "fn_PRM_MSI_Parameters.hpp"
#include "fn_PRM_SOL_Parameters.hpp"
#include "fn_PRM_VIS_Parameters.hpp"
#include "fn_PRM_HMR_Parameters.hpp"
#include "fn_PRM_GEN_Parameters.hpp"
#include "fn_PRM_XTK_Parameters.hpp"
#include "fn_PRM_OPT_Parameters.hpp"
#include "cl_HMR_Element.hpp"
#include "fn_equal_to.hpp"
#include "fn_stringify_matrix.hpp"

#include "AztecOO.h"


#ifdef  __cplusplus
extern "C"
{
#endif
//------------------------------------------------------------------------------
namespace moris
{
    /* ------------------------------------------------------------------------ */
    // For Parameter sweep
    
    // file name
    std::string tName = "Mach_Leading_Edge";
    
    // Hole Seeding 
    moris::sint tNumSeedHolesX = 3;
    moris::sint tNumSeedHolesY = 2;
    moris::real tHoleSeedDensityX = 1.0; 
    moris::real tHoleSeedDensityY = 0.7; 
    
    // Interpolation order
    std::string tOrder = "1";
    
    moris::uint tFigureCounter = 0;

    /* ------------------------------------------------------------------------ */
    // For Optimization

    int tNumMaxGcmmaIts = 2;

    // relative step size for MMA
    moris::real tStepSize = 0.1;

    // for volume constraint
    moris::real tInitialFinVolume    = 8.69684e-06;
    moris::real tMaxAllowedFinVolume = 1.0 * tInitialFinVolume;
    moris::real tVolumeScaling = 1.0;

    // for Perimeter penalty
    moris::real tInitialFinPerimeter = 0.0288672;
    moris::real tPerimeterPenaltyParam = 1.0;
	
	// for max temperature objective
    moris::real tRefMaxTemp = 0.0458574;
    moris::real tMaxTempScaling = 1.0;
    
    // for strain energy objective  
    moris::real tInitialStrainEnergy = 1.74024e-05;
    moris::real tStrainEnergyScaling = 5.0; 
    
    // max dof IQI
    std::string tIQIRefTemp = "350.0";
    std::string tExponent   = "10.0";
    std::string tShift      = "0.0";
    
    
    /* ------------------------------------------------------------------------ */
    /* ------------------------------------------------------------------------ */
    // Mesh Set Information

    // WITH PHASE TABLE
    
    ////Bulk Phases
    std::string tSkin  = "HMR_dummy_n_p2,HMR_dummy_c_p2";
    std::string tFins  = "HMR_dummy_n_p3,HMR_dummy_c_p3";
    std::string tShell = tSkin + "," + tFins;
    std::string tPCM   = "HMR_dummy_n_p4,HMR_dummy_c_p4";
    std::string tBSplineGeometry  = tFins;
    
    // interfaces
    std::string tSkinFinsInterface   = "dbl_iside_p0_2_p1_3";    
    std::string tShellShellInterface = tSkinFinsInterface;
    std::string tSkinPCMInterface    = "dbl_iside_p0_2_p1_4";  
    std::string tFinPCMInterface     = "dbl_iside_p0_3_p1_4";
    std::string tShellPCMInterface   = tSkinPCMInterface + "," + tFinPCMInterface;
    
    // boundaries
    std::string tOuterShellSurface = "iside_b0_2_b1_0";
    std::string tSkinBackWall      = "iside_b0_2_b1_1";
    
    // ghost
    std::string tSkinGhost  = "ghost_p2";
    std::string tFinsGhost  = "ghost_p3";
    std::string tShellGhost = tSkinGhost + "," + tFinsGhost;
    std::string tPCMGhost   = "ghost_p4";

    std::string tTotalDomain  = tShell + "," + tPCM;
    std::string tBSplinesPerimeter = "iside_b0_4_b1_3,iside_b0_4_b1_2,iside_b0_4_b1_1";  

    /* ------------------------------------------------------------------------ */    
    // geometry parameters

    // all messurements in millimeters
    moris::real mm = 1.0e-03; 
    
    // Shell  
    moris::real tLength = 12.0 * mm;
    moris::real tWallThickness = 2.0 * mm;
	moris::real tInnerRadius = 1.0 * mm;
    moris::real tEdgeAngle = 5.0; // in degrees
    moris::real tCenterX = 0.0 * mm;
    moris::real tCenterY = 0.0 * mm;
    moris::real tApproxHeight = tLength * std::tan( tEdgeAngle / 180.0 * M_PI );
    
    // Initialize Fins
    moris::sint tNumSeedFinsX = tNumSeedHolesX;
    moris::sint tNumSeedFinsY = tNumSeedHolesY;
    //moris::real tHoleSeedDensity = 1.5
    moris::real tHoleWidth     = tLength / ( tHoleSeedDensityX * tNumSeedFinsX);
    moris::real tHoleHeight    = ( 2.0 * tApproxHeight ) / ( tHoleSeedDensityY * tNumSeedFinsY);
    moris::real tFinExponent  = 6.0;
    
    moris::real tXcenterMin = ( tLength *        1.0           ) / ( (real) tNumSeedFinsX ) - 3.0 * mm; // + 1.0 );
    moris::real tXcenterMax = ( tLength * (real) tNumSeedFinsX ) / ( (real) tNumSeedFinsX ); // + 1.0 );
    moris::real tYcenterMax =  0.9 * tApproxHeight;                                                                                                                                                                                                                                                                                                                                                            
    moris::real tYcenterMin = -1.0 * tYcenterMax;
    
    /* ------------------------------------------------------------------------ */    
    // material parameters, kg is scaled with a factor 1e-6

    // Shell (skin & fins)
	std::string tDensityShell          = "1.0e-2";  // 10,000 kg/m^3
    std::string tHeatCapacityShell     = "200.0";   // 0.2 kJ/kg*K
    std::string tConductivityShell     = "1.5e-4";  // 150 W/m*K
    std::string tYoungsModulusShell    = "2.1e5";   // N/m^2
    std::string tPoissonRatioShell     = "0.31";    //
    std::string tThermalExpansionShell = "1.3e-5";  // 1/K
	
	// PCM
	std::string tDensityPCM          = "6.0e-3";    // 6,000 kg/m^3
    std::string tHeatCapacityPCM     = "500.0";     // 0.5 kJ/kg*K
    std::string tConductivityPCM     = "1.5e-5";    // 15 W/m*K
    std::string tLatentHeatPCM       = "220000.0";  // 220 kJ/kg
    std::string tPCTempPCM           = "750.0";     // deg C                        
    std::string tPCConstPCM          = "30.0";      // K
    std::string tYoungsModulusPCM    = "1.0e3";     // N/m^2
    std::string tPoissonRatioPCM     = "0.3";       //
    std::string tThermalExpansionPCM = "0.0";       // 1/K
    
    /* ------------------------------------------------------------------------ */    
    // boundary conditions

	// turn radiation on or off
	bool tHaveRadiation = true;

    // Pressure
    moris::real tAppliedPressure = 200.0; // in atms, modify to match objective gradients
    std::string tPressureDelta = std::to_string( tAppliedPressure * 1.01325e5 * 1.0e-6 );    
				// N/m^2; positive as pulling in direction of normal
    
    // bedding to supress RBM
    std::string tBedding = std::to_string( 2.1e5 * 1.0e-5 );

    // Initial Temperature
    moris::real tInitialTemp = 700.00;        // in deg C
    std::string tReferenceTemp = "20.0";

    // Heat flux
    std::string tHeatLoad = "3.75";           // 375 W/cm^2
	
	// For Radiation
	std::string tEmissivity = "5.0e-7";       // 0.5
    std::string tAmbientTemp = "0.0";         // in deg C
    std::string tAbsoluteZero = "-273.15";    // in deg C

    /* ------------------------------------------------------------------------ */
    // HMR parameters

    //std::string tOrder = "1";
    
    std::string tNumElemsPerDim = "31, 16";
    std::string tDomainDims = "0.016, 0.08";
    std::string tDomainOffset = "-0.004,-0.004";
    std::string tDomainSidesets = "1,2,3,4";

    int tRefineBuffer         = 2;
    int tAdaptiveRefineBuffer = 3;
    std::string tInitialRefinement    = "0";
    int tAdaptiveRefinements  = 0;

    /* ------------------------------------------------------------------------ */
    // Solver config

    moris::real tNLA_rel_res_norm_drop = 1.0e-05;
    moris::real tNLA_relaxation_parameter = 1.0;
    int tNLA_max_iter = 10;

	int tTSA_Num_Time_Steps = 4;
	moris::real tTSA_Time_Frame = 0.4;

    /* ------------------------------------------------------------------------ */
    // Output Config

    std::string tOutputFileName = tName + ".exo";
    std::string tLibraryName    = tName + ".so";
    std::string tHDF5Path       = tName + ".hdf5";
    std::string tGENOutputFile  = "GEN_" + tName + ".exo";
    bool tOutputCriterion = true;
    
    //------------------------------------------------------------------------------
    //-------------------------------- FUNCTIONS -----------------------------------
    //------------------------------------------------------------------------------    

    /* ------------------------------------------------------------------------ */
    // GEOMETRY (LEVEL-SET) FUNCTIONS
    /* ------------------------------------------------------------------------ */ 
    
    // Outer Wedge
    moris::real Outer_Wedge(
            const moris::Matrix< DDRMat >     & aCoordinates,
            const moris::Cell< moris::real* > & aGeometryParameters )
    {        
        // get angle in rads
        moris::real tAlpha = tEdgeAngle/180.0 * M_PI;

        // get outer radius
        moris::real tRadius = tInnerRadius + tWallThickness;

        // translate coordinate system
        moris::real tXi = aCoordinates(0) - tCenterX;
        moris::real tEta = aCoordinates(1) - tCenterY;
        moris::real tXiInsct = tRadius / std::sin(tAlpha);

        // check which sector the point lies in 
        bool tLeft = false;
        bool tRight = false;
        bool tTop = false;
        bool tBottom = false;

        if (tEta >= 0.0)
            tTop = true;
        else
            tBottom = true;

        if ( ( tTop && tEta < - std::tan(0.5*M_PI-tAlpha) * tXi )  ||  ( tBottom && tEta > std::tan(0.5*M_PI-tAlpha) * tXi ) )
            tLeft = true;
        else
            tRight = true;

        // Compute Signed-Distance field 
        moris::real tVal = 0.0;

        // left sector - circle section
        if (tLeft)
            tVal = tRadius - std::sqrt( std::pow(tXi,2.0) + std::pow(tEta,2.0) ); 

        // top right sector - straight section
        else if (tTop && tRight)
        {
            moris::real tDeltaEta = std::tan(tAlpha) * (tXi+tXiInsct) - tEta;
            tVal = std::cos(tAlpha) * tDeltaEta;
        }

        // bottom right sector - straight section
        else if (tBottom && tRight)
        {
            moris::real tDeltaEta = std::tan(tAlpha) * (tXi+tXiInsct) + tEta;
            tVal = std::cos(tAlpha) * tDeltaEta;
        }

        // clean return value to return non-zero value
        return std::abs(tVal)<1.0e-8 ? 1.0e-8 : tVal;
    }
    
    //-----------------------------------------------------------------------------
    
    // Inner Wall
    moris::real Inner_Wedge(
            const moris::Matrix< DDRMat >     & aCoordinates,
            const moris::Cell< moris::real* > & aGeometryParameters )
    {        
        // get angle in rads
        moris::real tAlpha = tEdgeAngle/180.0 * M_PI;

        // get outer radius
        moris::real tRadius = tInnerRadius;

        // translate coordinate system
        moris::real tXi = aCoordinates(0) - tCenterX;
        moris::real tEta = aCoordinates(1) - tCenterY;
        moris::real tXiInsct = tRadius / std::sin(tAlpha);

        // check which sector the point lies in 
        bool tLeft = false;
        bool tRight = false;
        bool tTop = false;
        bool tBottom = false;

        if (tEta >= 0.0)
            tTop = true;
        else
            tBottom = true;

        if ( ( tTop && tEta < - std::tan(0.5*M_PI-tAlpha) * tXi )  ||  ( tBottom && tEta > std::tan(0.5*M_PI-tAlpha) * tXi ) )
            tLeft = true;
        else
            tRight = true;

        // Compute Signed-Distance field 
        moris::real tVal = 0.0;

        // left sector - circle section
        if (tLeft)
            tVal = tRadius - std::sqrt( std::pow(tXi,2.0) + std::pow(tEta,2.0) ); 

        // top right sector - straight section
        else if (tTop && tRight)
        {
            moris::real tDeltaEta = std::tan(tAlpha) * (tXi+tXiInsct) - tEta;
            tVal = std::cos(tAlpha) * tDeltaEta;
        }

        // bottom right sector - straight section
        else if (tBottom && tRight)
        {
            moris::real tDeltaEta = std::tan(tAlpha) * (tXi+tXiInsct) + tEta;
            tVal = std::cos(tAlpha) * tDeltaEta;
        }

        // clean return value to return non-zero value
        return std::abs(tVal)<1.0e-8 ? 1.0e-8 : tVal;
    }
    
    //-----------------------------------------------------------------------------
    
    // Back Wall
    moris::real Back_Wall(
            const moris::Matrix< DDRMat >     & aCoordinates,
            const moris::Cell< moris::real* > & aGeometryParameters )
    {      
        // compute level set value
        moris::real aReturnValue = ( aCoordinates(0) - tLength + tInnerRadius + tWallThickness );

        // clean return value to return non-zero value
        return std::abs(aReturnValue) < 1e-8 ? 1e-8 : aReturnValue;
    }  

    /* ------------------------------------------------------------------------ */
    // PROPERTY FUNCTIONS (incl. INITIAL & BOUNDARY CONDITIONS)
    /* ------------------------------------------------------------------------ */       
    
    // Constant function for properties
    void Func_Const( 
            moris::Matrix< moris::DDRMat >                 & aPropMatrix,
            moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
            moris::fem::Field_Interpolator_Manager         * aFIManager )
    {
        aPropMatrix = aParameters( 0 );
    }
    
    // function for concentrating the heat load at the tip
    void Func_Heat_Load_Distribution( moris::Matrix<
            moris::DDRMat >                                & aPropMatrix,
            moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
            moris::fem::Field_Interpolator_Manager         * aFIManager )
    {
        // get coordinates
        moris::Matrix<DDRMat> tPosition = aFIManager->get_IG_geometry_interpolator()->valx();
        moris::real tEtaPosition = tPosition(1) - tCenterY;
        moris::real tXiPosition = tPosition(0) - tCenterX;

        // get surface angle of point on surface
        moris::real tTheta = std::atan2( std::abs(tEtaPosition), - tXiPosition );
        if ( tTheta > (90.0 - tEdgeAngle) * M_PI / 180.0 )
            tTheta = (90.0 - tEdgeAngle) * M_PI / 180.0;

        // compute heat load distribution
        // help parameter c
        moris::real tC = 1.0 / ( 1.4 * std::pow( 7.0, 2.0 ) );

        // help parameter G
        moris::real tG = (1.0 - tC) *
                ( std::pow( tTheta, 2.0 ) - 0.5 * tTheta * std::sin( 4.0 * tTheta ) + 0.125 * ( 1.0 - std::cos( 4.0 * tTheta ) ) )
                + 4.0 * tC * ( std::pow( tTheta, 2.0 ) - tTheta * std::sin( 2.0 * tTheta ) + 0.5 * ( 1.0 - std::cos( 2.0 * tTheta ) ) );

        // compute heat flux relative to stagnation point
        moris::real tR = 2.0 * tTheta * std::sin( tTheta ) * ( ( 1.0 - tC ) * std::pow( std::cos( tTheta ), 2.0 ) + tC ) * std::pow( tG, -0.5 );

        // compute final heat flux using stagnation point heat flux
        aPropMatrix = aParameters( 0 ) * tR;
    }
	
    // initial temperature
	void Func_Initial_Condition(
            moris::Matrix< moris::DDRMat >                 & aPropMatrix,
            moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
            moris::fem::Field_Interpolator_Manager         * aFIManager )
    {
        aPropMatrix = {{ tInitialTemp }};
    }
    
    /* ------------------------------------------------------------------------ */
    // DUMMY FUNCTIONS
    /* ------------------------------------------------------------------------ */       
    
    // Output criterion for VIS mesh
    bool Output_Criterion( moris::tsa::Time_Solver * aTimeSolver )
    {
        return tOutputCriterion;
    }
    
    // Dummy function for unused sensitivities if needed
    moris::Matrix< DDRMat > Func_Dummy_Sensitivity(
            const moris::Matrix< DDRMat >     & aCoordinates,
            const moris::Cell< moris::real* > & aGeometryParameters )
    {
        moris::Matrix< DDRMat > aReturnValue = {{0.0}};
        return aReturnValue;
    }
    
    /* ------------------------------------------------------------------------ */
    // FOR OPTIMIZATION
    /* ------------------------------------------------------------------------ */
    
    moris::Matrix< moris::DDSMat > get_constraint_types()
    {
        Matrix<DDSMat> tConstraintTypes( 1, 1, 1 );
        return tConstraintTypes;
    }

    moris::Matrix< moris::DDRMat > compute_objectives(
        Matrix<DDRMat> aADVs, 
        moris::Matrix< moris::DDRMat > aCriteria)
    {
        moris::Matrix< moris::DDRMat > tObjectives( 1, 1, 0.0 );
        
        tObjectives( 0, 0 ) = aCriteria( 3 ) * tStrainEnergyScaling / tInitialStrainEnergy + 
                              aCriteria( 0 ) * tMaxTempScaling / tRefMaxTemp + 
                              aCriteria( 2 ) * tPerimeterPenaltyParam  / tInitialFinPerimeter;
        
		std::cout << "% --------------------------------- % \n" << std::flush;
        std::cout << "% --------------------------------- % \n" << std::flush;
        std::cout << "Max Temp Value = " << aCriteria( 0 ) << " \n" << std::flush;
		std::cout << "Fin Volume = "     << aCriteria( 1 ) << " \n" << std::flush;
		std::cout << "Perimeter = "      << aCriteria( 2 ) << " \n" << std::flush;
        //std::cout << "Total Volume = "   << aCriteria( 3 ) << " \n" << std::flush;
        std::cout << "Strain Energy = "  << aCriteria( 3 ) << " \n" << std::flush;
		std::cout << "% --------------------------------- % \n" << std::flush;
        
        std::cout << " \n";
        std::cout << "% --------------------------------- % \n" << std::flush;

        std::cout << "min ADV       = " << aADVs.min()         << " \n";
        std::cout << "max ADV       = " << aADVs.max()         << " \n" << std::flush;
		
        std::cout << "Objective = " << tObjectives( 0, 0 ) << " \n" << std::flush;
		std::cout << "% --------------------------------- % \n" << std::flush;
        
		//  IQIMaxTemp, IQIStrainEnergy, IQIBSplineGeometryVolume, IQIBSplinesPerimeter, IQITotalVolume
        
        return tObjectives;
    }

    moris::Matrix< moris::DDRMat > compute_constraints(
        Matrix<DDRMat> aADVs, 
        moris::Matrix< moris::DDRMat > aCriteria)
    {
        moris::Matrix< moris::DDRMat > tConstraints( 1, 1, 0.0 );
        
        tConstraints( 0, 0 ) = aCriteria( 1 ) * tVolumeScaling / tMaxAllowedFinVolume - 1.0 * tVolumeScaling;
        
		std::cout << "Constraint = " << tConstraints( 0, 0 ) << " \n" << std::flush;
        std::cout << "% --------------------------------- % \n" << std::flush;
		std::cout << "% --------------------------------- % \n" << std::flush;
        
        return tConstraints;
    }

    moris::Matrix< moris::DDRMat > compute_dobjective_dadv(
        Matrix<DDRMat> aADVs, 
        moris::Matrix< moris::DDRMat > aCriteria)
    {
        moris::Matrix< moris::DDRMat > tDObjectiveDADV( 1, aADVs.length(), 0.0 );
        return tDObjectiveDADV;
    }
    
    //  IQIMaxTemp, IQIStrainEnergy, IQIBSplineGeometryVolume, IQIBSplinesPerimeter, IQITotalVolume

    moris::Matrix< moris::DDRMat > compute_dobjective_dcriteria(
        moris::Matrix< moris::DDRMat > aADVs, 
        moris::Matrix< moris::DDRMat > aCriteria)
    {
        moris::Matrix< moris::DDRMat > tDObjectiveDCriteria( 1, 4, 0.0 );
        
        tDObjectiveDCriteria( 0 ) = tMaxTempScaling / tRefMaxTemp;
        tDObjectiveDCriteria( 2 ) = tPerimeterPenaltyParam / tInitialFinPerimeter;
        tDObjectiveDCriteria( 3 ) = tStrainEnergyScaling / tInitialStrainEnergy;
        
        std::cout << "% --------------------------------- % \n" << std::flush;
        std::cout << "dz/d(Max_Temp) = " << tDObjectiveDCriteria( 0 ) << " \n" << std::flush;
        std::cout << "dz/d(Perimeter) = " << tDObjectiveDCriteria( 2 ) << " \n" << std::flush;
        //std::cout << "dz/d(Total_Volume) = " << tDObjectiveDCriteria( 3 ) << " \n" << std::flush;
        std::cout << "dz/d(Strain_Energy) = " << tDObjectiveDCriteria( 3 ) << " \n" << std::flush;
        
        
        return tDObjectiveDCriteria;
    }

    moris::Matrix< moris::DDRMat > compute_dconstraint_dadv(
        moris::Matrix< moris::DDRMat > aADVs, 
        moris::Matrix< moris::DDRMat > aCriteria)
    {
        moris::Matrix< moris::DDRMat > tDConstraintDADV( 1, aADVs.length(), 0.0 );
        return tDConstraintDADV;
    }
    
    moris::Matrix< moris::DDRMat > compute_dconstraint_dcriteria(
        moris::Matrix< moris::DDRMat > aADVs, 
        moris::Matrix< moris::DDRMat > aCriteria)
    {
        moris::Matrix< moris::DDRMat > tDConstraintDCriteria( 1, 4, 0.0 );
        
        tDConstraintDCriteria( 1 ) = tVolumeScaling / tMaxAllowedFinVolume;
        
        std::cout << "dg/d(Fin_Volume) = " << tDConstraintDCriteria( 1 ) << " \n" << std::flush;
        std::cout << "% --------------------------------- % \n" << std::flush;
        
        return tDConstraintDCriteria;
    }        

    /* ------------------------------------------------------------------------ */
    // PARAMETER LISTS
    /* ------------------------------------------------------------------------ */    

    void OPTParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterlist )
    {
        tParameterlist.resize( 3 );
        tParameterlist( 0 ).resize( 1 );
        tParameterlist( 1 ).resize( 0 );
        tParameterlist( 2 ).resize( 1 );
 
        tParameterlist(0)(0) = moris::prm::create_opt_problem_parameter_list();
        tParameterlist(0)(0).set( "is_optimization_problem", true );
        tParameterlist(0)(0).set( "problem", "user_defined" );
        tParameterlist(0)(0).set( "library", tLibraryName );
 
        tParameterlist(2)(0) = moris::prm::create_gcmma_parameter_list();
        tParameterlist(2)(0).set( "max_its", tNumMaxGcmmaIts );
        tParameterlist(2)(0).set( "step_size", tStepSize );
    }

    /* ------------------------------------------------------------------------ */

    void HMRParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterlist )
    {
        tParameterlist.resize( 1 );
        tParameterlist( 0 ).resize( 1 );

        tParameterlist( 0 )( 0 ) = prm::create_hmr_parameter_list();

        tParameterlist( 0 )( 0 ).set( "number_of_elements_per_dimension", tNumElemsPerDim );
        tParameterlist( 0 )( 0 ).set( "domain_dimensions",                tDomainDims );
        tParameterlist( 0 )( 0 ).set( "domain_offset",                    tDomainOffset );
        tParameterlist( 0 )( 0 ).set( "domain_sidesets",                  tDomainSidesets);
        tParameterlist( 0 )( 0 ).set( "lagrange_output_meshes",           std::string("0"));

        tParameterlist( 0 )( 0 ).set( "lagrange_orders",  tOrder );
        tParameterlist( 0 )( 0 ).set( "lagrange_pattern", std::string( "0" ) );
        tParameterlist( 0 )( 0 ).set( "bspline_orders",   tOrder );
        tParameterlist( 0 )( 0 ).set( "bspline_pattern",  std::string( "0" ) );

        tParameterlist( 0 )( 0 ).set( "lagrange_to_bspline", std::string("0") );

        tParameterlist( 0 )( 0 ).set( "truncate_bsplines",  1 );
        tParameterlist( 0 )( 0 ).set( "refinement_buffer",  tRefineBuffer );
        tParameterlist( 0 )( 0 ).set( "staircase_buffer",   tRefineBuffer );
        tParameterlist( 0 )( 0 ).set( "initial_refinement", tInitialRefinement );

        tParameterlist( 0 )( 0 ).set( "use_number_aura", 1);

        tParameterlist( 0 )( 0 ).set( "use_multigrid",  0 );
        tParameterlist( 0 )( 0 ).set( "severity_level", 0 );

        tParameterlist( 0 )( 0 ).set( "adaptive_refinement_level", tAdaptiveRefineBuffer );
    }

    /* ------------------------------------------------------------------------ */

    void XTKParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterlist )
    {
        tParameterlist.resize( 1 );
        tParameterlist( 0 ).resize( 1 );

        tParameterlist( 0 )( 0 ) = prm::create_xtk_parameter_list();
        tParameterlist( 0 )( 0 ).set( "decompose",                 true );
        tParameterlist( 0 )( 0 ).set( "decomposition_type",        std::string("conformal") );
        tParameterlist( 0 )( 0 ).set( "enrich",                    true );
        tParameterlist( 0 )( 0 ).set( "basis_rank",                std::string("bspline") );
        tParameterlist( 0 )( 0 ).set( "enrich_mesh_indices",       std::string("0") );
        tParameterlist( 0 )( 0 ).set( "ghost_stab",                true );
        tParameterlist( 0 )( 0 ).set( "multigrid",                 false );
        tParameterlist( 0 )( 0 ).set( "verbose",                   true );
        tParameterlist( 0 )( 0 ).set( "print_enriched_ig_mesh",    true );
        tParameterlist( 0 )( 0 ).set( "exodus_output_XTK_ig_mesh", true );
        tParameterlist( 0 )( 0 ).set( "high_to_low_dbl_side_sets", true );
    }

    /* ------------------------------------------------------------------------ */

    void GENParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterlist )
    {
        tParameterlist.resize( 3 );
        tParameterlist( 0 ).resize( 1 );

        // Main GEN parameter list
        tParameterlist( 0 )( 0 ) = prm::create_gen_parameter_list();      
        tParameterlist( 0 )( 0 ).set( "IQI_types", "IQIMaxTemp,IQIBSplineGeometryVolume,IQIBSplinesPerimeter,IQIStrainEnergy" ); // IQIBSplinesPerimeter        
        tParameterlist( 0 )( 0 ).set( "output_mesh_file", tGENOutputFile ); 
        
        // phase table
        Matrix< DDUMat > tPhaseMap( 16, 1, 0 );
        tPhaseMap(  8 ) = 2; // Skin
        tPhaseMap(  9 ) = 2; // Skin
        tPhaseMap( 10 ) = 1; // Back wall
        tPhaseMap( 11 ) = 1; // Back wall
        tPhaseMap( 12 ) = 4; // PCM
        tPhaseMap( 13 ) = 3; // Fins
        tPhaseMap( 14 ) = 1; // Back wall
        tPhaseMap( 15 ) = 1; // Back wall
        std::string tPhaseMapString = moris::ios::stringify( tPhaseMap );
        tParameterlist( 0 )( 0 ).set( "print_phase_table", true );
        tParameterlist( 0 )( 0 ).set( "phase_table", tPhaseMapString );

        // init geometry counter
        uint tGeoCounter = 0;
        
        // Outer Wall
        tParameterlist( 1 ).push_back( prm::create_user_defined_geometry_parameter_list() );
        tParameterlist( 1 )( tGeoCounter ).set( "field_function_name", "Outer_Wedge" );
        tParameterlist( 1 )( tGeoCounter ).set( "number_of_refinements", tAdaptiveRefinements );
        tGeoCounter++;
        
        // Inner Wall
        tParameterlist( 1 ).push_back( prm::create_user_defined_geometry_parameter_list() );
        tParameterlist( 1 )( tGeoCounter ).set( "field_function_name", "Inner_Wedge" );    
        tParameterlist( 1 )( tGeoCounter ).set( "number_of_refinements", tAdaptiveRefinements );
        tGeoCounter++;
        
        // Back Wall
        tParameterlist( 1 ).push_back( prm::create_user_defined_geometry_parameter_list() );
        tParameterlist( 1 )( tGeoCounter ).set( "field_function_name", "Back_Wall" );       
        tParameterlist( 1 )( tGeoCounter ).set( "number_of_refinements", tAdaptiveRefinements );
        tGeoCounter++;
                            
        // initialize fins as swiss cheese geometry
        tParameterlist( 1 ).push_back( prm::create_swiss_cheese_slice_parameter_list() );
        
        tParameterlist( 1 )( tGeoCounter ).set( "left_bound",            tXcenterMin );          // Left-most hole center
        tParameterlist( 1 )( tGeoCounter ).set( "right_bound",           tXcenterMax );          // Right-most hole center
        tParameterlist( 1 )( tGeoCounter ).set( "bottom_bound",          tYcenterMin );          // Bottom-most hole center
        tParameterlist( 1 )( tGeoCounter ).set( "top_bound",             tYcenterMax );          // Top-most hole center
        tParameterlist( 1 )( tGeoCounter ).set( "hole_x_semidiameter",   0.5 * tHoleWidth  );     // Superellipse semi-diameter in the x direction
        tParameterlist( 1 )( tGeoCounter ).set( "hole_y_semidiameter",   0.5 * tHoleHeight );     // Superellipse semi-diameter in the y direction
        tParameterlist( 1 )( tGeoCounter ).set( "superellipse_exponent", tFinExponent );         // Superellipse exponent
        tParameterlist( 1 )( tGeoCounter ).set( "superellipse_scaling",  std::sqrt(tHoleWidth*tHoleHeight));         // Superellipse exponent
        tParameterlist( 1 )( tGeoCounter ).set( "superellipse_regularization",  0.0);        // Superellipse exponent 1/tFinWidth/tFinHeight

        tParameterlist( 1 )( tGeoCounter ).set( "number_of_x_holes",     tNumSeedFinsX );        // Number of holes in the x direction
        tParameterlist( 1 )( tGeoCounter ).set( "number_of_y_holes",     tNumSeedFinsY );        // Number of holes in the y direction

        tParameterlist( 1 )( tGeoCounter ).set( "number_of_refinements", tAdaptiveRefinements ); // Number of refinement steps using HMR

        tParameterlist( 1 )( tGeoCounter ).set( "bspline_mesh_index",   0 );     // Index of B-spline mesh to create level set field on (-1 = none)
        tParameterlist( 1 )( tGeoCounter ).set( "bspline_lower_bound", -0.0025);   // Lower bound of level set field (if bspline_mesh_index >= 0)
        tParameterlist( 1 )( tGeoCounter ).set( "bspline_upper_bound",  0.0025);   // Upper bound of level set field (if bspline_mesh_index >= 0)
        tGeoCounter++;       
    }

    /* ------------------------------------------------------------------------ */

    void FEMParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterList )
    {
        // create a cell of cell of parameter list for fem
        tParameterList.resize( 6 );

        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        // init property counter
        uint tPropCounter = 0;
        
        //------------------------------------------------------------------------------
        // MATERIAL PARAMETERS - STRUCTURE (ni-w-alloy?)
        //------------------------------------------------------------------------------
		
		// Density Shell
		tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropDensity_Shell") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tDensityShell );                 // divide by 1000
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;
		
		// Heat Capacity Shell
		tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropHeatCapacity_Shell") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tHeatCapacityShell );                   
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;
		
        // Conductivity Shell
		tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropConductivity_Shell") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tConductivityShell );                  // divide by  1e6, assume ~90W/mK
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;
        
        // Youngs Modulus Shell
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropYoungsModulus_Shell") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tYoungsModulusShell );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // Poisson Ratio Shell
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropPoissonRatio_Shell") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tPoissonRatioShell );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;
        
        // CTE for Shell
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropThermalExpansion_Shell") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tThermalExpansionShell );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;
		
		//------------------------------------------------------------------------------
        // MATERIAL PARAMETERS - PCM (Al-Cu-Si-alloy?)
        //------------------------------------------------------------------------------
        
		// Density of PCM
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropDensity_PCM") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tDensityPCM );                  
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;	
		
        // Heat Capacity of PCM
		tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropHeatCapacity_PCM") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tHeatCapacityPCM );   
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;	

		// Conductivity of PCM
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropConductivity_PCM") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tConductivityPCM );   
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;	

        // Latent Heat of PCM
		tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropLatentHeat_PCM") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tLatentHeatPCM );            
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // Phase Change Temperature of PCM
		tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropPCTemp_PCM") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tPCTempPCM );            
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // Phase Change Temperature Range of PCM
		tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropPCconst_PCM") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tPCConstPCM );               
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;	

		// Cubic Phase State Function for phase change model
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropPhaseState_PCM") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("2.0") );  
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;
        
        // Youngs Modulus for PCM
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropYoungsModulus_PCM") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tYoungsModulusPCM );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // Poisson Ratio for PCM
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropPoissonRatio_PCM") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tPoissonRatioPCM );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // CTE for PCM
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropThermalExpansion_PCM") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tThermalExpansionPCM );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;
		
        //------------------------------------------------------------------------------
        // OTHER MATERIAL PARAMETERS
        //------------------------------------------------------------------------------		
		
        // properties for bedding (supression for RBMs, both Shell and PCM)
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            "PropBedding");
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tBedding);
        tParameterList( 0 )( tPropCounter ).set( "value_function",           "Func_Const");
        tPropCounter++;
		
		// Dummy latent heat for non-pc material
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropLatentHeat_Dummy") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("0.0") );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropPCTemp_Dummy") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("10000.0") );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;
        
        //------------------------------------------------------------------------------
        // BOUNDARY CONDITIONS 
        //------------------------------------------------------------------------------

        // heat flux from outside
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropImposedFlux") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tHeatLoad ); 
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Heat_Load_Distribution") );
		//tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // reference temperature for thermal expansion
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropReferenceTemp") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tReferenceTemp );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;       
        
        // pressure load
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropPressure") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tPressureDelta );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;  

        // Dirichlet structure
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropDirichletStruct") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      "0.0;0.0" );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;
		
        // emissivity for radiation BC
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropEmissivity") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tEmissivity );              
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // ambient temperature 
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropAmbientTemp") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tAmbientTemp );              
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // absolute zero
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropAbsoluteZero") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tAbsoluteZero );              
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;  		

        // time continuity weights        
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropWeightCurrent") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("100.0") );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropWeightPrevious") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("100.0") );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // Initial Temperature       
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropInitialCondition") );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Initial_Condition") );
        tPropCounter++;
		
		//------------------------------------------------------------------------------
        // IQIs
        //------------------------------------------------------------------------------
		
        // Reference Temperature for MAX_DOF - IQI
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropMaxTempReference") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tIQIRefTemp );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // Exponent for MAX_DOF - IQI
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropMaxTempExponent") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tExponent );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;
        
        // Shift for MAX_DOF - IQI
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropMaxTempShift") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      tShift );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;
		
        // Reference Temperature for MAX_VMStress - IQI
        // tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        // tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropMaxStressReference") );
        // tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("1.0") );
        // tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        // tPropCounter++;

        // Exponent for MAX_VMStress - IQI
        // tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        // tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropMaxStressExponent") );
        // tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("2.0") );
        // tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        // tPropCounter++;		

        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
		// init CM counter
        uint tCMCounter = 0;
		
		//------------------------------------------------------------------------------
        // DIFFUSION
        //------------------------------------------------------------------------------		

		// create parameter list for constitutive model - shell - 1
        tParameterList( 1 ).push_back( prm::create_constitutive_model_parameter_list() );
        tParameterList( 1 )( tCMCounter ).set( "constitutive_name", std::string("CMDiffusion_Shell_1") );
        tParameterList( 1 )( tCMCounter ).set( "constitutive_type", static_cast< uint >( fem::Constitutive_Type::DIFF_LIN_ISO ) );
        tParameterList( 1 )( tCMCounter ).set( "dof_dependencies",  std::pair< std::string, std::string >( "TEMP", "Temperature" ) );
        tParameterList( 1 )( tCMCounter ).set( "properties",
                std::string("PropConductivity_Shell , Conductivity;") +
                std::string("PropDensity_Shell      , Density;")      +
                std::string("PropHeatCapacity_Shell , HeatCapacity") );
        tCMCounter++;
		
		//create parameter list for constitutive model - shell - 2 (for Nitsche interfaces)
        tParameterList( 1 ).push_back( prm::create_constitutive_model_parameter_list() );
        tParameterList( 1 )( tCMCounter ).set( "constitutive_name", std::string("CMDiffusion_Shell_2") );
        tParameterList( 1 )( tCMCounter ).set( "constitutive_type", static_cast< uint >( fem::Constitutive_Type::DIFF_LIN_ISO ) );
        tParameterList( 1 )( tCMCounter ).set( "dof_dependencies",  std::pair< std::string, std::string >( "TEMP", "Temperature" ) );
        tParameterList( 1 )( tCMCounter ).set( "properties",
                std::string("PropConductivity_Shell , Conductivity;") +
                std::string("PropDensity_Shell      , Density;")      +
                std::string("PropHeatCapacity_Shell , HeatCapacity") );
        tCMCounter++;
		
        // diffusion with phase change - PCM - 1
        tParameterList( 1 ).push_back( prm::create_constitutive_model_parameter_list() );
        tParameterList( 1 )( tCMCounter ).set( "constitutive_name", std::string("CMDiffusion_PCM") );
        tParameterList( 1 )( tCMCounter ).set( "constitutive_type", static_cast< uint >( fem::Constitutive_Type::DIFF_LIN_ISO_PC ) );
        tParameterList( 1 )( tCMCounter ).set( "dof_dependencies",  std::pair< std::string, std::string >( "TEMP", "Temperature" ) );
        tParameterList( 1 )( tCMCounter ).set( "properties",
                std::string("PropConductivity_PCM, Conductivity;")         +
                std::string("PropDensity_PCM     , Density;")              +
                std::string("PropHeatCapacity_PCM, HeatCapacity;")        +
                std::string("PropLatentHeat_PCM  , LatentHeat;")          +
                std::string("PropPCTemp_PCM      , PCTemp;")              +
                std::string("PropPhaseState_PCM  , PhaseStateFunction;") +
                std::string("PropPCconst_PCM     , PhaseChangeConst")    );
        tCMCounter++;	
        
        //------------------------------------------------------------------------------
        // LINEAR ELASTICITY
        //------------------------------------------------------------------------------

        // linear elasticity - shell - 1
        tParameterList( 1 ).push_back( prm::create_constitutive_model_parameter_list() );
        tParameterList( 1 )( tCMCounter ).set( "constitutive_name", "CMStrucLinIso_Shell_1");
        tParameterList( 1 )( tCMCounter ).set( "model_type", static_cast< uint >( fem::Model_Type::PLANE_STRESS ) );
        tParameterList( 1 )( tCMCounter ).set( "constitutive_type", static_cast< uint >( fem::Constitutive_Type::STRUC_LIN_ISO ) );
        tParameterList( 1 )( tCMCounter ).set( "dof_dependencies",  std::pair< std::string, std::string >( "UX,UY;TEMP", "Displacement,Temperature" ) );
        tParameterList( 1 )( tCMCounter ).set( "properties",
                std::string("PropYoungsModulus_Shell,    YoungsModulus;") +
                std::string("PropPoissonRatio_Shell,     PoissonRatio;") +
                std::string("PropThermalExpansion_Shell, CTE;") +
                std::string("PropReferenceTemp,          ReferenceTemperature"));
        tCMCounter++;

        // linear elasticity - shell - 2
        tParameterList( 1 ).push_back( prm::create_constitutive_model_parameter_list() );
        tParameterList( 1 )( tCMCounter ).set( "constitutive_name", "CMStrucLinIso_Shell_2");
        tParameterList( 1 )( tCMCounter ).set( "model_type", static_cast< uint >( fem::Model_Type::PLANE_STRESS ) );
        tParameterList( 1 )( tCMCounter ).set( "constitutive_type", static_cast< uint >( fem::Constitutive_Type::STRUC_LIN_ISO ) );
        tParameterList( 1 )( tCMCounter ).set( "dof_dependencies",  std::pair< std::string, std::string >( "UX,UY;TEMP", "Displacement,Temperature" ) );
        tParameterList( 1 )( tCMCounter ).set( "properties",
                std::string("PropYoungsModulus_Shell,    YoungsModulus;") +
                std::string("PropPoissonRatio_Shell,     PoissonRatio;") +
                std::string("PropThermalExpansion_Shell, CTE;") +
                std::string("PropReferenceTemp,          ReferenceTemperature"));
        tCMCounter++;

		// linear elasticity - PCM - 1
        tParameterList( 1 ).push_back( prm::create_constitutive_model_parameter_list() );
        tParameterList( 1 )( tCMCounter ).set( "constitutive_name", "CMStrucLinIso_PCM");
        tParameterList( 1 )( tCMCounter ).set( "constitutive_type", static_cast< uint >( fem::Constitutive_Type::STRUC_LIN_ISO ) );
        tParameterList( 1 )( tCMCounter ).set( "dof_dependencies",  std::pair< std::string, std::string >( "UX,UY;TEMP", "Displacement,Temperature" ) );
        tParameterList( 1 )( tCMCounter ).set( "properties",
                std::string("PropYoungsModulus_PCM,    YoungsModulus;") +
                std::string("PropPoissonRatio_PCM,     PoissonRatio;") +
                std::string("PropThermalExpansion_PCM, CTE;") +
                std::string("PropReferenceTemp,         ReferenceTemperature"));
        tCMCounter++;

        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        // init SP counter
        uint tSPCounter = 0;
		
		//------------------------------------------------------------------------------
        // GGLS
        //------------------------------------------------------------------------------		

        // create parameter list for GGLS stabilization parameter for Skin 
        tParameterList( 2 ).push_back( prm::create_stabilization_parameter_parameter_list() );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_name",      std::string("SPGGLSDiffusion_Shell") );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_type",      static_cast< uint >( fem::Stabilization_Type::GGLS_DIFFUSION ) );
        tParameterList( 2 )( tSPCounter ).set( "master_dof_dependencies", std::pair< std::string, std::string >( "TEMP", "Temperature" ) );
        tParameterList( 2 )( tSPCounter ).set( "master_properties",
                std::string("PropConductivity_Shell , Conductivity;")       +
                std::string("PropDensity_Shell      , Density;")            +
                std::string("PropHeatCapacity_Shell , HeatCapacity;")       +
                std::string("PropLatentHeat_Dummy   , LatentHeat;")         +
                std::string("PropPCTemp_Dummy       , PCTemp;")             +
                std::string("PropPhaseState_PCM     , PhaseStateFunction;") +
                std::string("PropPCconst_PCM        , PhaseChangeConst")    );
        tSPCounter++;

        //create parameter list for GGLS stabilization parameter for PCM 
        tParameterList( 2 ).push_back( prm::create_stabilization_parameter_parameter_list() );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_name",      std::string("SPGGLSDiffusion_PCM") );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_type",      static_cast< uint >( fem::Stabilization_Type::GGLS_DIFFUSION ) );
        tParameterList( 2 )( tSPCounter ).set( "master_dof_dependencies", std::pair< std::string, std::string >( "TEMP", "Temperature" ) );
        tParameterList( 2 )( tSPCounter ).set( "master_properties",
                std::string("PropConductivity_PCM , Conductivity;")       +
                std::string("PropDensity_PCM      , Density;")            +
                std::string("PropHeatCapacity_PCM , HeatCapacity;")       +
                std::string("PropLatentHeat_PCM   , LatentHeat;")         +
                std::string("PropPCTemp_PCM       , PCTemp;")             +
                std::string("PropPhaseState_PCM   , PhaseStateFunction;") +
                std::string("PropPCconst_PCM      , PhaseChangeConst")    );
        tSPCounter++;
        
		//------------------------------------------------------------------------------
        // NITSCHE DIRICHLET
        //------------------------------------------------------------------------------
		
		// Displacements - Shell - back wall
        tParameterList( 2 ).push_back( prm::create_stabilization_parameter_parameter_list() );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_name",      "SPNitscheStruc");
        tParameterList( 2 )( tSPCounter ).set( "stabilization_type",      static_cast< uint >( fem::Stabilization_Type::DIRICHLET_NITSCHE ) );
        tParameterList( 2 )( tSPCounter ).set( "function_parameters",     "100.0");
        tParameterList( 2 )( tSPCounter ).set( "master_properties",       "PropYoungsModulus_Shell,Material");
        tSPCounter++;
		
		//------------------------------------------------------------------------------
        // NITSCHE INTERFACE
        //------------------------------------------------------------------------------

        // Temperature - Shell - PCM
        tParameterList( 2 ).push_back( prm::create_stabilization_parameter_parameter_list() );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_name",  std::string("SPInterfaceShellPCMNitsche") );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_type",  static_cast< uint >( fem::Stabilization_Type::NITSCHE_INTERFACE ) );
        tParameterList( 2 )( tSPCounter ).set( "function_parameters", std::string("100.0") );
        tParameterList( 2 )( tSPCounter ).set( "master_properties",   std::string("PropConductivity_Shell,Material") );
        tParameterList( 2 )( tSPCounter ).set( "slave_properties",    std::string("PropConductivity_PCM,Material") );
        tSPCounter++;
        
        // Displacements - Shell - PCM - not coupled!
		
		// Temperature - Shell - Shell
		tParameterList( 2 ).push_back( prm::create_stabilization_parameter_parameter_list() ); 
        tParameterList( 2 )( tSPCounter ).set( "stabilization_name",  std::string("SPInterfaceShellShellNitsche") );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_type",  static_cast< uint >( fem::Stabilization_Type::NITSCHE_INTERFACE ) );
        tParameterList( 2 )( tSPCounter ).set( "function_parameters", std::string("100.0") );
        tParameterList( 2 )( tSPCounter ).set( "master_properties",   std::string("PropConductivity_Shell,Material") );
        tParameterList( 2 )( tSPCounter ).set( "slave_properties",    std::string("PropConductivity_Shell,Material") );
        tSPCounter++;
        
		// Displacements - Shell - Shell
		tParameterList( 2 ).push_back( prm::create_stabilization_parameter_parameter_list() );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_name",  std::string("SPInterfaceShellShellNitscheStruct") );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_type",  static_cast< uint >( fem::Stabilization_Type::NITSCHE_INTERFACE ) );
        tParameterList( 2 )( tSPCounter ).set( "function_parameters", std::string("100.0") );
        tParameterList( 2 )( tSPCounter ).set( "master_properties",   std::string("PropYoungsModulus_Shell,Material") );
        tParameterList( 2 )( tSPCounter ).set( "slave_properties",    std::string("PropYoungsModulus_Shell,Material") );
        tSPCounter++;
        
		
		//------------------------------------------------------------------------------
        // GHOST
        //------------------------------------------------------------------------------	

        // bulk Ghost - Shell - Temperature
        tParameterList( 2 ).push_back( prm::create_stabilization_parameter_parameter_list() );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_name",      std::string("SPGPTemp_Shell") );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_type",      static_cast< uint >( fem::Stabilization_Type::GHOST_DISPL ) );
        tParameterList( 2 )( tSPCounter ).set( "function_parameters",     std::string("0.01") );
        tParameterList( 2 )( tSPCounter ).set( "master_properties",       std::string("PropConductivity_Shell,Material") );
        tSPCounter++;

        // bulk Ghost - PCM - Temperature
        tParameterList( 2 ).push_back( prm::create_stabilization_parameter_parameter_list() );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_name",      std::string("SPGPTemp_PCM") );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_type",      static_cast< uint >( fem::Stabilization_Type::GHOST_DISPL ) );
        tParameterList( 2 )( tSPCounter ).set( "function_parameters",     std::string("0.01") );
        tParameterList( 2 )( tSPCounter ).set( "master_properties",       std::string("PropConductivity_PCM,Material") );
        tSPCounter++;					
        
		// bulk Ghost - Shell - Displacements
        tParameterList( 2 ).push_back( prm::create_stabilization_parameter_parameter_list() );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_name",      std::string("SPGPStruct_Shell") );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_type",      static_cast< uint >( fem::Stabilization_Type::GHOST_DISPL ) );
        tParameterList( 2 )( tSPCounter ).set( "function_parameters",     std::string("0.01") );
        tParameterList( 2 )( tSPCounter ).set( "master_properties",       std::string("PropYoungsModulus_Shell,Material") );
        tSPCounter++;		

		// bulk Ghost - PCM - Displacements
        tParameterList( 2 ).push_back( prm::create_stabilization_parameter_parameter_list() );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_name",      std::string("SPGPStruct_PCM") );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_type",      static_cast< uint >( fem::Stabilization_Type::GHOST_DISPL ) );
        tParameterList( 2 )( tSPCounter ).set( "function_parameters",     std::string("0.01") );
        tParameterList( 2 )( tSPCounter ).set( "master_properties",       std::string("PropYoungsModulus_PCM,Material") );
        tSPCounter++;        
        
        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        // init IWG counter
        uint tIWGCounter = 0;
        
        //------------------------------------------------------------------------------
        // BULK IWGs
        //------------------------------------------------------------------------------
		
        // diffusion - Shell
        tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGDiffusionShellBulk") );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::SPATIALDIFF_BULK ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_constitutive_models", std::string("CMDiffusion_Shell_1,Diffusion") );
        tParameterList( 3 )( tIWGCounter ).set( "stabilization_parameters",   std::string("SPGGLSDiffusion_Shell,GGLSParam") );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tShell );
        tIWGCounter++; 
        
        // linear elasticity - Shell
        tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   "IWGStructShellBulk");
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::STRUC_LINEAR_BULK ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               "UX,UY");
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    "UX,UY;TEMP");
        tParameterList( 3 )( tIWGCounter ).set( "master_constitutive_models", "CMStrucLinIso_Shell_1,ElastLinIso");
        tParameterList( 3 )( tIWGCounter ).set( "master_properties",          "PropBedding,Bedding" );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tShell );
        tIWGCounter++;        
		
		// diffusion - PCM
		tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGDiffusionPCMBulk") );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::SPATIALDIFF_BULK ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_constitutive_models", std::string("CMDiffusion_PCM,Diffusion") );
        tParameterList( 3 )( tIWGCounter ).set( "stabilization_parameters",   std::string("SPGGLSDiffusion_PCM,GGLSParam") );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tPCM );
        tIWGCounter++;
        
        // linear elasticity - PCM
		tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   "IWGStructPCBulk");
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::STRUC_LINEAR_BULK ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               "UX,UY");
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    "UX,UY;TEMP");
        tParameterList( 3 )( tIWGCounter ).set( "master_constitutive_models", "CMStrucLinIso_PCM,ElastLinIso");
        tParameterList( 3 )( tIWGCounter ).set( "master_properties",          "PropBedding,Bedding" );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tPCM );
        tIWGCounter++;		
        
        //------------------------------------------------------------------------------
        // NEUMANN BCs - IWGs
        //------------------------------------------------------------------------------        

        // heat flux on outside of Shell
        tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGInletFlux") );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::SPATIALDIFF_NEUMANN ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("UX,UY;TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_properties",          std::string("PropImposedFlux,Neumann") );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tOuterShellSurface );
        tIWGCounter++;    
        
        // pressure pulling on outside of Shell
		tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   "IWGNeumannPressure");
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::STRUC_LINEAR_NEUMANN ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               "UX,UY");
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    "UX,UY;TEMP");
        tParameterList( 3 )( tIWGCounter ).set( "master_properties",          "PropPressure,Pressure");
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tOuterShellSurface);
        tIWGCounter++; 
		
		if ( tHaveRadiation)
		{
		    // radiation heat flux
            tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
            tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGHeatRadiation") );
            tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::SPATIALDIFF_RADIATION ) );
            tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("TEMP") );
            tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("UX,UY;TEMP") );
            tParameterList( 3 )( tIWGCounter ).set( "master_properties",          
                    std::string("PropEmissivity,Emissivity;") +
                    std::string("PropAmbientTemp,AmbientTemperature;") +
                    std::string("PropAbsoluteZero,AbsoluteZero")  );
            tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tOuterShellSurface );
            tIWGCounter++;
		}

        //------------------------------------------------------------------------------
        // INTERFACE BCS - IWGs
        //------------------------------------------------------------------------------ 

		// Temperature - Shell - Shell
		tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGInterfaceShellShellTEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::SPATIALDIFF_INTERFACE_SYMMETRIC_NITSCHE ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("UX,UY;TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "slave_dof_dependencies",     std::string("UX,UY;TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_constitutive_models", std::string("CMDiffusion_Shell_1,Diffusion") );
        tParameterList( 3 )( tIWGCounter ).set( "slave_constitutive_models",  std::string("CMDiffusion_Shell_2,Diffusion") );
        tParameterList( 3 )( tIWGCounter ).set( "stabilization_parameters",    
                std::string("SPInterfaceShellShellNitsche     ,NitscheInterface") );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tShellShellInterface );
        tIWGCounter++;
        
        // Displacements - Shell - Shell
        tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGInterfaceShellShellStruct") );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::STRUC_LINEAR_INTERFACE_SYMMETRIC_NITSCHE ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("UX,UY") );
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("UX,UY;TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "slave_dof_dependencies",     std::string("UX,UY;TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_constitutive_models", std::string("CMStrucLinIso_Shell_1,ElastLinIso") );
        tParameterList( 3 )( tIWGCounter ).set( "slave_constitutive_models",  std::string("CMStrucLinIso_Shell_2,ElastLinIso") );
        tParameterList( 3 )( tIWGCounter ).set( "stabilization_parameters",
               std::string("SPInterfaceShellShellNitscheStruct ,NitscheInterface") );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tShellShellInterface );
        tIWGCounter++;

        // Temperature - Shell - PCM
        tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGInterfaceShellPCMTEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::SPATIALDIFF_INTERFACE_SYMMETRIC_NITSCHE ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("UX,UY;TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "slave_dof_dependencies",     std::string("UX,UY;TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_constitutive_models", std::string("CMDiffusion_Shell_1,Diffusion") );
        tParameterList( 3 )( tIWGCounter ).set( "slave_constitutive_models",  std::string("CMDiffusion_PCM,Diffusion") );
        tParameterList( 3 )( tIWGCounter ).set( "stabilization_parameters",
                std::string("SPInterfaceShellPCMNitsche,NitscheInterface") );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tShellPCMInterface );
        tIWGCounter++;	
        
		//------------------------------------------------------------------------------
        // DIRICHLET BCS - IWGs
        //------------------------------------------------------------------------------ 

        // displacements - shell - back wall
        tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   "IWGDirichletStructShell");
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::STRUC_LINEAR_DIRICHLET_UNSYMMETRIC_NITSCHE ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               "UX,UY");
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    "UX,UY;TEMP");
        tParameterList( 3 )( tIWGCounter ).set( "master_properties",          "PropDirichletStruct,Dirichlet");
        tParameterList( 3 )( tIWGCounter ).set( "master_constitutive_models", "CMStrucLinIso_Shell_1,ElastLinIso");
        tParameterList( 3 )( tIWGCounter ).set( "stabilization_parameters",   "SPNitscheStruc,DirichletNitsche");
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tSkinBackWall );
        tIWGCounter++;       
        
        //------------------------------------------------------------------------------
        // IWGs - GHOST
        //------------------------------------------------------------------------------   

        // temperature - Shell
		tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                  std::string("IWGGPShellTemp") );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                  static_cast< uint >( fem::IWG_Type::GHOST_NORMAL_FIELD ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",              std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",   std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "slave_dof_dependencies",    std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "stabilization_parameters",  std::string("SPGPTemp_Shell,GhostSP") );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",            tShellGhost );
        tIWGCounter++;
        
        // displacements - Shell
        tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGGPShellStruct") );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::GHOST_NORMAL_FIELD ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("UX,UY") );
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("UX,UY") );
        tParameterList( 3 )( tIWGCounter ).set( "slave_dof_dependencies",     std::string("UX,UY") );
        tParameterList( 3 )( tIWGCounter ).set( "stabilization_parameters",   std::string("SPGPStruct_Shell,GhostSP") );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tShellGhost );
        tIWGCounter++;        
		
		// temperature - PCM
		tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                  std::string("IWGGPPCMTemp") );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                  static_cast< uint >( fem::IWG_Type::GHOST_NORMAL_FIELD ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",              std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",   std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "slave_dof_dependencies",    std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "stabilization_parameters",  std::string("SPGPTemp_PCM,GhostSP") );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",            tPCMGhost );
        tIWGCounter++;
        
		// displacements - PCM
		tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGGPPCMStrut") );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::GHOST_NORMAL_FIELD ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("UX,UY") );
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("UX,UY") );
        tParameterList( 3 )( tIWGCounter ).set( "slave_dof_dependencies",     std::string("UX,UY") );
        tParameterList( 3 )( tIWGCounter ).set( "stabilization_parameters",   std::string("SPGPStruct_PCM,GhostSP") );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tPCMGhost );
        tIWGCounter++;        
		
        //------------------------------------------------------------------------------
        // IWGs - TIME CONTINUITY
        //------------------------------------------------------------------------------

 		// Time continuity
        tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGTimeContinuityTemp") );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::TIME_CONTINUITY_DOF ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("UX,UY;TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_properties",
                std::string("PropWeightCurrent,WeightCurrent;")      +
                std::string("PropWeightPrevious,WeightPrevious;")    +
                std::string("PropInitialCondition,InitialCondition") );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tTotalDomain );
        tParameterList( 3 )( tIWGCounter ).set( "time_continuity",            true );
        tIWGCounter++;

        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        // init IQI counter
        uint tIQICounter = 0;
		
		// Nodal Temperature IQI
        tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
        tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   std::string("IQIBulkTEMP") );
        tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::DOF ) );
        tParameterList( 4 )( tIQICounter ).set( "dof_quantity",               "TEMP");
		tParameterList( 4 )( tIQICounter ).set( "master_dof_dependencies",    std::string("TEMP") );
        tParameterList( 4 )( tIQICounter ).set( "vectorial_field_index",      0 );
        tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             tTotalDomain );
        tIQICounter++; 
		
		// Max Temperature IQI
        tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
        tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   std::string("IQIMaxTemp") );
        tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::MAX_DOF ) );
        tParameterList( 4 )( tIQICounter ).set( "dof_quantity",               "TEMP");
		tParameterList( 4 )( tIQICounter ).set( "master_dof_dependencies",    std::string("TEMP") );
        tParameterList( 4 )( tIQICounter ).set( "function_parameters",        tIQIRefTemp + "/" + tExponent + "/" + tShift );
        tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             tSkin );
        tIQICounter++;
        
        // Strain Energy of Structure
        tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
        tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   "IQIStrainEnergy" );
        tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::STRAIN_ENERGY ) );
        tParameterList( 4 )( tIQICounter ).set( "master_dof_dependencies",    "UX,UY" );
        tParameterList( 4 )( tIQICounter ).set( "master_constitutive_models", "CMStrucLinIso_Shell_1,Elast" );
        tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             tShell );
        //tParameterList( 4 )( tIQICounter ).set( "normalization",              "design" ); // <-- what does this do?
        tIQICounter++;  
        
        // Volume IQI - Fin Volume - temporary to test optimization capabilities
        tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
        tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   "IQIBSplineGeometryVolume" ) ;
        tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::VOLUME ) );
        tParameterList( 4 )( tIQICounter ).set( "master_dof_dependencies",    "UX,UY;TEMP" ) ;
        tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             tBSplineGeometry );
        tIQICounter++;
        
        // Volume IQI - TotalDomain - use once to find total volume to compute max dof
        tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
        tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   "IQITotalVolume") ;
        tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::VOLUME ) );
        tParameterList( 4 )( tIQICounter ).set( "master_dof_dependencies",    "UX,UY;TEMP") ;
        tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             tTotalDomain );
        tIQICounter++;
        
        // Volume IQI - Fin Perimeter
        tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
        tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   "IQIBSplinesPerimeter" ) ;
        tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::VOLUME ) );
        tParameterList( 4 )( tIQICounter ).set( "master_dof_dependencies",    "UX,UY;TEMP" ) ;
        tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             tBSplinesPerimeter );
        tIQICounter++;
        
                // X-displacement
        tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
        tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   "IQIBulkDISPX" );
        tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::DOF ) );
        tParameterList( 4 )( tIQICounter ).set( "dof_quantity",               "UX,UY");
		tParameterList( 4 )( tIQICounter ).set( "master_dof_dependencies",    "UX,UY" );
        tParameterList( 4 )( tIQICounter ).set( "vectorial_field_index",      0 );
        tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             tTotalDomain );
        tIQICounter++;
        
        // Y-displacement
        tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
        tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   "IQIBulkDISPY");
        tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::DOF ) );
        tParameterList( 4 )( tIQICounter ).set( "dof_quantity",               "UX,UY");
		tParameterList( 4 )( tIQICounter ).set( "master_dof_dependencies",    "UX,UY" );
        tParameterList( 4 )( tIQICounter ).set( "vectorial_field_index",      1 );
        tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             tTotalDomain );
        tIQICounter++;
        
        // nodal von-mises stresses for shell
        tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
        tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   "IQINodalVMStress" );
        tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::VON_MISES_STRESS ) );
        tParameterList( 4 )( tIQICounter ).set( "master_dof_dependencies",    "UX,UY;TEMP" );
        tParameterList( 4 )( tIQICounter ).set( "master_constitutive_models", "CMStrucLinIso_Shell_1,ElastLinIso" );
        //tParameterList( 4 )( tIQICounter ).set( "vectorial_field_index",      0 );
        tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             tTotalDomain );
        tIQICounter++;	
        
        // create computation parameter list
        tParameterList( 5 ).resize( 1 );
        tParameterList( 5 )( 0 ) = prm::create_computation_parameter_list();

    }

    void SOLParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterlist )
    {
        tParameterlist.resize( 7 );
        for( uint Ik = 0; Ik < 7; Ik ++)
        {
            tParameterlist( Ik ).resize( 1 );
        }        
        tParameterlist( 0 )( 0 ) = moris::prm::create_linear_algorithm_parameter_list( sol::SolverType::AMESOS_IMPL );
        
        tParameterlist( 1 )( 0 ) = moris::prm::create_linear_solver_parameter_list();
		
		// ----------------------------------------------------------		
        tParameterlist( 2 ).resize( 3 ); 
		
		tParameterlist( 2 )( 0 ) = moris::prm::create_nonlinear_algorithm_parameter_list();
        tParameterlist( 2 )( 0 )("NLA_Solver_Implementation") = static_cast< uint >( moris::NLA::NonlinearSolverType::NEWTON_SOLVER );
        tParameterlist( 2 )( 0 ).set("NLA_combined_res_jac_assembly", false );
        tParameterlist( 2 )( 0 ).set("NLA_rel_res_norm_drop",    tNLA_rel_res_norm_drop );
        tParameterlist( 2 )( 0 ).set("NLA_relaxation_parameter", tNLA_relaxation_parameter  );
        tParameterlist( 2 )( 0 ).set("NLA_max_iter",             tNLA_max_iter );
		tParameterlist( 2 )( 0 ).set("NLA_combined_res_jac_assembly", true );
		
        tParameterlist( 2 )( 1 ) = moris::prm::create_nonlinear_algorithm_parameter_list();
        tParameterlist( 2 )( 1 )("NLA_Solver_Implementation") = static_cast< uint >( moris::NLA::NonlinearSolverType::NLBGS_SOLVER );
                
        tParameterlist( 2 )( 2 ) = moris::prm::create_nonlinear_algorithm_parameter_list();
        tParameterlist( 2 )( 2 ).set("NLA_rel_res_norm_drop",    1.0e-7 );
        tParameterlist( 2 )( 2 ).set("NLA_relaxation_parameter", 1.0  );
        tParameterlist( 2 )( 2 ).set("NLA_max_iter",             1 );
        tParameterlist( 2 )( 2 ).set("NLA_combined_res_jac_assembly", true );     
		
		// ----------------------------------------------------------		
		tParameterlist( 3 ).resize( 4 );
		
        tParameterlist( 3 )( 0 ) = moris::prm::create_nonlinear_solver_parameter_list();
        tParameterlist( 3 )( 0 )("NLA_Solver_Implementation") = static_cast< uint >( moris::NLA::NonlinearSolverType::NEWTON_SOLVER );
        tParameterlist( 3 )( 0 )("NLA_DofTypes") = std::string("UX,UY");
		
        tParameterlist( 3 )( 1 ) = moris::prm::create_nonlinear_solver_parameter_list();
        tParameterlist( 3 )( 1 )("NLA_Solver_Implementation") = static_cast< uint >( moris::NLA::NonlinearSolverType::NEWTON_SOLVER );
        tParameterlist( 3 )( 1 )("NLA_DofTypes") = std::string("TEMP");
		
        tParameterlist( 3 )( 2 ) = moris::prm::create_nonlinear_solver_parameter_list();
        tParameterlist( 3 )( 2 )("NLA_Solver_Implementation") = static_cast< uint >( moris::NLA::NonlinearSolverType::NLBGS_SOLVER );
        tParameterlist( 3 )( 2 )("NLA_Sub_Nonlinear_Solver") = std::string("1,0");
        tParameterlist( 3 )( 2 )("NLA_DofTypes") = std::string("UX,UY;TEMP");
        tParameterlist( 3 )( 2 )("NLA_Nonlinear_solver_algorithms") = std::string("1");
		        
        tParameterlist( 3 )( 3 ) = moris::prm::create_nonlinear_solver_parameter_list();
        tParameterlist( 3 )( 3 ).set("NLA_DofTypes"            , "UX,UY,TEMP" );
        tParameterlist( 3 )( 3 ).set("NLA_Nonlinear_solver_algorithms"  , "2" );
		
		// ----------------------------------------------------------
        
        tParameterlist( 4 )( 0 ) = moris::prm::create_time_solver_algorithm_parameter_list();
        tParameterlist( 4 )( 0 ).set("TSA_Num_Time_Steps", tTSA_Num_Time_Steps );
        tParameterlist( 4 )( 0 ).set("TSA_Time_Frame",     tTSA_Time_Frame );
		tParameterlist( 4 )( 0 ).set("TSA_Nonlinear_solver", 2 );
        tParameterlist( 4 )( 0 ).set("TSA_nonlinear_solver_for_adjoint_solve",  3 );
		
		// ----------------------------------------------------------

        tParameterlist( 5 )( 0 ) = moris::prm::create_time_solver_parameter_list();
        tParameterlist( 5 )( 0 ).set("TSA_DofTypes",           "UX,UY;TEMP"  );
        tParameterlist( 5 )( 0 ).set("TSA_Initialize_Sol_Vec", "UX,0.0;UY,0.0;TEMP,1.0" );
        tParameterlist( 5 )( 0 ).set("TSA_Output_Indices",     "0" );
        tParameterlist( 5 )( 0 ).set("TSA_Output_Crteria",     "Output_Criterion" );
        tParameterlist( 5 )( 0 ).set("TSA_time_level_per_type","UX,2;UY,2;TEMP,2" );

        tParameterlist( 6 )( 0 ) = moris::prm::create_solver_warehouse_parameterlist();
    }

    void MSIParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterlist )
    {
        tParameterlist.resize( 1 );
        tParameterlist( 0 ).resize( 1 );

        tParameterlist( 0 )( 0 ) = prm::create_msi_parameter_list();
    }

    void VISParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterlist )
    {
        tParameterlist.resize( 1 );
        tParameterlist( 0 ).resize( 1 );

        tParameterlist( 0 )( 0 ) = prm::create_vis_parameter_list();
        tParameterlist( 0 )( 0 ).set( "File_Name"    , std::pair< std::string, std::string >( "./", tOutputFileName ) );
		tParameterlist( 0 )( 0 ).set( "Time_Offset"  , 10.0 );
        tParameterlist( 0 )( 0 ).set( "Mesh_Type"  , static_cast< uint >( vis::VIS_Mesh_Type::STANDARD ) );
        //tParameterlist( 0 )( 0 ).set( "Mesh_Type"  , static_cast< uint >( vis::VIS_Mesh_Type::OVERLAPPING_INTERFACE ) );
        tParameterlist( 0 )( 0 ).set( "Set_Names"  , tTotalDomain );
        tParameterlist( 0 )( 0 ).set( "Field_Names", "TEMP,UX,UY,STRESS,"
		                                             "MAX_DOF,VOLUME,VOLUME"  );
        tParameterlist( 0 )( 0 ).set( "Field_Type" , "NODAL,NODAL,NODAL,NODAL,"
		                                             "GLOBAL,GLOBAL,GLOBAL"  );
        tParameterlist( 0 )( 0 ).set( "IQI_Names"  , "IQIBulkTEMP,IQIBulkDISPX,IQIBulkDISPY,IQINodalVMStress,"
		                                             "IQIMaxTemp,IQIBSplineGeometryVolume,IQITotalVolume" ); 
        tParameterlist( 0 )( 0 ).set( "Save_Frequency", 5 );
    }

    /* ------------------------------------------------------------------------ */
}

//------------------------------------------------------------------------------
#ifdef  __cplusplus
}
#endif
