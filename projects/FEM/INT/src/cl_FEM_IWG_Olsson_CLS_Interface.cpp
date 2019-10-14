
#include "cl_FEM_IWG_Olsson_CLS_Interface.hpp"

#include "fn_trans.hpp"
#include "fn_dot.hpp"

namespace moris
{
    namespace fem
    {
//------------------------------------------------------------------------------
        IWG_Olsson_CLS_Interface::IWG_Olsson_CLS_Interface()
        {
            //FIXME set field upper and lower bound
            mPhiUB = 1.0;
            mPhiLB = 0.0;

            //FIXME set Olsson CLS epsilon parameter
            mEpsilon = 1.0;

            // set the residual dof type
            //FIXME: level set scalar field not UX
            mResidualDofType = { MSI::Dof_Type::LS2 };

            // set the active dof type
            //FIXME: level set scalar field not UX
            //       level set normal field not UY
            mMasterDofTypes = {{ MSI::Dof_Type::LS2 },
                               { MSI::Dof_Type::NLSX, MSI::Dof_Type::NLSY, MSI::Dof_Type::NLSZ } };

        }

//------------------------------------------------------------------------------
        void IWG_Olsson_CLS_Interface::compute_residual( moris::Cell< Matrix< DDRMat > > & aResidual )
        {
            // check master field interpolators
            this->check_field_interpolators( mtk::Master_Slave::MASTER );

            // set field interpolators
            Field_Interpolator* phi  = mMasterFI( 0 );
            Field_Interpolator* nPhi = mMasterFI( 1 );

            //FIXME set the interface normal
            Matrix< DDRMat > aInterfaceNormal( phi->gradx( 1 ).n_cols() , 1, 1.0 );

            // set residual size
            this->set_residual( aResidual );

            //compute the residual
            aResidual( 0 ) = trans( phi->N() )
                           * ( ( phi->val()( 0 ) - mPhiLB ) * ( mPhiUB - phi->val()( 0 ) ) - mEpsilon * dot( phi->gradx( 1 ), nPhi->val() ) )
                           * trans( nPhi->val() ) * aInterfaceNormal;
        }

//------------------------------------------------------------------------------

        void IWG_Olsson_CLS_Interface::compute_jacobian( moris::Cell< moris::Cell< Matrix< DDRMat > > > & aJacobians )
        {
            // check master field interpolators
            this->check_field_interpolators( mtk::Master_Slave::MASTER );

            // set field interpolators
            Field_Interpolator* phi  = mMasterFI( 0 );
            Field_Interpolator* nPhi = mMasterFI( 1 );

            //FIXME set the interface normal
            Matrix< DDRMat > aInterfaceNormal( phi->gradx( 1 ).n_cols() , 1, 1.0 );

            // set the jacobian size
            this->set_jacobian( aJacobians );

            // compute the jacobian
            aJacobians( 0 )( 0 ) = trans( phi->N() )
                                 * ( ( mPhiLB + mPhiUB - 2 * phi->val()( 0 ) ) * phi->N() - mEpsilon * trans( nPhi->val() ) * phi->Bx() )
                                 * dot( nPhi->val(), aInterfaceNormal ) ;

            aJacobians( 0 )( 1 ) = trans( phi->N() ) * ( ( ( phi->val()( 0 ) - mPhiLB ) * ( mPhiUB - phi->val()( 0 ) )
                                 - 2 * mEpsilon * dot( phi->gradx( 1 ), nPhi->val() ) ) * nPhi->N() ) * aInterfaceNormal;
        }

//------------------------------------------------------------------------------

        void IWG_Olsson_CLS_Interface::compute_jacobian_and_residual( moris::Cell< moris::Cell< Matrix< DDRMat > > > & aJacobians,
                                                                      moris::Cell< Matrix< DDRMat > >                & aResidual )
        {
            // check master field interpolators
            this->check_field_interpolators( mtk::Master_Slave::MASTER );

            // set field interpolators
            Field_Interpolator* phi  = mMasterFI( 0 );
            Field_Interpolator* nPhi = mMasterFI( 1 );

            //FIXME set the interface normal
            Matrix< DDRMat > aInterfaceNormal( phi->gradx( 1 ).n_cols() , 1, 1.0 );

            // set residual size
            this->set_residual( aResidual );

            //compute the residual
            aResidual( 0 ) = trans( phi->N() )
                           * ( ( phi->val()( 0 ) - mPhiLB ) * ( mPhiUB - phi->val()( 0 ) ) - mEpsilon * dot( phi->gradx( 1 ), nPhi->val() ) )
                           * trans( nPhi->val() ) * aInterfaceNormal;

            // set the jacobian size
            this->set_jacobian( aJacobians );

            // compute the jacobian
            aJacobians( 0 )( 0 ) = phi->N()
                                 * ( ( mPhiLB + mPhiUB - 2 * phi->val()( 0 ) ) * phi->N() - mEpsilon * trans( nPhi->val() ) * phi->Bx() )
                                 * dot( nPhi->val(), aInterfaceNormal ) ;

            aJacobians( 0 )( 1 ) = trans( phi->N() ) * ( ( ( phi->val()( 0 ) - mPhiLB ) * ( mPhiUB - phi->val()( 0 ) )
                                 - 2 * mEpsilon * dot( phi->gradx( 1 ), nPhi->val() ) ) * nPhi->N() ) * aInterfaceNormal;
        }

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */
