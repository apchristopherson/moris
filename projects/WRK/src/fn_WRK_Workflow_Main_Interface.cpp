#include<iostream>
#include<vector>
#include<string>
#include<fn_print.hpp>

#include <cstdio>// nicer than streams in some respects
// C system files
#include <unistd.h>
// C++ system files
#include <stdio.h>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include "cl_Stopwatch.hpp"
#include "cl_Communication_Manager.hpp" // COM/src
#include "cl_Communication_Tools.hpp" // COM/src
#include "typedefs.hpp" // COR/src
// other header files
//#include <catch.hpp>
//#include "fn_equal_to.hpp" //ALG
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
#include "cl_Stopwatch.hpp" //CHR/src
#include "op_move.hpp"

#include "cl_Matrix.hpp"
#include "cl_Logger.hpp" // MRS/IOS/src

#include "cl_WRK_Performer_Manager.hpp"
#include "cl_WRK_Workflow.hpp"
#include "cl_OPT_Manager.hpp"

#include "fn_Exec_load_user_library.hpp"


using namespace moris;

int fn_WRK_Workflow_Main_Interface( int argc, char * argv[] )
{    
    if (argc < 2)
    {
        std::cout << "\n Error: input file required\n" << "\n";
        return -1;
    }

    std::string tInputArg = std::string(argv[ 1 ]);
    std::string tString = "Reading dynamically linked shared object " + tInputArg + ".";
    MORIS_LOG( tString.c_str() );

    //dynamically linked file
    std::shared_ptr< Library_IO >tLibrary = std::make_shared< Library_IO >( argv[ 1 ] );

    {
        // load the OPT parameter list
        std::string tOPTString = "OPTParameterList";
        MORIS_PARAMETER_FUNCTION tOPTParameterListFunc = tLibrary->load_parameter_file( tOPTString );
        moris::Cell< moris::Cell< ParameterList > > tOPTParameterList;
        tOPTParameterListFunc( tOPTParameterList );

            // Create workflow
            wrk::Performer_Manager tPerformerManager( tLibrary );
            tPerformerManager.initialize_performers();
            tPerformerManager.set_performer_cooperations();

        moris::Cell<std::shared_ptr<moris::opt::Criteria_Interface>> tWorkflows = { std::make_shared<wrk::Workflow>( &tPerformerManager ) };

        if( tOPTParameterList( 0 )( 0 ).get< bool >("is_optimization_problem") )
        {
            moris::opt::Manager tManager( tOPTParameterList, tWorkflows );
            tManager.perform();

        }
        else
        {
            Matrix< DDRMat > tDummyMat;
            tWorkflows(0)->initialize( tDummyMat,tDummyMat,tDummyMat );
            Matrix<DDRMat> tADVs(1, 1, 0.0);
            tWorkflows(0)->get_criteria(tADVs);
        }
    }

    return 0;
}
