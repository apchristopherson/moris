/*
 * fn_sort_points_by_coordinates
 *
 *  Created on: Jan 17, 2022
 *      Author: Kurt Maute
 */

#ifndef XTK_SRC_XTK_FN_SORT_POINTS_BY_COORDINATES_HPP_
#define XTK_SRC_XTK_FN_SORT_POINTS_BY_COORDINATES_HPP_

#include "cl_Matrix.hpp"
#include "cl_Cell.hpp"

#include <numeric>      // std::iota
#include <algorithm>    // std::sort, std::stable_sort

namespace moris
{
    struct SortPoint
    {
            Matrix< DDRMat > mX;

            SortPoint( Matrix<DDRMat> aX)
            : mX(aX)
            {}

            //------------------------------------------------------------------

            bool operator<(const SortPoint & aPoint) const {

                MORIS_ASSERT( mX.numel() == aPoint.mX.numel(),
                        "SortPoint - inconsistent points dimensions." );

                for ( uint i=0; i<mX.numel(); ++i)
                {
                    if ( std::abs( mX( i ) - aPoint.mX( i ) ) > MORIS_REAL_EPS )
                    {
                        return mX( i ) < aPoint.mX( i );
                    }
                }

                MORIS_ERROR( false,
                        "SortPoint - coordinates of points are identical." );

                return false;
            }
    };

    //------------------------------------------------------------------------------

    moris::Cell<moris_index>
    sort_points_by_coordinates( const moris::Cell< Matrix< DDRMat > > & tPoints)
    {
        // create vector of SortPoints
        std::vector<SortPoint> tSortPointsVector;

        // reserve memory
        tSortPointsVector.reserve ( tPoints.size() );

        // fill vector of SortPoints
        for (uint i=0; i<tPoints.size(); ++i)
        {
            tSortPointsVector.push_back( SortPoint( tPoints( i ) ) );
        }

        // initialize original index locations
        moris::Cell<moris_index> idx(tPoints.size());
        iota(idx.begin(), idx.end(), 0);

        // Sort by points by coordinates
        stable_sort(idx.begin(), idx.end(),
                [&tSortPointsVector](moris_index i1, moris_index i2) {return tSortPointsVector[i1] < tSortPointsVector[i2];});

        // return index vector
        return idx;
    }
}

#endif /* XTK_SRC_XTK_FN_SORT_POINTS_BY_COORDINATES_HPP_ */