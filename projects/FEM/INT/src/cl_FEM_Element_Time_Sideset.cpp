#include <iostream>
#include "cl_FEM_Element_Time_Sideset.hpp" //FEM/INT/src
#include "cl_FEM_Integrator.hpp"           //FEM/INT/src

namespace moris
{
    namespace fem
    {
//------------------------------------------------------------------------------

        Element_Time_Sideset::Element_Time_Sideset( mtk::Cell                 * aCell,
                                                    moris::Cell< IWG* >       & aIWGs,
                                                    moris::Cell< Node_Base* > & aNodes )
                                                  : Element( aCell, aIWGs, aNodes )
        {
            //create a geometry interpolation rule
            //FIXME: forced interpolation type and order
            Interpolation_Rule tGeometryInterpolationRule( mCell->get_geometry_type(),
                                                           Interpolation_Type::LAGRANGE,
                                                           this->get_auto_interpolation_order(),
                                                           Interpolation_Type::LAGRANGE,
                                                           mtk::Interpolation_Order::LINEAR );

            // create a geometry intepolator
            mGeometryInterpolator = new Geometry_Interpolator( tGeometryInterpolationRule );

            // create field interpolators for the element
            mFieldInterpolators = this->create_field_interpolators( mGeometryInterpolator );

        }

//------------------------------------------------------------------------------

        Element_Time_Sideset::~Element_Time_Sideset()
        {
            // delete the geometry interpolator pointer
            if ( mGeometryInterpolator != NULL )
            {
                delete mGeometryInterpolator;
            }

            // delete the field interpolator pointers
            for ( uint i = 0; i < mNumOfInterp; i++ )
            {
                if ( mFieldInterpolators( i ) != NULL )
                {
                    delete mFieldInterpolators( i );
                }
            }
        }

//------------------------------------------------------------------------------

        void Element_Time_Sideset::compute_residual()
        {
            // get the number of time ordinals
            uint tNumOfSideSets = mListOfTimeOrdinals.numel();

            // initialize mJacobianElement and mResidualElement
            this->initialize_mJacobianElement_and_mResidualElement( mFieldInterpolators );

            // get pdofs values for the element
            this->get_my_pdof_values();

            // set the field interpolators coefficients
            this->set_field_interpolators_coefficients( mFieldInterpolators );

            // set the geometry interpolator coefficients
            //FIXME: tHat are set by default but should come from solver
            Matrix< DDRMat > tTHat = { {0.0}, {1.0} };
            mGeometryInterpolator->set_coeff( mCell->get_vertex_coords(), tTHat );

            // loop over the sideset faces
            for ( uint iSideset = 0; iSideset < tNumOfSideSets; iSideset++ )
            {
                // get the treated time ordinal
                moris_index tTreatedTimeOrdinal = mListOfTimeOrdinals( iSideset );

                // loop over the IWGs
                for( uint iIWG = 0; iIWG < mNumOfIWGs; iIWG++ )
                {
                    // get the treated IWG
                    IWG* tTreatedIWG = mIWGs( iIWG );

                    // get the index of the residual dof type for the ith IWG in the list of element dof type
                    uint tIWGResDofIndex
                        = mInterpDofTypeMap( static_cast< int >( tTreatedIWG->get_residual_dof_type()( 0 ) ) );

                    Cell< Cell< MSI::Dof_Type > > tIWGActiveDofType = tTreatedIWG->get_active_dof_types();
                    uint tNumOfIWGActiveDof = tIWGActiveDofType.size();

                    // get the field interpolators for the ith IWG in the list of element dof type
                    Cell< Field_Interpolator* > tIWGInterpolators
                        = this->get_IWG_field_interpolators( tTreatedIWG, mFieldInterpolators );

                    // create an integration rule for the ith IWG
                    //FIXME: set by default
                    Integration_Rule tIntegrationRule( mCell->get_geometry_type(),
                                                       Integration_Type::GAUSS,
                                                       this->get_auto_integration_order( mCell->get_geometry_type() ),
                                                       Integration_Type::GAUSS,
                                                       Integration_Order::BAR_1 );

                    // create an integrator for the ith IWG
                    Integrator tIntegrator( tIntegrationRule );

                    //get number of integration points
                    uint tNumOfIntegPoints = tIntegrator.get_number_of_points();

                    // get integration points
                    Matrix< DDRMat > tSurfRefIntegPoints = tIntegrator.get_points();

                   // get integration weights
                   Matrix< DDRMat > tIntegWeights = tIntegrator.get_weights();

                   for( uint iGP = 0; iGP < tNumOfIntegPoints; iGP++ )
                   {
                       // get integration point location in the reference surface
                       Matrix< DDRMat > tSurfRefIntegPointI = tSurfRefIntegPoints.get_column( iGP );

                       // get integration point location in the reference volume
                       Matrix< DDRMat > tVolRefIntegPointI
                           = mGeometryInterpolator->time_surf_val( tSurfRefIntegPointI,
                                                                   tTreatedTimeOrdinal );

                       // set integration point
                       for ( uint iIWGFI = 0; iIWGFI < tNumOfIWGActiveDof; iIWGFI++ )
                       {
                           tIWGInterpolators( iIWGFI )->set_space_time( tVolRefIntegPointI );
                       }

                       // compute integration point weight x detJ
                       real tSurfDetJ;
                       mGeometryInterpolator->time_surf_det_J( tSurfDetJ,
                                                               tSurfRefIntegPointI,
                                                               tTreatedTimeOrdinal );
                       real tWStar = tIntegWeights( iGP ) * tSurfDetJ;

                       // compute jacobian at evaluation point
                       Matrix< DDRMat > tResidual;
                       tTreatedIWG->compute_residual( tResidual, tIWGInterpolators );

                       // add contribution to jacobian from evaluation point
                       mResidualElement( tIWGResDofIndex )
                           = mResidualElement( tIWGResDofIndex ) + tResidual * tWStar;
                   }
                }
            }
            // residual assembly
            uint tCounterI = 0;
            uint startI, stopI;

            // loop over the field interpolators
            for ( uint iBuild = 0; iBuild < mNumOfInterp; iBuild++ )
            {
                // get the row position in the residual matrix
                startI = tCounterI;
                stopI  = tCounterI + mFieldInterpolators( iBuild )->get_number_of_space_time_coefficients() - 1;

                // fill the global residual
                mResidual( { startI, stopI }, { 0 , 0 } ) = mResidualElement( iBuild ).matrix_data();

                // update the row counter
                tCounterI = stopI + 1;
            }
        }

//------------------------------------------------------------------------------

        void Element_Time_Sideset::compute_jacobian()
        {
            // get the number of time ordinal
            uint tNumOfSideSets = mListOfTimeOrdinals.numel();

            // initialize mJacobianElement and mResidualElement
            this->initialize_mJacobianElement_and_mResidualElement( mFieldInterpolators );

            // get pdofs values for the element
            this->get_my_pdof_values();

            // set the field interpolators coefficients
            this->set_field_interpolators_coefficients( mFieldInterpolators );

            // set the geometry interpolator coefficients
            //FIXME: tHat are set by default but should come from solver
            Matrix< DDRMat > tTHat = { {0.0}, {1.0} };
            mGeometryInterpolator->set_coeff( mCell->get_vertex_coords(), tTHat );

            // loop over the sideset faces
            for ( uint iSideset = 0; iSideset < tNumOfSideSets; iSideset++ )
            {
                // get the treated time ordinal
                moris_index tTreatedTimeOrdinal = mListOfTimeOrdinals( iSideset );

                // loop over the IWGs
                for( uint iIWG = 0; iIWG < mNumOfIWGs; iIWG++ )
                {
                    // get the treated IWG
                    IWG* tTreatedIWG = mIWGs( iIWG );

                    // get the index of the residual dof type for the ith IWG in the list of element dof type
                    uint tIWGResDofIndex
                        = mInterpDofTypeMap( static_cast< int >( tTreatedIWG->get_residual_dof_type()( 0 ) ) );

                    Cell< Cell< MSI::Dof_Type > > tIWGActiveDofType = tTreatedIWG->get_active_dof_types();
                    uint tNumOfIWGActiveDof = tIWGActiveDofType.size();

                    // get the field interpolators for the ith IWG in the list of element dof type
                    Cell< Field_Interpolator* > tIWGInterpolators
                        = this->get_IWG_field_interpolators( tTreatedIWG, mFieldInterpolators );

                    // create an integration rule for the ith IWG
                    //FIXME: set by default
                    Integration_Rule tIntegrationRule( mCell->get_geometry_type(),
                                                       Integration_Type::GAUSS,
                                                       this->get_auto_integration_order( mCell->get_geometry_type() ),
                                                       Integration_Type::GAUSS,
                                                       Integration_Order::BAR_1 );

                    // create an integrator for the ith IWG
                    Integrator tIntegrator( tIntegrationRule );

                    //get number of integration points
                    uint tNumOfIntegPoints = tIntegrator.get_number_of_points();

                    // get integration points
                    Matrix< DDRMat > tSurfRefIntegPoints = tIntegrator.get_points();

                   // get integration weights
                   Matrix< DDRMat > tIntegWeights = tIntegrator.get_weights();

                   // loop over the integration points
                   for( uint iGP = 0; iGP < tNumOfIntegPoints; iGP++ )
                   {
                       // get integration point location in the reference surface
                       Matrix< DDRMat > tSurfRefIntegPointI = tSurfRefIntegPoints.get_column( iGP );

                       // get integration point location in the reference volume
                       Matrix< DDRMat > tVolRefIntegPointI
                           = mGeometryInterpolator->time_surf_val( tSurfRefIntegPointI,
                                                                   tTreatedTimeOrdinal );

                       // set integration point
                       for ( uint iIWGFI = 0; iIWGFI < tNumOfIWGActiveDof; iIWGFI++ )
                       {
                           tIWGInterpolators( iIWGFI )->set_space_time( tVolRefIntegPointI );
                       }

                       // compute integration point weight x detJ
                       real tSurfDetJ;
                       mGeometryInterpolator->time_surf_det_J( tSurfDetJ,
                                                               tSurfRefIntegPointI,
                                                               tTreatedTimeOrdinal );
                       real tWStar = tIntegWeights( iGP ) * tSurfDetJ;

                       // compute jacobian at evaluation point
                       moris::Cell< Matrix< DDRMat > > tJacobians;
                       tTreatedIWG->compute_jacobian( tJacobians, tIWGInterpolators );

                       // add contribution to jacobian from evaluation point
                       for ( uint iIWGFI = 0; iIWGFI < tNumOfIWGActiveDof; iIWGFI++)
                       {
                           uint tIWGActiveDofIndex
                               = mInterpDofTypeMap( static_cast< int >( tIWGActiveDofType( iIWGFI )( 0 ) ) );

                           uint tJacIndex
                               = tIWGResDofIndex * mNumOfInterp + tIWGActiveDofIndex;

                           mJacobianElement( tJacIndex )
                               = mJacobianElement( tJacIndex ) + tWStar * tJacobians( iIWGFI );
                       }
                   }
                }
            }
            // jacobian assembly
            uint tCounterI = 0;
            uint tCounterJ = 0;
            uint startI, stopI, startJ, stopJ;

            for ( uint i = 0; i < mNumOfInterp; i++ )
            {
                startI = tCounterI;
                stopI  = tCounterI + mFieldInterpolators( i )->get_number_of_space_time_coefficients() - 1;

                tCounterJ = 0;
                for ( uint j = 0; j < mNumOfInterp; j++ )
                {
                    startJ = tCounterJ;
                    stopJ  = tCounterJ + mFieldInterpolators( j )->get_number_of_space_time_coefficients() - 1;

                    mJacobian({ startI, stopI },{ startJ, stopJ }) = mJacobianElement( i * mNumOfInterp + j ).matrix_data();

                    tCounterJ = stopJ + 1;
                }
                tCounterI = stopI + 1;
            }
        }

//------------------------------------------------------------------------------

        void Element_Time_Sideset::compute_jacobian_and_residual()
        {
            MORIS_ERROR( false, " Element_Time_Sideset::compute_jacobian_and_residual - not implemented. ");
        }

//------------------------------------------------------------------------------

//        moris::Cell< fem::Field_Interpolator* >
//        Element_Time_Sideset::create_field_interpolators
//        ( fem::Geometry_Interpolator* aGeometryInterpolator )
//        {
//            // cell of field interpolators
//            Cell< Field_Interpolator* > tFieldInterpolators( mNumOfInterp, nullptr );
//
//            // loop on the dof type groups and create a field interpolator for each
//            for( uint i = 0; i < mNumOfInterp; i++ )
//            {
//                // get the ith dof type group
//                Cell< MSI::Dof_Type > tDofTypeGroup = mInterpDofTypeList( i );
//
//                // create the field interpolation rule for the ith dof type group
//                //FIXME: space interpolation based on the mtk::Cell
//                //FIXME: time  interpolation set to constant
//                Interpolation_Rule tFieldInterpolationRule( mCell->get_geometry_type(),
//                                                            Interpolation_Type::LAGRANGE,
//                                                            this->get_auto_interpolation_order(),
//                                                            Interpolation_Type::LAGRANGE,
//                                                            mtk::Interpolation_Order::LINEAR );
//
//                // get number of field interpolated by the ith field interpolator
//                uint tNumOfFields = tDofTypeGroup.size();
//
//                // create an interpolator for the ith dof type group
//                tFieldInterpolators( i ) = new Field_Interpolator( tNumOfFields,
//                                                                   tFieldInterpolationRule,
//                                                                   aGeometryInterpolator );
//            }
//            return tFieldInterpolators;
//        }

//------------------------------------------------------------------------------

//        void Element_Time_Sideset::set_field_interpolators_coefficients
//            ( moris::Cell< Field_Interpolator* > & aFieldInterpolators )
//        {
//            // loop on the dof types
//            for( uint i = 0; i < mNumOfInterp; i++ )
//            {
//                // get the ith dof type group
//                Cell< MSI::Dof_Type > tDofTypeGroup = mInterpDofTypeList( i );
//
//                //FIXME:forced coefficients
//                // get the pdof values for the ith dof type group
//                Matrix< DDRMat > tCoeff = this->get_my_pdof_values( tDofTypeGroup, tCoeff );
//
//                // set the field coefficients
//                aFieldInterpolators( i )->set_coeff( tCoeff );
//            }
//        }
//

//------------------------------------------------------------------------------

    } /* namespace fem */
} /* namespace moris */