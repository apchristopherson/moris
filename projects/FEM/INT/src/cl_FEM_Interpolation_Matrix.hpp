/*
 * cl_FEM_Matrix.hpp
 *
 *  Created on: Jul 13, 2018
 *      Author: messe
 */

#ifndef SRC_FEM_CL_FEM_INTERPOLATION_MATRIX_HPP_
#define SRC_FEM_CL_FEM_INTERPOLATION_MATRIX_HPP_

#include <string>
#include <utility>
#include "typedefs.hpp" //MRS/COR/src
#include "cl_Mat.hpp" //LNA/src
#include "op_times.hpp" //LNA/src
#include "op_plus.hpp" //LNA/src
#include "op_minus.hpp" //LNA/src
#include "fn_trans.hpp" //LNA/src
#include "fn_det.hpp" //LNA/src
#include "fn_inv.hpp" //LNA/src

namespace moris
{
    namespace fem
    {
//------------------------------------------------------------------------------

        class Interpolation_Matrix
        {
            //! space derivative flag set during construction
            const uint mSpaceFlag;

            //! time derivative flag set during construction
            const uint mTimeFlag;

            //! coefficient flag set during construction
            const uint mCoeffFlag;

            //! matrix that contains data
            Mat< real > mData;
//------------------------------------------------------------------------------
        public :
//------------------------------------------------------------------------------

            /**
             *  constructor
             */
            Interpolation_Matrix(
                    const uint & aSpaceFlag,
                    const uint & aTimeFlag,
                    const uint & aCoeffFlag,
                    const uint & aNumberOfRows,
                    const uint & aNumberOfCols ) :
                        mSpaceFlag( aSpaceFlag ),
                        mTimeFlag( aTimeFlag ),
                        mCoeffFlag( aCoeffFlag )
            {
                mData.set_size(  aNumberOfRows, aNumberOfCols );
            }

//------------------------------------------------------------------------------

            /**
             * alternative constructor using moris::mat
             */
            Interpolation_Matrix(
                    const uint & aSpaceFlag,
                    const uint & aTimeFlag,
                    const uint & aCoeffFlag,
                    const Mat< real > & aData ) :
                        mSpaceFlag( aSpaceFlag ),
                        mTimeFlag( aTimeFlag ),
                        mCoeffFlag( aCoeffFlag ),
                        mData( aData )
            {

            }

//------------------------------------------------------------------------------

            /**
             * default destructor
             */
            ~Interpolation_Matrix(){};

//------------------------------------------------------------------------------

            /**
             * sets size of member matrix
             */
            void
            set_size(
                    const uint & aNumberOfRows,
                    const uint & aNumberOfCols )
            {
                mData.set_size( aNumberOfRows, aNumberOfCols );
            }

//------------------------------------------------------------------------------

            /**
             * sets size of member matrix
             */
            void
            set_size(
                    const uint & aNumberOfRows,
                    const uint & aNumberOfCols,
                    const real & aValue )
            {
                mData.set_size( aNumberOfRows, aNumberOfCols, aValue );
            }
//------------------------------------------------------------------------------

            /**
             *  expose data object, writable version
             */
            auto
            data() -> decltype( mData.data() )
            {
                return mData.data();
            }

//------------------------------------------------------------------------------

            /**
             * exposes the data object, const version
             */
            auto
            data() const -> decltype( mData.data() )
            {
                return mData.data();
            }

//------------------------------------------------------------------------------

            /**
             * allows access to an entry in a vector
             *
             * @param[ in ] aI  index in vector
             */
            auto
            operator()( const uint & aI )
                -> decltype( mData( aI ) )
            {
                return mData( aI );
            }

//------------------------------------------------------------------------------

            /**
             * allows access to an entry in a vector ( const version )
             *
             * @param[ in ] aI  index in vector
             */
            auto
            operator()( const uint & aI ) const
                -> decltype( mData( aI ) )
            {
                return mData( aI );
            }

//------------------------------------------------------------------------------

            /**
             * allows access to an entry in a matrix
             *
             * @param[ in ] aI  row index
             *
             * @param[ in ] aJ  column index
             *
             */
            auto
            operator()( const uint & aI, const uint & aJ )
                -> decltype( mData( aI, aJ ) )
            {
                return mData( aI, aJ );
            }

//------------------------------------------------------------------------------

            /**
             * allows access to an entry in a matrix ( const version )
             *
             * @param[ in ] aI  row index
             *
             * @param[ in ] aJ  column index
             *
             */
            auto
            operator()( const uint & aI, const uint & aJ ) const
                -> decltype( mData( aI, aJ ) )
            {
                return mData( aI, aJ );
            }

//------------------------------------------------------------------------------

            /**
             * returns the length of a vector
             */
            auto
            length() const -> decltype( mData.length() )
            {
                return mData.length();
            }

//------------------------------------------------------------------------------

            /**
             * returns the number of rows of a matrix
             */
            auto
            n_rows() const -> decltype ( mData.n_rows() )
            {
                return mData.n_rows();
            }

//------------------------------------------------------------------------------
            /**
             * returns the number of cols of a matrix
             */
            auto
            n_cols() const -> decltype ( mData.n_cols() )
            {
                return mData.n_cols();
            }

//------------------------------------------------------------------------------

            /**
             * calls the print routine of the moris::Mat
             */
            void
            print( const std::string & aVarName = std::string() )
            {
                mData.print( aVarName );
            }

//------------------------------------------------------------------------------

            /**
             * returns the sum of the data ( needed for testing of N unity )
             */
            real
            sum() const
            {

                real aSum = 0.0;
                uint tLength = mData.length();

                for( uint k=0; k<tLength; ++k )
                {
                    aSum += mData( k );
                }
                return aSum;
            }

//------------------------------------------------------------------------------

            /**
             * returns the space derivative flag
             */
            auto get_space_flag() const
                -> decltype ( mSpaceFlag )
            {
                return mSpaceFlag;
            }

//------------------------------------------------------------------------------

            /**
             * returns the time derivative flag
             */
            auto get_time_flag() const
                -> decltype ( mTimeFlag )
            {
                return mTimeFlag;
            }

//------------------------------------------------------------------------------

            /**
             * returns the flag for the coefficients
             */
            auto get_coeff_flag() const
                -> decltype ( mCoeffFlag )
            {
                return mCoeffFlag;
            }

//------------------------------------------------------------------------------
        };
//------------------------------------------------------------------------------
//  free operators
//------------------------------------------------------------------------------

        auto
        operator*(  const Interpolation_Matrix & aA,
                    const Mat< real >          & aB )
            ->  decltype( aA.data() * aB.data() )

        {
            return aA.data() * aB.data();
        }

//------------------------------------------------------------------------------

        auto
        operator*(  const Mat< real >           & aA,
                    const Interpolation_Matrix  & aB )
            ->  decltype( aA.data() * aB.data() )

        {
            return aA.data() * aB.data();
        }

//------------------------------------------------------------------------------

        auto
        operator*(  const Interpolation_Matrix  & aA,
                    const Interpolation_Matrix  & aB )
        ->  decltype( aA.data() * aB.data() )

        {
            return aA.data() * aB.data();
        }

//------------------------------------------------------------------------------

        /**
         * calculates the determinant of a matrix
         * @param[ in ] aA   matrix to process
         */
        auto
        det( const Interpolation_Matrix & aA ) -> decltype( det( aA.data() ) )
        {
            return det( aA.data() );
        }

//------------------------------------------------------------------------------

        /**
         * inverts a matrix
         * @param[ in ] aA   matrix to process
         */
        auto
        inv( const Interpolation_Matrix & aA ) -> decltype( inv( aA.data() ) )
        {
            return inv( aA.data() );
        }

//------------------------------------------------------------------------------

        /**
         * transposes a matrix
         * @param[ in ] aA   matrix to process
         */
        Interpolation_Matrix
        trans( const Interpolation_Matrix & aA )
        {

            return Interpolation_Matrix(
                    aA.get_space_flag(),
                    aA.get_time_flag(),
                    aA.get_coeff_flag(),
                    trans( aA.data() ) );
        }

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */


#endif /* SRC_FEM_CL_FEM_INTERPOLATION_MATRIX_HPP_ */
