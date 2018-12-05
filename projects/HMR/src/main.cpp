
// standard

#include <string>
#include <memory>


// communication
#include "cl_Communication_Manager.hpp"
#include "cl_Communication_Tools.hpp"

#include "cl_MTK_Mesh.hpp"

// core
#include "assert.hpp"
#include "typedefs.hpp"
#include "banner.hpp"

// containers
#include "cl_Cell.hpp"

// linalg

// MTK
#include "cl_Mesh_Enums.hpp"
#include "cl_MTK_Mesh.hpp"
#include "cl_MTK_Mapper.hpp"
#include "cl_Mesh_Factory.hpp"

// HMR
#include "cl_HMR_Arguments.hpp"
#include "cl_HMR_Field.hpp"
#include "cl_HMR_State.hpp"
#include "cl_HMR.hpp"

#include "fn_HMR_Exec_load_parameters.hpp"
#include "fn_HMR_Exec_dump_meshes.hpp"
#include "fn_HMR_Exec_dump_fields.hpp"
#include "fn_HMR_Exec_initialize_fields.hpp"
#include "fn_HMR_Exec_load_user_library.hpp"
#include "fn_HMR_Exec_perform_mapping.hpp"

moris::Comm_Manager gMorisComm;

using namespace moris;
using namespace hmr;

// -----------------------------------------------------------------------------

void
state_initialize_mesh( const Arguments & aArguments )
{
    // load parameters from xml path
    ParameterList tParamList = create_hmr_parameter_list();
    load_hmr_parameter_list_from_xml( aArguments.get_parameter_path(), tParamList );

    // create new HMR object from parameter list
    HMR * tHMR = new HMR( tParamList );

    // if there is no initial refinement, copy initial tensor mesh to output
    if(        tHMR->get_parameters()->get_initial_bspline_refinement()  == 0
            && tHMR->get_parameters()->get_additional_lagrange_refinement() == 0 )
    {
        // test if max polynomial is 3
        if ( tHMR->get_parameters()->get_max_polynomial() > 2 )
        {
            // activate extra pattern for exodus
            tHMR->get_database()->add_extra_refinement_step_for_exodus();
        }

    }
    else
    {
        // otherwise, refine all elements n times
        tHMR->perform_initial_refinement();
    }

    // finalize database
    tHMR->finalize();

    //tHMR->save_background_mesh_to_vtk("BG.vtk");

    // write mesh
    dump_meshes( aArguments, tHMR );

    // delete HMR object
    delete tHMR;
}

// -----------------------------------------------------------------------------

void
state_refine_mesh( const Arguments & aArguments )
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 1: Load Parameter Lists
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // load parameters from xml path
    ParameterList tParamList = create_hmr_parameter_list();
    load_hmr_parameter_list_from_xml( aArguments.get_parameter_path(), tParamList );

    // load file list from xml path
    ParameterList tFileList;
    load_file_list_from_xml( aArguments.get_parameter_path(), tFileList );

    // cell with field parameters
    moris::Cell< ParameterList > tFieldParams;

    // load parameters from XML
    load_field_parameters_from_xml( aArguments.get_parameter_path(), tFieldParams );

    // container with parameters for refinement
    ParameterList tRefinementParams;
    load_refinement_parameters_from_xml( aArguments.get_parameter_path(), tRefinementParams );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 2: Initialize HMR Object
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // create new HMR object from input DB
    HMR * tHMR = new HMR( tFileList.get< std::string >( "input_mesh_database" ) );

    // copy parameters from parameter list into HMR object
    tHMR->get_parameters()->copy_selected_parameters( tParamList );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 3: Load user defined functions
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // create library
    Library tLibrary(  tRefinementParams.get< std::string >( "library" ) );

    // load user defined function
    MORIS_HMR_USER_FUNCTION user_refinement = tLibrary.load_function(
            tRefinementParams.get< std::string >( "function" ) );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 4: Initialize Fields
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // initialize fields
    moris::Cell< std::shared_ptr< Field > > tFields;

    // create list of fields
    for( ParameterList tParams :  tFieldParams )
    {
        tFields.push_back( tHMR->create_field( tParams  ) );
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 5: Perform refinement
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // call user defined refinement function
    tHMR->user_defined_flagging(
            user_refinement,
            tFields,
            tRefinementParams );

    // perform refinement
    tHMR->perform_refinement( RefinementMode::LAGRANGE_REFINE );
    tHMR->perform_refinement( RefinementMode::BSPLINE_REFINE );

    // finalize mesh
    tHMR->finalize();

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 6: Map Field on Union Mesh and dump meshes ( optional )
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if( aArguments.map_while_refine() )
    {
        // reserve cell for output fields
        Cell< std::shared_ptr< Field > > tOutputFields;

        // call mapper
        perform_mapping( tHMR,  tFieldParams, tFields, tOutputFields );

        // write meshes
        dump_meshes( aArguments, tHMR );

        // write fields
        dump_fields( tFileList, tOutputFields );
    }
    else
    {
        // write meshes
        dump_meshes( aArguments, tHMR );
    }

    // delete HMR object
    delete tHMR;
}
// -----------------------------------------------------------------------------

void
state_map_fields( const Arguments & aArguments )
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 1: Load Parameter Lists
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // load parameters from xml path
    ParameterList tParamList = create_hmr_parameter_list();
    load_hmr_parameter_list_from_xml( aArguments.get_parameter_path(), tParamList );

    // load file list from xml path
    ParameterList tFileList;
    load_file_list_from_xml( aArguments.get_parameter_path(), tFileList );

    // cell with field parameters
    moris::Cell< ParameterList > tFieldParams;

    // load parameters from XML
    load_field_parameters_from_xml( aArguments.get_parameter_path(), tFieldParams );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 2: Initialize HMR Object
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // create new HMR object from input DB
    HMR * tHMR = new HMR(
            tFileList.get< std::string >( "input_mesh_database" ),
            tFileList.get< std::string >( "output_mesh_database") );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 3: Initialize Fields
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // initialize fields
    moris::Cell< std::shared_ptr< Field > > tFields;

    // create list of fields
    for( ParameterList tParams :  tFieldParams )
    {
        tFields.push_back( tHMR->create_field( tParams  ) );

    }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Step 4: Map Fields and Dump Output
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // reserve cell for output fields
    Cell< std::shared_ptr< Field > > tOutputFields;

    // call mapper
    perform_mapping( tHMR,  tFieldParams, tFields, tOutputFields );

    // write meshes
    dump_meshes( aArguments, tHMR );

    // write fields
    dump_fields( tFileList, tOutputFields );

    // delete HMR object
    delete tHMR;
}

// -----------------------------------------------------------------------------

int
main(
        int    argc,
        char * argv[] )
{
    // initialize MORIS global communication manager
    gMorisComm = moris::Comm_Manager( &argc, &argv );

    // create arguments object
    Arguments tArguments( argc, argv );

    // select runstate
    switch ( tArguments.get_state() )
    {
        case( State::PRINT_USAGE ) :
        {
            // print system usage
            tArguments.print_usage();
            break;
        }
        case( State::PRINT_VERSION ) :
        {
            // print welcome banner and system information
            moris::print_banner( argc, argv );
            break;
        }
        case( State::PRINT_HELP ) :
        {
            // print help line and exit
            tArguments.print_help();
            break;
        }
        case( State::INITIALIZE_MESH ) :
        {
            state_initialize_mesh( tArguments );
            break;
        }
        case( State::REFINE_MESH ) :
        {
            state_refine_mesh( tArguments );
            break;
        }
        case( State::MAP_FIELDS ) :
        {
            state_map_fields( tArguments );
            break;
        }
        default :
        {
            // print system usage
            tArguments.print_usage();
            break;
        }
    }

    // finalize MORIS global communication manager
    gMorisComm.finalize();

    return 0;

}
