/*
 * op_less.cpp
 *
 *  Created on: Aug 29, 2018
 *      Author: doble
 */


#include <catch.hpp>
#include "fn_equal_to.hpp" //ALG

#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
#include "op_less.hpp"

namespace moris
{
TEST_CASE(
        "moris::op_less",
        "[linalgebra],[Mat],[op_less]" )
{
    Matrix< DDRMat > Am( 3, 3 );
    Matrix< DDRMat > Bm( 3, 3 );

    Am( 0, 0 ) = 1.0; Am( 0, 1 ) = 2.0; Am( 0, 2 ) = 3.0;
    Am( 1, 0 ) = 4.0; Am( 1, 1 ) = 5.0; Am( 1, 2 ) = 6.0;
    Am( 2, 0 ) = 7.0; Am( 2, 1 ) = 7.0; Am( 2, 2 ) = 9.0;

    Bm( 0, 0 ) = 3.0; Bm( 0, 1 ) = 2.0; Bm( 0, 2 ) = 1.0;
    Bm( 1, 0 ) = 6.0; Bm( 1, 1 ) = 5.0; Bm( 1, 2 ) = 4.0;
    Bm( 2, 0 ) = 9.0; Bm( 2, 1 ) = 8.0; Bm( 2, 2 ) = 7.0;

    Matrix< DDBMat > Cm = ( Am < Bm );

    Matrix< DDRMat > Ac( 3,1 );
    Matrix< DDRMat > Bc( 3,1 );

    Ac( 0,0 ) = 1.0;
    Ac( 1,0 ) = 0.0;
    Ac( 2,0 ) = 3.0;

    Bc( 0,0 ) = 1.0;
    Bc( 1,0 ) = 2.0;
    Bc( 2,0 ) = 1.0;

    Matrix< DDBMat > Cc = ( Ac < Bc );

    SECTION(
            "moris::Mat < moris::Mat" )
    {
        REQUIRE(  Cm( 0, 0 ) );
        REQUIRE( !Cm( 0, 1 ) );
        REQUIRE( !Cm( 0, 2 ) );

        REQUIRE(  Cm( 1, 0 ) );
        REQUIRE( !Cm( 1, 1 ) );
        REQUIRE( !Cm( 1, 2 ) );

        REQUIRE(  Cm( 2, 0 ) );
        REQUIRE(  Cm( 2, 1 ) );
        REQUIRE( !Cm( 2, 2 ) );
    }

    SECTION(
            "moris::Col < moris::Col" )
    {
        REQUIRE( !Cc( 0, 0 ) );
        REQUIRE(  Cc( 1, 0 ) );
        REQUIRE( !Cc( 2, 0 ) );
    }

    SECTION( "moris::Col < scalar" )
     {
         Matrix< DDRMat > A( 3, 1 );
         moris::real B;

         A( 0, 0 ) = 1.0;
         A( 1, 0 ) = 2.0;
         A( 2, 0 ) = 3.0;

         B = 2.0;

         Matrix< DDBMat > C = ( A < B );

         REQUIRE( moris::equal_to( C( 0, 0 ), 1 ) );
         REQUIRE( moris::equal_to( C( 1, 0 ), 0 ) );
         REQUIRE( moris::equal_to( C( 2, 0 ), 0 ) );
     }

     SECTION( "moris::Mat < scalar" )
     {
         Matrix< DDRMat > A( 3, 2 );
         moris::real B;

         A( 0, 0 ) = 1.0;    A( 0, 1 ) = 4.0;
         A( 1, 0 ) = 2.0;    A( 1, 1 ) = 0.0;
         A( 2, 0 ) = 3.0;    A( 2, 1 ) = 2.0;

         B = 2.0;

         Matrix< DDBMat > C = ( A < B );

         REQUIRE( moris::equal_to( C.n_rows(), 3 ) );
         REQUIRE( moris::equal_to( C.n_cols(), 2 ) );

         REQUIRE( moris::equal_to( C( 0, 0 ), 1 ) );
         REQUIRE( moris::equal_to( C( 1, 0 ), 0 ) );
         REQUIRE( moris::equal_to( C( 2, 0 ), 0 ) );
         REQUIRE( moris::equal_to( C( 0, 1 ), 0 ) );
         REQUIRE( moris::equal_to( C( 1, 1 ), 1 ) );
         REQUIRE( moris::equal_to( C( 2, 1 ), 0 ) );
     }

     SECTION( "scalar < moris::Col" )
     {
         Matrix< DDRMat > A( 3, 1 );
         moris::real B;

         A( 0, 0 ) = 1.0;
         A( 1, 0 ) = 2.0;
         A( 2, 0 ) = 3.0;

         B = 2.0;

         Matrix< DDBMat > C = ( B < A );

         REQUIRE( moris::equal_to( C.n_rows(), 3 ) );
         REQUIRE( moris::equal_to( C.n_cols(), 1 ) );

         REQUIRE( moris::equal_to( C( 0, 0 ), 0 ) );
         REQUIRE( moris::equal_to( C( 1, 0 ), 0 ) );
         REQUIRE( moris::equal_to( C( 2, 0 ), 1 ) );
     }

     SECTION( "scalar < moris::Mat " )
     {
         Matrix< DDRMat > A( 3, 2 );
         moris::real B;

         A( 0, 0 ) = 1.0;    A( 0, 1 ) = 1.0;
         A( 1, 0 ) = 2.0;    A( 1, 1 ) = 3.0;
         A( 2, 0 ) = 3.0;    A( 2, 1 ) = 2.0;

         B = 2.0;

         Matrix< DDBMat > C = ( B < A );

         REQUIRE( moris::equal_to( C.n_rows(), 3 ) );
         REQUIRE( moris::equal_to( C.n_cols(), 2 ) );

         REQUIRE( moris::equal_to( C( 0, 0 ), 0 ) );
         REQUIRE( moris::equal_to( C( 1, 0 ), 0 ) );
         REQUIRE( moris::equal_to( C( 2, 0 ), 1 ) );
         REQUIRE( moris::equal_to( C( 0, 1 ), 0 ) );
         REQUIRE( moris::equal_to( C( 1, 1 ), 1 ) );
         REQUIRE( moris::equal_to( C( 2, 1 ), 0 ) );
     }
}
}

