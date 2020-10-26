/*
 * cl_Matrix.cpp
 *
 *  Created on: Aug 27, 2018
 *      Author: doble
 */
#define ARMA_ALLOW_FAKE_GCC
#include <armadillo>

#include <catch.hpp>
#include "fn_equal_to.hpp" //ALG

#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"

#include "op_times.hpp"
#include "fn_inv.hpp" //ALG
#include "fn_norm.hpp" //ALG
#include "fn_linsolve.hpp"

namespace moris
{

    template <typename T1, typename T2>
    auto
    test(T1 const & aA,
            T2 const & aB)
    ->decltype(aA*aB)
    {
        return aA*aB;
    }

    TEST_CASE("MORIS Linear Algebra Matrix Tests","[MATRIX]")
    {
        SECTION("Matrix Tests using default"){


            srand (time(0));
            moris::uint tNumCR = rand()%200;
            printf ("First number: %d\n", tNumCR);
            Matrix< DDRMat > tA(tNumCR,tNumCR,1.0);
            Matrix< DDRMat > tB(tNumCR,tNumCR,-1.0);
            Matrix< DDRMat > tC(tNumCR,tNumCR,-2.0);
            Matrix< DDRMat > tD(tNumCR,tNumCR,4.0);

            moris::Matrix<DDRMat> tTestET = test(tA,tB);
            moris::Matrix<DDRMat> tTestMat =  tTestET;
            std::cout<<tTestMat(0,0)<<std::endl;

            moris::uint tNumIts = 1;
            std::clock_t tTotalTime = std::clock();
            for(moris::uint i = 0; i <tNumIts; i++)
            {
                Matrix< DDRMat > tAB  = tA*tB;
                Matrix< DDRMat > tABC = tAB*tC;
                Matrix< DDRMat > tE   = tABC*tD;
            }
            std::cout<<"Matrix multiplication without expression templates completed in " <<(std::clock() - tTotalTime) / (double)(CLOCKS_PER_SEC) * 1000<<" s."<<std::endl;

            // Start clock
            DDRMat tAA(tNumCR,tNumCR);
            tAA.fill(1.0);
            DDRMat tBA(tNumCR,tNumCR);
            tBA.fill(-1.0);
            DDRMat tCA(tNumCR,tNumCR);
            tCA.fill(-2.0);
            DDRMat tDA(tNumCR,tNumCR);
            tDA.fill(4.0);

            tTotalTime = std::clock();

            for(moris::uint i = 0; i <tNumIts; i++)
            {
                DDRMat tE = (tAA*tBA*tCA*tDA);
            }
            std::cout<<"Matrix multiplication with armadillo direct completed in " <<(std::clock() - tTotalTime) / (double)(CLOCKS_PER_SEC)* 1000<<" s."<<std::endl;

            // Start clock
            tTotalTime = std::clock();

            for(moris::uint i = 0; i <tNumIts; i++)
            {
                Matrix< DDRMat > tE = tA*tB*tC*tD;
            }
            std::cout<<"Matrix multiplication with expression templates completed in " <<(std::clock() - tTotalTime) / (double)(CLOCKS_PER_SEC)* 1000<<" s."<<std::endl;

            // Create matrix base
            Matrix< DDRMat > tMatrix1(1,2);
            Matrix< DDRMat > tMatrix2(0,0);
            Matrix< DDRMat > tMatrix3(5,4);
            Matrix< DDRMat > tMatrix4(5,4,-1);

            // Check number of columns
            REQUIRE(tMatrix1.n_cols() == 2);
            REQUIRE(tMatrix2.n_cols() == 0);
            REQUIRE(tMatrix3.n_cols() == 4);

            // Check number of rows
            REQUIRE(tMatrix1.n_rows() == 1);
            REQUIRE(tMatrix2.n_rows() == 0);
            REQUIRE(tMatrix3.n_rows() == 5);

            // Check number of elements in matrices
            REQUIRE(tMatrix1.numel() == 2);
            REQUIRE(tMatrix2.numel() == 0);
            REQUIRE(tMatrix3.numel() == 20);

            // Resize tMatrix2 and check again
            tMatrix2.resize(12,4);
            REQUIRE(tMatrix2.n_rows() == 12);
            REQUIRE(tMatrix2.n_cols() == 4);
            REQUIRE(tMatrix2.numel() == 48);

            // Add Matrix Data by row index and location
            tMatrix1(0,0) = 1;
            tMatrix1(0,1) = 2;
            REQUIRE(tMatrix1(0,0) == 1);
            REQUIRE(tMatrix1(0,1) == 2);

            // Add Matrix data to std::shared_ptr<Matrix<type>>
            tMatrix2(0,0)= 1;
            tMatrix2(0,1)= 2;
            REQUIRE(tMatrix2(0,0) == 1);
            REQUIRE(tMatrix2(0,1) == 2);

            // Check filling operation of tMat4 (internal to create());
            REQUIRE(tMatrix4(0,0) == -1);

            // Check explicit fill call
            tMatrix4.fill(48);
            REQUIRE(tMatrix4(0,0) == 48);

            // Set and get Columns
            Matrix< DDRMat >  tMatrix5(4,4,10);

            Matrix< DDRMat > tMatrixRow1 = tMatrix5.get_row(2);
            REQUIRE(tMatrixRow1(0,0) == 10);

            Matrix< DDRMat >  tMatrixColumn1 = tMatrix5.get_column(1);
            REQUIRE(tMatrixColumn1(1,0) == 10);

            Matrix< DDRMat > tMatrixRow2(1,4,5);
            Matrix< DDRMat > tMatrixColumn2(4,1,7);

            tMatrix5.set_row(3,tMatrixRow2);
            tMatrix5.set_column(3,tMatrixColumn2);

            REQUIRE(tMatrix5(3,2) == 5);
            REQUIRE(tMatrix5(3,3) == 7);

            // Test maximum and minimum values
            Matrix< DDRMat > tMatrix6(10,7,0);
            tMatrix6(3,5) = 10;
            tMatrix6(5,5) = -11;

            REQUIRE(tMatrix6.max() == 10);
            REQUIRE(tMatrix6.min() == -11);

            // Create a matrix using a standard initializer list

            Matrix< DDRMat > tMatrix7({{1,2,3},{4,5,6},{7,8,9}});

            REQUIRE(tMatrix7(0,0) = 1);
            REQUIRE(tMatrix7(1,1) = 5);
            REQUIRE(tMatrix7(2,2) = 9);

            // Check Data function
            const real* tMatrix7Data = tMatrix7.data();

            // Column Major Data Structure Check
            REQUIRE(tMatrix7Data[0] == 1);
            REQUIRE(tMatrix7Data[1] == 4);
            REQUIRE(tMatrix7Data[2] == 7);
            REQUIRE(tMatrix7Data[3] == 2);
            REQUIRE(tMatrix7Data[4] == 5);
            REQUIRE(tMatrix7Data[5] == 8);
            REQUIRE(tMatrix7Data[6] == 3);
            REQUIRE(tMatrix7Data[7] == 6);
            REQUIRE(tMatrix7Data[8] == 9);

            // Copying an existing matrix
            Matrix< DDRMat > tMatrix7Copy = tMatrix7.copy();

            // Modify the original and see if the copy changes (it shouldn't)
            tMatrix7(0,0) = 44;


            Matrix< DDRMat > a( 3, 3 );
            a( 0, 0 ) = 1.0; a( 0, 1 ) = 2.0; a( 0, 2 ) = 3.0;
            a( 1, 0 ) = 4.0; a( 1, 1 ) = 5.0; a( 1, 2 ) = 6.0;
            a( 2, 0 ) = 9.0; a( 2, 1 ) = 8.0; a( 2, 2 ) = 9.0;

            Matrix< DDRMat > aSpan = a( {1, 2}, {1, 2} );
            REQUIRE( moris::equal_to( aSpan( 0, 0 ), 5.0 ) );
            REQUIRE( moris::equal_to( aSpan( 0, 1 ), 6.0 ) );
            REQUIRE( moris::equal_to( aSpan( 1, 0 ), 8.0 ) );
            REQUIRE( moris::equal_to( aSpan( 1, 1 ), 9.0 ) );


            //        // Index out of bounds
            //        REQUIRE_THROWS(tMatrix7(3,3) = 0);
            //
            //        // Initializer list with a mistake in it
            //        REQUIRE_THROWS(Matrix< DDRMat >({{1,2,3,4},{4,5,6},{7,8,9}}));

        }
    }

    TEST_CASE( "matrix_performance_arma", "[moris],[matrix_performance_arma]" )
    {
        // Initialize random matrix size
        srand (time(NULL));
        uint tDimX = rand() % 40 + 130;
        uint tDimY = rand() % 40 + 130;

        uint tNumRepetitions = 10000+tDimX;

        moris::Matrix<DDRMat> tMorisMat(tDimX,tDimY);
        arma::Mat<double>     tArmaMat(tDimX,tDimY);

        uint tRandNumerator   = 1;
        uint tRandDenominator = 1;
        real tRandNumber      = 0.0;

        MORIS_LOG_INFO( "Start create random MORIS-matrix and ARMA-matrix.....");

        for ( uint ii = 0; ii < tDimX; ii++ ){
            for ( uint jj = 0; jj < tDimY; jj++ ){

                // create a random number
                srand (time(NULL));
                tRandNumerator = rand() % 10 + 1;
                tRandDenominator = rand() % 10 + 1;
                tRandNumber = (moris::real) tRandNumerator / (moris::real) tRandDenominator;

                // copy random number to matrix
                tMorisMat(ii,jj) = tRandNumber;
                tArmaMat(ii,jj)  = tRandNumber;
            }
        }

        MORIS_LOG_INFO( "Done. Random %i x %i - MORIS-matrix and ARMA-matrix created.", tDimX, tDimY);

        // time Moris Matrix summation --------------------------------------------------

        real tSum = 0.0;

        std::clock_t tTimeStamp = std::clock();

        for( uint iCounter = 0; iCounter < tNumRepetitions; iCounter++ )
        {
            for ( uint ii = 0; ii < tDimX; ii++ ){
                for ( uint jj = 0; jj < tDimY; jj++ ){
                    tSum += tMorisMat(ii,jj);
                }
            }
        }

        moris::real tTimeForSummation = 1000 * (moris::real) ( clock() - tTimeStamp ) / CLOCKS_PER_SEC;
        MORIS_LOG_INFO( "Time to add all members on %i %i x %i MORIS-matrices: %5.2f milliseconds (result %e).",
                tNumRepetitions, tDimX, tDimY, ( double ) tTimeForSummation, tSum );

        // time Arma Matrix summation --------------------------------------------------

        tSum = 0.0;

        tTimeStamp = std::clock();

        for( uint iCounter = 0; iCounter < tNumRepetitions; iCounter++ )
        {
            for ( uint ii = 0; ii < tDimX; ii++ ){
                for ( uint jj = 0; jj < tDimY; jj++ ){
                    tSum += tArmaMat(ii,jj);
                }
            }
        }

        tTimeForSummation = 1000 * (moris::real) ( clock() - tTimeStamp ) / CLOCKS_PER_SEC;
        MORIS_LOG_INFO( "Time to add all members on %i %i x %i ARMA-matrices: %5.2f milliseconds (result %d).",
                tNumRepetitions, tDimX, tDimY, ( double ) tTimeForSummation, tSum );

        // time Moris Matrix += - operations --------------------------------------------------
        moris::Matrix<DDRMat> tResultMatMoris = tMorisMat;

        tNumRepetitions = 100000+tDimX;

        tTimeStamp = std::clock();

        tResultMatMoris = tResultMatMoris + tMorisMat;

        tResultMatMoris = tResultMatMoris + tMorisMat;

        for( uint iCounter = 0; iCounter < tNumRepetitions; iCounter++ )
        {
            tResultMatMoris = tResultMatMoris + tMorisMat;
        }

        real tTimeForSummationMoris1 = 1000 * (moris::real) ( clock() - tTimeStamp ) / CLOCKS_PER_SEC;
        MORIS_LOG_INFO( "Time to 'equal-mat-plus-mat' %i %i x %i MORIS-matrices: %5.2f milliseconds (result: %e).",
                tNumRepetitions, tDimX, tDimY, ( double ) tTimeForSummationMoris1, norm(tResultMatMoris) );

        tResultMatMoris = tMorisMat;

        tTimeStamp = std::clock();

        tResultMatMoris += tMorisMat;

        for( uint iCounter = 0; iCounter < tNumRepetitions; iCounter++ )
        {
            tResultMatMoris += tMorisMat;
        }

        real tTimeForSummationMoris2 = 1000 * (moris::real) ( clock() - tTimeStamp ) / CLOCKS_PER_SEC;
        MORIS_LOG_INFO( "Time to 'plus-equal'         %i %i x %i MORIS-matrices: %5.2f milliseconds (result: %e).",
                tNumRepetitions, tDimX, tDimY, ( double ) tTimeForSummationMoris2, norm(tResultMatMoris) );

        // time ARMA Matrix += - operations --------------------------------------------------
        arma::Mat<double> tResultMatArma = tArmaMat;

        tTimeStamp = std::clock();

        tResultMatArma = tResultMatArma + tArmaMat;

        for( uint iCounter = 0; iCounter < tNumRepetitions; iCounter++ )
        {
            tResultMatArma = tResultMatArma + tArmaMat;
        }

        real tTimeForSummationArma1 = 1000 * (moris::real) ( clock() - tTimeStamp ) / CLOCKS_PER_SEC;
        MORIS_LOG_INFO( "Time to 'equal-mat-plus-mat' %i %i x %i ARMA-matrices: %5.2f milliseconds (result: %e).",
                tNumRepetitions, tDimX, tDimY, ( double ) tTimeForSummationArma1, norm(tResultMatArma) );

        tResultMatArma = tArmaMat;

        tTimeStamp = std::clock();

        tResultMatArma += tArmaMat;

        for( uint iCounter = 0; iCounter < tNumRepetitions; iCounter++ )
        {
            tResultMatArma += tArmaMat;
        }

        real tTimeForSummationArma2 = 1000 * (moris::real) ( clock() - tTimeStamp ) / CLOCKS_PER_SEC;
        MORIS_LOG_INFO( "Time to 'plus-equal'         %i %i x %i ARMA-matrices: %5.2f milliseconds (result: %e).",
                tNumRepetitions, tDimX, tDimY, ( double ) tTimeForSummationArma2, norm(tResultMatArma) );

        MORIS_LOG_INFO( "Time comparison: Moris x=x+y ratio %f   Moris x+=y ratio %f  Arma x=x+y ratio %f Arma x+=y %f.",
                ( double ) tTimeForSummationMoris1/tTimeForSummationArma2,
                ( double ) tTimeForSummationMoris2/tTimeForSummationArma2,
                ( double ) tTimeForSummationArma1/tTimeForSummationArma2,
                ( double ) tTimeForSummationArma2/tTimeForSummationArma2 );

    }

    TEST_CASE( "matrix_solve", "[moris],[matrix_solve]" )
    {
        // this performance comparison test should be only performed in non-debug mode
        // as it will trigger error in solve and inv functions due to inefficient matrix sizes
#ifndef DEBUG
        uint tSizeIncr       = 5;
        uint tMaxSize        = 100/tSizeIncr;
        uint tMaxRHS         = 10/tSizeIncr;
        uint tNumRepetitions = 10000;

        // loop over increasing matrices sizes
        for (uint iz=0;iz<tMaxSize+1;++iz)
        {
            uint imat = iz==0 ? 2 : iz*tSizeIncr;

            // Create matrix (1-D truss problem)
            moris::Matrix<DDRMat> tKmat(imat,imat,0.0);

            tKmat(0,0) = 2.0*imat;
            tKmat(0,1) =-1.0*imat;

            for (uint ir=1;ir<imat-1;++ir)
            {
                tKmat(ir,ir-1)=-1.0*imat;
                tKmat(ir,ir  )= 2.0*imat;
                tKmat(ir,ir+1)=-1.0*imat;
            }

            tKmat(imat-1,imat-2) =-1.0*imat;
            tKmat(imat-1,imat-1) = 2.0*imat;

            // loop over size of RHS

            for (uint ir=0;ir<tMaxRHS;ir++)
            {
                uint irhs = ir==0 ? 1 : ir*tSizeIncr;

                // Create rhs
                moris::Matrix<DDRMat> tRHSsolve(imat,irhs,1.0/imat);
                moris::Matrix<DDRMat> tRHSinv(imat,irhs,1.0/imat);

                // Allocate solution vector
                moris::Matrix<DDRMat> tSolsolve(imat,irhs,0.0);
                moris::Matrix<DDRMat> tSolinv(imat,irhs,0.0);

                // time solves
                std::clock_t tTimeStampSolve = std::clock();

                for( uint iCounter = 0; iCounter < tNumRepetitions; iCounter++ )
                {
                    // increment RHS and add solution to sol vector to avoid being optimized out
                    tRHSsolve = tRHSsolve + pow(-1.0,iCounter)/tNumRepetitions * tSolsolve;
                    tSolsolve = tSolsolve + solve(tKmat,tRHSsolve);
                }

                moris::real tTimeForSolve = 1000 * (moris::real) ( clock() - tTimeStampSolve ) / CLOCKS_PER_SEC;

                // time solves
                std::clock_t tTimeStampInv = std::clock();

                for( uint iCounter = 0; iCounter < tNumRepetitions; iCounter++ )
                {
                    tRHSinv = tRHSinv + pow(-1.0,iCounter)/tNumRepetitions * tSolinv;
                    tSolinv = tSolinv + inv(tKmat) * tRHSinv;
                }

                moris::real tTimeForInv = 1000 * (moris::real) ( clock() - tTimeStampInv ) / CLOCKS_PER_SEC;

                moris::real tTimeRatio = tTimeForSolve/tTimeForInv;

                MORIS_LOG_INFO( "Time for %i solves verses inverse of matrix of size %i and RHS of size %i: ratio = %5.2f : solve = %5.2f milliseconds inverse = %5.2f  (difference in results = %5.2f perc).",
                        tNumRepetitions, imat, irhs, (double) tTimeRatio, ( double ) tTimeForSolve, ( double ) tTimeForInv, norm(tSolsolve-tSolinv)/norm(tSolsolve)*100.0 );
            }
        }
#endif
    }
}
