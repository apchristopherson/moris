#include <catch.hpp>
#include "fn_equal_to.hpp" //ALG

#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
#include "fn_unique.hpp"

TEST_CASE("moris::unique",
          "[linalgebra],[unique]")
{
    //Demonstrates the functionality of "unique". It provides a vector, in which every value from a given vector is only once in there.
    // Unique first sorts the vector and takes then the unique entries.
    SECTION( "unique of uint row vector" )
    {
        //Uniqueness of row vector

        moris::Matrix< moris::uint, moris::DDUMat > A( 7, 1 );

        A( 0, 0 ) = 1;
        A( 1, 0 ) = 2;
        A( 2, 0 ) = 3;
        A( 3, 0 ) = 2;
        A( 4, 0 ) = 2;
        A( 5, 0 ) = 4;
        A( 6, 0 ) = 1;

        moris::Matrix< moris::uint, moris::DDUMat > C = moris::unique(A);
        REQUIRE( moris::equal_to( C(0,0), 1 ) );
        REQUIRE( moris::equal_to( C(1,0), 2 ) );
        REQUIRE( moris::equal_to( C(2,0), 3 ) );
    }

    SECTION( "unique of real row vector" )
    {

        //Uniqueness of row vector
        moris::Matrix< moris::real, moris::DDRMat >A( 7, 1 );

        A( 0, 0 ) = 1.1;
        A( 1, 0 ) = 2.1;
        A( 2, 0 ) = 2.1;
        A( 3, 0 ) = 3.1;
        A( 4, 0 ) = 2.1;
        A( 5, 0 ) = 4.1;
        A( 6, 0 ) = 1.1;

        moris::Matrix< moris::real, moris::DDRMat > C = moris::unique(A);
        REQUIRE( moris::equal_to( C(0,0), 1.1 ) );
        REQUIRE( moris::equal_to( C(1,0), 2.1 ) );
        REQUIRE( moris::equal_to( C(2,0), 3.1 ) );
    }

    SECTION( "unique of uint col vector" )
    {
        //Uniqueness of col vector

        moris::Matrix< moris::uint, moris::DDUMat > A( 1,7 );

        A( 0, 0 ) = 1;
        A( 0, 1 ) = 2;
        A( 0, 2 ) = 2;
        A( 0, 3 ) = 3;
        A( 0, 4 ) = 1;
        A( 0, 5 ) = 2;
        A( 0, 6 ) = 4;

        moris::Matrix< moris::uint, moris::DDUMat > C = moris::unique(A);
        REQUIRE( moris::equal_to( C(0,0), 1 ) );
        REQUIRE( moris::equal_to( C(0,1), 2 ) );
        REQUIRE( moris::equal_to( C(0,2), 3 ) );
        REQUIRE( moris::equal_to( C(0,3), 4 ) );
    }

    SECTION( "unique of real col vector" )
    {
        //Uniqueness of col vector

        moris::Matrix< moris::real, moris::DDRMat> A( 1,8 );

        A( 0, 0 ) = 1.1;
        A( 0, 1 ) = 2.1;
        A( 0, 2 ) = 2.1;
        A( 0, 3 ) = 3.1;
        A( 0, 4 ) = 1.1;
        A( 0, 5 ) = 4.1;
        A( 0, 6 ) = 2.3;
        A( 0, 7 ) = 5.3;
        moris::Matrix< moris::real, moris::DDRMat > C = moris::unique(A);
        REQUIRE( moris::equal_to( C(0,0), 1.1 ) );
        REQUIRE( moris::equal_to( C(0,1), 2.1 ) );
        REQUIRE( moris::equal_to( C(0,2), 2.3 ) );
        REQUIRE( moris::equal_to( C(0,3), 3.1 ) );
        REQUIRE( moris::equal_to( C(0,4), 4.1 ) );
    }
}
