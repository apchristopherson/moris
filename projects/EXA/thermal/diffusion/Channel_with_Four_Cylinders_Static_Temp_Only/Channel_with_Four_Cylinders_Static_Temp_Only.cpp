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
    bool isGhost = true;

    // Minimum value to be returned by level set function
    moris::real tMinLSvalue      = 1.0e-6;

    // Geometry Parameters
    moris::real tPlaneBottom     = 0.0;                /* y bottom plane (m) */
    moris::real tPlaneTop        = 0.41;               /* y top plane    (m) */
    moris::real tPlaneLeft       = 0.0;                /* x left plane   (m) */
    moris::real tPlaneRight      = 2.2;                /* x right plane  (m) */
    moris::real tCylinderCenterX = 0.2;
    moris::real tCylinderCenterY = 0.2;
    moris::real tCylinderRadius  = 0.05;
    moris::real tCylinderOffset  = 0.10;

    // Constant function for properties
    void Func_Const(
            moris::Matrix< moris::DDRMat >                 & aPropMatrix,
            moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
            moris::fem::Field_Interpolator_Manager         * aFIManager )
    {
        aPropMatrix = aParameters( 0 );
    }

    // Inlet velocity function
    void Func_Inlet_U(
            moris::Matrix< moris::DDRMat >                 & aPropMatrix,
            moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
            moris::fem::Field_Interpolator_Manager         * aFIManager )
    {
        aPropMatrix.set_size( 2, 1, 0.0 );
        real tY = aFIManager->get_IP_geometry_interpolator()->valx()( 1 );
        aPropMatrix( 0 ) = 4.0 * aParameters( 0 )( 0 ) * tY * ( 0.41 - tY ) / ( std::pow( 0.41, 2.0 ) );
    }

    bool Output_Criterion( moris::tsa::Time_Solver * aTimeSolver )
    {
        return true;
    }

    moris::real Func_Bottom_Plane(
            const moris::Matrix< DDRMat >     & aCoordinates,
            const moris::Cell< moris::real* > & aGeometryParameters )
    {
        moris::real tValue = aCoordinates( 1 ) - tPlaneBottom;

        return std::abs(tValue) < tMinLSvalue ? tMinLSvalue : tValue;
    }

    moris::real Func_Top_Plane(
            const moris::Matrix< DDRMat >     & aCoordinates,
            const moris::Cell< moris::real* > & aGeometryParameters )
    {
        moris::real tValue = aCoordinates( 1 ) - tPlaneTop;

        return std::abs(tValue) < tMinLSvalue ? tMinLSvalue : tValue;
    }

    moris::real Func_Left_Plane(
            const moris::Matrix< DDRMat >     & aCoordinates,
            const moris::Cell< moris::real* > & aGeometryParameters )
    {
        moris::real tValue = aCoordinates( 0 ) - tPlaneLeft;

        return std::abs(tValue) < tMinLSvalue ? tMinLSvalue : tValue;
    }

    moris::real Func_Right_Plane(
            const moris::Matrix< DDRMat >     & aCoordinates,
            const moris::Cell< moris::real* > & aGeometryParameters )
    {
        moris::real tValue = aCoordinates( 0 ) - tPlaneRight;

        return std::abs(tValue) < tMinLSvalue ? tMinLSvalue : tValue;
    }

    moris::real Func_Cylinder(
            const moris::Matrix< DDRMat >     & aCoordinates,
            const moris::Cell< moris::real* > & aGeometryParameters )
    {
        moris::real  tValue = tCylinderRadius -
                std::pow(
                        std::pow( aCoordinates( 0 ) - 0.4, 2.0 ) +
                        std::pow( aCoordinates( 1 ) - (0.2 - tCylinderOffset), 2.0 ), 0.5 );

        return std::abs(tValue) < tMinLSvalue ? tMinLSvalue : tValue;
    }

    moris::real Func_Cylinder2(
            const moris::Matrix< DDRMat >     & aCoordinates,
            const moris::Cell< moris::real* > & aGeometryParameters )
    {
        moris::real tValue = tCylinderRadius -
                std::pow(
                        std::pow( aCoordinates( 0 ) - 0.8, 2.0 ) +
                        std::pow( aCoordinates( 1 ) - (0.2 + tCylinderOffset ), 2.0 ), 0.5 );

        return std::abs(tValue) < tMinLSvalue ? tMinLSvalue : tValue;
    }

    moris::real Func_Cylinder3(
            const moris::Matrix< DDRMat >     & aCoordinates,
            const moris::Cell< moris::real* > & aGeometryParameters )
    {
        moris::real tValue = tCylinderRadius -
                std::pow(
                        std::pow( aCoordinates( 0 ) - 1.2, 2.0 ) +
                        std::pow( aCoordinates( 1 ) - (0.2 - tCylinderOffset), 2.0 ), 0.5 );

        return std::abs(tValue) < tMinLSvalue ? tMinLSvalue : tValue;
    }

    moris::real Func_Cylinder4(
            const moris::Matrix< DDRMat >     & aCoordinates,
            const moris::Cell< moris::real* > & aGeometryParameters )
    {
        moris::real tValue = tCylinderRadius -
                std::pow(
                        std::pow( aCoordinates( 0 ) - 1.6, 2.0 ) +
                        std::pow( aCoordinates( 1 ) - (0.2 + tCylinderOffset ), 2.0 ), 0.5 );

        return std::abs(tValue) < tMinLSvalue ? tMinLSvalue : tValue;
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

        tParameterlist( 0 )( 0 ).set( "number_of_elements_per_dimension", std::string("176,88"));
        tParameterlist( 0 )( 0 ).set( "domain_dimensions",                std::string("4,2"));
        tParameterlist( 0 )( 0 ).set( "domain_offset",                    std::string("-1.24,-0.86"));
        tParameterlist( 0 )( 0 ).set( "domain_sidesets",                  std::string("1,2,3,4"));
        tParameterlist( 0 )( 0 ).set( "lagrange_output_meshes",           std::string("0"));

        tParameterlist( 0 )( 0 ).set( "lagrange_orders",  std::to_string(gInterpolationOrder));
        tParameterlist( 0 )( 0 ).set( "lagrange_pattern", std::string("0" ));
        tParameterlist( 0 )( 0 ).set( "bspline_orders",   std::to_string(gInterpolationOrder));
        tParameterlist( 0 )( 0 ).set( "bspline_pattern",  std::string("0" ));

        tParameterlist( 0 )( 0 ).set( "lagrange_to_bspline", std::string("0") );

        tParameterlist( 0 )( 0 ).set( "truncate_bsplines",  1 );
        tParameterlist( 0 )( 0 ).set( "refinement_buffer",  1 );
        tParameterlist( 0 )( 0 ).set( "staircase_buffer",   1 );
        tParameterlist( 0 )( 0 ).set( "initial_refinement", 0 );

        tParameterlist( 0 )( 0 ).set( "use_number_aura",    1 );

        tParameterlist( 0 )( 0 ).set( "use_multigrid",  0 );
        tParameterlist( 0 )( 0 ).set( "severity_level", 0 );
    }

    void XTKParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterlist )
    {
        tParameterlist.resize( 1 );
        tParameterlist( 0 ).resize( 1 );

        tParameterlist( 0 )( 0 ) = prm::create_xtk_parameter_list();
        tParameterlist( 0 )( 0 ).set( "decompose",                   true );
        tParameterlist( 0 )( 0 ).set( "decomposition_type",          std::string("conformal") );
        tParameterlist( 0 )( 0 ).set( "enrich",                      true );
        tParameterlist( 0 )( 0 ).set( "basis_rank",                  std::string("bspline") );
        tParameterlist( 0 )( 0 ).set( "enrich_mesh_indices",         std::string("0") );
        tParameterlist( 0 )( 0 ).set( "ghost_stab",                  isGhost );
        tParameterlist( 0 )( 0 ).set( "multigrid",                   false );
        tParameterlist( 0 )( 0 ).set( "verbose",                     true );
        tParameterlist( 0 )( 0 ).set( "print_enriched_ig_mesh",      false );
        tParameterlist( 0 )( 0 ).set( "exodus_output_XTK_ghost_mesh",false );
        tParameterlist( 0 )( 0 ).set( "exodus_output_XTK_ig_mesh",   false );
    }

    void GENParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterlist )
    {
        tParameterlist.resize( 3 );
        tParameterlist( 0 ).resize( 1 );
        tParameterlist( 1 ).resize( 8 );

        tParameterlist( 0 )( 0 ) = prm::create_gen_parameter_list();

        // Geometry parameter lists
        tParameterlist( 1 )( 0 ) = prm::create_user_defined_geometry_parameter_list();
        tParameterlist( 1 )( 0 ).set( "field_function_name", "Func_Bottom_Plane");
        tParameterlist( 1 )( 0 ).set( "number_of_refinements", 1);

        tParameterlist( 1 )( 1 ) = prm::create_user_defined_geometry_parameter_list();
        tParameterlist( 1 )( 1 ).set( "field_function_name", "Func_Top_Plane");
        tParameterlist( 1 )( 1 ).set( "number_of_refinements", 1);

        tParameterlist( 1 )( 2 ) = prm::create_user_defined_geometry_parameter_list();
        tParameterlist( 1 )( 2 ).set( "field_function_name", "Func_Left_Plane");
        tParameterlist( 1 )( 2 ).set( "number_of_refinements", 0);

        tParameterlist( 1 )( 3 ) = prm::create_user_defined_geometry_parameter_list();
        tParameterlist( 1 )( 3 ).set( "field_function_name", "Func_Right_Plane");
        tParameterlist( 1 )( 3 ).set( "number_of_refinements", 0);

        tParameterlist( 1 )( 4 ) = prm::create_user_defined_geometry_parameter_list();
        tParameterlist( 1 )( 4 ).set( "field_function_name", "Func_Cylinder");
        tParameterlist( 1 )( 4 ).set( "number_of_refinements", 1);

        tParameterlist( 1 )( 5 ) = prm::create_user_defined_geometry_parameter_list();
        tParameterlist( 1 )( 5 ).set( "field_function_name", "Func_Cylinder2");
        tParameterlist( 1 )( 5 ).set( "number_of_refinements", 1);

        tParameterlist( 1 )( 6 ) = prm::create_user_defined_geometry_parameter_list();
        tParameterlist( 1 )( 6 ).set( "field_function_name", "Func_Cylinder3");
        tParameterlist( 1 )( 6 ).set( "number_of_refinements", 1);

        tParameterlist( 1 )( 7 ) = prm::create_user_defined_geometry_parameter_list();
        tParameterlist( 1 )( 7 ).set( "field_function_name", "Func_Cylinder4");
        tParameterlist( 1 )( 7 ).set( "number_of_refinements", 1);
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
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropViscosity") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("0.001") );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // create parameter list for property 2
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropDensity") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("1.0") );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // create parameter list for property 2
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropCapacity") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("1.0") );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // create parameter list for property 3
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropDirichletInletU") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("1.5") );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Inlet_U") );
        tPropCounter++;

        // create parameter list for property 4
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropDirichletZeroU") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("0.0;0.0") );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // create parameter list for property 3
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropConductivity") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("0.00005") );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // create parameter list for property 5
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropInletTemp") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("0.0") );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        // create parameter list for property 6
        tParameterList( 0 ).push_back( prm::create_property_parameter_list() );
        tParameterList( 0 )( tPropCounter ).set( "property_name",            std::string("PropSideFlux") );
        tParameterList( 0 )( tPropCounter ).set( "function_parameters",      std::string("2.0") );
        tParameterList( 0 )( tPropCounter ).set( "value_function",           std::string("Func_Const") );
        tPropCounter++;

        //------------------------------------------------------------------------------
        // fill the constitutive model part of the parameter list

        // init CM counter
        uint tCMCounter = 0;

        // create parameter list for constitutive model 2
        tParameterList( 1 ).push_back( prm::create_constitutive_model_parameter_list() );
        tParameterList( 1 )( tCMCounter ).set( "constitutive_name", std::string("CMDiffusion") );
        tParameterList( 1 )( tCMCounter ).set( "constitutive_type", static_cast< uint >( fem::Constitutive_Type::DIFF_LIN_ISO ) );
        tParameterList( 1 )( tCMCounter ).set( "dof_dependencies",  std::pair< std::string, std::string >( "TEMP", "Temperature" ) );
        tParameterList( 1 )( tCMCounter ).set( "properties",
                std::string("PropConductivity,Conductivity;") +
                std::string("PropDensity     ,Density;")      +
                std::string("PropCapacity    ,HeatCapacity") );
        tCMCounter++;

        //------------------------------------------------------------------------------
        // fill the stabilization parameter part of the parameter list

        // init SP counter
        uint tSPCounter = 0;


        // create parameter list for stabilization parameter 2
        tParameterList( 2 ).push_back( prm::create_stabilization_parameter_parameter_list() );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_name",      std::string("SPDirichletNitscheT") );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_type",      static_cast< uint >( fem::Stabilization_Type::DIRICHLET_NITSCHE ) );
        tParameterList( 2 )( tSPCounter ).set( "function_parameters",     std::string("100.0") );
        tParameterList( 2 )( tSPCounter ).set( "master_properties",       std::string("PropConductivity,Material") );
        tSPCounter++;

        // create parameter list for stabilization parameter 8
        tParameterList( 2 ).push_back( prm::create_stabilization_parameter_parameter_list() );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_name",      std::string("SPGPTemp") );
        tParameterList( 2 )( tSPCounter ).set( "stabilization_type",      static_cast< uint >( fem::Stabilization_Type::GHOST_DISPL ) );
        tParameterList( 2 )( tSPCounter ).set( "function_parameters",     std::string("0.005") );
        tParameterList( 2 )( tSPCounter ).set( "master_properties",       std::string("PropConductivity,Material") );
        tSPCounter++;

        //------------------------------------------------------------------------------
        // fill the IWG part of the parameter list

        // init IWG counter
        uint tIWGCounter = 0;

         // create parameter list for IWG 3
        tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGDiffusionBulk") );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::SPATIALDIFF_BULK ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_constitutive_models", std::string("CMDiffusion,Diffusion") );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             std::string("HMR_dummy_c_p160,HMR_dummy_n_p160") );
        tIWGCounter++;

        // create parameter list for IWG 11
        tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGInletTemp") );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::SPATIALDIFF_DIRICHLET_SYMMETRIC_NITSCHE ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_properties",          std::string("PropInletTemp,Dirichlet") );
        tParameterList( 3 )( tIWGCounter ).set( "master_constitutive_models", std::string("CMDiffusion,Diffusion") );
        tParameterList( 3 )( tIWGCounter ).set( "stabilization_parameters",   std::string("SPDirichletNitscheT,DirichletNitsche") );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             std::string("iside_b0_160_b1_128") );
        tIWGCounter++;

        // create parameter list for IWG 11
        tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGCylinderFluxTemp") );
        tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::SPATIALDIFF_NEUMANN ) );
        tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("TEMP") );
        tParameterList( 3 )( tIWGCounter ).set( "master_properties",          std::string("PropSideFlux,Neumann") );
        tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             std::string("iside_b0_160_b1_168,iside_b0_160_b1_164,iside_b0_160_b1_162,iside_b0_160_b1_161") );
        tIWGCounter++;

        if (isGhost)
        {
            // create parameter list for IWG 16
            tParameterList( 3 ).push_back( prm::create_IWG_parameter_list() );
            tParameterList( 3 )( tIWGCounter ).set( "IWG_name",                   std::string("IWGGPTemp") );
            tParameterList( 3 )( tIWGCounter ).set( "IWG_type",                   static_cast< uint >( fem::IWG_Type::SPATIALDIFF_GHOST ) );
            tParameterList( 3 )( tIWGCounter ).set( "dof_residual",               std::string("TEMP") );
            tParameterList( 3 )( tIWGCounter ).set( "master_dof_dependencies",    std::string("TEMP") );
            tParameterList( 3 )( tIWGCounter ).set( "slave_dof_dependencies",     std::string("TEMP") );
            tParameterList( 3 )( tIWGCounter ).set( "stabilization_parameters",   std::string("SPGPTemp,GhostDispl") );
            tParameterList( 3 )( tIWGCounter ).set( "mesh_set_names",             std::string("ghost_p160") );
            tIWGCounter++;
        }

        //------------------------------------------------------------------------------
        // fill the IQI part of the parameter list

        // init IQI counter
        uint tIQICounter = 0;

        // create parameter list for IQI 3
        tParameterList( 4 ).push_back( prm::create_IQI_parameter_list() );
        tParameterList( 4 )( tIQICounter ).set( "IQI_name",                   std::string("IQIBulkTEMP") );
        tParameterList( 4 )( tIQICounter ).set( "IQI_type",                   static_cast< uint >( fem::IQI_Type::DOF ) );
        tParameterList( 4 )( tIQICounter ).set( "IQI_output_type",            static_cast< uint >( vis::Output_Type::TEMP ) );
        tParameterList( 4 )( tIQICounter ).set( "master_dof_dependencies",    std::string("TEMP") );
        tParameterList( 4 )( tIQICounter ).set( "vectorial_field_index",      0 );
        tParameterList( 4 )( tIQICounter ).set( "mesh_set_names",             std::string("HMR_dummy_n_p160,HMR_dummy_c_p160") );
        tIQICounter++;
    }

    void SOLParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterlist )
    {
        tParameterlist.resize( 7 );
        for( uint Ik = 0; Ik < 7; Ik ++ )
        {
            tParameterlist( Ik ).resize( 1 );
        }

        tParameterlist( 0 )( 0 ) = moris::prm::create_linear_algorithm_parameter_list( sol::SolverType::AMESOS_IMPL );

        tParameterlist( 1 )( 0 ) = moris::prm::create_linear_solver_parameter_list();

        tParameterlist( 2 )( 0 ) = moris::prm::create_nonlinear_algorithm_parameter_list();
        tParameterlist( 2 )( 0 ).set("NLA_rel_res_norm_drop",    1e-04 );
        tParameterlist( 2 )( 0 ).set("NLA_relaxation_parameter", 1.0  );
        tParameterlist( 2 )( 0 ).set("NLA_max_iter", 20 );

        tParameterlist( 3 )( 0 ) = moris::prm::create_nonlinear_solver_parameter_list();
        tParameterlist( 3 )( 0 ).set("NLA_DofTypes"      , std::string("TEMP") );

        tParameterlist( 4 )( 0 ) = moris::prm::create_time_solver_algorithm_parameter_list();

        tParameterlist( 5 )( 0 ) = moris::prm::create_time_solver_parameter_list();
        tParameterlist( 5 )( 0 ).set("TSA_DofTypes"       , std::string("TEMP") );
        tParameterlist( 5 )( 0 ).set("TSA_Initialize_Sol_Vec" , std::string("TEMP,0.0") );
        tParameterlist( 5 )( 0 ).set("TSA_Output_Indices" , std::string("0") );
        tParameterlist( 5 )( 0 ).set("TSA_Output_Crteria" , std::string("Output_Criterion") );

        tParameterlist( 6 )( 0 ) = moris::prm::create_solver_warehouse_parameterlist();
    }

    void MSIParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterlist )
    {
        tParameterlist.resize( 1 );
        tParameterlist( 0 ).resize( 1 );

        tParameterlist( 0 )( 0 ) = prm::create_msi_parameter_list();
        tParameterlist( 0 )( 0 ).set("order_adofs_by_host",false);
    }

    void VISParameterList( moris::Cell< moris::Cell< ParameterList > > & tParameterlist )
    {
        tParameterlist.resize( 1 );
        tParameterlist( 0 ).resize( 1 );

        tParameterlist( 0 )( 0 ) = prm::create_vis_parameter_list();
        tParameterlist( 0 )( 0 ).set( "File_Name"     , std::pair< std::string, std::string >( "./", "Channel_with_Four_Cylinders_Static_Temp_Only.exo" ) );
        tParameterlist( 0 )( 0 ).set( "Mesh_Type"     , static_cast< uint >( vis::VIS_Mesh_Type::STANDARD ) );
        tParameterlist( 0 )( 0 ).set( "Set_Names"     , std::string( "HMR_dummy_n_p160,HMR_dummy_c_p160" ) );
        tParameterlist( 0 )( 0 ).set( "Field_Names"   , std::string( "TEMP,IQIBulkTEMP" ) );
        tParameterlist( 0 )( 0 ).set( "Field_Type"    , std::string( "NODAL,GLOBAL" ) );
        tParameterlist( 0 )( 0 ).set( "Output_Type"   , std::string( "TEMP,TEMP" ) );
        tParameterlist( 0 )( 0 ).set( "Save_Frequency", 1 );
    }

    //------------------------------------------------------------------------------
}

//------------------------------------------------------------------------------
#ifdef  __cplusplus
}
#endif
