/*
 * fn_bubble_sort.hpp
 *
 *  Created on: Jan 10, 2018
 *      Author: doble
 */

#ifndef SRC_TOOLS_FN_BUBBLE_SORT_HPP_
#define SRC_TOOLS_FN_BUBBLE_SORT_HPP_

#include <iostream>

#include "core/xtk_typedefs.hpp"
#include "linalg/cl_XTK_Matrix.hpp"



namespace xtk
{

/*
 * Swaps value xp with value yp
 */
template<typename Type>
void swap(Type & xp,
          Type &yp)
{
    Type temp = xp;
    xp = yp;
    yp = temp;
}

/**
 * Sorts the rows of a matrix in ascending order
 * Note: this algorithm modifies the original matrix
 * based on https://www.geeksforgeeks.org/bubble-sort/
 */
template<typename Type, typename Matrix_Type>
void
row_bubble_sort(moris::Matrix<Type,Matrix_Type> & aMatrix)
{
    xtk::size_t tNumCols = aMatrix.n_cols();
    xtk::size_t tNumRows = aMatrix.n_rows();
    xtk::size_t i;
    xtk::size_t j;
    xtk::size_t k;
    bool tSwapped;
    for (i = 0; i<tNumRows; i++)
    {
        tSwapped = false;
        for (j = 0; j < tNumCols-1; j++)
        {
            // Last i elements are already in place
            for (k = 0; k < tNumCols-j-1; k++)
            {
                if (aMatrix(i,k) > aMatrix(i,k+1))
                {
                    swap(aMatrix(i,k), aMatrix(i,k+1)); // swap these values
                    tSwapped = true;
                }
            }
        }
        if(tSwapped == false)
        {
            continue; // continue to next row
        }
    }
}

/**
 * Sorts the rows of a matrix in ascending order
 * and returns the sorted indices. (aOrder)
 * Note: this algorithm does not modify the orginal matrix
 * based on https://www.geeksforgeeks.org/bubble-sort/
 */
template<typename Type, typename Matrix_Type>
void
row_bubble_sort_indices(moris::Matrix<Type,Matrix_Type> const & aMatrix,
                        moris::Matrix<Type,Matrix_Type> & aOrder)
{
    xtk::size_t tNumCols = aMatrix.n_cols();
    xtk::size_t tNumRows = aMatrix.n_rows();
    xtk::size_t i;
    xtk::size_t j;
    xtk::size_t k;
    bool tSwapped;

    aOrder = aMatrix.create(tNumRows,tNumCols);
    for(i = 0; i<tNumRows; i++)
    {
        k = 0;
        for(j = 0; j<tNumCols; j++)
        {
            aOrder(i,j) = k;
            k++;
        }
    }

    for (i = 0; i<tNumRows; i++)
    {
        tSwapped = false;
        for (j = 0; j < tNumCols-1; j++)
        {
            // Last i elements are already in place
            for (k = 0; k < tNumCols-j-1; k++)
            {
                if (aMatrix(i, aOrder(i,k)) > aMatrix(i,aOrder(i,k+1)))
                {
                    swap(aOrder(i,k), aOrder(i,k+1)); // swap these values
                    tSwapped = true;
                }
            }
        }
        if(tSwapped == false)
        {
            continue; // continue to next row
        }
    }
}
}


#endif /* SRC_TOOLS_FN_BUBBLE_SORT_HPP_ */
