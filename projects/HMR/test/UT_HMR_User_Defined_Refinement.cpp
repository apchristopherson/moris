
#include <catch.hpp>
#include "cl_HMR.hpp"
#include "cl_HMR_Database.hpp"
#include "cl_HMR_Field.hpp"
#include "HMR_Globals.hpp"

#include "cl_Communication_Manager.hpp" // COM/src
#include "cl_Communication_Tools.hpp" // COM/src
#include "typedefs.hpp" // COR/src
#include "banner.hpp" // COR/src
#include "cl_Matrix.hpp" // LINALG/src
#include "linalg_typedefs.hpp" // LINALG/src
#include "fn_norm.hpp"
#include "fn_equal_to.hpp"

using namespace moris;
using namespace hmr;

moris::real
LevelSetFunction( const moris::Matrix< moris::DDRMat > & aPoint )
{
    return ( norm( aPoint ) - 1.3 ) * (-1.0) ;
}

moris::real
LevelSetFunction_2( const moris::Matrix< moris::DDRMat > & aPoint )
{
    return ( norm( aPoint ) - 1.4 )* (-1.0) ;
}

int user_defined_refinement(       Element                  * aElement,
                             const Cell< Matrix< DDRMat > > & aElementLocalValues,
                                   ParameterList            & aParameters )
{
    int aDoRefine = -1;

    // abs variable field threshold
    real lsth = 0.0;

    // abs variable field bandwidth (absolute)
    real lsbwabs = 0.1;

    // maximum refinement level
    uint maxlevel = 4;

    // min refinement level
    uint minlevel = 0;

    // max refinement level along interface
    uint maxifcref = 4;

    // max refinement level in volume
    uint maxvolref = 1;

    // current refinement level of element
    uint curlevel = aElement->get_level();

    // refinement strategy
    if ( aElementLocalValues( 0 ).max() >= lsth - lsbwabs )
    {
        // for volume refinement
        if ( aElementLocalValues( 0 ).min() >= lsth + lsbwabs )
        {
            if( curlevel < maxvolref && curlevel < maxlevel )
            {
                aDoRefine = 1; // refine
            }
            else if ( curlevel ==  maxvolref || curlevel == minlevel )
            {
                aDoRefine = 0; // keep
            }
            else
            {
                aDoRefine = -1; // coarsen
            }
        }
        // for interface refinement
        else
        {
            if( curlevel < maxifcref && curlevel < maxlevel )
            {
                aDoRefine = 1; // refine
            }
            else if ( curlevel ==  maxifcref || curlevel == minlevel )
            {
                aDoRefine = 0; // keep
            }
            else
            {
                aDoRefine = -1; // coarsen
            }
        }
    }
    else
    {
        if( curlevel <  minlevel )
        {
            aDoRefine = 1; // refine
        }
        else if ( curlevel == minlevel )
        {
            aDoRefine = 0; // keep
        }
    }

    return aDoRefine;
}


TEST_CASE("HMR_User_Defined_Refinement", "[moris],[mesh],[hmr],[HMR_User_Defined_Refinement]")
{
    //    gLogger.set_severity_level( 0 );
    // can only perform test for 1, 2 or 4 procs
    // do this test for 2 and 3 dimensions

    for( moris::uint tDimension=2; tDimension<=2; ++tDimension )
    {
    for( moris::uint tOrder=1; tOrder<=2; tOrder++ )
    {
    //------------------------------------------------------------------------------
    //  HMR Parameters setup
    //------------------------------------------------------------------------------

            uint tLagrangeMeshIndex = 0;

            // Dummy parameter list
            ParameterList tParam = create_hmr_parameter_list();

            // The parameter object controls the behavior of HMR.
            moris::hmr::Parameters tParameters;

            // set values to parameters
            moris::Matrix< moris::DDLUMat > tNumberOfElements;

            // set element size
            tNumberOfElements.set_size( tDimension, 1, 8 );

            tParameters.set_number_of_elements_per_dimension( { tNumberOfElements } );

            tParameters.set_domain_dimensions({ {2}, {2} });
            tParameters.set_domain_offset({ {0.0}, {0.0} });

            // B-Spline truncation is turned on by default.
            // It is recommended to leave this setting as is.
            tParameters.set_bspline_truncation( true );

            tParameters.set_lagrange_orders  ( { {1} });
            tParameters.set_lagrange_patterns({ {0}  });

            tParameters.set_bspline_orders   ( { {1} } );
            tParameters.set_bspline_patterns ( { {0} } );

            tParameters.set_staircase_buffer( 2 );

            tParameters.set_lagrange_input_mesh( { { 0 } } );

            tParameters.set_initial_refinement( 1 );

            Cell< Matrix< DDUMat > > tLagrangeToBSplineMesh( 1 );
            tLagrangeToBSplineMesh( 0 ) = { {0} };

            tParameters.set_lagrange_to_bspline_mesh( tLagrangeToBSplineMesh );

    //------------------------------------------------------------------------------
    //  HMR Initialization
    //------------------------------------------------------------------------------

            // create the HMR object by passing the settings to the constructor
            moris::hmr::HMR tHMR( tParameters );

            // std::shared_ptr< Database >
            auto tDatabase = tHMR.get_database();

            // initial refinement
            tHMR.perform_initial_refinement( 0 );

            //-----------------------------------------------------------------
            // First refinement with field 1
            std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tLagrangeMeshIndex );

            // create field
            std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( "Circle", tLagrangeMeshIndex );

            tField->evaluate_scalar_function( LevelSetFunction );

            moris::Cell< std::shared_ptr< moris::hmr::Field > > tFields( 1, tField );

            tHMR.user_defined_flagging( user_defined_refinement, tFields, tParam, 0 );

            tHMR.perform_refinement_based_on_working_pattern( 0, true );

            //-----------------------------------------------------------------
            // Second refinement with field 2
            tField->evaluate_scalar_function( LevelSetFunction_2 );

            tHMR.user_defined_flagging( user_defined_refinement, tFields, tParam, 0 );

            tHMR.perform_refinement_based_on_working_pattern( 0, true );

            // calculate T-Matrices etc
            tDatabase->finalize();

//            tField->evaluate_scalar_function( LevelSetFunction );
//            tHMR.save_to_exodus( 0,"UserDefinedRef.exo" );

             uint tNumElements = tMesh->get_num_elems();
             uint tNumNodes = tMesh->get_num_nodes();
             uint tNumCoeffs = tMesh->get_num_coeffs( 0 );

             // perform test
             REQUIRE( tNumElements == 619 );
//             REQUIRE( tNumNodes == 722 );
//             REQUIRE( tNumCoeffs == 584 );
    }
    }
}

