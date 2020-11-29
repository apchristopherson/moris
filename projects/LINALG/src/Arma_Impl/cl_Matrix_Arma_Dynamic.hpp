/*
 * cl_Matrix_Arma_3x3.hpp
 *
 *  Created on: Aug 24, 2018
 *      Author: doble
 */

#ifndef PROJECTS_LINALG_SRC_ARMA_IMPL_CL_MATRIX_ARMA_DYNAMIC_HPP_
#define PROJECTS_LINALG_SRC_ARMA_IMPL_CL_MATRIX_ARMA_DYNAMIC_HPP_
#define ARMA_ALLOW_FAKE_GCC
#include <armadillo>

#include "typedefs.hpp"

#include "cl_Matrix.hpp"
#include "fn_iscol.hpp"
#include "fn_isvector.hpp"

namespace moris
{


    template<typename Type>
    class Matrix<arma::Mat<Type>>
    {
        private:
            arma::Mat<Type> mMatrix;

            void fill_with_NANs()
            {
#ifdef MATRIX_FILL
                if (std::numeric_limits<Data_Type>::has_quiet_NaN)
                {
                    mMatrix.fill( std::numeric_limits<Data_Type>::quiet_NaN() );
                }
                else
                {
                    mMatrix.fill( std::numeric_limits<Data_Type>::max() );
                }
#endif
            }

        public:

            typedef Type                      Data_Type;

            // Iterators
            typedef typename arma::Mat<Type>::iterator       Mat_It;
            typedef typename arma::Mat<Type>::const_iterator Const_Mat_It;

            Matrix()
            {

            };

            Matrix(
                    size_t const & aNumRows,
                    size_t const & aNumCols)
            : mMatrix(aNumRows,aNumCols)
            {
                MORIS_ASSERT(sizeof(Type)*aNumRows*aNumCols < MORIS_MAX_MATRIX_SIZE,
                        "Matrix::Matrix: Maximum allowable size exceeded: %f MB.\n",sizeof(Type)*aNumRows*aNumCols/1e6);

                this->fill_with_NANs();
            }

            Matrix(
                    size_t const & aNumEl)
            : mMatrix(aNumEl,1)
            {
                MORIS_ASSERT(sizeof(Type)*aNumEl < MORIS_MAX_MATRIX_SIZE,
                        "Matrix::Matrix: Maximum allowable size exceeded: %f MB.\n",sizeof(Type)*aNumEl/1e6);

                this->fill_with_NANs();
            }

            // template constructor
            Matrix(arma::Mat<Type> const & X )
            : mMatrix(X)
            {
            }

            // template constructor
            template< typename A >
            Matrix(A const & X )
            : mMatrix(X)
            {
            }

            Matrix(
                    size_t const & aNumRows,
                    size_t const & aNumCols,
                    Type   const & aFillVal)
            : mMatrix( aNumRows, aNumCols )
            {
                MORIS_ASSERT(sizeof(Type)*aNumRows*aNumCols < MORIS_MAX_MATRIX_SIZE,
                         "Matrix::Matrix: Maximum allowable size exceeded: %f MB.\n",sizeof(Type)*aNumRows*aNumCols/1e6);

                mMatrix.fill(aFillVal);
            }

            Matrix(
                    Type*  const & aArray,
                    size_t const & aNumRows,
                    size_t const & aNumCols)
            : mMatrix( aArray, aNumRows, aNumCols )
            {
                MORIS_ASSERT(sizeof(Type)*aNumRows*aNumCols < MORIS_MAX_MATRIX_SIZE,
                         "Matrix::Matrix: Maximum allowable size exceeded: %f MB.\n",sizeof(Type)*aNumRows*aNumCols/1e6);
            }

            Matrix(std::initializer_list<std::initializer_list<Type> > const & aInitList)
            : mMatrix(aInitList)
            {
            }

            // Copy operations
            Matrix<arma::Mat<Type>>
            copy() const
            {
                Matrix<arma::Mat<Type>> tMatCopy(this->n_rows(),this->n_cols());
                tMatCopy.matrix_data() = mMatrix;
                return tMatCopy;
            }

            void
            resize(
                    const size_t & aNumRows,
                    const size_t & aNumCols)
            {
                // check that resize on large cells does not indicate overly conservative initial size allocation
                MORIS_ASSERT(
                        sizeof(Type)*this->numel() > MORIS_MATRIX_RESIZE_CHECK_LIMIT * MORIS_MAX_MATRIX_SIZE ?
                        aNumRows*aNumCols > MORIS_MATRIX_RESIZE_FRACTION_LIMIT * this->numel() : true,
                        "Matrix::resize: resize to less than 1 percent of large matrix - reduce initial allocation.\n");

                mMatrix.resize(aNumRows, aNumCols);
            }

            void
            set_size(
                    const size_t & aNumRows,
                    const size_t & aNumCols)
            {
                MORIS_ASSERT(sizeof(Type)*aNumRows*aNumCols < MORIS_MAX_MATRIX_SIZE,
                         "Matrix::Matrix: Maximum allowable size exceeded: %f MB.\n",sizeof(Type)*aNumRows*aNumCols/1e6);

                mMatrix.set_size(aNumRows, aNumCols);

                this->fill_with_NANs();
            }

            void
            set_size(
                    const size_t & aNumRows,
                    const size_t & aNumCols,
                    const Type   & aFillValue )
            {
                MORIS_ASSERT(sizeof(Type)*aNumRows*aNumCols < MORIS_MAX_MATRIX_SIZE,
                         "Matrix::Matrix: Maximum allowable size exceeded: %f MB.\n",sizeof(Type)*aNumRows*aNumCols/1e6);

                mMatrix.set_size( aNumRows, aNumCols );

                mMatrix.fill( aFillValue );
            }

            void
            fill(const Type & aFillValue)
            {
                mMatrix.fill(aFillValue);
            }

            /**
             * Get the number of columns in a data set, similar to Matlab cols().
             *
             * @return Number of columns.
             */
            size_t
            n_cols() const
            {
                return mMatrix.n_cols;
            }

            /**
             * Get the number of rows in a data set, similar to Matlab rows().
             *
             * @return Number of rows.
             */
            size_t
            n_rows() const
            {
                return mMatrix.n_rows;
            }

            size_t
            size(size_t aDim)
            {
                switch (aDim)
                {
                    case 0:
                    {
                        return this->n_rows();
                        break;
                    }
                    case 1:
                    {
                        return this->n_cols();
                        break;
                    }
                    default:
                    {
                        MORIS_ASSERT(false,"Invalid matrix dimension specified, 0-for n_rows, 1- for n_cols");
                        return 0;
                    }
                }
            };

            /**
             * Returns the number of elements in the %matrix.
             *
             * @return Number of elements in the %matrix.
             *
             */
            size_t
            numel() const
            {
                return mMatrix.n_elem;
            }

            /*!
             * Returns memory used by matrix
             *
             * @return Memory in bytes.
             */
            size_t
            capacity() const
            {
                return this->numel() * sizeof(Data_Type);
            }

            void set_row(
                    size_t                          aRowIndex,
                    const Matrix<arma::Mat<Type>> & aVec)
            {
                MORIS_ASSERT(isvector(aVec), "aVec needs to be a vector");
                MORIS_ASSERT(aRowIndex < this->n_rows(), "Specified row index out of bounds");
                MORIS_ASSERT(aVec.numel() == this->n_cols(),
                        "Dimension mismatch (argument matrix and member matrix do not have same number of columns)");

                size_t tROW_INDEX = 0;
                if(!iscol(aVec))
                {
                    mMatrix.row(aRowIndex) = aVec.matrix_data().row(tROW_INDEX);
                }
                else
                {
                    mMatrix.row(aRowIndex) = arma::strans(aVec.matrix_data().col(tROW_INDEX));
                }
            }

            void set_column(
                    size_t                          aColumnIndex,
                    const Matrix<arma::Mat<Type>> & aColumn)
            {

                MORIS_ASSERT(aColumn.n_cols() == 1, "aColumn needs to be a column matrix");
                MORIS_ASSERT(aColumnIndex < this->n_cols(), "Specified column index out of bounds");
                MORIS_ASSERT(aColumn.n_rows() == this->n_rows(),
                        "Dimension mismatch (argument matrix and member matrix do not have same number of rows)");

                size_t tCOLUMN_INDEX = 0;
                mMatrix.col(aColumnIndex) = aColumn.matrix_data().col(tCOLUMN_INDEX);
            }

            void
            get_column(
                    size_t                    aColumnIndex,
                    Matrix<arma::Mat<Type>> & aColumn) const
            {
                MORIS_ASSERT(aColumn.n_cols() == 1,"aColumn needs to be a column matrix");
                MORIS_ASSERT(aColumnIndex < this->n_cols(),"Specified column index out of bounds");
                MORIS_ASSERT(aColumn.n_rows() == this->n_rows(),"Dimension mismatch (argument matrix and member matrix do not have same number of rows)");
                const size_t tCOLUMN_INDEX = 0;
                aColumn.matrix_data().col(tCOLUMN_INDEX) = mMatrix.col(aColumnIndex);
            }


            auto
            get_column(size_t aColumnIndex) const
            ->decltype(mMatrix.col(aColumnIndex))
            {
                MORIS_ASSERT(aColumnIndex < this->n_cols(),"Specified column index out of bounds");
                return mMatrix.col(aColumnIndex);
            }

            auto
            get_column(size_t aColumnIndex)
            ->decltype(mMatrix.col(aColumnIndex))
            {
                MORIS_ASSERT(aColumnIndex < this->n_cols(),"Specified column index out of bounds");
                return mMatrix.col(aColumnIndex);
            }

            void get_row(
                    size_t                    aRowIndex,
                    Matrix<arma::Mat<Type>> & aRow) const
            {
                MORIS_ASSERT(aRow.n_rows() == 1,"aRow needs to be a row matrix");
                MORIS_ASSERT(aRowIndex < this->n_rows(),"Specified row index out of bounds");
                MORIS_ASSERT(aRow.n_cols() == this->n_cols(),"Dimension mismatch (argument matrix and member matrix do not have same number of columns)");

                const size_t tROW_INDEX = 0;
                aRow.mMatrix.row(tROW_INDEX) = mMatrix.row(aRowIndex);
            }

            auto
            get_row(size_t aRowIndex) const
            ->decltype(mMatrix.row(aRowIndex))
            {
                MORIS_ASSERT(aRowIndex < this->n_rows(),"Specified row index out of bounds");
                return mMatrix.row(aRowIndex);
            }

            auto
            get_row(size_t aRowIndex)
            ->decltype(mMatrix.row(aRowIndex))
            {
                MORIS_ASSERT(aRowIndex < this->n_rows(),"Specified row index out of bounds");
                return mMatrix.row(aRowIndex);
            }

            const Type*
            data() const
            {
                return mMatrix.memptr();
            }

            Type*
            data()
            {
                return mMatrix.memptr();
            }

            inline
            arma::Mat<Type> &
            matrix_data()
            {
                return mMatrix;
            }

            inline
            arma::Mat<Type> const &
            matrix_data() const
            {
                return mMatrix;
            }

            Type
            max() const
            {
                MORIS_ASSERT(this->n_rows() != 0 && this->n_cols() !=0,"Max called on empty matrix       ");
                return mMatrix.max();
            }

            Type
            min() const
            {
                MORIS_ASSERT(this->n_rows() != 0 && this->n_cols() !=0,"Min called on empty matrix       ");
                return mMatrix.min();
            }

            Type
            max(
                    uint & aRowIndex,
                    uint & aColIndex) const
            {
                arma::uword rowIndex;
                arma::uword colIndex;

                auto retValue =  this->mMatrix.max( rowIndex, colIndex );

                aRowIndex = rowIndex;
                aColIndex = colIndex;

                return retValue;
            }

            Type
            min(
                    uint & aRowIndex,
                    uint & aColIndex) const
            {
                arma::uword rowIndex;
                arma::uword colIndex;

                auto retValue =  this->mMatrix.min( rowIndex, colIndex );

                aRowIndex = rowIndex;
                aColIndex = colIndex;

                return retValue;
            }

            /**
             * @brief Overloaded moris::Matrix_Base::operator()
             *
             * @param[in] aRowIndex Row index for which data should be accessed.
             * @param[in] aColIndex Column index for which data should be accessed.
             */
            inline
            Type &
            operator()(
                    size_t const & aRowIndex,
                    size_t const & aColIndex )
            {
                MORIS_ASSERT(aRowIndex<this->n_rows(),"Row index out of bounds");
                MORIS_ASSERT(aColIndex<this->n_cols(),"Col index out of bounds");

                return mMatrix(aRowIndex,aColIndex);
            }

            /**
             * @brief Overloaded moris::Matrix_Base::operator()
             *
             * @param[in] aRowIndex Row index for which data should be accessed.
             * @param[in] aColIndex Column index for which data should be accessed.
             */
            const
            Type &
            operator()(
                    const size_t & aRowIndex,
                    const size_t & aColIndex) const
            {
                MORIS_ASSERT(aRowIndex<this->n_rows(),"Row index out of bounds");
                MORIS_ASSERT(aColIndex<this->n_cols(),"Col index out of bounds");
                return mMatrix(aRowIndex,aColIndex);
            }

            /**
             * @brief Overloaded moris::Matrix_Base::operator()
             *
             * @param[in] aIndex Index for which data should be accessed.
             */
            inline
            Type &
            operator()( size_t const & aIndex )
            {
                MORIS_ASSERT(isvector(*this),"Using vector () operator on non-vector");
                MORIS_ASSERT(aIndex<this->numel(),"Vector index out of bounds");
                return mMatrix( aIndex );
            }

            /**
             * @brief Overloaded moris::Matrix_Base::operator()
             *
             * @param[in] aIndex  Index for which data should be accessed.
             */
            const
            Type &
            operator()(const size_t & aIndex ) const
            {
                MORIS_ASSERT(isvector(*this),"Using vector () operator on non-vector");
                MORIS_ASSERT(aIndex<this->numel(),"Vector index out of bounds");
                return mMatrix( aIndex );
            }

            /*
             * Block operations
             */
            auto
            operator()(
                    std::pair< moris::size_t, moris::size_t > const & aI,
                    std::pair< moris::size_t, moris::size_t > const & aJ )
            ->decltype(mMatrix( arma::span( aI.first, aI.second ), arma::span( aJ.first, aJ.second ) ) )
            {
                return mMatrix( arma::span( aI.first, aI.second ), arma::span( aJ.first, aJ.second ) );
            }

            /*
             * Block operations
             */
            auto
            operator()(
                    std::pair< moris::size_t, moris::size_t > const & aI,
                    std::pair< moris::size_t, moris::size_t > const & aJ ) const
            ->decltype(mMatrix( arma::span( aI.first, aI.second ), arma::span( aJ.first, aJ.second ) ) )
            {
                return mMatrix( arma::span( aI.first, aI.second ), arma::span( aJ.first, aJ.second ) );
            }

            /*
             * Block operations for columns of matrices
             */

            auto
            operator()(
                    std::pair< moris::size_t, moris::size_t > const & aI, const moris::size_t  aColIndex = 0 )
            ->decltype(mMatrix( arma::span( aI.first, aI.second ), aColIndex ) )
            {
                return mMatrix( arma::span( aI.first, aI.second ), aColIndex );
            }

            /*
             * Block operations for vector-like matrices
             */

            auto
            operator()(
                    std::pair< moris::size_t, moris::size_t > const & aI, const moris::size_t  aColIndex = 0 ) const
            ->decltype(mMatrix( arma::span( aI.first, aI.second ), aColIndex ) )
            {
                return mMatrix( arma::span( aI.first, aI.second ), aColIndex );
            }

            // -------------------------------------------------------------------------

            /**
             * Returns the length of a vector. Throws error neither rows nor cols are equal 1.
             */

            size_t
            length() const
            {
                // get number of rows from matrix implementation
                size_t n_rows = this->n_rows();

                // get number of cols from matrix implementation
                size_t n_cols = this->n_cols();

                // catch special case of zero length
                if( n_rows == 0 || n_cols == 0 )
                {
                    return 0;
                }
                else
                {
                    // assert that this is really a vector
                    MORIS_ASSERT(  n_rows == 1 || n_cols == 1,
                            "Tried to get length of a matrix. Check dimensions." );

                    // return the smaller of both values
                    return ( n_rows < n_cols ) ? n_cols : n_rows;
                }
            }

            // -------------------------------------------------------------------------

            /**
             * @brief Armadillo implementation of moris::Matrix_Base::operator =
             *
             * @param[in] moris matrix.
             */

            inline
            Matrix<arma::Mat<Type>>&
            operator=( const Matrix<arma::Mat<Type>> & aMatrix )
            {
                mMatrix = aMatrix.matrix_data();
                return *this;
            }

            // -------------------------------------------------------------------------

            /**
             * @brief Armadillo implementation of moris::Matrix_Base::operator =
             *
             * @param[in] Data_Type.
             */

            inline
            Matrix<arma::Mat<Type>>&
            operator=( const Type & aData )
            {
                mMatrix = aData;
                return *this;
            }

            // -------------------------------------------------------------------------

            /**
             * @brief Armadillo implementation of moris::Matrix_Base::operator =
             *
             * @param[in] Expression.
             */

            template <typename E>
            inline
            Matrix<arma::Mat<Type>>&
            operator=( const E& aExpression )
            {
                mMatrix = aExpression;
                return *this;
            }

            // -------------------------------------------------------------------------

            /**
             * @brief Armadillo implementation of moris::Matrix_Base::operator +=
             *
             * @param[in] moris matrix.
             */

            void
            operator+=( const Matrix<arma::Mat<Type>> & aMatrix )
            {
                mMatrix += aMatrix.matrix_data();
            }

            // -------------------------------------------------------------------------

            /**
             * @brief Armadillo implementation of moris::Matrix_Base::operator +=
             *
             * @param[in] Data_Type.
             */

            void
            operator+=( const Data_Type & aData )
            {
                mMatrix += aData;
            }

            // -------------------------------------------------------------------------

            /**
             * @brief Armadillo implementation of moris::Matrix_Base::operator +=
             *
             * @param[in] Expression.
             */

            template <typename E>
            void
            operator+=( const E& aExpression )
            {
                mMatrix += aExpression;
            }

            // -------------------------------------------------------------------------

            /**
             * @brief Armadillo implementation of moris::Matrix_Base::operator -=
             *
             * @param[in] moris matrix.
             */

            void
            operator-=( const Matrix<arma::Mat<Type>> & aMatrix )
            {
                mMatrix -= aMatrix.matrix_data();
            }

            // -------------------------------------------------------------------------

            /**
             * @brief Armadillo implementation of moris::Matrix_Base::operator -=
             *
             * @param[in] Data_Type.
             */

            void
            operator-=( const Data_Type & aData )
            {
                mMatrix -= aData;
            }

            // -------------------------------------------------------------------------

            /**
             * @brief Armadillo implementation of moris::Matrix_Base::operator -=
             *
             * @param[in] Expression.
             */

            template <typename E>
            void
            operator-=( const E& aExpression )
            {
                mMatrix -= aExpression;
            }

            // -------------------------------------------------------------------------

            /*
             * Iterators and Iterator related functions
             */

            /*!
             * Non const iterator returning the first element of the
             * matrix.
             *
             * @param[out] Matrix Iterator of First element in Matrix
             */
            Mat_It
            begin()
            {
                return mMatrix.begin();
            }

            Mat_It
            end()
            {
                return mMatrix.end();
            }
    };
}

#endif /* PROJECTS_LINALG_SRC_ARMA_IMPL_CL_MATRIX_ARMA_DYNAMIC_HPP_ */
