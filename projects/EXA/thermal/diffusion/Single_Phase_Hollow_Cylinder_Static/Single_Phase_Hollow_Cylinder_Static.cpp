
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
#include "cl_PRM_FEM_Parameters.hpp"
#include "cl_PRM_MSI_Parameters.hpp"
#include "cl_PRM_SOL_Parameters.hpp"
#include "cl_PRM_VIS_Parameters.hpp"
#include "cl_PRM_HMR_Parameters.hpp"
#include "fn_PRM_GEN_Parameters.hpp"
#include "cl_PRM_XTK_Parameters.hpp"
#include "cl_PRM_OPT_Parameters.hpp"
#include "fn_equal_to.hpp"

#include "AztecOO.h"

extern uint gInterpolationOrder;

#ifdef  __cplusplus
extern "C"
{
#endif
    //------------------------------------------------------------------------------
    namespace moris
    {
        // set for FEM
        std::string tSolid      = "HMR_dummy_c_p2,HMR_dummy_n_p2";
        std::string tInSide     = "iside_b0_2_b1_3";
        std::string tOutSide    = "iside_b0_2_b1_0";
        std::string tSolidGhost = "ghost_p2";

        // Geometry parameter
        moris::real tRIn     = 1.0;
        moris::real tROut    = 2.0;
        moris::real tXCenter = 0.0;
        moris::real tYCenter = 0.0;

        // Boundary conditions
        moris::real tImposedTemp = 5.0;
        moris::real tImposedFlux = 20.0;
        moris::real tDirichletNitsche = 100.0;
        moris::real tGhostPenalty = 0.001;

        // Material property
        moris::real tConductivity = 1.0;

        void AnalyticTemperatureFunc(
                moris::Matrix< moris::DDRMat >                 & aPropMatrix,
                moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
                moris::fem::Field_Interpolator_Manager         * aFIManager )
        {
            // get parameters
            real RInner  = tRIn; // inner radius
            real ROuter  = tROut; // outer radius
            real xCenter = tXCenter; // x coord of center
            real yCenter = tYCenter; // y coord of center
            real TInner  = tImposedTemp; // imposed temperature at inner radius
            real Q       = tImposedFlux * 2 * M_PI * ROuter; // heat load (W)
            real kappa   = tConductivity; // conductivity (W/m^2)

            // get x and y coords
            real tXCoord = aFIManager->get_IP_geometry_interpolator()->valx()( 0 );
            real tYCoord = aFIManager->get_IP_geometry_interpolator()->valx()( 1 );

            // compute radius
            real R = std::sqrt( std::pow( tXCoord - xCenter, 2 ) + std::pow( tYCoord - yCenter, 2 ) );

            aPropMatrix = { { TInner + ( Q * std::log( R/RInner ) )/( kappa * 2 * M_PI ) } };
        }

        // Constant function for properties
        void Func_Const(
                moris::Matrix< moris::DDRMat >                 & aPropMatrix,
                moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
                moris::fem::Field_Interpolator_Manager         * aFIManager )
        {
            aPropMatrix = aParameters( 0 );
        }

        void AnalyticdTemperaturedxFunc(
                moris::Matrix< moris::DDRMat >                 & aPropMatrix,
                moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
                moris::fem::Field_Interpolator_Manager         * aFIManager )
        {
            // flux
            real tQ = tImposedFlux * 2 * M_PI * tROut; // heat load (W)

            // radius
            real tX = aFIManager->get_IP_geometry_interpolator()->valx()( 0 );
            real tY = aFIManager->get_IP_geometry_interpolator()->valx()( 1 );
            real tR = std::pow( std::pow( tX - tXCenter, 2.0 ) + std::pow( tY - tYCenter, 2.0 ), 0.5 );

            // set size for aPropMatrix
            aPropMatrix.set_size( 2, 1, 0.0 );
            aPropMatrix( 0, 0 ) = tQ * tX / ( 2.0 * M_PI * tConductivity * std::pow( tR, 2.0 ) );
            aPropMatrix( 1, 0 ) = tQ * tY / ( 2.0 * M_PI * tConductivity * std::pow( tR, 2.0 ) );
        }

        bool Output_Criterion( moris::tsa::Time_Solver * aTimeSolver )
        {
            return true;
        }

        moris::real Func_Circle(
                const moris::Matrix< DDRMat >     & aCoordinates,
                const moris::Cell< moris::real* > & aGeometryParameters )
        {
            moris::real tR = *( aGeometryParameters( 0 ) );
            moris::real tXCenter = *( aGeometryParameters( 1 ) );
            moris::real tYCenter = *( aGeometryParameters( 2 ) );

            moris::real aReturnValue = tR - std::pow(
                    std::pow( aCoordinates( 0 ) - tXCenter, 2.0 ) +
                    std::pow( aCoordinates( 1 ) - tYCenter, 2.0 ), 0.5 );

            return aReturnValue;
        }

        moris::Matrix< DDRMat > Func_Sensitivity(
                const moris::Matrix< DDRMat >     & aCoordinates,
                const moris::Cell< moris::real* > & aGeometryParameters )
            {
            moris::Matrix< DDRMat > aReturnValue;
            return aReturnValue;
            }

        void OPTParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterlist )
        {
            tParameterlist.resize( 1 );
            tParameterlist( 0 ).resize( 1 );

            tParameterlist( 0 )( 0 ) = prm::create_opt_problem_parameter_list();

            tParameterlist( 0 )( 0 ).set( "is_optimization_problem", false);
        }

        void HMRParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterlist )
        {
            tParameterlist.resize( 1 );
            tParameterlist( 0 ).resize( 1 );

            tParameterlist( 0 )( 0 ) = prm::create_hmr_parameter_list();

            tParameterlist( 0 )( 0 ).set( "number_of_elements_per_dimension", std::string("21,21") );
            tParameterlist( 0 )( 0 ).set( "domain_dimensions",                std::string("5,5") );
            tParameterlist( 0 )( 0 ).set( "domain_offset",                    std::string("-2.5+0.13,-2.5+0.18") );
            tParameterlist( 0 )( 0 ).set( "domain_sidesets",                  std::string("1,2,3,4") );
            tParameterlist( 0 )( 0 ).set( "lagrange_output_meshes",           std::string("0") );

            tParameterlist( 0 )( 0 ).set( "lagrange_orders",  std::to_string(gInterpolationOrder) );
            tParameterlist( 0 )( 0 ).set( "lagrange_pattern", std::string("0") );
            tParameterlist( 0 )( 0 ).set( "bspline_orders",   std::to_string(gInterpolationOrder) );
            tParameterlist( 0 )( 0 ).set( "bspline_pattern",  std::string("0") );

            tParameterlist( 0 )( 0 ).set( "lagrange_to_bspline", std::string("0") );

            tParameterlist( 0 )( 0 ).set( "truncate_bsplines",  1 );
            tParameterlist( 0 )( 0 ).set( "refinement_buffer",  3 );
            tParameterlist( 0 )( 0 ).set( "staircase_buffer",   3 );
            tParameterlist( 0 )( 0 ).set( "initial_refinement", 0 );

            tParameterlist( 0 )( 0 ).set( "use_multigrid",  0 );
            tParameterlist( 0 )( 0 ).set( "severity_level", 0 );

            tParameterlist( 0 )( 0 ).set( "adaptive_refinement_level", 2 );
        }

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
            tParameterlist( 0 )( 0 ).set( "print_enriched_ig_mesh",    true );
            tParameterlist( 0 )( 0 ).set( "exodus_output_XTK_ig_mesh", true );
        }

        void GENParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterlist )
        {
            tParameterlist.resize( 3 );
            tParameterlist( 0 ).resize( 1 );

            // Main GEN parameter list
            tParameterlist( 0 )( 0 ) = prm::create_gen_parameter_list();

            // init geometry counter
            uint tGeoCounter = 0;

            // Geometry parameter lists
            tParameterlist( 1 ).push_back( prm::create_user_defined_geometry_parameter_list() );
            tParameterlist( 1 )( tGeoCounter ).set( "field_function_name", "Func_Circle");
            tParameterlist( 1 )( tGeoCounter ).set( "sensitivity_function_name", "Func_Sensitivity");
            tParameterlist( 1 )( tGeoCounter ).set( "constant_parameters", "2.0,0.0,0.0");
            tGeoCounter++;

            tParameterlist( 1 ).push_back( prm::create_user_defined_geometry_parameter_list() );
            tParameterlist( 1 )( tGeoCounter ).set( "field_function_name", "Func_Circle");
            tParameterlist( 1 )( tGeoCounter ).set( "sensitivity_function_name", "Func_Sensitivity");
            tParameterlist( 1 )( tGeoCounter ).set( "constant_parameters", "1.0,0.0,0.0");
            tGeoCounter++;
        }

        void FEMParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterList )
        {
            // create a cell of cell of parameter list for fem
            tParameterList.resize( 5 );

            //------------------------------------------------------------------------------
            // fill the property part of the parameter list

            // init property counter
            uint tPropCounter = 0;

            // create parameter list for property 1
            tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
            tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropConductivity") );
            tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::to_string( tConductivity ) );
            tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
            tPropCounter++;

            // create parameter list for property 2
            tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
            tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropImposedTemp") );
            tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::to_string( tImposedTemp ) );
            tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
            tPropCounter++;

            // create parameter list for property 3
            tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
            tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropImposedFlux") );
            tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::to_string( tImposedFlux ) );
            tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
            tPropCounter++;

            // create parameter list for property 4
            tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
            tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropAnalyticTemp") );
            tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("AnalyticTemperatureFunc") );
            tPropCounter++;

            // create parameter list for property 5
            tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
            tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropAnalyticdTempdx") );
            tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("AnalyticdTemperaturedxFunc") );
            tPropCounter++;

            // create parameter list for property 6
            tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
            tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropMaxTempReference") );
            tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("5.0") );
            tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
            tPropCounter++;

            // create parameter list for property 7
            tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
            tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropMaxTempExponent") );
            tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("30.0") );
            tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
            tPropCounter++;

            //------------------------------------------------------------------------------
            // fill the constitutive model part of the parameter list

            // init CM counter
            uint tCMCounter = 0;

            // create parameter list for constitutive model 1
            tParameterList( 1 ).push_back( prm::create_constitutive_model_parameter_list() );
            tParameterList( 1 )( tCMCounter ).set( "constitutive_name", std::string("CMDiffusion") );
            tParameterList( 1 )( tCMCounter ).set( "constitutive_type", static_cast< uint >( fem::Constitutive_Type::DIFF_LIN_ISO ) );
            tParameterList( 1 )( tCMCounter ).set( "dof_dependencies",  std::pair< std::string, std::string >( "TEMP", "Temperature" ) );
            tParameterList( 1 )( tCMCounter ).set( "properties",        std::string("PropConductivity,Conductivity") );
            tCMCounter++;

            //------------------------------------------------------------------------------
            // fill the stabilization parameter part of the parameter list

            // init SP counter
            uint tSPCounter = 0;

            // create parameter list for stabilization parameter 1
            tParameterList( 2 ).push_back( prm::create_stabilization_parameter_parameter_list() );
            tParameterList( 2 )( tSPCounter ).set( "stabilization_name",      std::string("SPDirichletNitscheTemp") );
            tParameterList( 2 )( tSPCounter ).set( "stabilization_type",      static_cast< uint >( fem::Stabilization_Type::DIRICHLET_NITSCHE ) );
            tParameterList( 2 )( tSPCounter ).set( "function_parameters",     std::to_string( tDirichletNitsche ) );
            tParameterList( 2 )( tSPCounter ).set( "master_properties",       std::string("PropConductivity,Material") );
            tSPCounter++;

            // create parameter list for stabilization parameter 2
            tParameterList( 2 ).push_back( prm::create_stabilization_parameter_parameter_list() );
            tParameterList( 2 )( tSPCounter ).set( "stabilization_name",  std::string("SPGPTemperature") );
            tParameterList( 2 )( tSPCounter ).set( "stabilization_type",  static_cast< uint >( fem::Stabilization_Type::GHOST_DISPL ) );
            tParameterList( 2 )( tSPCounter ).set( "function_parameters", std::to_string( tGhostPenalty ) );
            tParameterList( 2 )( tSPCounter ).set( "master_properties",   std::string("PropConductivity,Material") );
            tSPCounter++;

            //------------------------------------------------------------------------------
            // fill the IWG part of the parameter list

            // init IWG counter
            uint tIWGCounter = 0;

            // create parameter list for IWG 1
            tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
            tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGDiffusionBulk") );
            tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::SPATIALDIFF_BULK ) );
            tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("TEMP") );
            tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("TEMP") );
            tParameterList( 3 )( tIWGCounter ).set( "master_constitutive_models", std::string("CMDiffusion,Diffusion") );
            tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tSolid );
            tIWGCounter++;

            // create parameter list for IWG 2
            tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
            tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGImposedTempIn") );
            tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::SPATIALDIFF_DIRICHLET_UNSYMMETRIC_NITSCHE ) );
            tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("TEMP") );
            tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("TEMP") );
            tParameterList( 3 )( tIWGCounter ).set( "master_properties",          std::string("PropImposedTemp,Dirichlet") );
            tParameterList( 3 )( tIWGCounter ).set( "master_constitutive_models", std::string("CMDiffusion,Diffusion") );
            tParameterList( 3 )( tIWGCounter ).set( "stabilization_parameters",   std::string("SPDirichletNitscheTemp,DirichletNitsche") );
            tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tInSide );
            tIWGCounter++;

            // create parameter list for IWG 3
            tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
            tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGImposedFluxOut") );
            tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::SPATIALDIFF_NEUMANN ) );
            tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("TEMP") );
            tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("TEMP") );
            tParameterList( 3 )( tIWGCounter ).set( "master_properties",          std::string("PropImposedFlux,Neumann") );
            tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tOutSide );
            tIWGCounter++;

            // create parameter list for IWG 5
            tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
            tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGGhostTemp") );
            tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::SPATIALDIFF_GHOST ) );
            tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("TEMP") );
            tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("TEMP") );
            tParameterList( 3 )( tIWGCounter ).set( "slave_dof_dependencies",     std::string("TEMP") );
            tParameterList( 3 )( tIWGCounter ).set( "stabilization_parameters",   std::string("SPGPTemperature,GhostDispl") );
            tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             tSolidGhost );
            tIWGCounter++;

            //------------------------------------------------------------------------------
            // fill the IQI part of the parameter list

            // init IQI counter
            uint tIQICounter = 0;

            // create parameter list for IQI 1
            tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
            tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   std::string("IQIBulkTEMP") );
            tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::DOF ) );
            tParameterList( 4 )( tIQICounter ).set( "IQI_output_type",            static_cast< uint >( vis::Output_Type::TEMP ) );
            tParameterList( 4 )( tIQICounter ).set( "master_dof_dependencies",    std::string("TEMP") );
            tParameterList( 4 )( tIQICounter ).set( "vectorial_field_index",      0 );
            tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             tSolid );
            tIQICounter++;

            // create parameter list for IQI 1
            tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
            tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   std::string("IQIBulkTEMPAnalytic") );
            tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::PROPERTY ) );
            tParameterList( 4 )( tIQICounter ).set( "IQI_output_type",            static_cast< uint >( vis::Output_Type::PROPERTY ) );
            tParameterList( 4 )( tIQICounter ).set( "master_properties",          std::string("PropAnalyticTemp,Property") );
            tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             tSolid );
            tIQICounter++;

            // create parameter list for IQI 2
            tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
            tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   std::string("IQIBulkL2Error") );
            tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::L2_ERROR_ANALYTIC ) );
            tParameterList( 4 )( tIQICounter ).set( "IQI_output_type",            static_cast< uint >( vis::Output_Type::L2_ERROR_ANALYTIC ) );
            tParameterList( 4 )( tIQICounter ).set( "master_dof_dependencies",    std::string("TEMP") );
            tParameterList( 4 )( tIQICounter ).set( "master_properties",          std::string("PropAnalyticTemp,L2Check") );
            tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             tSolid );
            tIQICounter++;

            // create parameter list for IQI 3
            tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
            tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   std::string("IQIBulkH1Error") );
            tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::H1_ERROR_ANALYTIC ) );
            tParameterList( 4 )( tIQICounter ).set( "IQI_output_type",            static_cast< uint >( vis::Output_Type::H1_ERROR_ANALYTIC ) );
            tParameterList( 4 )( tIQICounter ).set( "master_dof_dependencies",    std::string("TEMP") );
            tParameterList( 4 )( tIQICounter ).set( "master_properties",          std::string("PropAnalyticdTempdx,H1Check") );
            tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             tSolid );
            tIQICounter++;

            tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
            tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   std::string("IQIMaxTEMP") );
            tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::MAX_DOF ) );
            tParameterList( 4 )( tIQICounter ).set( "IQI_output_type",            static_cast< uint >( vis::Output_Type::MAX_DOF ) );
            tParameterList( 4 )( tIQICounter ).set( "master_dof_dependencies",    std::string("TEMP") );
            tParameterList( 4 )( tIQICounter ).set( "master_properties",
                    std::string("PropMaxTempReference,ReferenceValue;") +
                    std::string("PropMaxTempExponent,Exponent") );
            tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             tSolid );
            tIQICounter++;

            tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
            tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   std::string("IQIVolume") );
            tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::VOLUME ) );
            tParameterList( 4 )( tIQICounter ).set( "IQI_output_type",            static_cast< uint >( vis::Output_Type::VOLUME ) );
            tParameterList( 4 )( tIQICounter ).set( "master_dof_dependencies",    std::string("TEMP") );
            tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             tSolid );
            tIQICounter++;
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

            tParameterlist( 2 )( 0 ) = moris::prm::create_nonlinear_algorithm_parameter_list();
            tParameterlist( 2 )( 0 ).set( "NLA_rel_res_norm_drop", 1e-07 );

            tParameterlist( 3 )( 0 ) = moris::prm::create_nonlinear_solver_parameter_list();
            tParameterlist( 3 )( 0 ).set( "NLA_DofTypes"      , std::string("TEMP") );

            tParameterlist( 4 )( 0 ) = moris::prm::create_time_solver_algorithm_parameter_list();

            tParameterlist( 5 )( 0 ) = moris::prm::create_time_solver_parameter_list();
            tParameterlist( 5 )( 0 ).set( "TSA_DofTypes"       , std::string("TEMP") );
            tParameterlist( 5 )( 0 ).set( "TSA_Initialize_Sol_Vec" , std::string("TEMP,0.0") );
            tParameterlist( 5 )( 0 ).set( "TSA_Output_Indices" , std::string("0") );
            tParameterlist( 5 )( 0 ).set( "TSA_Output_Crteria" , std::string("Output_Criterion") );

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
            tParameterlist( 0 )( 0 ).set( "File_Name"  , std::pair< std::string, std::string >( "./", "Single_Phase_Hollow_Cylinder_Static.exo") );
            tParameterlist( 0 )( 0 ).set( "Mesh_Type"  , static_cast< uint >( vis::VIS_Mesh_Type::STANDARD ) );
            tParameterlist( 0 )( 0 ).set( "Set_Names"  , std::string("HMR_dummy_n_p2,HMR_dummy_c_p2") );
            tParameterlist( 0 )( 0 ).set( "Field_Names", std::string("TEMP,TEMP_ANALYTIC,L2_ERROR_ANALYTIC,H1_ERROR_ANALYTIC,MAX_DOF,VOLUME") );
            tParameterlist( 0 )( 0 ).set( "Field_Type" , std::string("NODAL,NODAL,GLOBAL,GLOBAL,GLOBAL,GLOBAL") );
            tParameterlist( 0 )( 0 ).set( "Output_Type", std::string("TEMP,PROPERTY,L2_ERROR_ANALYTIC,H1_ERROR_ANALYTIC,MAX_DOF,VOLUME") );
            tParameterlist( 0 )( 0 ).set( "Save_Frequency", 1 );
        }

        //------------------------------------------------------------------------------
    }

    //------------------------------------------------------------------------------
#ifdef  __cplusplus
}
#endif
