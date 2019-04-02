
#include "cl_FEM_IWG_Hamilton_Jacobi_Bulk_Test.hpp"
#include "fn_trans.hpp"

namespace moris
{
    namespace fem
    {

//------------------------------------------------------------------------------

        IWG_Hamilton_Jacobi_Bulk_Test::IWG_Hamilton_Jacobi_Bulk_Test()
        {
            // set the residual dof type
            mResidualDofType = { MSI::Dof_Type::LS1 };

            // set the active dof type
            mActiveDofTypes = {{ MSI::Dof_Type::LS1 }};

        }

//------------------------------------------------------------------------------

        void IWG_Hamilton_Jacobi_Bulk_Test::compute_residual( Matrix< DDRMat >            & aResidual,
                                                         Cell< Field_Interpolator* > & aFieldInterpolators )
        {
            // set field interpolators
            Field_Interpolator* phi = aFieldInterpolators( 0 );

            // velocity field value
            Matrix< DDRMat > aVN( 1, 3, 1.0 );

           //compute the residual
           aResidual = trans( phi->N() ) * ( phi->gradt( 1 ) + aVN * phi->gradx( 1 ) );
        }

//------------------------------------------------------------------------------

        void IWG_Hamilton_Jacobi_Bulk_Test::compute_jacobian( Cell< Matrix< DDRMat > >    & aJacobians,
                                                         Cell< Field_Interpolator* > & aFieldInterpolators )
        {
            // set field interpolators
            Field_Interpolator* phi = aFieldInterpolators( 0 );

            // velocity field value
            Matrix< DDRMat > aVN( 1, 3, 1.0 );

            // set the jacobian size
            aJacobians.resize( 1 );

            // compute the jacobian Jphiphi
            aJacobians( 0 ) = trans( phi->N() ) * ( phi->Bt() + aVN * phi->Bx() );

        }

//------------------------------------------------------------------------------

        void IWG_Hamilton_Jacobi_Bulk_Test::compute_jacobian_and_residual( Cell< Matrix< DDRMat > >    & aJacobians,
                                                                      Matrix< DDRMat >            & aResidual,
                                                                      Cell< Field_Interpolator* > & aFieldInterpolators )
        {
            // set field interpolators
            Field_Interpolator* phi = aFieldInterpolators( 0 );

            // velocity field value
            Matrix< DDRMat > aVN( 1, 3, 1.0 );

            //compute the residual
            aResidual = trans( phi->N() ) * ( phi->gradt( 1 ) + aVN * phi->gradx( 1 ) );

            // set the jacobian size
            aJacobians.resize( 1 );

            // compute the jacobian Jphiphi
            aJacobians( 0 ) = trans( phi->N() ) * ( phi->Bt() + aVN * phi->Bx() );
        }

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */