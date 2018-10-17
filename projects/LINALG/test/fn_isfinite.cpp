/*
 * fn_isfinite.cpp
 *
 *  Created on: Aug 29, 2018
 *      Author: schmidt
 */

// Third-party header files.
#include <catch.hpp>
#include "fn_equal_to.hpp" // ALG/src
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
#include "typedefs.hpp"
#include "fn_isfinite.hpp"

namespace moris
{
TEST_CASE( "moris::isfinite", "[linalgebra],[isfinite]" )
    {
    moris::real tZero = 0.0;
    Matrix< DDRMat > a( 3, 3 );
    Matrix< DDRMat > b( 3, 3 );

    a( 0, 0 ) = 1.0; a( 0, 1 ) = 2.0; a( 0, 2 ) = 3.0;
    a( 1, 0 ) = 0.0; a( 1, 1 ) = 1.0; a( 1, 2 ) = 4.0;
    a( 2, 0 ) = 5.0; a( 2, 1 ) = 6.0; a( 2, 2 ) = 0.0;

    b( 0, 0 ) = 1.0; b( 0, 1 ) = 2.0; b( 0, 2 ) = 3.0;
    b( 1, 0 ) = 0.0; b( 1, 1 ) = 1.0; b( 1, 2 ) = 4.0;
    b( 2, 0 ) = 5.0; b( 2, 1 ) = 6.0; b( 2, 2 ) = 1/tZero;

    bool tIsFinite_1 = isfinite( a );
    bool tIsFinite_2 = isfinite( b );

    CHECK( equal_to( tIsFinite_1, true ) );
    CHECK( equal_to( tIsFinite_2, false ) );
    }
}