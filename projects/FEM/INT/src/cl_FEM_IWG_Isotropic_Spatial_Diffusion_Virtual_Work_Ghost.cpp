
#include "cl_FEM_IWG_Isotropic_Spatial_Diffusion_Virtual_Work_Ghost.hpp"
#include "fn_trans.hpp"
#include "fn_eye.hpp"
#include "fn_dot.hpp"

namespace moris
{
    namespace fem
    {
//------------------------------------------------------------------------------
        IWG_Isotropic_Spatial_Diffusion_Virtual_Work_Ghost::IWG_Isotropic_Spatial_Diffusion_Virtual_Work_Ghost()
        {
            // FIXME set penalty from outside
            // penalty parameter
            mGammaGhost = 1.0;

            // FIXME set mesh parameter from outside
            // mesh parameter
            mMeshParameter = 1.0;

            // FIXME set order from outside
            // interpolation order
            mOrder = 1;
        }

//------------------------------------------------------------------------------
        void IWG_Isotropic_Spatial_Diffusion_Virtual_Work_Ghost::compute_residual( moris::Cell< Matrix< DDRMat > > & aResidual )
        {
            // check, if order is supported
            MORIS_ERROR( mOrder < 4, " IWG_Isotropic_Spatial_Diffusion_Virtual_Work_Ghost::compute_residual"
                                     " - order not supported. " );

            // check master and slave field interpolators
            this->check_dof_field_interpolators( mtk::Master_Slave::MASTER );
            this->check_dof_field_interpolators( mtk::Master_Slave::SLAVE );
            this->check_dv_field_interpolators( mtk::Master_Slave::MASTER );
            this->check_dv_field_interpolators( mtk::Master_Slave::SLAVE );

            // check master and slave properties
            this->check_properties( mtk::Master_Slave::MASTER );
            this->check_properties( mtk::Master_Slave::SLAVE );

            // check master and slave constitutive models
            this->check_constitutive_models( mtk::Master_Slave::MASTER );
            this->check_constitutive_models( mtk::Master_Slave::SLAVE );

            // set residual cell size
            this->set_residual_double( aResidual );

            // loop over the interpolation order
            for ( uint iOrder = 1; iOrder <= mOrder; iOrder++ )
            {
                 // get flattened normal matrix
                 Matrix< DDRMat > tNormalMatrix = this->get_normal_matrix( iOrder );

                 // penalty parameter
                 real tGhostPenalty = mGammaGhost * std::pow( mMeshParameter, 2 * ( iOrder - 1 ) + 1 );

                 // premultiply common terms
                 Matrix< DDRMat > tPreMultiply = tGhostPenalty * trans( tNormalMatrix ) * tNormalMatrix * ( mMasterCM( 0 )->flux() - mSlaveCM( 0 )->flux() );

                 // compute master and slave residuals
                 aResidual( 0 ).matrix_data() +=   trans( mMasterFI( 0 )->dnNdxn( iOrder ) ) * tPreMultiply;
                 aResidual( 1 ).matrix_data() += - trans( mSlaveFI( 0 )->dnNdxn( iOrder ) )  * tPreMultiply;
            }
        }

//------------------------------------------------------------------------------
        void IWG_Isotropic_Spatial_Diffusion_Virtual_Work_Ghost::compute_jacobian( moris::Cell< moris::Cell< Matrix< DDRMat > > > & aJacobians )
        {
            // check, if order is supported
            MORIS_ERROR( mOrder < 4, " IWG_Isotropic_Spatial_Diffusion_Ghost::compute_jacobian - Ghost stabilization for this order not supported yet. " );

            // check master and slave field interpolators
            this->check_dof_field_interpolators( mtk::Master_Slave::MASTER );
            this->check_dof_field_interpolators( mtk::Master_Slave::SLAVE );
            this->check_dv_field_interpolators( mtk::Master_Slave::MASTER );
            this->check_dv_field_interpolators( mtk::Master_Slave::SLAVE );

            // check master and slave properties
            this->check_properties( mtk::Master_Slave::MASTER );
            this->check_properties( mtk::Master_Slave::SLAVE );

            // check master and slave constitutive models
            this->check_constitutive_models( mtk::Master_Slave::MASTER );
            this->check_constitutive_models( mtk::Master_Slave::SLAVE );

            // set the jacobian cell size
            this->set_jacobian_double( aJacobians );

            // loop over the interpolation orders
            for ( uint iOrder = 1; iOrder <= mOrder; iOrder++ )
            {
                // get normal matrix
                Matrix< DDRMat > tNormalMatrix = this->get_normal_matrix( iOrder );

                // penalty parameter
                real tGhostPenalty = mGammaGhost * std::pow( mMeshParameter, 2 * ( iOrder - 1 ) + 1 );

                // premultiply common terms
                Matrix< DDRMat > tPreMultiply = tGhostPenalty * trans( tNormalMatrix ) * tNormalMatrix;

                // compute Jacobian direct dependencies
                aJacobians( 0 )( 0 ).matrix_data()
                +=   trans( mMasterFI( 0 )->dnNdxn( iOrder ) ) * tPreMultiply * mMasterCM( 0 )->constitutive() * mMasterFI( 0 )->dnNdxn( iOrder );
                aJacobians( 0 )( 1 ).matrix_data()
                += - trans( mMasterFI( 0 )->dnNdxn( iOrder ) ) * tPreMultiply * mSlaveCM( 0 )->constitutive() * mSlaveFI( 0 )->dnNdxn( iOrder );

                aJacobians( 1 )( 0 ).matrix_data()
                += - trans( mSlaveFI( 0 )->dnNdxn( iOrder ) ) * tPreMultiply * mMasterCM( 0 )->constitutive() * mMasterFI( 0 )->dnNdxn( iOrder );
                aJacobians( 1 )( 1 ).matrix_data()
                +=   trans( mSlaveFI( 0 )->dnNdxn( iOrder ) ) * tPreMultiply * mSlaveCM( 0 )->constitutive() * mSlaveFI( 0 )->dnNdxn( iOrder );

                // compute the jacobian for indirect dof dependencies through master constitutive models
                uint tMasterNumDofDependencies = mMasterGlobalDofTypes.size();
                for( uint iDOF = 0; iDOF < tMasterNumDofDependencies; iDOF++ )
                {
                    // get the dof type
                    Cell< MSI::Dof_Type > tDofType = mMasterGlobalDofTypes( iDOF );

                    // if dependency on the dof type
                    if ( mMasterCM( 0 )->check_dof_dependency( tDofType ) )
                    {
                        // add contribution to jacobian
                        aJacobians( 0 )( iDOF ).matrix_data()
                        +=   trans( mMasterFI( 0 )->dnNdxn( iOrder ) ) * tPreMultiply * mMasterFI( 0 )->gradx( iOrder ) * mMasterCM( 0 )->dConstdDOF( tDofType );

                        aJacobians( 1 )( iDOF ).matrix_data()
                        += - trans( mSlaveFI( 0 )->dnNdxn( iOrder ) ) * tPreMultiply * mMasterFI( 0 )->gradx( iOrder ) * mMasterCM( 0 )->dConstdDOF( tDofType );
                    }
                }

                // compute the jacobian for indirect dof dependencies through slave constitutive models
                uint tSlaveNumDofDependencies = mSlaveGlobalDofTypes.size();
                for( uint iDOF = 0; iDOF < tSlaveNumDofDependencies; iDOF++ )
                {
                    // get dof type
                    Cell< MSI::Dof_Type > tDofType = mSlaveGlobalDofTypes( iDOF );

                    // if dependency on the dof type
                    if ( mSlaveCM( 0 )->check_dof_dependency( tDofType ) )
                    {
                        // add contribution to jacobian
                        aJacobians( 0 )( tMasterNumDofDependencies + iDOF ).matrix_data()
                        += - trans( mMasterFI( 0 )->dnNdxn( iOrder ) ) * tPreMultiply * mSlaveFI( 0 )->gradx( iOrder ) * mSlaveCM( 0 )->dConstdDOF( tDofType );

                        aJacobians( 1 )( tMasterNumDofDependencies + iDOF ).matrix_data()
                        +=   trans( mSlaveFI( 0 )->dnNdxn( iOrder ) ) * tPreMultiply * mSlaveFI( 0 )->gradx( iOrder ) * mSlaveCM( 0 )->dConstdDOF( tDofType );
                    }
                }
            }
        }

//------------------------------------------------------------------------------
        void IWG_Isotropic_Spatial_Diffusion_Virtual_Work_Ghost::compute_jacobian_and_residual( moris::Cell< moris::Cell< Matrix< DDRMat > > > & aJacobians,
                                                                                                moris::Cell< Matrix< DDRMat > >                & aResidual )
        {
            MORIS_ERROR( false, "IWG_Isotropic_Spatial_Diffusion_Virtual_Work_Ghost::compute_jacobian_and_residual - Not implemented." );
        }

//------------------------------------------------------------------------------
        Matrix< DDRMat > IWG_Isotropic_Spatial_Diffusion_Virtual_Work_Ghost::get_normal_matrix ( uint aOrderGhost )
        {
            // init the normal matrix
            Matrix< DDRMat > tNormalMatrix;

            // get spatial dimensions
            uint tSpaceDim = mNormal.numel();

            // switch on the ghost order
            switch( aOrderGhost )
            {
                case ( 1 ):
                {
                    switch ( tSpaceDim )
                    {
                        case ( 2 ):
                        {
                            tNormalMatrix = trans( mNormal );
                            break;
                        }
                        case ( 3 ):
                        {
                            tNormalMatrix = trans( mNormal );
                            break;
                        }
                        default:
                        {
                            MORIS_ERROR( false, "IWG_Isotropic_Spatial_Diffusion_Virtual_Work_Ghost::get_normal_matrix - Spatial dimensions can only be 2, 3." );
                            break;
                        }
                    }
                    break;
                }

                case ( 2 ):
                {
                    switch ( tSpaceDim )
                    {
                        case ( 2 ):
                        {
                            // set the normal matrix size
                            tNormalMatrix.set_size( 2, 3, 0.0 );

                            // fill the normal matrix
                            tNormalMatrix( 0, 0 ) = mNormal( 0 );
                            tNormalMatrix( 1, 1 ) = mNormal( 1 );

                            tNormalMatrix( 0, 2 ) = mNormal( 1 );
                            tNormalMatrix( 1, 2 ) = mNormal( 0 );

                            break;
                        }
                        case ( 3 ):
                        {
                            // set the normal matrix size
                            tNormalMatrix.set_size( 3, 6, 0.0 );

                            // fill the normal matrix
                            tNormalMatrix( 0, 0 ) = mNormal( 0 );
                            tNormalMatrix( 1, 1 ) = mNormal( 1 );
                            tNormalMatrix( 2, 2 ) = mNormal( 2 );

                            tNormalMatrix( 1, 3 ) = mNormal( 2 );
                            tNormalMatrix( 2, 3 ) = mNormal( 1 );

                            tNormalMatrix( 0, 4 ) = mNormal( 2 );
                            tNormalMatrix( 2, 4 ) = mNormal( 0 );

                            tNormalMatrix( 0, 5 ) = mNormal( 1 );
                            tNormalMatrix( 1, 5 ) = mNormal( 0 );

                            break;
                        }
                        default:
                        {
                            MORIS_ERROR( false, "IWG_Isotropic_Spatial_Diffusion_Virtual_Work_Ghost::get_normal_matrix - Spatial dimensions can only be 2, 3." );
                            break;
                        }
                    }
                    break;
                }

                case ( 3 ):
                {
                    switch ( tSpaceDim )
                    {
                        case ( 2 ):
                        {
                            // set the normal matrix size
                            tNormalMatrix.set_size( 3, 4, 0.0 );

                            tNormalMatrix( 0, 0 ) = mNormal( 0 );
                            tNormalMatrix( 1, 1 ) = mNormal( 1 );

                            tNormalMatrix( 0, 2 ) = mNormal( 1 );
                            tNormalMatrix( 1, 3 ) = mNormal( 0 );

                            real tSqrtOf2 = std::sqrt( 2 );

                            tNormalMatrix( 2, 2 ) = tSqrtOf2 * mNormal( 0 );
                            tNormalMatrix( 2, 3 ) = tSqrtOf2 * mNormal( 1 );

                            break;
                        }
                        case ( 3 ):
                        {
                            // set the normal matrix size
                            tNormalMatrix.set_size( 6, 10, 0.0 );

                            tNormalMatrix( 0, 0 ) = mNormal( 0 );
                            tNormalMatrix( 1, 1 ) = mNormal( 1 );
                            tNormalMatrix( 2, 2 ) = mNormal( 2 );

                            tNormalMatrix( 0, 3 ) = mNormal( 1 );
                            tNormalMatrix( 0, 4 ) = mNormal( 2 );

                            tNormalMatrix( 1, 5 ) = mNormal( 0 );
                            tNormalMatrix( 1, 6 ) = mNormal( 2 );

                            tNormalMatrix( 2, 7 ) = mNormal( 0 );
                            tNormalMatrix( 2, 8 ) = mNormal( 1 );

                            real tSqrtOf2 = std::sqrt( 2 );

                            tNormalMatrix( 3, 3 ) = tSqrtOf2 * mNormal( 0 );
                            tNormalMatrix( 3, 5 ) = tSqrtOf2 * mNormal( 1 );
                            tNormalMatrix( 3, 9 ) = tSqrtOf2 * mNormal( 2 );

                            tNormalMatrix( 4, 6 ) = tSqrtOf2 * mNormal( 1 );
                            tNormalMatrix( 4, 8 ) = tSqrtOf2 * mNormal( 2 );
                            tNormalMatrix( 4, 9 ) = tSqrtOf2 * mNormal( 0 );

                            tNormalMatrix( 5, 4 ) = tSqrtOf2 * mNormal( 0 );
                            tNormalMatrix( 5, 7 ) = tSqrtOf2 * mNormal( 2 );
                            tNormalMatrix( 5, 9 ) = tSqrtOf2 * mNormal( 1 );

                            break;
                        }
                        default:
                        {
                            MORIS_ERROR( false, "IWG_Isotropic_Spatial_Diffusion_Virtual_Work_Ghost::get_normal_matrix - Spatial dimensions can only be 2, 3." );
                            break;
                        }
                    }
                    break;
                }

                default:
                {
                    MORIS_ERROR( false, "IWG_Isotropic_Spatial_Diffusion_Virtual_Work_Ghost::get_normal_matrix - order not supported." );
                    break;
                }
            }
            return tNormalMatrix;

        }

//------------------------------------------------------------------------------
        void IWG_Isotropic_Spatial_Diffusion_Virtual_Work_Ghost::set_interpolation_order( uint aOrder )
        {
            // check that the order is supported
            MORIS_ERROR( aOrder < 4 , "IWG_Isotropic_Spatial_Diffusion_Virtual_Work_Ghost::set_interpolation_order - order not supported." );

            // set the order
            mOrder = aOrder;
        }

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */