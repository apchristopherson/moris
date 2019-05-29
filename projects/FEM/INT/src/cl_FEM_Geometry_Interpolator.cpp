#include "fn_norm.hpp"
#include "fn_cross.hpp"
#include "fn_dot.hpp"
#include "fn_sum.hpp"
#include "op_div.hpp"
#include "cl_FEM_Geometry_Interpolator.hpp"

namespace moris
{
    namespace fem
    {
//------------------------------------------------------------------------------

        Geometry_Interpolator::Geometry_Interpolator( const Interpolation_Rule & aInterpolationRule,
                                                      const bool                 aSpaceSideset )
        {
            // create member pointer to space interpolation function
            mSpaceInterpolation = aInterpolationRule.create_space_interpolation_function();

            // create member pointer to time interpolation function
            mTimeInterpolation  = aInterpolationRule.create_time_interpolation_function();

            // number of space bases and dimensions
            mNumSpaceBases    = mSpaceInterpolation->get_number_of_bases();
            mNumSpaceDim      = mSpaceInterpolation->get_number_of_dimensions();
            mNumSpaceParamDim = mSpaceInterpolation->get_number_of_param_dimensions();

            // number of time bases and dimensions
            mNumTimeBases = mTimeInterpolation->get_number_of_bases();
            mNumTimeDim   = mTimeInterpolation->get_number_of_dimensions();

            // set member geometry type
            mGeometryType = aInterpolationRule.get_geometry_type();
            mTimeGeometryType = mtk::Geometry_Type::LINE; //Fixme if point for time sideset

            // set pointers for second derivative depending on space and time dimensions
            this->set_function_pointers();

            // if there is a side interpolation
            // fixme to remove
            if ( aSpaceSideset )
            {
                // set bool for side interpolation to true
                mSpaceSideset = true;

//                // set side geometry type
//                this->get_auto_side_geometry_type();
//
//                // create side interpolation rule
//                Interpolation_Rule tSideInterpolationRule( mSideGeometryType,
//                                                           aInterpolationRule.get_space_interpolation_type(),
//                                                           aInterpolationRule.get_space_interpolation_order(),
//                                                           aInterpolationRule.get_time_interpolation_type(),
//                                                           aInterpolationRule.get_time_interpolation_order() );
//
//                // create member pointer to side space interpolation function
//                mSideSpaceInterpolation = tSideInterpolationRule.create_space_interpolation_function();
//
//                // number of space bases and dimensions for the side
//                mSideNumSpaceBases    = mSideSpaceInterpolation->get_number_of_bases();
//                mSideNumSpaceDim      = mSideSpaceInterpolation->get_number_of_dimensions();
//                mSideNumSpaceParamDim = mSideSpaceInterpolation->get_number_of_param_dimensions();
//
//                // get the vertices ordinals for each face of the element
//                this->get_face_vertices_ordinals();
            }
        }

//------------------------------------------------------------------------------

        Geometry_Interpolator::~Geometry_Interpolator()
        {
            // delete interpolation functions
            if( mSpaceInterpolation != NULL )
             {
                 delete mSpaceInterpolation;
             }

             if( mTimeInterpolation != NULL )
             {
                 delete mTimeInterpolation;
             }

             if( mSideSpaceInterpolation != NULL )
             {
                 delete mSideSpaceInterpolation;
             }
        }

//------------------------------------------------------------------------------

        void Geometry_Interpolator::set_coeff( const Matrix< DDRMat > & aXHat,
                                               const Matrix< DDRMat > & aTHat )
        {
            // check the space coefficients input size
            // fixme can not check the number of cols for aXHat and aTHat
            MORIS_ASSERT( aXHat.n_rows() == mNumSpaceBases , " Geometry_Interpolator::set_coeff - Wrong input size (aXHat). ");

            // set the space coefficients
            mXHat = aXHat;

            //check the time coefficients input size
            MORIS_ASSERT( aTHat.n_rows() == mNumTimeBases, " Geometry_Interpolator::set_coeff - Wrong input size (aTHat). ");

            // set the time coefficients
            mTHat = aTHat;
        }

//------------------------------------------------------------------------------

        void Geometry_Interpolator::set_space_coeff( const Matrix< DDRMat > & aXHat )
        {
            //check the space coefficients input size
            // fixme can not check the number of cols for aXHat
            MORIS_ASSERT( aXHat.n_rows() == mNumSpaceBases, " Geometry_Interpolator::set_space_coeff - Wrong input size (aXHat). ");

            // set the space coefficients
            mXHat = aXHat;
        }

//------------------------------------------------------------------------------

        void Geometry_Interpolator::set_time_coeff( const Matrix< DDRMat > & aTHat )
        {
            //check the time coefficients input size
            // fixme can not check the number of cols for aTHat
            MORIS_ASSERT( aTHat.n_rows() == mNumTimeBases, " Geometry_Interpolator::set_time_coeff - Wrong input size (aTHat). ");

            // set the time coefficients
            mTHat = aTHat;
        }

//------------------------------------------------------------------------------

        void Geometry_Interpolator::set_param_coeff()
        {
            // fixme should come from cluster
            //set space and time param coords
            mXiHat  = mSpaceInterpolation->get_param_coords();
            mXiHat = trans( mXiHat );

            mTauHat = mTimeInterpolation->get_param_coords();
            mTauHat = trans( mTauHat );
        }

//------------------------------------------------------------------------------

        void Geometry_Interpolator::set_space_param_coeff( const Matrix< DDRMat > & aXiHat )
        {
            //check the space param coefficients input size
            // fixme can not check the number of cols for aXiHat
            MORIS_ASSERT( aXiHat.n_rows() == mNumSpaceBases, " Geometry_Interpolator::set_space_param_coeff - Wrong input size (aXiHat). ");

            // set the space coefficients
            mXiHat = aXiHat;
        }

//------------------------------------------------------------------------------

        void Geometry_Interpolator::set_time_param_coeff( const Matrix< DDRMat > & aTauHat )
        {
            //check the time param coefficients input size
            // fixme can not check the number of cols for aTauHat
            MORIS_ASSERT( aTauHat.n_rows() == mNumTimeBases, " Geometry_Interpolator::set_time_coeff - Wrong input size (aTauHat). ");

            // set the time coefficients
            mTauHat = aTauHat;
        }

//------------------------------------------------------------------------------
        /**
         * set the space param coefficients for the side
         * @param[ in ] side space coefficients
         */
        // fixme to remove
        void Geometry_Interpolator::set_side_space_param_coeff( const Matrix< DDRMat > & aSideXiHat )
        {
            // fixme check values of aSideXiHat

            //check the space param coefficients input size
            MORIS_ASSERT( ( ( aSideXiHat.n_cols() == mSideNumSpaceParamDim ) && ( aSideXiHat.n_rows() == mSideNumSpaceBases )),
                          " Geometry_Interpolator::set_side_space_param_coeff - Wrong input size (aSideXiHat). ");

            // set the side space coefficients
            mSideXiHat = aSideXiHat;
        }

//------------------------------------------------------------------------------
        /**
         * set the time param coefficients for the side
         * @param[ in ] side time coefficients
         */
        // fixme to remove
        void Geometry_Interpolator::set_side_time_param_coeff( const Matrix< DDRMat > & aSideTauHat )
        {
            // fixme check values of aSideTauHat

            //check the space param coefficients input size
            MORIS_ASSERT( ( ( aSideTauHat.n_cols() == 1 ) && ( aSideTauHat.n_rows() == 1 )),
                          " Geometry_Interpolator::set_side_time_param_coeff - Wrong input size (aSideTauHat). ");

            // set the side space coefficients
            mSideTauHat = aSideTauHat;
        }

//------------------------------------------------------------------------------

        Matrix< DDRMat > Geometry_Interpolator::extract_space_side_space_param_coeff( moris_index aSpaceOrdinal )
        {
            // get the vertices per side map
            this->get_face_vertices_ordinals();

            // check if the space ordinal is appropriate
            MORIS_ASSERT( aSpaceOrdinal < static_cast< moris_index > ( mVerticesPerFace.size() ) ,
                          "Geometry_Interpolator::extract_space_side_space_param_coeff - wrong ordinal" );

            // get space side vertices ordinals
            moris::Cell< moris::moris_index > tVerticesOrdinals = mVerticesPerFace( aSpaceOrdinal );

             // initialize the time sideset parametric coordinates matrix
             uint tNumOfVertices = tVerticesOrdinals.size();

             // init the matrix with the face param coords
             Matrix< DDRMat > tSideXiHat( tNumOfVertices, mNumSpaceParamDim );

             // loop over the vertices of the face
             for( uint i = 0; i < tNumOfVertices; i++ )
             {
                 // get the treated vertex
                 moris_index tTreatedVertex = tVerticesOrdinals( i );

                 // get the vertex parametric coordinates for node tTreatedVertex
                 tSideXiHat.get_row( i ) = mXiHat.get_row( tTreatedVertex );
             }
              return tSideXiHat;
         }

//------------------------------------------------------------------------------

        Matrix< DDRMat > Geometry_Interpolator::extract_space_param_coeff()
        {
            return trans( mSpaceInterpolation->get_param_coords() );
        }

//------------------------------------------------------------------------------

        Matrix < DDRMat > Geometry_Interpolator::NXi( const Matrix< DDRMat > & aXi ) const
        {
            // pass data through interpolation function
            Matrix <DDRMat> tN = mSpaceInterpolation->eval_N( aXi );
            return tN;
         }

//------------------------------------------------------------------------------

         Matrix < DDRMat > Geometry_Interpolator::NTau( const Matrix< DDRMat > & aTau ) const
         {
             // pass data through interpolation function
             Matrix <DDRMat> tN = mTimeInterpolation->eval_N( aTau );
             return tN;
         }

//------------------------------------------------------------------------------

        Matrix< DDRMat > Geometry_Interpolator::dNdXi( const Matrix< DDRMat > & aXi ) const
        {
            // pass data through interpolation function
            Matrix <DDRMat> tdNdXi = mSpaceInterpolation->eval_dNdXi( aXi );
            return tdNdXi;
        }

//------------------------------------------------------------------------------

        Matrix< DDRMat > Geometry_Interpolator::dNdTau( const Matrix< DDRMat > & aTau) const
        {
            // pass data through interpolation function
            Matrix <DDRMat> tdNdTau = mTimeInterpolation->eval_dNdXi( aTau );
            return tdNdTau;
        }

//------------------------------------------------------------------------------

        Matrix< DDRMat > Geometry_Interpolator::d2NdXi2( const Matrix< DDRMat > & aXi ) const
        {
            // pass data through interpolation function
            Matrix <DDRMat> td2NdXi2 = mSpaceInterpolation->eval_d2NdXi2( aXi );
            return td2NdXi2;
        }

//------------------------------------------------------------------------------

        Matrix< DDRMat > Geometry_Interpolator::d2NdTau2( const Matrix< DDRMat > & aTau ) const
        {
            // pass data through interpolation function
            Matrix <DDRMat> td2NdTau2 = mTimeInterpolation->eval_d2NdXi2( aTau );
            return td2NdTau2;
        }
//------------------------------------------------------------------------------

        Matrix< DDRMat > Geometry_Interpolator::space_jacobian( const Matrix< DDRMat > & adNdXi ) const
        {
            // check that mXHat is set
            MORIS_ASSERT( mXHat.numel()>0, "Geometry_Interpolator::space_jacobian - mXHat is not set." );

            // compute the Jacobian
            Matrix< DDRMat > tJt = adNdXi * mXHat ;
            return tJt;
        }

//------------------------------------------------------------------------------

        Matrix< DDRMat > Geometry_Interpolator::time_jacobian( const Matrix< DDRMat > & adNdTau ) const
        {
            // check that mTHat is set
            MORIS_ASSERT( mTHat.numel()>0, "Geometry_Interpolator::time_jacobian - mTHat is not set." );

            // compute the Jacobian
            Matrix< DDRMat > tJt = adNdTau * mTHat ;
            return tJt;
        }

//------------------------------------------------------------------------------

        real Geometry_Interpolator::det_J( const Matrix< DDRMat > & aParamPoint )
        {
            // get tXi and tTau
            Matrix< DDRMat > tXi = aParamPoint( { 0, mNumSpaceParamDim-1 }, { 0, 0 } );
            Matrix< DDRMat > tTau( 1, 1, aParamPoint( mNumSpaceParamDim ) );

            // get the space jacobian
            Matrix< DDRMat > tdNSpacedXi = this->dNdXi( tXi );
            Matrix< DDRMat > tSpaceJt    = this->space_jacobian( tdNSpacedXi );

            // switch Geometry_Type
            real detJSpace;
            if( mSpaceSideset )
            {
                switch( mGeometryType )
                {
                    case ( mtk::Geometry_Type::LINE ) :
                    {
                        detJSpace = norm( tSpaceJt );
                        break;
                    }
                    case ( mtk::Geometry_Type::TRI ) :
                    {
                        detJSpace = norm( cross( tSpaceJt.get_row( 0 ) - tSpaceJt.get_row( 2 ),
                                                 tSpaceJt.get_row( 1 ) - tSpaceJt.get_row( 2 ) ) ) / 2.0;
                        break;
                    }
                    case ( mtk::Geometry_Type::QUAD ) :
                    {
                        detJSpace = norm( cross( tSpaceJt.get_row( 0 ), tSpaceJt.get_row( 1 ) ) );
                        break;
                    }
                    default :
                    {
                        MORIS_ERROR( false, "Geometry_Interpolator::det_J - unknown or not implemented geometry type ");
                        break;
                    }
                }
            }
            else
            {
                switch( mGeometryType )
                {
                    case ( mtk::Geometry_Type::LINE ) :
                    case ( mtk::Geometry_Type::QUAD ) :
                    case ( mtk::Geometry_Type::HEX ) :
                    {
                        detJSpace = det( tSpaceJt );
                        break;
                    }

                    case ( mtk::Geometry_Type::TRI ) :
                    {
                        Matrix< DDRMat > tSpaceJt2( mNumSpaceParamDim, mNumSpaceParamDim, 1.0 );
                        tSpaceJt2({ 1, mNumSpaceParamDim-1 },{ 0, mNumSpaceParamDim-1 }) = trans( tSpaceJt );
                        detJSpace = det( tSpaceJt2 ) / 2.0;
                        break;
                    }

                    case ( mtk::Geometry_Type::TET ) :
                    {
                        Matrix< DDRMat > tSpaceJt2( mNumSpaceParamDim, mNumSpaceParamDim, 1.0 );
                        tSpaceJt2({ 1, mNumSpaceParamDim-1 },{ 0, mNumSpaceParamDim-1 }) = trans( tSpaceJt );
                        detJSpace = det( tSpaceJt2 ) / 6.0;
                        break;
                    }

                    default :
                    {
                        MORIS_ERROR( false, "Geometry_Interpolator::det_J - unknown or not implemented space geometry type ");
                        break;
                    }
                }
            }

            // get the time Jacobian
            Matrix< DDRMat > tdNTimedTau = this->dNdTau( tTau );
            Matrix< DDRMat > tTimeJt     = this->time_jacobian( tdNTimedTau );

            // switch Geometry_Type
            real detJTime;
            switch ( mTimeGeometryType )
            {
                case ( mtk::Geometry_Type::POINT ) :
                {
                    detJTime = 1.0;
                    break;
                }
                case ( mtk::Geometry_Type::LINE ) :
                {
                    detJTime = det( tTimeJt );
                    break;
                }
                default:
                {
                    MORIS_ERROR( false, "Geometry_Interpolator::det_J - unknown or not implemented time geometry type ");
                    break;
                }
            }

            // compute the determinant of the space time Jacobian
            return detJSpace * detJTime;

        }
//------------------------------------------------------------------------------

        // fixme to remove
        real Geometry_Interpolator::time_surf_det_J( const Matrix< DDRMat > & aSideParamPoint )
        {
            // fixme check aSideParamPoint

            // fixme point in time

            // unpack the space and time param coords of the side param point
            Matrix< DDRMat > tXi = aSideParamPoint( { 0, mNumSpaceParamDim-1 }, { 0, 0 } );
            //Matrix< DDRMat > tTau( 1, 1, aSideParamPoint( mNumSpaceParamDim ) );

            // get the space jacobian
            Matrix< DDRMat > tSpaceJt    = this->space_jacobian( this->dNdXi( tXi ) );

            // init tTimeSurfDetJ
            real tTimeSurfDetJ = -1.0;

            switch ( mGeometryType )
            {
                case ( mtk::Geometry_Type::TRI ):
                {
                    Matrix< DDRMat > tSpaceJt2( mNumSpaceParamDim, mNumSpaceParamDim, 1.0 );
                    tSpaceJt2({ 1, mNumSpaceParamDim-1 },{ 0, mNumSpaceParamDim-1 }) = trans( tSpaceJt );
                    tTimeSurfDetJ = det( tSpaceJt2 )/2.0/2.0;
                    break;
                }
                case ( mtk::Geometry_Type::TET ):
                {
                    Matrix< DDRMat > tSpaceJt2( mNumSpaceParamDim, mNumSpaceParamDim, 1.0 );
                    tSpaceJt2({ 1, mNumSpaceParamDim-1 },{ 0, mNumSpaceParamDim-1 }) = trans( tSpaceJt );
                    tTimeSurfDetJ = det( tSpaceJt2 )/2.0/6.0;
                    break;
                }
                default:
                {
                    tTimeSurfDetJ = det( tSpaceJt )/2.0;
                    break;
                }
            }
            return tTimeSurfDetJ;
        }

//------------------------------------------------------------------------------

        Matrix< DDRMat > Geometry_Interpolator::get_normal( const Matrix< DDRMat > & aSideParamPoint )
         {
             // fixme check aSideParamPoint

             // check that there is a side interpolation
             MORIS_ASSERT( mSpaceSideset, "Geometry_Interpolator::normal - not a side." );

             // unpack the space and time param coords of the side param point
             Matrix< DDRMat > tXi = aSideParamPoint( { 0, mNumSpaceParamDim-1 }, { 0, 0 } );

             // evaluate side space interpolation shape functions first parametric derivatives at aParamPoint
             Matrix< DDRMat > tdNSpacedXi = mSpaceInterpolation->eval_dNdXi( tXi );

             // evaluation of tangent vectors to the space side in the physical space
             Matrix< DDRMat > tRealTangents = trans( tdNSpacedXi * mXHat );

             // init the normal
             Matrix< DDRMat > tNormal;

             // switch on geometry type
             switch ( mGeometryType )
             {
                 case ( mtk::Geometry_Type::LINE ):
                 {
                     // computing the normal from the real tangent vectors
                     tNormal = {{  tRealTangents( 1 ) },
                                { -tRealTangents( 0 ) }};
                     tNormal = tNormal / norm( tNormal );
                     return tNormal;
                     break;
                 }

                 case ( mtk::Geometry_Type::QUAD ):
                 {
                     // compute the normal from the real tangent vectors
                     tNormal = cross( tRealTangents.get_column( 0 ), tRealTangents.get_column( 1 ) );
                     tNormal = tNormal / norm( tNormal );
                     return tNormal;
                     break;
                 }

                 case ( mtk::Geometry_Type::TRI ):
                 {
                     // computing the normal from the tangent vector in the physical space
                     tNormal = cross( tRealTangents.get_column( 0 ) - tRealTangents.get_column( 2 ),
                                      tRealTangents.get_column( 1 ) - tRealTangents.get_column( 2 ) );
                     tNormal = tNormal / norm( tNormal );
                     return tNormal;
                     break;
                 }

                 default:
                 {
                     MORIS_ERROR( false, "Geometry_Interpolator::get_normal - wrong geometry type or geometry type not implemented");
                     return tNormal;
                     break;
                 }
             }
         }

//------------------------------------------------------------------------------

        Matrix< DDRMat > Geometry_Interpolator::valx( const Matrix< DDRMat > & aXi )
        {
            // check that mTHat is set
            MORIS_ASSERT( mXHat.numel()>0, "Geometry_Interpolator::time_jacobian - mXHat is not set." );

            //evaluate the field
            return this->NXi( aXi ) * mXHat ;
        }

//------------------------------------------------------------------------------

        Matrix< DDRMat > Geometry_Interpolator::valt( const Matrix< DDRMat > & aTau )
        {
            // check that mTHat is set
            MORIS_ASSERT( mTHat.numel()>0, "Geometry_Interpolator::time_jacobian - mTHat is not set." );

            //evaluate the field
            return this->NTau( aTau ) * mTHat ;
        }

//------------------------------------------------------------------------------
        Matrix< DDRMat > Geometry_Interpolator::map_integration_point( const Matrix< DDRMat > & aLocalParamPoint )
        {
            // check that mXiHat and mTauHat are set
            MORIS_ASSERT( mXiHat.numel()>0, "Geometry_Interpolator::map_integration_point - mXiHat is not set." );
            MORIS_ASSERT( mTauHat.numel()>0, "Geometry_Interpolator::map_integration_point - mTauHat is not set." );

            // unpack the coords of the local param point
            Matrix< DDRMat > aLocalXi  = aLocalParamPoint( { 0, mNumSpaceParamDim-1 }, {0,0} );
            Matrix< DDRMat > aLocalTau = aLocalParamPoint( { mNumSpaceParamDim, mNumSpaceParamDim }  , {0,0} );

            // evaluate the coords of the mapped param point
            uint tNumSpaceCoords = mXiHat.n_cols();
            Matrix< DDRMat > aParamPoint( tNumSpaceCoords + 1, 1 );
            aParamPoint( {0, tNumSpaceCoords-1 }, {0,0} )            = trans( this->NXi( aLocalXi )   * mXiHat  ) ;
            aParamPoint( {tNumSpaceCoords, tNumSpaceCoords}, {0,0} ) = trans( this->NTau( aLocalTau ) * mTauHat ) ;

            // return the mapped param point
            return aParamPoint;
        }

//------------------------------------------------------------------------------

         // fixme to remove
        Matrix < DDRMat > Geometry_Interpolator::time_surf_val( const Matrix< DDRMat > & aSideParamPoint )
        {
            // fixme check aSideParamPoint

            // unpack the space and time param coords of the time side param point
            Matrix< DDRMat > tXi = aSideParamPoint( { 0, mNumSpaceParamDim-1 }, { 0, 0 } );
            Matrix< DDRMat > tTau( 1, 1, aSideParamPoint( mNumSpaceParamDim ) );

            // build space time interpolation functions for side
            Matrix< DDRMat > tNSpace = mSpaceInterpolation->eval_N( tXi );

            //compute the parametric coordinates of the SideParamPoint in the parent reference element
            Matrix< DDRMat > tParentParamPoint( 1, mNumSpaceParamDim + mNumTimeDim );
            tParentParamPoint({ 0, 0 }, { 0, mNumSpaceParamDim-1 })               = tNSpace * mXiHat;
            tParentParamPoint({ 0, 0 }, { mNumSpaceParamDim, mNumSpaceParamDim }) = mSideTauHat.matrix_data();

            return trans( tParentParamPoint );
        }

//------------------------------------------------------------------------------

        void Geometry_Interpolator::space_jacobian_and_matrices_for_second_derivatives(
                      Matrix< DDRMat > & aJt,
                      Matrix< DDRMat > & aKt,
                      Matrix< DDRMat > & aLt,
                const Matrix< DDRMat > & adNdXi,
                const Matrix< DDRMat > & ad2NdXi2 ) const
        {
            // check that mXHat is set
            MORIS_ASSERT( mXHat.numel()>0, "Geometry_Interpolator::space_jacobian_and_matrices_for_second_derivatives - mXHat is not set." );

            // evaluate transposed of geometry Jacobian
            aJt = this->space_jacobian( adNdXi );

            // call calculator for second derivatives
            this->mSecondDerivativeMatricesSpace( aJt,
                                                  aKt,
                                                  aLt,
                                                  ad2NdXi2,
                                                  mXHat);
        }

//------------------------------------------------------------------------------

        void Geometry_Interpolator::time_jacobian_and_matrices_for_second_derivatives(
                      Matrix< DDRMat > & aJt,
                      Matrix< DDRMat > & aKt,
                      Matrix< DDRMat > & aLt,
                const Matrix< DDRMat > & adNdTau,
                const Matrix< DDRMat > & ad2NdTau2 ) const
        {
            // check that mXHat is set
            MORIS_ASSERT( mTHat.numel()>0, "Geometry_Interpolator::time_jacobian_and_matrices_for_second_derivatives - mTHat is not set." );

            // evaluate transposed of geometry Jacobian
            aJt = this->time_jacobian( adNdTau );

            // call calculator for second derivatives
            this->mSecondDerivativeMatricesTime( aJt,
                                                 aKt,
                                                 aLt,
                                                 ad2NdTau2,
                                                 mTHat );
        }

//------------------------------------------------------------------------------

        void Geometry_Interpolator::eval_matrices_for_second_derivative_1d(
                const Matrix< DDRMat > & aJt,
                      Matrix< DDRMat > & aKt,
                      Matrix< DDRMat > & aLt,
                const Matrix< DDRMat > & ad2NdXi2,
                const Matrix< DDRMat > & aXHat)
        {
            // help matrix K
            aKt = ad2NdXi2 * aXHat;

            // help matrix L
            aLt.set_size( 1, 1 );
            aLt( 0, 0 ) = std::pow( aJt( 0, 0 ), 2 );
        }

//------------------------------------------------------------------------------

        void Geometry_Interpolator::eval_matrices_for_second_derivative_2d(
                const Matrix< DDRMat > & aJt,
                      Matrix< DDRMat > & aKt,
                      Matrix< DDRMat > & aLt,
                const Matrix< DDRMat > & ad2NdXi2,
                const Matrix< DDRMat > & aXHat )
        {
            // help matrix K
            aKt = ad2NdXi2 * aXHat;

            // help matrix L
            aLt.set_size( 3, 3 );
            aLt( 0, 0 ) = std::pow( aJt( 0, 0 ), 2 );
            aLt( 1, 0 ) = std::pow( aJt( 1, 0 ), 2 );
            aLt( 2, 0 ) = aJt( 0 , 0 ) * aJt( 1 , 0 );

            aLt( 0, 1 ) = std::pow( aJt( 0, 1 ), 2 );
            aLt( 1, 1 ) = std::pow( aJt( 1, 1 ), 2 );
            aLt( 2, 1 ) = aJt( 0 , 1 ) * aJt( 1, 1 );

            aLt( 0, 2 ) = 2.0 * aJt( 0, 0 ) * aJt( 0, 1 );
            aLt( 1, 2 ) = 2.0 * aJt( 1, 0 ) * aJt( 1, 1 );
            aLt( 2, 2 ) = aJt( 0, 0 )* aJt( 1, 1 ) +  aJt( 0, 1 ) * aJt( 1, 0 );
        }

//------------------------------------------------------------------------------

        void Geometry_Interpolator::eval_matrices_for_second_derivative_3d(
                const Matrix< DDRMat > & aJt,
                      Matrix< DDRMat > & aKt,
                      Matrix< DDRMat > & aLt,
                const Matrix< DDRMat > & ad2NdXi2,
                const Matrix< DDRMat > & aXHat )
        {
            // help matrix K
            aKt = ad2NdXi2 * aXHat;

            // help matrix L
            aLt.set_size( 6, 6 );
            for( uint j=0; j<3; ++j )
            {
                aLt( 0, j ) = std::pow( aJt( 0, j ), 2 );
                aLt( 1, j ) = std::pow( aJt( 1, j ), 2 );
                aLt( 2, j ) = std::pow( aJt( 2, j ), 2 );
                aLt( 3, j ) = aJt( 1 , j ) * aJt( 2 , j );
                aLt( 4, j ) = aJt( 0 , j ) * aJt( 2 , j );
                aLt( 5, j ) = aJt( 0 , j ) * aJt( 1 , j );
            }

            aLt( 0, 3 ) = 2.0 * aJt( 0, 1 ) * aJt( 0, 2 );
            aLt( 1, 3 ) = 2.0 * aJt( 1, 1 ) * aJt( 1, 2 );
            aLt( 2, 3 ) = 2.0 * aJt( 2, 1 ) * aJt( 2, 2 );
            aLt( 3, 3 ) = aJt( 1, 1 ) * aJt( 2, 2 ) + aJt( 2, 1 ) * aJt( 1, 2 );
            aLt( 4, 3 ) = aJt( 0, 1 ) * aJt( 2, 2 ) + aJt( 2, 1 ) * aJt( 0, 2 );
            aLt( 5, 3 ) = aJt( 0, 1 ) * aJt( 1, 2 ) + aJt( 1, 1 ) * aJt( 0, 2 );

            aLt( 0, 4 ) = 2.0 * aJt( 0, 0 ) * aJt( 0, 2 );
            aLt( 1, 4 ) = 2.0 * aJt( 1, 0 ) * aJt( 1, 2 );
            aLt( 2, 4 ) = 2.0 * aJt( 2, 0 ) * aJt( 2, 2 );
            aLt( 3, 4 ) = aJt( 1, 0 ) * aJt( 2, 2 ) + aJt( 2, 0 ) * aJt( 1, 2 );
            aLt( 4, 4 ) = aJt( 0, 0 ) * aJt( 2, 2 ) + aJt( 2, 0 ) * aJt( 0, 2 );
            aLt( 5, 4 ) = aJt( 0, 0 ) * aJt( 1, 2 ) + aJt( 1, 0 ) * aJt( 0, 2 );

            aLt( 0, 5 ) = 2.0 * aJt( 0, 0 ) * aJt( 0, 1 );
            aLt( 1, 5 ) = 2.0 * aJt( 1, 0 ) * aJt( 1, 1 );
            aLt( 2, 5 ) = 2.0 * aJt( 2, 0 ) * aJt( 2, 1 );
            aLt( 3, 5 ) = aJt( 1, 0 ) * aJt( 2, 1 ) + aJt( 2, 0 ) * aJt( 1, 1 );
            aLt( 4, 5 ) = aJt( 0, 0 ) * aJt( 2, 1 ) + aJt( 2, 0 ) * aJt( 0, 1 );
            aLt( 5, 5 ) = aJt( 0, 0 ) * aJt( 1, 1 ) + aJt( 1, 0 ) * aJt( 0, 1 );
        }

//------------------------------------------------------------------------------

        void Geometry_Interpolator::set_function_pointers()
        {
            // get number of dimensions and set pointer to function
            // for second space derivative
            switch ( mNumSpaceDim )
            {
                case( 1 ) :
                {
                    mSecondDerivativeMatricesSpace = this->eval_matrices_for_second_derivative_1d;
                    break;
                }
                case( 2 ) :
                {
                    mSecondDerivativeMatricesSpace = this->eval_matrices_for_second_derivative_2d;
                    break;
                }
                case( 3 ) :
                {
                    mSecondDerivativeMatricesSpace = this->eval_matrices_for_second_derivative_3d;
                    break;
                }
                default :
                {
                    MORIS_ERROR( false, " Geometry_Interpolator::set_function_pointers - unknown number of dimensions. " );
                    break;
                }
            }

            // get number of dimensions and set pointer to function
            // for second time derivative
            switch ( mNumTimeDim )
            {
                case( 1 ) :
                {
                    mSecondDerivativeMatricesTime = this->eval_matrices_for_second_derivative_1d;
                    break;
                }
                case( 2 ) :
                {
                    mSecondDerivativeMatricesTime = this->eval_matrices_for_second_derivative_2d;
                    break;
                }
                case( 3 ) :
                {
                    mSecondDerivativeMatricesTime = this->eval_matrices_for_second_derivative_3d;
                    break;
                }
                default :
                {
                    MORIS_ERROR( false, " Geometry_Interpolator::set_function_pointers - unknown number of dimensions. " );
                    break;
                }
            }
        }

//------------------------------------------------------------------------------

        void Geometry_Interpolator::get_auto_side_geometry_type()
        {
            // depending on the parent geometry type
            switch ( mGeometryType )
            {
                // FIXME should we consider lines?
                case ( mtk::Geometry_Type::LINE ):
                {
                    mSideGeometryType = mtk::Geometry_Type::POINT;
                    break;
                }
                case ( mtk::Geometry_Type::QUAD ):
                {
                    mSideGeometryType = mtk::Geometry_Type::LINE;
                    break;
                }
                case ( mtk::Geometry_Type::HEX ):
                {
                    mSideGeometryType = mtk::Geometry_Type::QUAD;
                    break;
                }
                case ( mtk::Geometry_Type::TRI ):
                {
                    mSideGeometryType = mtk::Geometry_Type::LINE;
                    break;
                }
                case ( mtk::Geometry_Type::TET ):
                {
                    mSideGeometryType = mtk::Geometry_Type::TRI;
                    break;
                }
                default:
                {
                    MORIS_ERROR( false, " Geometry_Interpolator::get_auto_side_geometry_type - undefined geometry type. " );
                    mSideGeometryType = mtk::Geometry_Type::UNDEFINED;
                }
            }
        }

//------------------------------------------------------------------------------

        void Geometry_Interpolator::get_face_vertices_ordinals()
        {
            // depending on the parent geometry
            switch ( mGeometryType )
            {
                case ( mtk::Geometry_Type::LINE ):
                {
                    switch ( mNumSpaceBases )
                    {
                        case ( 1 ):
                            mVerticesPerFace = { { 0 } };
                            break;
                        case ( 2 ):
                            mVerticesPerFace = { { 0 }, { 1 } };
                            break;
                        case ( 3 ):
                            mVerticesPerFace = { { 0 }, { 1 }, { 2 } };
		                    break;
                        case ( 4 ):
                            mVerticesPerFace = { { 0 }, { 1 }, { 2 }, { 3 } };
                            break;
                        default:
                            MORIS_ERROR( false, "Geometry_Interpolator::get_face_vertices_ordinals - LINE order not implemented " );
                            break;
                    }
                    break;
                }

                case ( mtk::Geometry_Type::QUAD ):
                {
                    switch ( mNumSpaceBases )
                    {
                        case ( 4 ):
                            mVerticesPerFace = { { 0, 1 },
                                                 { 1, 2 },
                                                 { 2, 3 },
                                                 { 3, 0 } };
                            break;
                        case ( 8 ):
                            mVerticesPerFace = { { 0, 1, 4 },
                                                 { 1, 2, 5 },
                                                 { 2, 3, 6 },
                                                 { 3, 0, 7 } };
                            break;
                        case ( 9 ):
                            mVerticesPerFace = { { 0, 1, 4 },
                                                 { 1, 2, 5 },
                                                 { 2, 3, 6 },
                                                 { 3, 0, 7 } };
                            break;
                        case ( 16 ):
                            mVerticesPerFace = { { 0, 1,  4,  5 },
                                                 { 1, 2,  6,  7 },
                                                 { 2, 3,  8,  9 },
                                                 { 3, 0, 10, 11 } };
                            break;
                        default:
                            MORIS_ERROR( false, "Geometry_Interpolator::get_face_vertices_ordinals - QUAD order not implemented " );
                            break;
                    }
                    break;
                }

                case ( mtk::Geometry_Type::HEX ):
                {
                    switch( mNumSpaceBases )
                    {
                        case ( 8 ):
                            mVerticesPerFace = { { 0, 1, 5, 4 },
                                                 { 1, 2, 6, 5 },
                                                 { 2, 3, 7, 6 },
                                                 { 0, 4, 7, 3 },
                                                 { 0, 3, 2, 1 },
                                                 { 4, 5, 6, 7 } };
                            break;
                        case ( 20 ):
                            mVerticesPerFace = { { 0, 1, 5, 4,  8, 13, 16, 12 },
                                                 { 1, 2, 6, 5,  9, 14, 17, 13 },
                                                 { 2, 3, 7, 6, 10, 15, 18, 14 },
                                                 { 0, 4, 7, 3, 12, 19, 15, 11 },
                                                 { 0, 3, 2, 1, 11, 10,  9,  8 },
                                                 { 4, 5, 6, 7, 16, 17, 18, 19 } };
                            break;
                        case ( 27 ):
                            mVerticesPerFace = { { 0, 1, 5, 4,  8, 13, 16, 12, 23 },
                                                 { 1, 2, 6, 5,  9, 14, 17, 13, 24 },
                                                 { 2, 3, 7, 6, 10, 15, 18, 14, 26 },
                                                 { 0, 4, 7, 3, 12, 19, 15, 11, 23 },
                                                 { 0, 3, 2, 1, 11, 10,  9,  8, 21 },
                                                 { 4, 5, 6, 7, 16, 17, 18, 19, 22 } };
                            break;
                        case ( 64 ):
                            mVerticesPerFace = { { 0, 1, 5, 4,  8,  9, 16, 17, 25, 24, 13, 12, 36, 37, 38, 39 },
                                                 { 1, 2, 6, 5, 14, 15, 20, 21, 29, 28, 17, 16, 44, 45, 46, 47 },
                                                 { 2, 3, 7, 6, 18, 19, 22, 23, 31, 30, 21, 20, 48, 49, 50, 51 },
                                                 { 0, 4, 7, 3, 12, 13, 26, 27, 23, 22, 11, 10, 40, 41, 42, 43 },
                                                 { 0, 3, 2, 1, 10, 11, 19, 18, 15, 14,  9,  8, 32, 33, 34, 35 },
                                                 { 4, 5, 6, 7, 24, 25, 28, 29, 30, 31, 27, 26, 52, 53, 54, 55 }};
                            break;
                        default:
                            MORIS_ERROR( false, "Geometry_Interpolator::get_face_vertices_ordinals - HEX order not implemented " );
                            break;
                    }
                    break;
                }

                case ( mtk::Geometry_Type::TRI ):
                {
                    switch( mNumSpaceBases )
                    {
                        case ( 3 ):
                            mVerticesPerFace = { { 0, 1 },
                                                 { 1, 2 },
                                                 { 2, 0 }};
                            break;
                        case ( 6 ):
                            mVerticesPerFace = { { 0, 1, 3 },
                                                 { 1, 2, 4 },
                                                 { 2, 0, 5 }};
                            break;
                        case ( 10 ):
                            mVerticesPerFace = { { 0, 1, 3, 4 },
                                                 { 1, 2, 5, 6 },
                                                 { 2, 0, 7, 8 }};
                            break;
                        default:
                            MORIS_ERROR( false, "Geometry_Interpolator::get_face_vertices_ordinals - TRI order not implemented " );
                            break;
                    }
                    break;
                }

                case ( mtk::Geometry_Type::TET ):
                {
                    switch( mNumSpaceBases )
                    {
                        case ( 4 ):
                            mVerticesPerFace = {{ 0, 1, 3 },
                                                { 1, 2, 3 },
                                                { 0, 3, 2 },
                                                { 0, 2, 1 }};
                            break;
                        case ( 10 ):
                            mVerticesPerFace = {{ 0, 1, 3, 4, 8, 7 },
                                                { 1, 2, 3, 5, 9, 8 },
                                                { 0, 3, 2, 7, 9, 6 },
                                                { 0, 2, 1, 6, 5, 4 }};
                            break;
                        case ( 20 ):
                            mVerticesPerFace = {{ 0, 1, 3,  4,  5, 12, 13, 11, 10, 17 },
                                                { 1, 2, 3,  6,  7, 14, 15, 13, 12, 18 },
                                                { 0, 3, 2, 10, 11, 15, 14,  9,  8, 19 },
                                                { 0, 2, 1,  8,  9,  7,  6,  5,  4, 16 }};
                            break;
                        default:
                            MORIS_ERROR( false, "Geometry_Interpolator::get_face_vertices_ordinals - TET order not implemented " );
                            break;
                    }
                    break;
                }

                default:
                {
                    MORIS_ERROR( false, "Geometry_Interpolator::get_space_sideset_param_coeff - undefined geometry type " );
                    break;
                }
            }
        }

//------------------------------------------------------------------------------

//        void Geometry_Interpolator::surf_det_J(       real             & aSurfDetJ,
//                                                      Matrix< DDRMat > & aNormal,
//                                                const Matrix< DDRMat > & aSideParamPoint,
//                                                      moris_index        aSpaceOrdinal )
//        {
//            // fixme check aSideParamPoint
//
//            // check that there is a side interpolation
//            MORIS_ASSERT( mSpaceSideset, "Geometry_Interpolator::surf_val - no side interpolation." );
//
//            // check if the space ordinal is appropriate
//            MORIS_ASSERT( aSpaceOrdinal < static_cast< moris_index > ( mVerticesPerFace.size() ) ,
//                          "Geometry_Interpolator::get_space_sideset_param_coords - wrong ordinal" );
//
////            // get the number of space time bases
////            uint tSideBases = mSideNumSpaceBases * mNumTimeBases;
//
//            // unpack the space and time param coords of the side param point
//            Matrix< DDRMat > tXi = aSideParamPoint( { 0, mSideNumSpaceParamDim-1 }, { 0, 0 } );
//            Matrix< DDRMat > tTau( 1, 1, aSideParamPoint( mSideNumSpaceParamDim ) );
//
////            // evaluate side space interpolation shape functions at aParamPoint
////            Matrix< DDRMat > tNSideSpace = mSideSpaceInterpolation->eval_N( tXi );
////
////            // evaluate time interpolation shape functions at aParamPoint
////            Matrix< DDRMat > tNTime      = mTimeInterpolation->eval_N( tTau );
//
//            // evaluate side space interpolation shape functions first parametric derivatives at aParamPoint
//            Matrix< DDRMat > tdNSideSpacedXi = mSideSpaceInterpolation->eval_dNdXi( tXi );
//            tdNSideSpacedXi = trans( tdNSideSpacedXi );
//
//            // evaluate time interpolation shape functions first parametric derivatives at aParamPoint
//            Matrix< DDRMat > tdNTimedTau     = mTimeInterpolation->eval_dNdXi( tTau );
//
////            // build the space time dNdXi row by row
////            Matrix< DDRMat > tdNdXi( mSideNumSpaceParamDim, tSideBases );
////            for ( uint Ik = 0; Ik < mSideNumSpaceParamDim; Ik++ )
////            {
////                tdNdXi.get_row( Ik ) = reshape( tdNSideSpacedXi.get_column( Ik ) * tNTime , 1, tSideBases );
////            }
////
////            // build the space time dNdTau row by row
////            Matrix< DDRMat > tdNdTau = reshape( trans( tNSideSpace ) * tdNTimedTau, 1, tSideBases);
////
////            // get the side parametric coordinates in the reference volume space
////            Matrix< DDRMat > tSideParamCoords = this->get_space_side_param_coords( aSpaceOrdinal );
////
////            // evaluation of tangent vectors to the space side in the reference volume space
////            Matrix< DDRMat > tRefTangents( mSideNumSpaceParamDim + mNumTimeDim, mNumSpaceParamDim + mNumTimeDim );
////            tRefTangents( { 0, mSideNumSpaceParamDim-1 }, { 0, mNumSpaceParamDim + mNumTimeDim-1 } )
////                = tdNdXi * trans( tSideParamCoords );
////            tRefTangents( { mSideNumSpaceParamDim, mSideNumSpaceParamDim }, { 0, mNumSpaceParamDim + mNumTimeDim-1 } )
////                = tdNdTau * trans( tSideParamCoords );
////            tRefTangents = trans( tRefTangents );
////
////            // get the location of the side integration point in the reference parent element
////            Matrix< DDRMat > tParentParamPoint = this->surf_val( aSideParamPoint, aSpaceOrdinal );
////
////            // unpack the space and time param coords of the parent param point
////            Matrix< DDRMat > tParentXi  = tParentParamPoint( { 0, mNumSpaceParamDim-1 }, { 0, 0 });
////            Matrix< DDRMat > tParentTau = tParentParamPoint( { mNumSpaceParamDim, mNumSpaceParamDim }, { 0, 0 });
////
////            // get the jacobian mapping from volume reference space to volume physical space
////            Matrix< DDRMat > tdNParentSpacedXi = this->dNdXi(  tParentXi );
////            Matrix< DDRMat > tdNParentTimedTau = this->dNdTau( tParentTau );
////            Matrix< DDRMat > tJParentt( mNumSpaceParamDim + mNumTimeDim, mNumSpaceDim + mNumTimeDim, 0.0 );
////            tJParentt( {0, mNumSpaceParamDim-1}, {0, mNumSpaceDim-1} )
////                = this->space_jacobian( tdNParentSpacedXi ).matrix_data();
////            tJParentt( {mNumSpaceParamDim, mNumSpaceParamDim}, {mNumSpaceDim, mNumSpaceDim} )
////                = this->time_jacobian( tdNParentTimedTau ).matrix_data();
////
////            // evaluation of tangent vectors to the space side in the physical space
////            Matrix< DDRMat > tRealTangents = trans( tJParentt ) * tRefTangents;
//
////            // get the side physical coordinates in the physical volume space
////            this->build_space_time_phys_coords();
////            Matrix< DDRMat > tSidePhysCoords = this->get_space_side_phys_coords( aSpaceOrdinal );
////
////            // evaluation of tangent vectors to the space side in the physical space
////            Matrix< DDRMat > tRealTangents( mSideNumSpaceParamDim + mNumTimeDim, mNumSpaceDim + mNumTimeDim, 0.0 );
////            tRealTangents( { 0, mSideNumSpaceParamDim-1 }, { 0, mNumSpaceDim -1 } )
////                = trans( tdNSideSpacedXi ) * trans( tSidePhysCoords({ 0, mNumSpaceDim-1 },{ 0, mSideNumSpaceBases-1 }) );
//
//            // evaluation of tangent vectors to the space side in the physical space
//            Matrix< DDRMat > tRealTangents( mSideNumSpaceParamDim + mNumTimeDim, mNumSpaceDim + mNumTimeDim, 0.0 );
//            tRealTangents( { 0, mSideNumSpaceParamDim-1 }, { 0, mNumSpaceDim -1 } )
//                = trans( tdNSideSpacedXi ) * mSideXHat;
//            tRealTangents( { mSideNumSpaceParamDim + mNumTimeDim-1, mSideNumSpaceParamDim+ mNumTimeDim-1 },
//                           { mNumSpaceDim + mNumTimeDim-1         , mNumSpaceDim + mNumTimeDim-1 } )
//                = tdNTimedTau * mTHat;
//            tRealTangents = trans( tRealTangents );
//
//            // switch on geometry type
//            switch ( mGeometryType )
//            {
//                case ( mtk::Geometry_Type::QUAD ):
//                {
//                    // unpack the real tangent vectors
//                    Matrix< DDRMat > tTReal1 = tRealTangents.get_column( 0 );
//                    Matrix< DDRMat > tTReal2 = tRealTangents.get_column( 1 );
//
//                    // evaluate the surfDetJ from the real tangent vectors
//                    aSurfDetJ = norm( cross( tTReal1, tTReal2 ) );
//
//                    // computing the normal from the real tangent vectors
//                    aNormal = {{  tTReal1( 1 ) },
//                               { -tTReal1( 0 ) }};
//                    aNormal = aNormal / norm( aNormal );
//                    break;
//                }
//
//                case ( mtk::Geometry_Type::TRI ):
//                {
//                    // unpack the real tangent vectors
//                    Matrix< DDRMat > tTReal1 = tRealTangents.get_column( 0 );
//                    Matrix< DDRMat > tTReal2 = tRealTangents.get_column( 1 );
//
//                    // evaluate the surfDetJ from the real tangent vectors
//                    aSurfDetJ = norm( cross( tTReal1, tTReal2 ) );
//
//                    // computing the normal from the real tangent vectors
//                    aNormal = {{  tTReal1( 1 ) },
//                               { -tTReal1( 0 ) }};
//                    aNormal = aNormal / norm( aNormal );
//                    break;
//                }
//
//                case ( mtk::Geometry_Type::HEX ):
//                {
//                    // evaluate the surfDetJ from the real tangent vectors
//                    Matrix< DDRMat > tVector( 4, 1, 0.0 );
//                    for( uint i = 0; i < 4; i++ )
//                    {
//                        Matrix< DDRMat > tMatrixForDet( 4, 4, 0.0 );
//                        tMatrixForDet({ 1, 3 },{ 0, 3 }) = trans( tRealTangents );
//                        tMatrixForDet( 0, i ) = 1.0;
//                        //FIXME check that matrix is not singular before doing det()
//                        tVector( i ) = det( tMatrixForDet );
//                    }
//                    aSurfDetJ = norm( tVector );
//
//                    // unpack the real tangent vectors
//                    Matrix< DDRMat > tTReal1 = tRealTangents({ 0, 2 },{ 0, 0 });
//                    Matrix< DDRMat > tTReal2 = tRealTangents({ 0, 2 },{ 1, 1 });
//
//                    // FIXME compute the normal from the real tangent vectors
//                    aNormal = cross( tTReal1, tTReal2 );
//                    aNormal = aNormal / norm( aNormal );
//                    break;
//                }
//
//                case ( mtk::Geometry_Type::TET ):
//                    {
//                        // bring back the real tangent vectors in Cartesian coords
//                        Matrix< DDRMat > tRealTangentsCart( 4, 3, 0.0 );
//                        tRealTangentsCart.get_column( 0 ) = tRealTangents.get_column( 0 ) - tRealTangents.get_column( 2 );
//                        tRealTangentsCart.get_column( 1 ) = tRealTangents.get_column( 1 ) - tRealTangents.get_column( 2 );
//                        tRealTangentsCart.get_column( 2 ) = tRealTangents.get_column( 3 );
//
//                        // evaluate the surfDetJ from the real tangent vectors
//                        Matrix< DDRMat > tVector( 4, 1, 0.0 );
//                        for( uint i = 0; i < 4; i++ )
//                        {
//                            Matrix< DDRMat > tMatrixForDet( 4, 4, 0.0 );
//                            tMatrixForDet({ 1, 3 },{ 0, 3 }) = trans( tRealTangentsCart );
//                            tMatrixForDet( 0, i ) = 1.0;
//                            //FIXME check that matrix is not singular before doing det()
//                            tVector( i ) = det( tMatrixForDet );
//                        }
//                        aSurfDetJ = norm( tVector ) / 2.0;
//
//                        // unpack the real tangent vectors
//                        Matrix< DDRMat > tTReal1 = tRealTangentsCart({ 0, 2 },{ 0, 0 });
//                        Matrix< DDRMat > tTReal2 = tRealTangentsCart({ 0, 2 },{ 1, 1 });
//
//                        // FIXME computing the normal from the tangent vector in the physical space
//                        aNormal = cross( tTReal1, tTReal2 );
//                        aNormal = aNormal / norm( aNormal );
//                        break;
//                    }
//
//                default:
//                {
//                    MORIS_ERROR( false, "Geometry_Interpolator::surf_det_J - wrong geometry type or geometry type not implemented");
//                    break;
//                }
//            }
//        }

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */
