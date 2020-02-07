
#include "cl_FEM_IWG_Helmholtz_Bulk.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"
#include "cl_FEM_Set.hpp"
#include "fn_trans.hpp"

namespace moris
{
    namespace fem
    {
//------------------------------------------------------------------------------
        IWG_Helmholtz_Bulk::IWG_Helmholtz_Bulk()
        {
            //FIXME set the Helmholtz filter parameter
            mFilterParam = 1.0;

//            // set the residual dof type
//            mResidualDofType = { MSI::Dof_Type::VX };
//
//            // set the active dof type
//            mMasterDofTypes = {{ MSI::Dof_Type::VX }};
        }

//------------------------------------------------------------------------------
        void IWG_Helmholtz_Bulk::compute_residual( real tWStar )
        {
            //FIXME set unfiltered velocity values at nodes
            Matrix< DDRMat > tVHat  = mNodalWeakBCs;

            // set field interpolator
            Field_Interpolator* vN = mMasterFI( 0 );

            uint tDofIndex = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );

            // compute the residual
            mSet->get_residual()( 0 )( { mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 0 ), mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 1 ) }, { 0, 0 } )
                     += ( mFilterParam * trans( vN->dnNdxn( 1 ) ) * vN->gradx( 1 )
                           + trans( vN->N() ) * ( vN->val() - vN->N() * tVHat ) ) * tWStar;
        }

//------------------------------------------------------------------------------
        void IWG_Helmholtz_Bulk::compute_jacobian( real tWStar )
        {
            // set field interpolator
            Field_Interpolator* vN = mMasterFI( 0 );

            uint tDofIndex = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );

            // compute the jacobian
            mSet->get_jacobian()( { mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 0 ), mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 1 ) },
                                  { mSet->get_jac_dof_assembly_map()( tDofIndex )( tDofIndex, 0 ), mSet->get_jac_dof_assembly_map()( tDofIndex )( tDofIndex, 1 ) } )
                    += ( mFilterParam * trans( vN->dnNdxn( 1 ) ) * vN->dnNdxn( 1 )
                                 + trans( vN->N() ) * vN->N() ) * tWStar;
        }

//------------------------------------------------------------------------------
        void IWG_Helmholtz_Bulk::compute_jacobian_and_residual( moris::Cell< moris::Cell< Matrix< DDRMat > > > & aJacobians,
                                                                moris::Cell< Matrix< DDRMat > >                & aResidual )
        {
//            //FIXME set unfiltered velocity values at nodes
//            Matrix< DDRMat > tVHat  = mNodalWeakBCs;
//
//            // set field interpolator
//            Field_Interpolator* vN = mMasterFI( 0 );
//
//            // set residual size
//            this->set_residual( aResidual );
//
//            // compute the residual
//            aResidual( 0 ) = mFilterParam * trans( vN->dnNdxn( 1 ) ) * vN->gradx( 1 )
//                           + trans( vN->N() ) * ( vN->val() - vN->N() * tVHat );
//
//            // set the jacobian size
//            this->set_jacobian( aJacobians );
//
//            // compute the residual
//            aJacobians( 0 )( 0 ) = mFilterParam * trans( vN->dnNdxn( 1 ) ) * vN->dnNdxn( 1 )
//                                 + trans( vN->N() ) * vN->N();
        }

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */
