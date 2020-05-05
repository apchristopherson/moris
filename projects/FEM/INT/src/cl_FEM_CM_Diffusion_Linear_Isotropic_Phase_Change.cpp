
#include "cl_FEM_CM_Diffusion_Linear_Isotropic_Phase_Change.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"

#include "fn_trans.hpp"
#include "fn_norm.hpp"
#include "fn_eye.hpp"
#include "fn_dot.hpp"
#include "fn_sum.hpp"

namespace moris
{
    namespace fem
    {

//------------------------------------------------------------------------------
        CM_Diffusion_Linear_Isotropic_Phase_Change::CM_Diffusion_Linear_Isotropic_Phase_Change()
        {
            // set the property pointer cell size
            mProperties.resize( static_cast< uint >( Property_Type::MAX_ENUM ), nullptr );

            // populate the property map
            mPropertyMap[ "Conductivity" ]       = Property_Type::CONDUCTIVITY;
            mPropertyMap[ "Density" ]            = Property_Type::DENSITY;
            mPropertyMap[ "Heat_Capacity" ]      = Property_Type::HEAT_CAPACITY;
            mPropertyMap[ "Lower_PC_Temp" ]      = Property_Type::LOWER_PC_TEMP;
            mPropertyMap[ "Upper_PC_Temp" ]      = Property_Type::UPPER_PC_TEMP;
            mPropertyMap[ "Phase_Change_Const" ] = Property_Type::PHASE_CHANGE_CONST;

            // populate dof map
            mDofMap[ "Temp" ] = MSI::Dof_Type::TEMP;
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_Hdot()
        {
            // get properties
            moris::real tDensity = mProperties( static_cast< uint >( Property_Type::DENSITY ) )->val()( 0 );
            moris::real tHeatCap = mProperties( static_cast< uint >( Property_Type::HEAT_CAPACITY ) )->val()( 0 );
            moris::real tTlower  = mProperties( static_cast< uint >( Property_Type::LOWER_PC_TEMP ) )->val()( 0 );
            moris::real tTupper  = mProperties( static_cast< uint >( Property_Type::UPPER_PC_TEMP ) )->val()( 0 );
            moris::real tPCconst = mProperties( static_cast< uint >( Property_Type::PHASE_CHANGE_CONST ) )->val()( 0 );

            // check inputs
            MORIS_ASSERT(tPCconst >= 0.0,
                    "CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dPCfuncdT: Phase change constant must be >= 0.  " );


            // initialize value of derivative of Phase State Function
            real tdfdT = 0.0;

            // get temperature
            real tTemp = mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->val()( 0 );

            // if phase change constant is set to zero, use linear phase change model
            if (tPCconst == 0.0 )
            {
                if ( (tTemp > tTupper) || (tTemp < tTlower) )
                    tdfdT = 0;
                else
                    tdfdT = 1 / (tTupper - tTlower);
            }

            // logistic phase change model
            else
            {
                // logistic function parameter k
                real tLogisticParam = ( 2 * std::log(1/tPCconst - 1) ) / ( tTupper - 3 * tTlower );

                // compute df/dT
                real tExp = std::exp(  tLogisticParam * ( tTemp - (tTupper + tTlower)/2 ) );
                tdfdT = ( tLogisticParam * tExp )  / std::pow( 1 + tExp , 2 );
            }

            // compute derivative of enthalpy
            mHdot = tDensity * ( tHeatCap + tdfdT )
                                  * mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->gradt( 1 );
        }


//------------------------------------------------------------------------------
//        const Matrix< DDRMat > & CM_Diffusion_Linear_Isotropic_Phase_Change::Hdot()
//        {
//            // if the flux was not evaluated
//            if( mHdotEval)
//            {
//                // evaluate the flux
//                this->eval_Hdot();
//
//                // set bool for evaluation
//                mHdotEval = false;
//            }
//            // return the flux value
//            return mHdot;
//        }


//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_gradHdot()
        {
            moris::real tDensity = mProperties( static_cast< uint >( Property_Type::DENSITY ) )->val()( 0 );
            moris::real tHeatCap = mProperties( static_cast< uint >( Property_Type::HEAT_CAPACITY ) )->val()( 0 );
            moris::real tTlower  = mProperties( static_cast< uint >( Property_Type::LOWER_PC_TEMP ) )->val()( 0 );
            moris::real tTupper  = mProperties( static_cast< uint >( Property_Type::UPPER_PC_TEMP ) )->val()( 0 );
            moris::real tPCconst = mProperties( static_cast< uint >( Property_Type::PHASE_CHANGE_CONST ) )->val()( 0 );

            // check inputs
            MORIS_ASSERT(tPCconst >= 0.0,
                    "CM_Diffusion_Linear_Isotropic_Phase_Change::eval_gradHdot: Phase change constant must be >= 0.  " );


            // initialize value of derivative of Phase State Function
            real tdfdT = 0.0;

            // get temperature
            real tTemp = mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->val()( 0 );

            // if phase change constant is set to zero, use linear phase change model
            if (tPCconst == 0.0 )
            {
                if ( (tTemp > tTupper) || (tTemp < tTlower) )
                    tdfdT = 0;
                else
                    tdfdT = 1 / (tTupper - tTlower);
            }

            // logistic phase change model
            else
            {
                // logistic function parameter k
                real tLogisticParam = ( 2 * std::log(1/tPCconst - 1) ) / ( tTupper - 3 * tTlower );

                // compute df/dT
                real tExp = std::exp(  tLogisticParam * ( tTemp - (tTupper + tTlower)/2 ) );
                tdfdT = ( tLogisticParam * tExp )  / std::pow( 1 + tExp , 2 );
            }

            // compute gradient of
            mGradHdot = tDensity * ( tHeatCap + tdfdT )
                                    * mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->gradxt();
        }

//------------------------------------------------------------------------------
//        const Matrix< DDRMat > & CM_Diffusion_Linear_Isotropic_Phase_Change::gradHdot()
//        {
//            // if the flux was not evaluated
//            if( mGradHdotEval)
//            {
//                // evaluate the flux
//                this->eval_gradHdot();
//
//                // set bool for evaluation
//                mGradHdotEval = false;
//            }
//            // return the flux value
//            return mGradHdot;
//        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_graddivflux()
        {
            // gets added later
            mGradDivFlux = {{0}};

            moris::real tK = mProperties( static_cast< uint >( Property_Type::CONDUCTIVITY ) )->val()( 0 );

            //FIXME: Check for 2D or 3D is missing
            // matrix for purely isotropic case
//            uint tDims = ?
//            if (tDims == 2)
//            {
            Matrix<DDRMat> tKijIsotropic = {{tK,  0,  0, tK},{ 0, tK, tK,  0}};
//            }
//            else if (tDims == 3)
//            {
//            Matrix<DDRMat> tKijIsotropic = {{tK, 0, 0, 0, 0,tK, 0,tK, 0, 0},
//                                            { 0,tK, 0,tK, 0, 0, 0, 0,tK, 0},
//                                            { 0, 0,tK, 0,tK, 0,tK, 0, 0, 0}};
//            }
//            else
//            MORIS_ASSERT(false, "CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dGradDivFluxdDOF: Number of spatial dimensions must be 2 or 3");

            mGradDivFlux = tKijIsotropic * mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->gradx(3);
        }

//------------------------------------------------------------------------------
//        const Matrix< DDRMat > & CM_Diffusion_Linear_Isotropic_Phase_Change::graddivflux()
//        {
//            // if the flux was not evaluated
//            if( mGradDivFluxEval)
//            {
//                // evaluate the flux
//                this->eval_graddivflux();
//
//                // set bool for evaluation
//                mGradDivFluxEval = false;
//            }
//            // return the flux value
//            return mGradDivFlux;
//        }



//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_flux()
        {
            // get the conductivity property
            std::shared_ptr< Property > tPropConductivity
            = mProperties( static_cast< uint >( Property_Type::CONDUCTIVITY ) );

            // compute flux
            mFlux = tPropConductivity->val()( 0 )
                  * mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->gradx( 1 );
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_divflux()
        {
            // get the conductivity property
            std::shared_ptr< Property > tPropConductivity
            = mProperties( static_cast< uint >( Property_Type::CONDUCTIVITY ) );

            // compute the divergence of the flux
            mDivFlux = tPropConductivity->val() * this->divstrain();
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_traction( const Matrix< DDRMat > & aNormal )
        {
            // compute traction
            mTraction = trans( this->flux() ) * aNormal;
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_testTraction( const Matrix< DDRMat > & aNormal,
                                                               const moris::Cell< MSI::Dof_Type > & aTestDofTypes )
        {
            // get test dof type index
            uint tTestDofIndex = mDofTypeMap( static_cast< uint >( aTestDofTypes( 0 ) ) );

            // get the conductivity property
            std::shared_ptr< Property > tPropConductivity
            = mProperties( static_cast< uint >( Property_Type::CONDUCTIVITY ) );

            // compute test traction
            mTestTraction( tTestDofIndex ) = trans( mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->dnNdxn( 1 ) )
                                           * tPropConductivity->val()( 0 ) * aNormal;
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_strain()
        {
            // compute strain
            mStrain = mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->gradx( 1 );
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_divstrain()
        {
            // get the temperature gradient
            Matrix< DDRMat > tTempGrad
            = mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->gradx( 2 );

            // evaluate the divergence of the strain
            mDivStrain = sum( tTempGrad( { 0, mSpaceDim - 1 }, { 0, 0 } ) );
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_testStrain()
        {
            // compute test strain
            mTestStrain = mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->dnNdxn( 1 );
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_const()
        {
            // build an identity matrix
            Matrix< DDRMat > I;
            eye( mSpaceDim, mSpaceDim, I );

            // compute conductivity matrix
            mConst = mProperties( static_cast< uint >( Property_Type::CONDUCTIVITY ) )->val()( 0 ) * I;
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dFluxdDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get the dof type as a uint
            uint tDofType = static_cast< uint >( aDofTypes( 0 ) );

            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( tDofType );

            // get the conductivity property
            std::shared_ptr< Property > tPropConductivity
            = mProperties( static_cast< uint >( Property_Type::CONDUCTIVITY ) );

            // if direct dependency on the dof type
            if( aDofTypes( 0 ) == mDofMap[ "Temp" ] )
            {
                // compute derivative with direct dependency
                mdFluxdDof( tDofIndex ) = tPropConductivity->val()( 0 )
                                        * mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->dnNdxn( 1 );
            }
            else
            {
                // reset the matrix
                mdFluxdDof( tDofIndex ).set_size( mSpaceDim, mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) )->get_number_of_space_time_coefficients(), 0.0 );
            }

            // if indirect dependency on the dof type
            if ( mProperties( static_cast< uint >( Property_Type::CONDUCTIVITY ) )->check_dof_dependency( aDofTypes ) )
            {
                // compute derivative with indirect dependency through properties
                mdFluxdDof( tDofIndex ).matrix_data()
                += mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->gradx( 1 )
                 * tPropConductivity->dPropdDOF( aDofTypes );
            }
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dHdotdDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get properties
            moris::real tDensity = mProperties( static_cast< uint >( Property_Type::DENSITY ) )->val()( 0 );
            moris::real tHeatCap = mProperties( static_cast< uint >( Property_Type::HEAT_CAPACITY ) )->val()( 0 );
            moris::real tTlower  = mProperties( static_cast< uint >( Property_Type::LOWER_PC_TEMP ) )->val()( 0 );
            moris::real tTupper  = mProperties( static_cast< uint >( Property_Type::UPPER_PC_TEMP ) )->val()( 0 );
            moris::real tPCconst = mProperties( static_cast< uint >( Property_Type::PHASE_CHANGE_CONST ) )->val()( 0 );

            // check inputs
            MORIS_ASSERT(tPCconst >= 0.0,
                    "CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dPCfuncdT: Phase change constant must be >= 0.  " );

            // get the dof type as a uint
            uint tDofType = static_cast< uint >( aDofTypes( 0 ) );

            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( tDofType );

            // initialize value of derivative of Phase State Function
            real tdfdT = 0.0;

            // get temperature
            real tTemp = mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->val()( 0 );

            // if phase change constant is set to zero, use linear phase change model
            if (tPCconst == 0.0 )
            {
                if ( (tTemp > tTupper) || (tTemp < tTlower) )
                    tdfdT = 0;
                else
                    tdfdT = 1 / (tTupper - tTlower);
            }

            // logistic phase change model
            else
            {
                // logistic function parameter k
                real tLogisticParam = ( 2 * std::log(1/tPCconst - 1) ) / ( tTupper - 3 * tTlower );

                // compute df/dT
                real tExp = std::exp(  tLogisticParam * ( tTemp - (tTupper + tTlower)/2 ) );
                tdfdT = ( tLogisticParam * tExp )  / std::pow( 1 + tExp , 2 );
            }


            // if direct dependency on the dof type
            if( aDofTypes( 0 ) == mDofMap[ "Temp" ] )
            {
                // compute derivative with direct dependency
                mHdotDof( tDofIndex ) = tDensity * ( tHeatCap + tdfdT )
                                                          * mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->dnNdtn(1);
            }
//            else
//            {
//                // reset the matrix
//                mHdotDof( tDofIndex ).set_size( mSpaceDim, mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) )->get_number_of_space_time_coefficients(), 0.0 );
//            }
        }

//------------------------------------------------------------------------------
//        const Matrix< DDRMat > & CM_Diffusion_Linear_Isotropic_Phase_Change::dHdotdDOF( const moris::Cell< MSI::Dof_Type > & aDofType)
//        {
//            // if aDofType is not an active dof type for the CM
//            MORIS_ERROR( this->check_dof_dependency( aDofType ), "Constitutive_Model::dHdotdDOF - no dependency in this dof type." );
//
//            // get the dof index
//            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofType( 0 ) ) );
//
//            // if the derivative has not been evaluated yet
//            if( mHdotDofEval( tDofIndex ) )
//            {
//                // evaluate the derivative
//                this->eval_dHdotdDOF( aDofType );
//
//                // set bool for evaluation
//                mHdotDofEval( tDofIndex ) = false;
//            }
//
//            // return the derivative
//            return mHdotDof( tDofIndex );
//        }

//--------------------------------------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dGradHdotdDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get properties
            moris::real tDensity = mProperties( static_cast< uint >( Property_Type::DENSITY ) )->val()( 0 );
            moris::real tHeatCap = mProperties( static_cast< uint >( Property_Type::HEAT_CAPACITY ) )->val()( 0 );
            moris::real tTlower  = mProperties( static_cast< uint >( Property_Type::LOWER_PC_TEMP ) )->val()( 0 );
            moris::real tTupper  = mProperties( static_cast< uint >( Property_Type::UPPER_PC_TEMP ) )->val()( 0 );
            moris::real tPCconst = mProperties( static_cast< uint >( Property_Type::PHASE_CHANGE_CONST ) )->val()( 0 );

            // check inputs
            MORIS_ASSERT(tPCconst >= 0.0,
                    "CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dPCfuncdT: Phase change constant must be >= 0.  " );

            // get the dof type as a uint
            uint tDofType = static_cast< uint >( aDofTypes( 0 ) );

            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( tDofType );


            // initialize value of derivative of Phase State Function
            real tdfdT = 0.0;

            // get temperature
            real tTemp = mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->val()( 0 );

            // if phase change constant is set to zero, use linear phase change model
            if (tPCconst == 0.0 )
            {
                if ( (tTemp > tTupper) || (tTemp < tTlower) )
                    tdfdT = 0;
                else
                    tdfdT = 1 / (tTupper - tTlower);
            }

            // logistic phase change model
            else
            {
                // logistic function parameter k
                real tLogisticParam = ( 2 * std::log(1/tPCconst - 1) ) / ( tTupper - 3 * tTlower );

                // compute df/dT
                real tExp = std::exp(  tLogisticParam * ( tTemp - (tTupper + tTlower)/2 ) );
                tdfdT = ( tLogisticParam * tExp )  / std::pow( 1 + tExp , 2 );
            }


            // if direct dependency on the dof type
            if( aDofTypes( 0 ) == mDofMap[ "Temp" ] )
            {
                // compute derivative with direct dependency
                mGradHdotDof( tDofIndex ) = tDensity * ( tHeatCap + tdfdT )
                                                          * mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->d2Ndxt();
            }
//            else
//            {
//                // reset the matrix
//                mGradHdotDof( tDofIndex ).set_size( mSpaceDim, mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) )->get_number_of_space_time_coefficients(), 0.0 );
//            }
        }

//--------------------------------------------------------------------------------------------------------------
//        const Matrix< DDRMat > & CM_Diffusion_Linear_Isotropic_Phase_Change::dGradHdotdDOF( const moris::Cell< MSI::Dof_Type > & aDofType)
//        {
//            // if aDofType is not an active dof type for the CM
//            MORIS_ERROR( this->check_dof_dependency( aDofType ), "Constitutive_Model::dHdotdDOF - no dependency in this dof type." );
//
//            // get the dof index
//            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofType( 0 ) ) );
//
//            // if the derivative has not been evaluated yet
//            if( mGradHdotDofEval( tDofIndex ) )
//            {
//                // evaluate the derivative
//                this->eval_dGradHdotdDOF( aDofType );
//
//                // set bool for evaluation
//                mGradHdotDofEval( tDofIndex ) = false;
//            }
//
//            // return the derivative
//            return mGradHdotDof( tDofIndex );
//        }


//--------------------------------------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dGradDivFluxdDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // gets added later
            mGradDivFlux = {{0}};

            moris::real tK = mProperties( static_cast< uint >( Property_Type::CONDUCTIVITY ) )->val()( 0 );

            // get the dof type as a uint
            uint tDofType = static_cast< uint >( aDofTypes( 0 ) );

            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( tDofType );

            //FIXME: Check for 2D or 3D is missing
            // matrix for purely isotropic case
//            uint tDims = ?
//            if (tDims == 2)
//            {
            Matrix<DDRMat> tKijIsotropic = {{tK,  0,  0, tK},
                                            { 0, tK, tK,  0}};
//            }
//            else if (tDims == 3)
//            {
//            Matrix<DDRMat> tKijIsotropic = {{tK, 0, 0, 0, 0,tK, 0,tK, 0, 0},
//                                            { 0,tK, 0,tK, 0, 0, 0, 0,tK, 0},
//                                            { 0, 0,tK, 0,tK, 0,tK, 0, 0, 0}};
//            }
//            else
//            MORIS_ASSERT(false, "CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dGradDivFluxdDOF: Number of spatial dimensions must be 2 or 3");


            // if direct dependency on the dof type
            if( aDofTypes( 0 ) == mDofMap[ "Temp" ] )
            {
                // compute derivative with direct dependency
                mGradDivFluxDof(tDofIndex) = mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->dnNdxn(3);
            }
//            else
//            {
//                // reset the matrix
//                mGradDivFluxDof( tDofIndex ).set_size( mSpaceDim, mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) )->get_number_of_space_time_coefficients(), 0.0 );
//            }
        }

////--------------------------------------------------------------------------------------------------------------
//        const Matrix< DDRMat > & CM_Diffusion_Linear_Isotropic_Phase_Change::dGradDivFluxdDOF( const moris::Cell< MSI::Dof_Type > & aDofType)
//        {
//            // if aDofType is not an active dof type for the CM
//            MORIS_ERROR( this->check_dof_dependency( aDofType ), "Constitutive_Model::dGradDivFluxdDOF - no dependency in this dof type." );
//
//            // get the dof index
//            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofType( 0 ) ) );
//
//            // if the derivative has not been evaluated yet
//            if( mGradDivFluxDofEval( tDofIndex ) )
//            {
//                // evaluate the derivative
//                this->eval_dHdotdDOF( aDofType );
//
//                // set bool for evaluation
//                mGradDivFluxDofEval( tDofIndex ) = false;
//            }
//
//            // return the derivative
//            return mGradDivFluxDof( tDofIndex );
//        }

//--------------------------------------------------------------------------------------------------------------






//--------------------------------------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_ddivfluxdu( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // get the corresponding FI
            Field_Interpolator * tFI = mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // set size for ddivflux/du
            mddivfluxdu( tDofIndex ).set_size( 1, tFI->get_number_of_space_time_coefficients(), 0.0 );

            // get the conductivity property
            std::shared_ptr< Property > tPropConductivity
            = mProperties( static_cast< uint >( Property_Type::CONDUCTIVITY ) );

            // if temperature dof
            if( aDofTypes( 0 ) == mDofMap[ "Temp" ] )
            {
                // fill ddivstrain/dv
                mddivfluxdu( tDofIndex ).matrix_data() += tPropConductivity->val()( 0 ) * this->ddivstraindu( aDofTypes );
            }

            if( tPropConductivity->check_dof_dependency( aDofTypes ) )
            {
                // fill ddivstrain/du
                mddivfluxdu( tDofIndex ).matrix_data() += this->divstrain() * tPropConductivity->dPropdDOF( aDofTypes );
            }
        }

//--------------------------------------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_ddivstraindu( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // get the dof type FI
            Field_Interpolator * tFI = mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // set size for ddivstrain/du
            mddivstraindu( tDofIndex ).set_size( 1, tFI->get_number_of_space_time_coefficients(), 0.0 );

            if( aDofTypes( 0 ) == mDofMap[ "Temp" ] )
            {
                // get the 2nd order derivative of the shape functions d2Ndx2
                Matrix< DDRMat > tTempd2Ndx2 = tFI->dnNdxn( 2 );

                // fill ddivstrain/du
                mddivstraindu( tDofIndex ) = tTempd2Ndx2.get_row( 0 ) + tTempd2Ndx2.get_row( 1 );

                if( tTempd2Ndx2.n_rows() == 6 )
                {
                    mddivstraindu( tDofIndex ).matrix_data() += tTempd2Ndx2.get_row( 2 );
                }
            }
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dTractiondDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes,
                                                                const Matrix< DDRMat >             & aNormal )
        {
            // get the dof type as a uint
            uint tDofType = static_cast< uint >( aDofTypes( 0 ) );

            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( tDofType );

            // compute derivative
            mdTractiondDof( tDofIndex ) = trans( aNormal ) * this->dFluxdDOF( aDofTypes );
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dTestTractiondDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes,
                                                                    const Matrix< DDRMat >             & aNormal,
                                                                    const moris::Cell< MSI::Dof_Type > & aTestDofTypes )
        {
            // get test dof type index
            uint tTestDofIndex = mDofTypeMap( static_cast< uint >( aTestDofTypes( 0 ) ) );

            // get the dof type as a uint
            uint tDofType = static_cast< uint >( aDofTypes( 0 ) );

            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( tDofType );

            // init the dTestTractiondDof
            mdTestTractiondDof( tTestDofIndex )( tDofIndex ).set_size( mFIManager->get_field_interpolators_for_type( aTestDofTypes( 0 ) )->get_number_of_space_time_coefficients(),
                                                                       mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) )->get_number_of_space_time_coefficients(), 0.0 );

            // get the conductivity property
            std::shared_ptr< Property > tPropConductivity
            = mProperties( static_cast< uint >( Property_Type::CONDUCTIVITY ) );

            // if conductivity depends on dof type
            if( tPropConductivity->check_dof_dependency( aDofTypes ) )
            {
                // add contribution
                mdTestTractiondDof( tTestDofIndex )( tDofIndex ).matrix_data()
                += trans( mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->dnNdxn( 1 ) )
                 * aNormal * tPropConductivity->dPropdDOF( aDofTypes );
            }
        }

        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dTestTractiondDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes,
                                                                    const Matrix< DDRMat >             & aNormal,
                                                                    const Matrix< DDRMat >             & aJump,
                                                                    const moris::Cell< MSI::Dof_Type > & aTestDofTypes )
        {
            // get test dof type index
            uint tTestDofIndex = mDofTypeMap( static_cast< uint >( aTestDofTypes( 0 ) ) );

            // get the dof type as a uint
            uint tDofType = static_cast< uint >( aDofTypes( 0 ) );

            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( tDofType );

            // init the dTestTractiondDof
            mdTestTractiondDof( tTestDofIndex )( tDofIndex ).set_size( mFIManager->get_field_interpolators_for_type( aTestDofTypes( 0 ) )->get_number_of_space_time_coefficients(),
                                                                       mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) )->get_number_of_space_time_coefficients(), 0.0 );

            // get the conductivity property
            std::shared_ptr< Property > tPropConductivity
            = mProperties( static_cast< uint >( Property_Type::CONDUCTIVITY ) );

            // if conductivity depends on dof type
            if( tPropConductivity->check_dof_dependency( aDofTypes ) )
            {
                // add contribution
                mdTestTractiondDof( tTestDofIndex )( tDofIndex ).matrix_data()
                += trans( mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->dnNdxn( 1 ) )
                 * aNormal * tPropConductivity->dPropdDOF( aDofTypes );
            }
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dStraindDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get the dof type as a uint
            uint tDofType = static_cast< uint >( aDofTypes( 0 ) );

            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( tDofType );

            // if direct dependency on the dof type
            if( aDofTypes( 0 ) == mDofMap[ "Temp" ] )
            {
                // compute derivative with direct dependency
                mdStraindDof( tDofIndex ) = mFIManager->get_field_interpolators_for_type( mDofMap[ "Temp" ] )->dnNdxn( 1 );
            }
            else
            {
                // reset the matrix
                mdStraindDof( tDofIndex ).set_size( mSpaceDim, mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) )->get_number_of_space_time_coefficients(), 0.0 );
            }
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dConstdDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get the dof type as a uint
            uint tDofType = static_cast< uint >( aDofTypes( 0 ) );

            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( tDofType );

            // reset the matrix
            mdConstdDof( tDofIndex ).set_size( 1, mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) )->get_number_of_space_time_coefficients(), 0.0 );

            // if indirect dependency on the dof type
            if ( mProperties( static_cast< uint >( Property_Type::CONDUCTIVITY ) )->check_dof_dependency( aDofTypes ) )
            {
                // compute derivative with indirect dependency through properties
                mdConstdDof( tDofIndex ) = mProperties( static_cast< uint >( Property_Type::CONDUCTIVITY ) )->dPropdDOF( aDofTypes );
            }
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dFluxdDV( const moris::Cell< GEN_DV > & aDvTypes )
        {
            MORIS_ASSERT( false, " CM_Diffusion_Linear_Isotropic::eval_dFluxdDV - This function is not implemented.");
        }

//------------------------------------------------------------------------------
        void CM_Diffusion_Linear_Isotropic_Phase_Change::eval_dStraindDV( const moris::Cell< GEN_DV > & aDvTypes )
        {
            MORIS_ASSERT( false, " CM_Diffusion_Linear_Isotropic::eval_dStraindDV - This function is not implemented.");
        }

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */
