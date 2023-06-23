/*
 * Copyright (c) 2022 University of Colorado
 * Licensed under the MIT license. See LICENSE.txt file in the MORIS root for details.
 *
 *------------------------------------------------------------------------------------
 *
 * cl_HMR_T_Matrix.hpp
 *
 */

#ifndef SRC_HMR_CL_HMR_T_MATRIX_HPP_
#define SRC_HMR_CL_HMR_T_MATRIX_HPP_

#include "cl_HMR_T_Matrix_Base.hpp"
#include "cl_HMR_BSpline_Mesh_Base.hpp" //HMR/src
#include "cl_HMR_Lagrange_Mesh_Base.hpp" //HMR/src
#include "cl_HMR_Parameters.hpp" //HMR/src
#include "typedefs.hpp" //COR/src
#include "cl_Matrix.hpp" //LINALG/src
#include "cl_Cell.hpp" //CNT/src
#include "fn_eye.hpp"

namespace moris::hmr
{

    /**
     * quad4 shape function
     */
    inline void N_quad4(
            const Matrix< DDRMat >& aXi,
            Matrix< DDRMat >&       aN )
    {
        // unpack xi and eta from input vector
        auto xi  = aXi( 0 );
        auto eta = aXi( 1 );

        // populate matrix with values
        aN( 0 ) = ( ( 1.0 - xi ) * ( 1.0 - eta ) ) * 0.25;
        aN( 1 ) = ( ( 1.0 + xi ) * ( 1.0 - eta ) ) * 0.25;
        aN( 2 ) = ( ( 1.0 + xi ) * ( 1.0 + eta ) ) * 0.25;
        aN( 3 ) = ( ( 1.0 - xi ) * ( 1.0 + eta ) ) * 0.25;
    }

    //------------------------------------------------------------------------------

    /**
     * hex8 shape function
     */
    inline void N_hex8(
            const Matrix< DDRMat >& aXi,
            Matrix< DDRMat >&       aN )
    {
        // unpack xi and eta from input vector
        auto xi   = aXi( 0 );
        auto eta  = aXi( 1 );
        auto zeta = aXi( 2 );

        // populate output matrix
        aN( 0 ) = -( eta - 1.0 ) * ( xi - 1.0 ) * ( zeta - 1.0 ) * 0.125;
        aN( 1 ) = ( eta - 1.0 ) * ( xi + 1.0 ) * ( zeta - 1.0 ) * 0.125;
        aN( 2 ) = -( eta + 1.0 ) * ( xi + 1.0 ) * ( zeta - 1.0 ) * 0.125;
        aN( 3 ) = ( eta + 1.0 ) * ( xi - 1.0 ) * ( zeta - 1.0 ) * 0.125;
        aN( 4 ) = ( eta - 1.0 ) * ( xi - 1.0 ) * ( zeta + 1.0 ) * 0.125;
        aN( 5 ) = -( eta - 1.0 ) * ( xi + 1.0 ) * ( zeta + 1.0 ) * 0.125;
        aN( 6 ) = ( eta + 1.0 ) * ( xi + 1.0 ) * ( zeta + 1.0 ) * 0.125;
        aN( 7 ) = -( eta + 1.0 ) * ( xi - 1.0 ) * ( zeta + 1.0 ) * 0.125;
    }

    //------------------------------------------------------------------------------

    /**
     * returns the corner nodes of a child and dimension
     */
    inline void get_child_corner_nodes_2d(
            uint       aChildIndex,
            Matrix< DDRMat >& aXi )
    {
        switch ( aChildIndex )
        {
            case ( 0 ):
            {
                aXi( 0, 0 ) = -1.0;
                aXi( 1, 0 ) = 0.0;
                aXi( 2, 0 ) = 0.0;
                aXi( 3, 0 ) = -1.0;

                aXi( 0, 1 ) = -1.0;
                aXi( 1, 1 ) = -1.0;
                aXi( 2, 1 ) = 0.0;
                aXi( 3, 1 ) = 0.0;

                break;
            }
            case ( 1 ):
            {
                aXi( 0, 0 ) = 0.0;
                aXi( 1, 0 ) = 1.0;
                aXi( 2, 0 ) = 1.0;
                aXi( 3, 0 ) = 0.0;

                aXi( 0, 1 ) = -1.0;
                aXi( 1, 1 ) = -1.0;
                aXi( 2, 1 ) = 0.0;
                aXi( 3, 1 ) = 0.0;

                break;
            }

            case ( 2 ):
            {
                aXi( 0, 0 ) = -1.0;
                aXi( 1, 0 ) = 0.0;
                aXi( 2, 0 ) = 0.0;
                aXi( 3, 0 ) = -1.0;

                aXi( 0, 1 ) = 0.0;
                aXi( 1, 1 ) = 0.0;
                aXi( 2, 1 ) = 1.0;
                aXi( 3, 1 ) = 1.0;

                break;
            }

            case ( 3 ):
            {
                aXi( 0, 0 ) = 0.0;
                aXi( 1, 0 ) = 1.0;
                aXi( 2, 0 ) = 1.0;
                aXi( 3, 0 ) = 0.0;

                aXi( 0, 1 ) = 0.0;
                aXi( 1, 1 ) = 0.0;
                aXi( 2, 1 ) = 1.0;
                aXi( 3, 1 ) = 1.0;

                break;
            }
            default:
            {
                MORIS_ERROR( false, "invalid child index" );
            }
        }
    }

    //------------------------------------------------------------------------------

    /**
     * returns the corner nodes of a child and dimension
     */
    inline void get_child_corner_nodes_3d(
            uint       aChildIndex,
            Matrix< DDRMat >& aXi )
    {
        switch ( aChildIndex )
        {
            case ( 0 ):
            {
                aXi( 0, 0 ) = -1.0;
                aXi( 1, 0 ) = 0.0;
                aXi( 2, 0 ) = 0.0;
                aXi( 3, 0 ) = -1.0;
                aXi( 4, 0 ) = -1.0;
                aXi( 5, 0 ) = 0.0;
                aXi( 6, 0 ) = 0.0;
                aXi( 7, 0 ) = -1.0;

                aXi( 0, 1 ) = -1.0;
                aXi( 1, 1 ) = -1.0;
                aXi( 2, 1 ) = 0.0;
                aXi( 3, 1 ) = 0.0;
                aXi( 4, 1 ) = -1.0;
                aXi( 5, 1 ) = -1.0;
                aXi( 6, 1 ) = 0.0;
                aXi( 7, 1 ) = 0.0;

                aXi( 0, 2 ) = -1.0;
                aXi( 1, 2 ) = -1.0;
                aXi( 2, 2 ) = -1.0;
                aXi( 3, 2 ) = -1.0;
                aXi( 4, 2 ) = 0.0;
                aXi( 5, 2 ) = 0.0;
                aXi( 6, 2 ) = 0.0;
                aXi( 7, 2 ) = 0.0;

                break;
            }
            case ( 1 ):
            {
                aXi( 0, 0 ) = 0.0;
                aXi( 1, 0 ) = 1.0;
                aXi( 2, 0 ) = 1.0;
                aXi( 3, 0 ) = 0.0;
                aXi( 4, 0 ) = 0.0;
                aXi( 5, 0 ) = 1.0;
                aXi( 6, 0 ) = 1.0;
                aXi( 7, 0 ) = 0.0;

                aXi( 0, 1 ) = -1.0;
                aXi( 1, 1 ) = -1.0;
                aXi( 2, 1 ) = 0.0;
                aXi( 3, 1 ) = 0.0;
                aXi( 4, 1 ) = -1.0;
                aXi( 5, 1 ) = -1.0;
                aXi( 6, 1 ) = 0.0;
                aXi( 7, 1 ) = 0.0;

                aXi( 0, 2 ) = -1.0;
                aXi( 1, 2 ) = -1.0;
                aXi( 2, 2 ) = -1.0;
                aXi( 3, 2 ) = -1.0;
                aXi( 4, 2 ) = 0.0;
                aXi( 5, 2 ) = 0.0;
                aXi( 6, 2 ) = 0.0;
                aXi( 7, 2 ) = 0.0;

                break;
            }
            case ( 2 ):
            {
                aXi( 0, 0 ) = -1.0;
                aXi( 1, 0 ) = 0.0;
                aXi( 2, 0 ) = 0.0;
                aXi( 3, 0 ) = -1.0;
                aXi( 4, 0 ) = -1.0;
                aXi( 5, 0 ) = 0.0;
                aXi( 6, 0 ) = 0.0;
                aXi( 7, 0 ) = -1.0;

                aXi( 0, 1 ) = 0.0;
                aXi( 1, 1 ) = 0.0;
                aXi( 2, 1 ) = 1.0;
                aXi( 3, 1 ) = 1.0;
                aXi( 4, 1 ) = 0.0;
                aXi( 5, 1 ) = 0.0;
                aXi( 6, 1 ) = 1.0;
                aXi( 7, 1 ) = 1.0;

                aXi( 0, 2 ) = -1.0;
                aXi( 1, 2 ) = -1.0;
                aXi( 2, 2 ) = -1.0;
                aXi( 3, 2 ) = -1.0;
                aXi( 4, 2 ) = 0.0;
                aXi( 5, 2 ) = 0.0;
                aXi( 6, 2 ) = 0.0;
                aXi( 7, 2 ) = 0.0;

                break;
            }
            case ( 3 ):
            {
                aXi( 0, 0 ) = 0.0;
                aXi( 1, 0 ) = 1.0;
                aXi( 2, 0 ) = 1.0;
                aXi( 3, 0 ) = 0.0;
                aXi( 4, 0 ) = 0.0;
                aXi( 5, 0 ) = 1.0;
                aXi( 6, 0 ) = 1.0;
                aXi( 7, 0 ) = 0.0;

                aXi( 0, 1 ) = 0.0;
                aXi( 1, 1 ) = 0.0;
                aXi( 2, 1 ) = 1.0;
                aXi( 3, 1 ) = 1.0;
                aXi( 4, 1 ) = 0.0;
                aXi( 5, 1 ) = 0.0;
                aXi( 6, 1 ) = 1.0;
                aXi( 7, 1 ) = 1.0;

                aXi( 0, 2 ) = -1.0;
                aXi( 1, 2 ) = -1.0;
                aXi( 2, 2 ) = -1.0;
                aXi( 3, 2 ) = -1.0;
                aXi( 4, 2 ) = 0.0;
                aXi( 5, 2 ) = 0.0;
                aXi( 6, 2 ) = 0.0;
                aXi( 7, 2 ) = 0.0;

                break;
            }
            case ( 4 ):
            {
                aXi( 0, 0 ) = -1.0;
                aXi( 1, 0 ) = 0.0;
                aXi( 2, 0 ) = 0.0;
                aXi( 3, 0 ) = -1.0;
                aXi( 4, 0 ) = -1.0;
                aXi( 5, 0 ) = 0.0;
                aXi( 6, 0 ) = 0.0;
                aXi( 7, 0 ) = -1.0;

                aXi( 0, 1 ) = -1.0;
                aXi( 1, 1 ) = -1.0;
                aXi( 2, 1 ) = 0.0;
                aXi( 3, 1 ) = 0.0;
                aXi( 4, 1 ) = -1.0;
                aXi( 5, 1 ) = -1.0;
                aXi( 6, 1 ) = 0.0;
                aXi( 7, 1 ) = 0.0;

                aXi( 0, 2 ) = 0.0;
                aXi( 1, 2 ) = 0.0;
                aXi( 2, 2 ) = 0.0;
                aXi( 3, 2 ) = 0.0;
                aXi( 4, 2 ) = 1.0;
                aXi( 5, 2 ) = 1.0;
                aXi( 6, 2 ) = 1.0;
                aXi( 7, 2 ) = 1.0;

                break;
            }
            case ( 5 ):
            {
                aXi( 0, 0 ) = 0.0;
                aXi( 1, 0 ) = 1.0;
                aXi( 2, 0 ) = 1.0;
                aXi( 3, 0 ) = 0.0;
                aXi( 4, 0 ) = 0.0;
                aXi( 5, 0 ) = 1.0;
                aXi( 6, 0 ) = 1.0;
                aXi( 7, 0 ) = 0.0;

                aXi( 0, 1 ) = -1.0;
                aXi( 1, 1 ) = -1.0;
                aXi( 2, 1 ) = 0.0;
                aXi( 3, 1 ) = 0.0;
                aXi( 4, 1 ) = -1.0;
                aXi( 5, 1 ) = -1.0;
                aXi( 6, 1 ) = 0.0;
                aXi( 7, 1 ) = 0.0;

                aXi( 0, 2 ) = 0.0;
                aXi( 1, 2 ) = 0.0;
                aXi( 2, 2 ) = 0.0;
                aXi( 3, 2 ) = 0.0;
                aXi( 4, 2 ) = 1.0;
                aXi( 5, 2 ) = 1.0;
                aXi( 6, 2 ) = 1.0;
                aXi( 7, 2 ) = 1.0;

                break;
            }
            case ( 6 ):
            {
                aXi( 0, 0 ) = -1.0;
                aXi( 1, 0 ) = 0.0;
                aXi( 2, 0 ) = 0.0;
                aXi( 3, 0 ) = -1.0;
                aXi( 4, 0 ) = -1.0;
                aXi( 5, 0 ) = 0.0;
                aXi( 6, 0 ) = 0.0;
                aXi( 7, 0 ) = -1.0;

                aXi( 0, 1 ) = 0.0;
                aXi( 1, 1 ) = 0.0;
                aXi( 2, 1 ) = 1.0;
                aXi( 3, 1 ) = 1.0;
                aXi( 4, 1 ) = 0.0;
                aXi( 5, 1 ) = 0.0;
                aXi( 6, 1 ) = 1.0;
                aXi( 7, 1 ) = 1.0;

                aXi( 0, 2 ) = 0.0;
                aXi( 1, 2 ) = 0.0;
                aXi( 2, 2 ) = 0.0;
                aXi( 3, 2 ) = 0.0;
                aXi( 4, 2 ) = 1.0;
                aXi( 5, 2 ) = 1.0;
                aXi( 6, 2 ) = 1.0;
                aXi( 7, 2 ) = 1.0;

                break;
            }
            case ( 7 ):
            {
                aXi( 0, 0 ) = 0.0;
                aXi( 1, 0 ) = 1.0;
                aXi( 2, 0 ) = 1.0;
                aXi( 3, 0 ) = 0.0;
                aXi( 4, 0 ) = 0.0;
                aXi( 5, 0 ) = 1.0;
                aXi( 6, 0 ) = 1.0;
                aXi( 7, 0 ) = 0.0;

                aXi( 0, 1 ) = 0.0;
                aXi( 1, 1 ) = 0.0;
                aXi( 2, 1 ) = 1.0;
                aXi( 3, 1 ) = 1.0;
                aXi( 4, 1 ) = 0.0;
                aXi( 5, 1 ) = 0.0;
                aXi( 6, 1 ) = 1.0;
                aXi( 7, 1 ) = 1.0;

                aXi( 0, 2 ) = 0.0;
                aXi( 1, 2 ) = 0.0;
                aXi( 2, 2 ) = 0.0;
                aXi( 3, 2 ) = 0.0;
                aXi( 4, 2 ) = 1.0;
                aXi( 5, 2 ) = 1.0;
                aXi( 6, 2 ) = 1.0;
                aXi( 7, 2 ) = 1.0;

                break;
            }
            default:
            {
                MORIS_ERROR( false, "invalid child index" );
            }
        }
    }


    /**
     * T-matrix generator class
     *
     * @tparam N Number of dimensions
     */
    template< uint N >
    class T_Matrix : public T_Matrix_Base
    {
    private:

        //! pointer to shape function
        void ( T_Matrix< N >:: *mEvalN )( const Matrix< DDRMat > & aXi, Matrix< DDRMat > & aN ) const;

    public:

        T_Matrix(
                const Parameters*   aParameters,
                BSpline_Mesh_Base*  aBSplineMesh,
                Lagrange_Mesh_Base* aLagrangeMesh )
                : T_Matrix_Base( aParameters, aBSplineMesh, aLagrangeMesh )
        {
            // Initializations
            this->init_basis_index();
            this->init_unity_matrix();
            this->init_child_matrices();
            this->init_truncation_weights();
            this->init_lagrange_parameter_coordinates();
            this->init_lagrange_matrix();

            // TODO
            switch ( N )
            {
                case 2:
                {
                    mEvalNGeo   = &N_quad4;
                    mEvalN      = &T_Matrix<N>::lagrange_shape_2d;
                    mGetCorners = &get_child_corner_nodes_2d;

                    break;
                }
                case 3:
                {
                    mEvalNGeo   = &N_hex8;
                    mEvalN      = &T_Matrix<N>::lagrange_shape_3d;
                    mGetCorners = &get_child_corner_nodes_3d;
                    break;
                }
                default:
                {
                    MORIS_ERROR( false, "unknown number of dimensions" );
                    break;
                }
            }

            this->init_lagrange_refinement_matrices();
            this->init_lagrange_change_order_matrices();

            // set function pointer
            if ( aParameters->truncate_bsplines() )
            {
                mTMatrixFunction = &T_Matrix_Base::calculate_truncated_t_matrix;
            }
            else
            {
                mTMatrixFunction = &T_Matrix_Base::calculate_untruncated_t_matrix;
            }
        }

        //-------------------------------------------------------------------------------

        T_Matrix(
                const Parameters*   aParameters,
                Lagrange_Mesh_Base* aLagrangeMesh )
                : T_Matrix_Base( aParameters, aLagrangeMesh )
        {
            this->init_lagrange_parameter_coordinates();

            switch ( N )
            {
                case 2:
                {
                    mEvalNGeo   = &N_quad4;
                    mEvalN      = &T_Matrix<N>::lagrange_shape_2d;
                    mGetCorners = &get_child_corner_nodes_2d;

                    break;
                }
                case 3:
                {
                    mEvalNGeo   = &N_hex8;
                    mEvalN      = &T_Matrix<N>::lagrange_shape_3d;
                    mGetCorners = &get_child_corner_nodes_3d;
                    break;
                }
                default:
                {
                    MORIS_ERROR( false, "unknown number of dimensions" );
                    break;
                }
            }

            this->init_lagrange_refinement_matrices();
            this->init_lagrange_change_order_matrices();
        }

//-------------------------------------------------------------------------------

        // destructor
        ~T_Matrix()
        {
        }

    private:

        /**
         * determines the sorting order of the nodes
         */
        void init_basis_index()
        {
            // Create background element
            Background_Element_Base* tBackElement = this->create_background_element();

            // create a prototype of a B-Spline Element
            Element* tElement = mBSplineMesh->create_element( tBackElement );

            // B-Spline order
            mBSplineOrder = mBSplineMesh->get_order();

            // number of bspline coefficients per direction
            uint tNodesPerDirection = mBSplineOrder + 1;

            // calculate number of basis per element
            uint tNumberOfBasis = static_cast<uint>( std::pow( tNodesPerDirection, N ) );

            // initialize index matrix
            mBasisIndex.set_size( tNumberOfBasis, 1 );
            mBSplineIJK.set_size( N, tNumberOfBasis );

            // loop over all basis
            if ( N == 2 )
            {
                // container for ijk position of basis
                luint tIJ[ 2 ];
                for ( uint iBasisIndex = 0; iBasisIndex < tNumberOfBasis; iBasisIndex++ )
                {
                    // get position from element
                    tElement->get_ijk_of_basis( iBasisIndex, tIJ );

                    // calculate index in matrix
                    uint tIndex = tIJ[ 0 ] + tIJ[ 1 ] * tNodesPerDirection;

                    mBasisIndex( tIndex ) = iBasisIndex;
                    mBSplineIJK( 0, iBasisIndex )   = tIJ[ 0 ];
                    mBSplineIJK( 1, iBasisIndex )   = tIJ[ 1 ];
                }
            }
            else if ( N == 3 )
            {
                // container for ijk position of basis
                luint tIJK[ 3 ];
                for ( uint iBasisIndex = 0; iBasisIndex < tNumberOfBasis; iBasisIndex++ )
                {
                    // get position from element
                    tElement->get_ijk_of_basis( iBasisIndex, tIJK );

                    // calculate index in matrix
                    uint tIndex =
                            tIJK[ 0 ] + tNodesPerDirection * ( tIJK[ 1 ] + tIJK[ 2 ] * tNodesPerDirection );

                    mBasisIndex( tIndex ) = iBasisIndex;

                    mBSplineIJK( 0, iBasisIndex ) = tIJK[ 0 ];
                    mBSplineIJK( 1, iBasisIndex ) = tIJK[ 1 ];
                    mBSplineIJK( 2, iBasisIndex ) = tIJK[ 2 ];
                }
            }

            // tidy up
            delete tElement;
            delete tBackElement;
        }

//-------------------------------------------------------------------------------

        /**
         * initializes the container for the unity matrix
         */
        void init_unity_matrix()
        {
            // get number of basis per element
            uint tNumberOfBasis = static_cast<uint>(
                    std::pow( mBSplineMesh->get_order() + 1, N ) );

            // Set unity and zero matrices
            eye( tNumberOfBasis, tNumberOfBasis, mEye );
            mZero.set_size( tNumberOfBasis, 1, 0.0 );
        }

//-------------------------------------------------------------------------------
        /**
         * pre-calculates the child relation matrices
         */
        void init_child_matrices()
        {
            // get order of mesh
            uint tOrder = mBSplineMesh->get_order();

            // create temporary matrix
            Matrix< DDRMat > tFactors( tOrder + 1, tOrder + 2, 0.0 );

            // number of bspline coefficients per direction
            uint tNumCoefficients = tOrder + 1;

            // weight factor
            real tWeight = 1.0 / std::pow( 2, tOrder );

            for ( uint iCoefficient = 0; iCoefficient <= tNumCoefficients; iCoefficient++ )
            {
                for ( uint iOrder = 0; iOrder <= tOrder; iOrder++ )
                {
                    uint k = tOrder - 2 * iOrder + iCoefficient;
                    if ( k <= tNumCoefficients )
                    {
                        tFactors( iOrder, iCoefficient ) = tWeight * nchoosek( tNumCoefficients, k );
                    }
                }
            }

            // left and right matrices
            Matrix< DDRMat > TL( tNumCoefficients, tNumCoefficients, 0.0 );
            Matrix< DDRMat > TR( tNumCoefficients, tNumCoefficients, 0.0 );

            // Fill matrices
            for ( uint iOrder = 0; iOrder <= tOrder; iOrder++ )
            {
                TL.set_column( iOrder, tFactors.get_column( iOrder ) );
                TR.set_column( iOrder, tFactors.get_column( iOrder + 1 ) );
            }

            // determine number of children
            uint tNumberOfChildren = static_cast<uint>( std::pow( 2, N ) );

            // determine number of basis per element
            uint tNumberOfBasis = static_cast<uint>( std::pow( tOrder + 1, N ) );

            // empty matrix
            Matrix< DDRMat > tEmpty( tNumberOfBasis, tNumberOfBasis, 0.0 );

            // container for child relation matrices ( transposed! )
            mChild.resize( tNumberOfChildren, tEmpty );

            // populate child matrices. Tensor product
            if ( N == 2 )
            {
                uint tChildCol = 0;
                for ( uint iTRow2 = 0; iTRow2 < tNumCoefficients; iTRow2++ )
                {
                    for ( uint iTRow1 = 0; iTRow1 < tNumCoefficients; iTRow1++ )
                    {
                        uint tChildRow = 0;
                        for ( uint iTCol2 = 0; iTCol2 < tNumCoefficients; iTCol2++ )
                        {
                            for ( uint iTCol1 = 0; iTCol1 < tNumCoefficients; iTCol1++ )
                            {
                                mChild( 0 )( mBasisIndex( tChildRow ), mBasisIndex( tChildCol ) ) = TL( iTRow1, iTCol1 ) * TL( iTRow2, iTCol2 );
                                mChild( 1 )( mBasisIndex( tChildRow ), mBasisIndex( tChildCol ) ) = TR( iTRow1, iTCol1 ) * TL( iTRow2, iTCol2 );
                                mChild( 2 )( mBasisIndex( tChildRow ), mBasisIndex( tChildCol ) ) = TL( iTRow1, iTCol1 ) * TR( iTRow2, iTCol2 );
                                mChild( 3 )( mBasisIndex( tChildRow ), mBasisIndex( tChildCol ) ) = TR( iTRow1, iTCol1 ) * TR( iTRow2, iTCol2 );
                                tChildRow++;
                            }
                        }
                        tChildCol++;
                    }
                }
            }
            else if ( N == 3 )
            {
                uint tChildCol = 0;
                for ( uint iTRow3 = 0; iTRow3 < tNumCoefficients; iTRow3++ )
                {
                    for ( uint iTRow2 = 0; iTRow2 < tNumCoefficients; iTRow2++ )
                    {
                        for ( uint iTRow1 = 0; iTRow1 < tNumCoefficients; iTRow1++ )
                        {
                            uint tChildRow = 0;
                            for ( uint iTCol3 = 0; iTCol3 < tNumCoefficients; iTCol3++ )
                            {
                                for ( uint iTCol2 = 0; iTCol2 < tNumCoefficients; iTCol2++ )
                                {
                                    for ( uint iTCol1 = 0; iTCol1 < tNumCoefficients; iTCol1++ )
                                    {
                                        mChild( 0 )( mBasisIndex( tChildRow ), mBasisIndex( tChildCol ) ) = TL( iTRow1, iTCol1 ) * TL( iTRow2, iTCol2 ) * TL( iTRow3, iTCol3 );
                                        mChild( 1 )( mBasisIndex( tChildRow ), mBasisIndex( tChildCol ) ) = TR( iTRow1, iTCol1 ) * TL( iTRow2, iTCol2 ) * TL( iTRow3, iTCol3 );
                                        mChild( 2 )( mBasisIndex( tChildRow ), mBasisIndex( tChildCol ) ) = TL( iTRow1, iTCol1 ) * TR( iTRow2, iTCol2 ) * TL( iTRow3, iTCol3 );
                                        mChild( 3 )( mBasisIndex( tChildRow ), mBasisIndex( tChildCol ) ) = TR( iTRow1, iTCol1 ) * TR( iTRow2, iTCol2 ) * TL( iTRow3, iTCol3 );
                                        mChild( 4 )( mBasisIndex( tChildRow ), mBasisIndex( tChildCol ) ) = TL( iTRow1, iTCol1 ) * TL( iTRow2, iTCol2 ) * TR( iTRow3, iTCol3 );
                                        mChild( 5 )( mBasisIndex( tChildRow ), mBasisIndex( tChildCol ) ) = TR( iTRow1, iTCol1 ) * TL( iTRow2, iTCol2 ) * TR( iTRow3, iTCol3 );
                                        mChild( 6 )( mBasisIndex( tChildRow ), mBasisIndex( tChildCol ) ) = TL( iTRow1, iTCol1 ) * TR( iTRow2, iTCol2 ) * TR( iTRow3, iTCol3 );
                                        mChild( 7 )( mBasisIndex( tChildRow ), mBasisIndex( tChildCol ) ) = TR( iTRow1, iTCol1 ) * TR( iTRow2, iTCol2 ) * TR( iTRow3, iTCol3 );
                                        tChildRow++;
                                    }
                                }
                            }
                            tChildCol++;
                        }
                    }
                }
            }

            this->child_multiplication();
        }

//-------------------------------------------------------------------------------

        void child_multiplication()
        {
            // determine number of children
            uint tNumberOfChildren = static_cast<uint>( std::pow( 2, N ) );
            mChildMultiplied.resize( tNumberOfChildren );

            for ( uint Ik = 0; Ik < tNumberOfChildren; ++Ik )
            {
                mChildMultiplied( Ik ).resize( tNumberOfChildren );
            }

            // multiply child matrices
            for ( uint iChildRow = 0; iChildRow < tNumberOfChildren; iChildRow++ )
            {
                for ( uint iChildCol = 0; iChildCol < tNumberOfChildren; iChildCol++ )
                {
                    mChildMultiplied( iChildRow )( iChildCol ) = mChild( iChildRow ) * mChild( iChildCol );
                }
            }
        }

//------------------------------------------------------------------------------

        void init_truncation_weights()
        {
            // get order of mesh
            uint tOrder = mBSplineMesh->get_order();

            // number of children per direction
            uint tNumberOfChildren = tOrder + 2;

            // matrix containing 1D weights
            Matrix< DDRMat > tWeights( tNumberOfChildren, 1 );

            // scale factor for 1D weights
            real tScale = 1.0 / ( (real)std::pow( 2, tOrder ) );

            // calculate 1D weights
            for ( uint iChildIndex = 0; iChildIndex < tNumberOfChildren; iChildIndex++ )
            {
                tWeights( iChildIndex ) = tScale * nchoosek( tOrder + 1, iChildIndex );
            }

            // allocate weights
            mTruncationWeights.set_size( static_cast<uint>( std::pow( tNumberOfChildren, N ) ), 1 );

            // init counter
            uint tTruncationWeightIndex = 0;

            if ( N == 2 )
            {
                // loop over all positions
                for ( real iWeightI : tWeights )
                {
                    for ( real iWeightJ : tWeights )
                    {
                        mTruncationWeights( tTruncationWeightIndex++ ) = iWeightI * iWeightJ;
                    }
                }
            }
            else if ( N == 3 )
            {
                // loop over all positions
                for ( real iWeightI : tWeights )
                {
                    for ( real iWeightJ : tWeights )
                    {
                        for ( real iWeightK : tWeights )
                        {
                            mTruncationWeights( tTruncationWeightIndex++ ) = iWeightI * iWeightJ * iWeightK;
                        }
                    }
                }
            }
        }

//------------------------------------------------------------------------------

        void init_lagrange_parameter_coordinates()
        {
            // create background element for reference
            Background_Element_Base* tBackElement = this->create_background_element();

            // number of nodes per direction
            uint tNodesPerDirection = mLagrangeMesh->get_order() + 1;

            // number of nodes
            mNumberOfNodes = static_cast<uint>( std::pow( tNodesPerDirection, N ) );

            // create a Lagrange element
            Element* tElement = mLagrangeMesh->create_element( tBackElement );

            // assign memory for parameter coordinates
            mLagrangeParam.set_size( N, mNumberOfNodes );

            // scaling factor
            real tScale = 1.0 / ( (real)mLagrangeMesh->get_order() );

            // container for ijk position of basis
            luint tIJK[ 3 ];

            // ijk positions for reference Lagrange element
            mLagrangeIJK.set_size( N, mNumberOfNodes );

            for ( uint iNodeIndex = 0; iNodeIndex < mNumberOfNodes; iNodeIndex++ )
            {
                // get position from element
                tElement->get_ijk_of_basis( iNodeIndex, tIJK );

                // save coordinate into memory
                for ( uint iDimensionIndex = 0; iDimensionIndex < N; iDimensionIndex++ )
                {
                    // fill in node ijk positions in element
                    mLagrangeParam( iDimensionIndex, iNodeIndex ) = 2 * tScale * tIJK[ iDimensionIndex ] - 1.0;

                    // fill in nodal natural coordinates for this element
                    mLagrangeIJK( iDimensionIndex, iNodeIndex ) = tIJK[ iDimensionIndex ];
                }
            }

            // tidy up
            delete tElement;
            delete tBackElement;
        }

//------------------------------------------------------------------------------

        /**
         * calculates the matrix that converts B-Spline DOFs per element to Lagrange DOFs.
         */
        void init_lagrange_matrix()
        {
            // get number of basis per element of B-Spline mesh
            uint tNumberOfBasis = mBSplineIJK.n_cols();

            // get number of Lagrange nodes
            uint tNumberOfNodes = mLagrangeParam.n_cols();

            // initialize T-Matrix for B-Spline to Lagrange conversion
            mTMatrixLagrange.set_size( tNumberOfNodes, tNumberOfBasis, 1 );

            // get order of B-Spline mesh
            uint tOrder = mBSplineMesh->get_order();

            // loop over all Lagrange nodes
            for ( uint iNodeIndex = 0; iNodeIndex < tNumberOfNodes; iNodeIndex++ )
            {
                // loop over all B-Spline Basis
                for ( uint iBasisIndex = 0; iBasisIndex < tNumberOfBasis; iBasisIndex++ )
                {
                    // loop over all dimensions
                    for ( uint iDimensionIndex = 0; iDimensionIndex < N; iDimensionIndex++ )
                    {
                        mTMatrixLagrange( iNodeIndex, iBasisIndex ) *= T_Matrix::b_spline_shape_1d( tOrder,
                                                                                                    mBSplineIJK( iDimensionIndex, iBasisIndex ),
                                                                                                    mLagrangeParam( iDimensionIndex, iNodeIndex ) );
                    }
                }
            }
        }

//------------------------------------------------------------------------------

        void init_lagrange_refinement_matrices()
        {
            // tidy up memory
            mLagrangeRefinementMatrix.clear();

            // get number of nodes
            uint tNumberOfNodes = mLagrangeParam.n_cols();

            // number of children
            uint tNumberOfChildren = static_cast<uint>( std::pow( 2, N ) );

            // initialize container
            Matrix< DDRMat > tEmpty( tNumberOfNodes, tNumberOfNodes, 0.0 );
            mLagrangeRefinementMatrix.resize( tNumberOfChildren, tEmpty );

            // matrix containing corner nodes
            Matrix< DDRMat > tCorners( tNumberOfChildren, N );

            // shape function for "geometry"
            Matrix< DDRMat > tNGeo( 1, tNumberOfChildren );

            // shape function
            Matrix< DDRMat > tN( 1, tNumberOfNodes );

            // step 1: get parameter coordinates of child

            // matrix with parameter coordinates
            // Matrix< DDRMat > tXi( tNumberOfNodes, N );

            // loop over all children
            for ( uint iChildIndex = 0; iChildIndex < tNumberOfChildren; iChildIndex++ )
            {
                // get matrix with  corner nodes
                mGetCorners( iChildIndex, tCorners );

                for ( uint iNodeIndex = 0; iNodeIndex < tNumberOfNodes; iNodeIndex++ )
                {
                    // evaluate shape function for "geometry"
                    mEvalNGeo( mLagrangeParam.get_column( iNodeIndex ), tNGeo );

                    // get parameter coordinates
                    Matrix< DDRMat > tXi = tNGeo * tCorners;

                    // evaluate shape function
                    ( this->*mEvalN )( tXi,
                                       tN );

                    // copy result into matrix
                    mLagrangeRefinementMatrix( iChildIndex ).set_row( iNodeIndex, tN.get_row( 0 ) );
                }
            }
        }

//-------------------------------------------------------------------------------

        void init_lagrange_change_order_matrices()
        {
            // empty matrix
            Matrix< DDRMat > tEmpty;

            // tidy up memory
            mLagrangeChangeOrderMatrix.clear();
            mLagrangeChangeOrderMatrix.push_back( tEmpty );

            // get number of nodes
            uint tNumberOfNodesThisMesh = mLagrangeParam.n_cols();

            for ( uint tOrder = 1; tOrder <= 3; ++tOrder )
            {
                // grab the parameter coordinates
                Matrix< DDRMat > tXi = get_supporting_points( N, tOrder );

                // get the number of nodes of the other mesh
                uint tNumberOfNodesOtherMesh = tXi.n_cols();

                // pointer to T-Matrix
                Matrix< DDRMat > tT(tNumberOfNodesOtherMesh, tNumberOfNodesThisMesh );

                // the shape function
                Matrix< DDRMat > tN(1, tNumberOfNodesThisMesh );

                // Point coordinate matrix
                Matrix< DDRMat > tPoint( N, 1 );

                // populate matrix
                for ( uint iNodeOtherMesh = 0; iNodeOtherMesh < tNumberOfNodesOtherMesh; iNodeOtherMesh++ )
                {
                    // copy coordinates into point TODO can we be more efficient?
                    for ( uint iDimension = 0; iDimension < N; iDimension++ )
                    {
                        tPoint( iDimension ) = tXi(iDimension, iNodeOtherMesh );
                    }

                    // evaluate shape function
                    ( this->*mEvalN )( tPoint,
                                       tN );

                    for ( uint iNodeThisMesh = 0; iNodeThisMesh < tNumberOfNodesThisMesh; iNodeThisMesh++ )
                    {
                        tT(iNodeOtherMesh, iNodeThisMesh ) = tN(iNodeThisMesh );
                    }
                }
                mLagrangeChangeOrderMatrix.push_back( tT );
            }
        }

//-------------------------------------------------------------------------------

        Background_Element_Base* create_background_element()
        {
            Background_Element_Base* aBackElement = nullptr;

            // create a prototype for a background element
            switch ( N )
            {
                case ( 2 ):
                {
                    luint tIJ[ 2 ] = { 0, 0 };
                    aBackElement   = new Background_Element< 2, 4, 8, 4, 0 >( nullptr,
                                                                              0,
                                                                              tIJ,
                                                                              0,
                                                                              0,
                                                                              0,
                                                                              gNoProcOwner );
                    break;
                }
                case ( 3 ):
                {
                    luint tIJK[ 3 ] = { 0, 0, 0 };
                    aBackElement    = new Background_Element< 3, 8, 26, 6, 12 >( nullptr,
                                                                                 0,
                                                                                 tIJK,
                                                                                 0,
                                                                                 0,
                                                                                 0,
                                                                                 gNoProcOwner );
                    break;
                }
                default:
                {
                    MORIS_ERROR( false, "unknown number of dimensions." );
                }
            }

            return aBackElement;
        }

//------------------------------------------------------------------------------

        /**
         * 1D shape function
         */
        static real b_spline_shape_1d(
                uint aOrder,
                uint aK,
                real aXi )
        {
            // max number of entries in lookup table
            uint tSteps = 2 * ( aOrder + 1 );

            // temporary matrix that contains B-Spline segments
            Matrix< DDRMat > tDeltaXi( tSteps, 1, 0 );
            for ( uint i = 0; i < tSteps; ++i )
            {
                tDeltaXi( i ) = ( ( (real)i ) - ( (real)aOrder ) ) * 2.0 - 1.0;
            }

            // temporary matrix that contains evaluated values
            Matrix< DDRMat > tN( aOrder + 1, 1, 0 );

            // initialize zero order values
            for ( uint i = 0; i <= aOrder; ++i )
            {
                if ( tDeltaXi( i + aK ) <= aXi && aXi < tDeltaXi( i + aK + 1 ) )
                {
                    tN( i ) = 1.0;
                }
            }

            // loop over all orders
            for ( uint r = 1; r <= aOrder; ++r )
            {
                // copy values of tN into old matrix
                Matrix< DDRMat > tNold( tN );

                // loop over all contributions
                for ( uint i = 0; i <= aOrder - r; ++i )
                {
                    // help values
                    real tA = aXi - tDeltaXi( i + aK );
                    real tB = tDeltaXi( i + aK + r + 1 ) - aXi;

                    tN( i ) = 0.5 * ( tA * tNold( i ) + tB * ( tNold( i + 1 ) ) ) / ( (real)r );
                }
            }

            // first value in entry is shape value
            return tN( 0 );
        }

//------------------------------------------------------------------------------

        /**
         * 1D shape function
         */
        real lagrange_shape_1d(
                uint aBasisNumber,
                real aXi ) const
        {
            // use horner scheme to evaluate 1D Lagrange function
            real aResult = 0.0;
            for ( uint i = 0; i < mLagrangeOrder; i++ )
            {
                aResult = ( aResult + mLagrangeCoefficients( i, aBasisNumber ) ) * aXi;
            }

            return aResult + mLagrangeCoefficients( mLagrangeOrder, aBasisNumber );
        }

        /**
        * 2D shape function
        */
        void lagrange_shape_2d(
                const Matrix< DDRMat >& aXi,
                Matrix< DDRMat >&       aN ) const
        {
            // evaluate contributions for xi and eta
            Matrix< DDRMat > tNxi( mLagrangeOrder + 1, 1 );
            Matrix< DDRMat > tNeta( mLagrangeOrder + 1, 1 );

            for ( uint i = 0; i <= mLagrangeOrder; i++ )
            {
                tNxi( i ) = this->lagrange_shape_1d( i, aXi( 0 ) );
            }
            for ( uint j = 0; j <= mLagrangeOrder; j++ )
            {
                tNeta( j ) = this->lagrange_shape_1d( j, aXi( 1 ) );
            }

            // create shape vector in correct order
            for ( uint iNodeIndex = 0; iNodeIndex < mNumberOfNodes; iNodeIndex++ )
            {
                aN( iNodeIndex ) = tNxi( mLagrangeIJK( 0, iNodeIndex ) ) * tNeta( mLagrangeIJK( 1, iNodeIndex ) );
            }
        }

        //------------------------------------------------------------------------------

        /**
         * 3D shape function
         */
        void lagrange_shape_3d(
                const Matrix< DDRMat >& aXi,
                Matrix< DDRMat >&       aN ) const
        {
            // evaluate contributions for xi and eta and zeta
            Matrix< DDRMat > tNxi( mLagrangeOrder + 1, 1 );
            Matrix< DDRMat > tNeta( mLagrangeOrder + 1, 1 );
            Matrix< DDRMat > tNzeta( mLagrangeOrder + 1, 1 );

            for ( uint i = 0; i <= mLagrangeOrder; i++ )
            {
                tNxi( i ) = this->lagrange_shape_1d( i, aXi( 0 ) );
            }

            for ( uint j = 0; j <= mLagrangeOrder; j++ )
            {
                tNeta( j ) = this->lagrange_shape_1d( j, aXi( 1 ) );
            }

            for ( uint k = 0; k <= mLagrangeOrder; k++ )
            {
                tNzeta( k ) = this->lagrange_shape_1d( k, aXi( 2 ) );
            }

            // create shape vector in correct order
            for ( uint iNodeIndex = 0; iNodeIndex < mNumberOfNodes; iNodeIndex++ )
            {
                aN( iNodeIndex ) =
                        tNxi( mLagrangeIJK( 0, iNodeIndex ) )
                                * tNeta( mLagrangeIJK( 1, iNodeIndex ) )
                                * tNzeta( mLagrangeIJK( 2, iNodeIndex ) );
            }
        }

        //------------------------------------------------------------------------------

        static Matrix< DDRMat > get_supporting_points(
                const uint aDimension,
                const uint aOrder )
        {
            // the following lines are needed to get the interpolation points
            Matrix< DDRMat > aXihat;

            switch ( aDimension )
            {
                case 2:
                {
                    switch ( aOrder )
                    {
                        case 1:
                        {
                            // quad 4
                            aXihat.set_size( 2, 4 );

                            aXihat( 0, 0 ) = -1.000000;
                            aXihat( 1, 0 ) = -1.000000;
                            aXihat( 0, 1 ) = 1.000000;
                            aXihat( 1, 1 ) = -1.000000;
                            aXihat( 0, 2 ) = 1.000000;
                            aXihat( 1, 2 ) = 1.000000;
                            aXihat( 0, 3 ) = -1.000000;
                            aXihat( 1, 3 ) = 1.000000;
                            break;
                        }
                        case 2:
                        {
                            // quad 9
                            aXihat.set_size( 2, 9 );

                            aXihat( 0, 0 ) = -1.000000;
                            aXihat( 1, 0 ) = -1.000000;
                            aXihat( 0, 1 ) = 1.000000;
                            aXihat( 1, 1 ) = -1.000000;
                            aXihat( 0, 2 ) = 1.000000;
                            aXihat( 1, 2 ) = 1.000000;
                            aXihat( 0, 3 ) = -1.000000;
                            aXihat( 1, 3 ) = 1.000000;
                            aXihat( 0, 4 ) = 0.000000;
                            aXihat( 1, 4 ) = -1.000000;
                            aXihat( 0, 5 ) = 1.000000;
                            aXihat( 1, 5 ) = 0.000000;
                            aXihat( 0, 6 ) = 0.000000;
                            aXihat( 1, 6 ) = 1.000000;
                            aXihat( 0, 7 ) = -1.000000;
                            aXihat( 1, 7 ) = 0.000000;
                            aXihat( 0, 8 ) = 0.000000;
                            aXihat( 1, 8 ) = 0.000000;
                            break;
                        }
                        case 3:
                        {
                            // quad 16
                            aXihat.set_size( 2, 16 );

                            real c = 1.0 / 3.0;

                            aXihat( 0, 0 )  = -1.000000;
                            aXihat( 1, 0 )  = -1.000000;
                            aXihat( 0, 1 )  = 1.000000;
                            aXihat( 1, 1 )  = -1.000000;
                            aXihat( 0, 2 )  = 1.000000;
                            aXihat( 1, 2 )  = 1.000000;
                            aXihat( 0, 3 )  = -1.000000;
                            aXihat( 1, 3 )  = 1.000000;
                            aXihat( 0, 4 )  = -c;
                            aXihat( 1, 4 )  = -1.000000;
                            aXihat( 0, 5 )  = c;
                            aXihat( 1, 5 )  = -1.000000;
                            aXihat( 0, 6 )  = 1.000000;
                            aXihat( 1, 6 )  = -c;
                            aXihat( 0, 7 )  = 1.000000;
                            aXihat( 1, 7 )  = c;
                            aXihat( 0, 8 )  = c;
                            aXihat( 1, 8 )  = 1.000000;
                            aXihat( 0, 9 )  = -c;
                            aXihat( 1, 9 )  = 1.000000;
                            aXihat( 0, 10 ) = -1.000000;
                            aXihat( 1, 10 ) = c;
                            aXihat( 0, 11 ) = -1.000000;
                            aXihat( 1, 11 ) = -c;
                            aXihat( 0, 12 ) = -c;
                            aXihat( 1, 12 ) = -c;
                            aXihat( 0, 13 ) = c;
                            aXihat( 1, 13 ) = -c;
                            aXihat( 0, 14 ) = c;
                            aXihat( 1, 14 ) = c;
                            aXihat( 0, 15 ) = -c;
                            aXihat( 1, 15 ) = c;
                            break;
                        }
                        default:
                        {
                            MORIS_ERROR( false, "something went wrong while creating T-Matrices." );
                            break;
                        }
                    }
                    break;
                }
                case 3:
                {
                    switch ( aOrder )
                    {
                        case 1:
                        {
                            // hex 8
                            aXihat.set_size( 3, 8 );

                            aXihat( 0, 0 ) = -1.000000;
                            aXihat( 1, 0 ) = -1.000000;
                            aXihat( 2, 0 ) = -1.000000;
                            aXihat( 0, 1 ) = 1.000000;
                            aXihat( 1, 1 ) = -1.000000;
                            aXihat( 2, 1 ) = -1.000000;
                            aXihat( 0, 2 ) = 1.000000;
                            aXihat( 1, 2 ) = 1.000000;
                            aXihat( 2, 2 ) = -1.000000;
                            aXihat( 0, 3 ) = -1.000000;
                            aXihat( 1, 3 ) = 1.000000;
                            aXihat( 2, 3 ) = -1.000000;
                            aXihat( 0, 4 ) = -1.000000;
                            aXihat( 1, 4 ) = -1.000000;
                            aXihat( 2, 4 ) = 1.000000;
                            aXihat( 0, 5 ) = 1.000000;
                            aXihat( 1, 5 ) = -1.000000;
                            aXihat( 2, 5 ) = 1.000000;
                            aXihat( 0, 6 ) = 1.000000;
                            aXihat( 1, 6 ) = 1.000000;
                            aXihat( 2, 6 ) = 1.000000;
                            aXihat( 0, 7 ) = -1.000000;
                            aXihat( 1, 7 ) = 1.000000;
                            aXihat( 2, 7 ) = 1.000000;
                            break;
                        }
                        case 2:
                        {
                            // hex 27
                            aXihat.set_size( 3, 27 );

                            aXihat( 0, 0 )  = -1.000000;
                            aXihat( 1, 0 )  = -1.000000;
                            aXihat( 2, 0 )  = -1.000000;
                            aXihat( 0, 1 )  = 1.000000;
                            aXihat( 1, 1 )  = -1.000000;
                            aXihat( 2, 1 )  = -1.000000;
                            aXihat( 0, 2 )  = 1.000000;
                            aXihat( 1, 2 )  = 1.000000;
                            aXihat( 2, 2 )  = -1.000000;
                            aXihat( 0, 3 )  = -1.000000;
                            aXihat( 1, 3 )  = 1.000000;
                            aXihat( 2, 3 )  = -1.000000;
                            aXihat( 0, 4 )  = -1.000000;
                            aXihat( 1, 4 )  = -1.000000;
                            aXihat( 2, 4 )  = 1.000000;
                            aXihat( 0, 5 )  = 1.000000;
                            aXihat( 1, 5 )  = -1.000000;
                            aXihat( 2, 5 )  = 1.000000;
                            aXihat( 0, 6 )  = 1.000000;
                            aXihat( 1, 6 )  = 1.000000;
                            aXihat( 2, 6 )  = 1.000000;
                            aXihat( 0, 7 )  = -1.000000;
                            aXihat( 1, 7 )  = 1.000000;
                            aXihat( 2, 7 )  = 1.000000;
                            aXihat( 0, 8 )  = 0.000000;
                            aXihat( 1, 8 )  = -1.000000;
                            aXihat( 2, 8 )  = -1.000000;
                            aXihat( 0, 9 )  = 1.000000;
                            aXihat( 1, 9 )  = 0.000000;
                            aXihat( 2, 9 )  = -1.000000;
                            aXihat( 0, 10 ) = 0.000000;
                            aXihat( 1, 10 ) = 1.000000;
                            aXihat( 2, 10 ) = -1.000000;
                            aXihat( 0, 11 ) = -1.000000;
                            aXihat( 1, 11 ) = 0.000000;
                            aXihat( 2, 11 ) = -1.000000;
                            aXihat( 0, 12 ) = -1.000000;
                            aXihat( 1, 12 ) = -1.000000;
                            aXihat( 2, 12 ) = 0.000000;
                            aXihat( 0, 13 ) = 1.000000;
                            aXihat( 1, 13 ) = -1.000000;
                            aXihat( 2, 13 ) = 0.000000;
                            aXihat( 0, 14 ) = 1.000000;
                            aXihat( 1, 14 ) = 1.000000;
                            aXihat( 2, 14 ) = 0.000000;
                            aXihat( 0, 15 ) = -1.000000;
                            aXihat( 1, 15 ) = 1.000000;
                            aXihat( 2, 15 ) = 0.000000;
                            aXihat( 0, 16 ) = 0.000000;
                            aXihat( 1, 16 ) = -1.000000;
                            aXihat( 2, 16 ) = 1.000000;
                            aXihat( 0, 17 ) = 1.000000;
                            aXihat( 1, 17 ) = 0.000000;
                            aXihat( 2, 17 ) = 1.000000;
                            aXihat( 0, 18 ) = 0.000000;
                            aXihat( 1, 18 ) = 1.000000;
                            aXihat( 2, 18 ) = 1.000000;
                            aXihat( 0, 19 ) = -1.000000;
                            aXihat( 1, 19 ) = 0.000000;
                            aXihat( 2, 19 ) = 1.000000;
                            aXihat( 0, 20 ) = 0.000000;
                            aXihat( 1, 20 ) = 0.000000;
                            aXihat( 2, 20 ) = 0.000000;
                            aXihat( 0, 21 ) = 0.000000;
                            aXihat( 1, 21 ) = 0.000000;
                            aXihat( 2, 21 ) = -1.000000;
                            aXihat( 0, 22 ) = 0.000000;
                            aXihat( 1, 22 ) = 0.000000;
                            aXihat( 2, 22 ) = 1.000000;
                            aXihat( 0, 23 ) = -1.000000;
                            aXihat( 1, 23 ) = 0.000000;
                            aXihat( 2, 23 ) = 0.000000;
                            aXihat( 0, 24 ) = 1.000000;
                            aXihat( 1, 24 ) = 0.000000;
                            aXihat( 2, 24 ) = 0.000000;
                            aXihat( 0, 25 ) = 0.000000;
                            aXihat( 1, 25 ) = -1.000000;
                            aXihat( 2, 25 ) = 0.000000;
                            aXihat( 0, 26 ) = 0.000000;
                            aXihat( 1, 26 ) = 1.000000;
                            aXihat( 2, 26 ) = 0.000000;
                            break;
                        }
                        case 3:
                        {
                            // hex 64
                            aXihat.set_size( 3, 64 );

                            real c = 1.0 / 3.0;

                            aXihat( 0, 0 )  = -1.0;
                            aXihat( 1, 0 )  = -1.0;
                            aXihat( 2, 0 )  = -1.0;
                            aXihat( 0, 1 )  = 1.0;
                            aXihat( 1, 1 )  = -1.0;
                            aXihat( 2, 1 )  = -1.0;
                            aXihat( 0, 2 )  = 1.0;
                            aXihat( 1, 2 )  = 1.0;
                            aXihat( 2, 2 )  = -1.0;
                            aXihat( 0, 3 )  = -1.0;
                            aXihat( 1, 3 )  = 1.0;
                            aXihat( 2, 3 )  = -1.0;
                            aXihat( 0, 4 )  = -1.0;
                            aXihat( 1, 4 )  = -1.0;
                            aXihat( 2, 4 )  = 1.0;
                            aXihat( 0, 5 )  = 1.0;
                            aXihat( 1, 5 )  = -1.0;
                            aXihat( 2, 5 )  = 1.0;
                            aXihat( 0, 6 )  = 1.0;
                            aXihat( 1, 6 )  = 1.0;
                            aXihat( 2, 6 )  = 1.0;
                            aXihat( 0, 7 )  = -1.0;
                            aXihat( 1, 7 )  = 1.0;
                            aXihat( 2, 7 )  = 1.0;
                            aXihat( 0, 8 )  = -c;
                            aXihat( 1, 8 )  = -1.0;
                            aXihat( 2, 8 )  = -1.0;
                            aXihat( 0, 9 )  = c;
                            aXihat( 1, 9 )  = -1.0;
                            aXihat( 2, 9 )  = -1.0;
                            aXihat( 0, 10 ) = -1.0;
                            aXihat( 1, 10 ) = -c;
                            aXihat( 2, 10 ) = -1.0;
                            aXihat( 0, 11 ) = -1.0;
                            aXihat( 1, 11 ) = c;
                            aXihat( 2, 11 ) = -1.0;
                            aXihat( 0, 12 ) = -1.0;
                            aXihat( 1, 12 ) = -1.0;
                            aXihat( 2, 12 ) = -c;
                            aXihat( 0, 13 ) = -1.0;
                            aXihat( 1, 13 ) = -1.0;
                            aXihat( 2, 13 ) = c;
                            aXihat( 0, 14 ) = 1.0;
                            aXihat( 1, 14 ) = -c;
                            aXihat( 2, 14 ) = -1.0;
                            aXihat( 0, 15 ) = 1.0;
                            aXihat( 1, 15 ) = c;
                            aXihat( 2, 15 ) = -1.0;
                            aXihat( 0, 16 ) = 1.0;
                            aXihat( 1, 16 ) = -1.0;
                            aXihat( 2, 16 ) = -c;
                            aXihat( 0, 17 ) = 1.0;
                            aXihat( 1, 17 ) = -1.0;
                            aXihat( 2, 17 ) = c;
                            aXihat( 0, 18 ) = c;
                            aXihat( 1, 18 ) = 1.0;
                            aXihat( 2, 18 ) = -1.0;
                            aXihat( 0, 19 ) = -c;
                            aXihat( 1, 19 ) = 1.0;
                            aXihat( 2, 19 ) = -1.0;
                            aXihat( 0, 20 ) = 1.0;
                            aXihat( 1, 20 ) = 1.0;
                            aXihat( 2, 20 ) = -c;
                            aXihat( 0, 21 ) = 1.0;
                            aXihat( 1, 21 ) = 1.0;
                            aXihat( 2, 21 ) = c;
                            aXihat( 0, 22 ) = -1.0;
                            aXihat( 1, 22 ) = 1.0;
                            aXihat( 2, 22 ) = -c;
                            aXihat( 0, 23 ) = -1.0;
                            aXihat( 1, 23 ) = 1.0;
                            aXihat( 2, 23 ) = c;
                            aXihat( 0, 24 ) = -c;
                            aXihat( 1, 24 ) = -1.0;
                            aXihat( 2, 24 ) = 1.0;
                            aXihat( 0, 25 ) = c;
                            aXihat( 1, 25 ) = -1.0;
                            aXihat( 2, 25 ) = 1.0;
                            aXihat( 0, 26 ) = -1.0;
                            aXihat( 1, 26 ) = -c;
                            aXihat( 2, 26 ) = 1.0;
                            aXihat( 0, 27 ) = -1.0;
                            aXihat( 1, 27 ) = c;
                            aXihat( 2, 27 ) = 1.0;
                            aXihat( 0, 28 ) = 1.0;
                            aXihat( 1, 28 ) = -c;
                            aXihat( 2, 28 ) = 1.0;
                            aXihat( 0, 29 ) = 1.0;
                            aXihat( 1, 29 ) = c;
                            aXihat( 2, 29 ) = 1.0;
                            aXihat( 0, 30 ) = c;
                            aXihat( 1, 30 ) = 1.0;
                            aXihat( 2, 30 ) = 1.0;
                            aXihat( 0, 31 ) = -c;
                            aXihat( 1, 31 ) = 1.0;
                            aXihat( 2, 31 ) = 1.0;
                            aXihat( 0, 32 ) = -c;
                            aXihat( 1, 32 ) = -c;
                            aXihat( 2, 32 ) = -1.0;
                            aXihat( 0, 33 ) = -c;
                            aXihat( 1, 33 ) = c;
                            aXihat( 2, 33 ) = -1.0;
                            aXihat( 0, 34 ) = c;
                            aXihat( 1, 34 ) = c;
                            aXihat( 2, 34 ) = -1.0;
                            aXihat( 0, 35 ) = c;
                            aXihat( 1, 35 ) = -c;
                            aXihat( 2, 35 ) = -1.0;
                            aXihat( 0, 36 ) = -c;
                            aXihat( 1, 36 ) = -1.0;
                            aXihat( 2, 36 ) = -c;
                            aXihat( 0, 37 ) = c;
                            aXihat( 1, 37 ) = -1.0;
                            aXihat( 2, 37 ) = -c;
                            aXihat( 0, 38 ) = c;
                            aXihat( 1, 38 ) = -1.0;
                            aXihat( 2, 38 ) = c;
                            aXihat( 0, 39 ) = -c;
                            aXihat( 1, 39 ) = -1.0;
                            aXihat( 2, 39 ) = c;
                            aXihat( 0, 40 ) = -1.0;
                            aXihat( 1, 40 ) = -c;
                            aXihat( 2, 40 ) = -c;
                            aXihat( 0, 41 ) = -1.0;
                            aXihat( 1, 41 ) = -c;
                            aXihat( 2, 41 ) = c;
                            aXihat( 0, 42 ) = -1.0;
                            aXihat( 1, 42 ) = c;
                            aXihat( 2, 42 ) = c;
                            aXihat( 0, 43 ) = -1.0;
                            aXihat( 1, 43 ) = c;
                            aXihat( 2, 43 ) = -c;
                            aXihat( 0, 44 ) = 1.0;
                            aXihat( 1, 44 ) = -c;
                            aXihat( 2, 44 ) = -c;
                            aXihat( 0, 45 ) = 1.0;
                            aXihat( 1, 45 ) = c;
                            aXihat( 2, 45 ) = -c;
                            aXihat( 0, 46 ) = 1.0;
                            aXihat( 1, 46 ) = c;
                            aXihat( 2, 46 ) = c;
                            aXihat( 0, 47 ) = 1.0;
                            aXihat( 1, 47 ) = -c;
                            aXihat( 2, 47 ) = c;
                            aXihat( 0, 48 ) = c;
                            aXihat( 1, 48 ) = 1.0;
                            aXihat( 2, 48 ) = -c;
                            aXihat( 0, 49 ) = -c;
                            aXihat( 1, 49 ) = 1.0;
                            aXihat( 2, 49 ) = -c;
                            aXihat( 0, 50 ) = -c;
                            aXihat( 1, 50 ) = 1.0;
                            aXihat( 2, 50 ) = c;
                            aXihat( 0, 51 ) = c;
                            aXihat( 1, 51 ) = 1.0;
                            aXihat( 2, 51 ) = c;
                            aXihat( 0, 52 ) = -c;
                            aXihat( 1, 52 ) = -c;
                            aXihat( 2, 52 ) = 1.0;
                            aXihat( 0, 53 ) = c;
                            aXihat( 1, 53 ) = -c;
                            aXihat( 2, 53 ) = 1.0;
                            aXihat( 0, 54 ) = c;
                            aXihat( 1, 54 ) = c;
                            aXihat( 2, 54 ) = 1.0;
                            aXihat( 0, 55 ) = -c;
                            aXihat( 1, 55 ) = c;
                            aXihat( 2, 55 ) = 1.0;
                            aXihat( 0, 56 ) = -c;
                            aXihat( 1, 56 ) = -c;
                            aXihat( 2, 56 ) = -c;
                            aXihat( 0, 57 ) = c;
                            aXihat( 1, 57 ) = -c;
                            aXihat( 2, 57 ) = -c;
                            aXihat( 0, 58 ) = c;
                            aXihat( 1, 58 ) = c;
                            aXihat( 2, 58 ) = -c;
                            aXihat( 0, 59 ) = -c;
                            aXihat( 1, 59 ) = c;
                            aXihat( 2, 59 ) = -c;
                            aXihat( 0, 60 ) = -c;
                            aXihat( 1, 60 ) = -c;
                            aXihat( 2, 60 ) = c;
                            aXihat( 0, 61 ) = c;
                            aXihat( 1, 61 ) = -c;
                            aXihat( 2, 61 ) = c;
                            aXihat( 0, 62 ) = c;
                            aXihat( 1, 62 ) = c;
                            aXihat( 2, 62 ) = c;
                            aXihat( 0, 63 ) = -c;
                            aXihat( 1, 63 ) = c;
                            aXihat( 2, 63 ) = c;
                            break;
                        }
                        default:
                        {
                            MORIS_ERROR( false, "something went wrong while creating T-Matrices." );
                            break;
                        }
                    }
                    break;
                }
                default:
                {
                    MORIS_ERROR( false, "something went wrong while creating T-Matrices." );
                }
            }

            return aXihat;
        }
    };
}

#endif /* SRC_HMR_CL_HMR_T_MATRIX_HPP_ */
