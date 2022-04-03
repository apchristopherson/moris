
#include "cl_FEM_CM_Struc_Linear_MoriTanaka.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"

#include "fn_trans.hpp"
#include "fn_norm.hpp"
#include "fn_eye.hpp"
#include "fn_dot.hpp"

namespace moris
{
    namespace fem
    {
        //------------------------------------------------------------------------------

        CM_Struc_Linear_MoriTanaka::CM_Struc_Linear_MoriTanaka()
                : mEshelbyTensor( 6, 6, 0.0 )
                , mConstMatrix( 6, 6, 0.0 )
                , mConstFiber( 6, 6, 0.0 )
        {
            //  set the property pointer cell size
            mProperties.resize( mProperties.size() + static_cast< uint >( CM_Property_Type_MT::MAX_ENUM ), nullptr );

            // This index is due to the existence of properties in the parent class
            uint tCurrentIndexOffSet = static_cast< uint >( CM_Property_Type_Lin::MAX_ENUM );

            // populate the map
            mPropertyMap[ "YoungsModulusMatrix" ] = static_cast< uint >( CM_Property_Type_MT::EMOD1 ) + tCurrentIndexOffSet;
            mPropertyMap[ "PoissonRatioMatrix" ]  = static_cast< uint >( CM_Property_Type_MT::NU1 ) + tCurrentIndexOffSet;
            mPropertyMap[ "YoungsModulusFiber" ]  = static_cast< uint >( CM_Property_Type_MT::EMOD2 ) + tCurrentIndexOffSet;
            mPropertyMap[ "PoissonRatioFiber" ]   = static_cast< uint >( CM_Property_Type_MT::NU2 ) + tCurrentIndexOffSet;
            mPropertyMap[ "VolumeFraction" ]      = static_cast< uint >( CM_Property_Type_MT::VF ) + tCurrentIndexOffSet;
            mPropertyMap[ "OrientationInPlane" ]  = static_cast< uint >( CM_Property_Type_MT::THETA_IP ) + tCurrentIndexOffSet;
            mPropertyMap[ "OrientationOutPlane" ] = static_cast< uint >( CM_Property_Type_MT::THETA_OP ) + tCurrentIndexOffSet;
            mPropertyMap[ "AspectRatio" ]         = static_cast< uint >( CM_Property_Type_MT::AR ) + tCurrentIndexOffSet;
        }

        //--------------------------------------------------------------------------------------------------------------

        void
        CM_Struc_Linear_MoriTanaka::set_local_properties()
        {
            // set the parent properties
            CM_Struc_Linear::set_local_properties();

            // default local properties
            mPropEModFib        = this->get_property( "YoungsModulusFiber" );
            mPropPoissonFib     = this->get_property( "PoissonRatioFiber" );
            mPropEModMat        = this->get_property( "YoungsModulusMatrix" );
            mPropPoissonMat     = this->get_property( "PoissonRatioMatrix" );
            mPropVolumeFraction = this->get_property( "VolumeFraction" );
            mPropThetaIp        = this->get_property( "OrientationInPlane" );
            mPropThetaOp        = this->get_property( "OrientationOutPlane" );
            mPropAspectRatio    = this->get_property( "AspectRatio" );
        }

        //--------------------------------------------------------------------------------------------------------------

        void
        CM_Struc_Linear_MoriTanaka::eval_const()
        {
            //  get the properties of the CM model
            const real& tNuMat   = mPropPoissonMat->val()( 0 );
            const real& tEmodMat = mPropEModMat->val()( 0 );
            const real& tEmodFib = mPropEModFib->val()( 0 );
            const real& tNuFib   = mPropPoissonFib->val()( 0 );
            const real& tAr      = mPropAspectRatio->val()( 0 );
            const real& tVf      = mPropVolumeFraction->val()( 0 );
            const real& tThetaIp = mPropThetaIp->val()( 0 );
            const real& tThetaOp = mPropThetaOp->val()( 0 );

            // evaluate the constitutive matrix
            ( this->*mConstFunc )( { tEmodMat, tEmodFib, tNuMat, tNuFib, tVf, tThetaIp, tThetaOp, tAr } );
        }

        //--------------------------------------------------------------------------------------------------------------

        void
        CM_Struc_Linear_MoriTanaka::full_plane_stress(
                std::initializer_list< const real >&& tParams )
        {
            //  reset the constitutive tensor
            mConstPrime.fill( 0.0 );

            // get the constitutive model parameters
            const real& tEm    = tParams.begin()[ 0 ];
            const real& tEf    = tParams.begin()[ 1 ];
            const real& tNum   = tParams.begin()[ 2 ];
            const real& tNuf   = tParams.begin()[ 3 ];
            const real& tVf    = tParams.begin()[ 4 ];
            const real& theta1 = tParams.begin()[ 5 ];
            // const real& theta2 = tParams.begin()[6];
            const real& tAspectRatio = tParams.begin()[ 7 ];

            // High aspect ratio => continuous fiber
            if ( tAspectRatio > 1000. || tAspectRatio == 0. )
            {
                real tC11 = ( std::pow( tEf, 2 ) * ( -1 + tVf ) * tVf * std::pow( 1 + tNum, 2 ) * ( -1 + 2 * tNum ) + std::pow( tEm, 2 ) * ( -1 + tVf ) * ( 1 + tVf + ( -1 + tVf ) * tNum ) * ( -1 + tNuf + 2 * std::pow( tNuf, 2 ) ) - tEm * tEf * ( 1 + tNum ) * ( -1 + tNum + tVf * ( 1 + tNuf - 6 * tNum * tNuf + tVf * ( -2 + tNum + tNuf + 4 * tNum * tNuf ) ) ) ) / ( ( 1 + tNum ) * ( tEf * ( -1 + tVf ) * ( -1 + tNum + 2 * std::pow( tNum, 2 ) ) - tEm * ( 1 + tVf - 2 * tNum ) * ( -1 + tNuf + 2 * std::pow( tNuf, 2 ) ) ) );    // 1st row
                real tC12 = ( tEm * ( -( tEf * ( 1 + tNum ) * ( ( -1 + tVf ) * tNum + 2 * tVf * ( -1 + tNum ) * tNuf ) ) + tEm * ( -1 + tVf ) * tNum * ( -1 + tNuf + 2 * std::pow( tNuf, 2 ) ) ) ) / ( ( 1 + tNum ) * ( tEf * ( -1 + tVf ) * ( -1 + tNum + 2 * std::pow( tNum, 2 ) ) - tEm * ( 1 + tVf - 2 * tNum ) * ( -1 + tNuf + 2 * std::pow( tNuf, 2 ) ) ) );
                real tC22 = -( ( tEm * ( -1 + tNum ) * ( std::pow( tEf, 2 ) * ( -1 + tVf ) * std::pow( 1 + tNum, 2 ) * ( -3 - 2 * tVf + 4 * ( 1 + tVf ) * tNum ) + std::pow( tEm, 2 ) * ( -1 + tVf ) * ( 1 + 2 * tVf ) * std::pow( 1 + tNuf, 2 ) * ( -1 + 2 * tNuf ) + 2 * tEm * tEf * ( 1 + tNum ) * ( 1 + tNuf ) * ( 2 - 2 * tNum - 3 * tNuf + tVf * tNuf + 4 * tNum * tNuf - 2 * std::pow( tVf, 2 ) * ( -1 + tNum + tNuf ) ) ) ) / ( ( 1 + tNum ) * ( tEf * ( -1 + tVf ) * ( -3 + tNum + 4 * std::pow( tNum, 2 ) ) - tEm * ( -1 + tVf * ( -3 + 4 * tNum ) ) * ( 1 + tNuf ) ) * ( tEf * ( -1 + tVf ) * ( -1 + tNum + 2 * std::pow( tNum, 2 ) ) - tEm * ( 1 + tVf - 2 * tNum ) * ( -1 + tNuf + 2 * std::pow( tNuf, 2 ) ) ) ) );
                real tC23 = -( ( tEm * ( pow( tEf, 2 ) * ( -1 + tVf ) * pow( 1 + tNum, 2 ) * ( tVf * pow( 1 - 2 * tNum, 2 ) + ( 3 - 4 * tNum ) * tNum ) + std::pow( tEm, 2 ) * ( -1 + tVf ) * ( -tNum + tVf * ( -1 + 2 * tNum ) ) * pow( 1 + tNuf, 2 ) * ( -1 + 2 * tNuf ) - 2 * tEm * tEf * ( 1 + tNum ) * ( 1 + tNuf ) * ( pow( tVf, 2 ) * ( -1 + 2 * tNum ) * ( -1 + tNum + tNuf ) + tVf * ( -1 + tNum + 5 * tNuf - 7 * tNum * tNuf ) + tNum * ( 2 - 2 * tNum - 3 * tNuf + 4 * tNum * tNuf ) ) ) ) / ( ( 1 + tNum ) * ( tEf * ( -1 + tVf ) * ( -3 + tNum + 4 * pow( tNum, 2 ) ) - tEm * ( -1 + tVf * ( -3 + 4 * tNum ) ) * ( 1 + tNuf ) ) * ( tEf * ( -1 + tVf ) * ( -1 + tNum + 2 * pow( tNum, 2 ) ) - tEm * ( 1 + tVf - 2 * tNum ) * ( -1 + tNuf + 2 * pow( tNuf, 2 ) ) ) ) );
                real tC33 = tC22;
                real tC66 = ( tEm * ( -( tEf * ( 1 + tVf ) * ( 1 + tNum ) ) + tEm * ( -1 + tVf ) * ( 1 + tNuf ) ) ) / ( 2. * ( 1 + tNum ) * ( tEf * ( -1 + tVf ) * ( 1 + tNum ) - tEm * ( 1 + tVf ) * ( 1 + tNuf ) ) );

                // first row
                mConstPrime( 0, 0 ) = tC11 - ( tC12 * tC12 / tC33 );
                mConstPrime( 0, 1 ) = tC12 - ( tC12 * tC23 / tC33 );
                // second row
                mConstPrime( 1, 0 ) = mConstPrime( 0, 1 );
                mConstPrime( 1, 1 ) = tC22 - ( tC23 * tC23 / tC33 );
                // third row
                mConstPrime( 2, 2 ) = tC66;
            }
            else
            {
                // Eshelby Tensor
                this->set_eshelby_tensor( tAspectRatio, tNum );

                // Matrix and fiber material stiffness matrices
                this->set_isotopic_tensor( tEm, tNum, mConstMatrix );
                this->set_isotopic_tensor( tEf, tNuf, mConstFiber );

                // build an identity matrix
                Matrix< DDRMat > I;
                eye( 6, 6, I );

                // Dilute concentration factor
                // Adil = [I + S.C1^-1.(C2 - C1)]^-1
                Matrix< DDRMat > tAdil = inv( I + mEshelbyTensor * inv( mConstMatrix ) * ( mConstFiber - mConstMatrix ) );

                // Mori-Tanaka concetration tensor
                // AMT = Adil.[(1 - tVf)*I + tVf*Adil]^-1
                Matrix< DDRMat > tAMT = tAdil * inv( ( 1 - tVf ) * I + tVf * tAdil );

                // Get the effective stiffness matrix
                // Ceff = C1 + tVf*(C2 - C1).AMT
                Matrix< DDRMat > tCeff = mConstMatrix + tVf * ( mConstFiber - mConstMatrix ) * tAMT;

                // first row
                mConstPrime( 0, 0 ) = tCeff( 0, 0 ) - ( tCeff( 0, 1 ) * tCeff( 0, 1 ) / tCeff( 2, 2 ) );
                mConstPrime( 0, 1 ) = tCeff( 0, 1 ) - ( tCeff( 0, 1 ) * tCeff( 1, 2 ) / tCeff( 2, 2 ) );
                // second row
                mConstPrime( 1, 0 ) = mConstPrime( 0, 1 );
                mConstPrime( 1, 1 ) = tCeff( 1, 1 ) - ( tCeff( 1, 2 ) * tCeff( 1, 2 ) / tCeff( 2, 2 ) );
                // third row
                mConstPrime( 2, 2 ) = tCeff( 5, 5 );
            }

            real lx = std::cos( theta1 );
            real ly = std::sin( theta1 );

            real rx = -std::sin( theta1 );
            real ry = std::cos( theta1 );

            // first row
            mRotation( 0, 0 ) = lx * lx;
            mRotation( 0, 1 ) = ly * ly;
            mRotation( 0, 2 ) = lx * ly;
            // second row
            mRotation( 1, 0 ) = rx * rx;
            mRotation( 1, 1 ) = ry * ry;
            mRotation( 1, 2 ) = rx * ry;
            // third row
            mRotation( 2, 0 ) = 2.0 * lx * rx;
            mRotation( 2, 1 ) = 2.0 * ly * ry;
            mRotation( 2, 2 ) = lx * ry + ly * rx;

            mConst = trans( mRotation ) * mConstPrime * mRotation;
        }

        //--------------------------------------------------------------------------------------------------------------

        void
        CM_Struc_Linear_MoriTanaka::full_3d(
                std::initializer_list< const real >&& tParams )
        {
            // reset the constitutive tensor
            mConst.fill( 0.0 );

            // get the constitutive model parameters
            const real& tEm          = tParams.begin()[ 0 ];
            const real& tEf          = tParams.begin()[ 1 ];
            const real& tNum         = tParams.begin()[ 2 ];
            const real& tNuf         = tParams.begin()[ 3 ];
            const real& tVf          = tParams.begin()[ 4 ];
            const real& theta1       = tParams.begin()[ 5 ];
            const real& theta2       = tParams.begin()[ 6 ];
            const real& tAspectRatio = tParams.begin()[ 7 ];

            // High aspect ratio => continuous fiber
            if ( tAspectRatio > 1000. || tAspectRatio == 0. )
            {
                // first row
                mConst( 0, 0 ) = ( std::pow( tEf, 2 ) * ( -1 + tVf ) * tVf * std::pow( 1 + tNum, 2 ) * ( -1 + 2 * tNum ) + std::pow( tEm, 2 ) * ( -1 + tVf ) * ( 1 + tVf + ( -1 + tVf ) * tNum ) * ( -1 + tNuf + 2 * std::pow( tNuf, 2 ) ) - tEm * tEf * ( 1 + tNum ) * ( -1 + tNum + tVf * ( 1 + tNuf - 6 * tNum * tNuf + tVf * ( -2 + tNum + tNuf + 4 * tNum * tNuf ) ) ) ) / ( ( 1 + tNum ) * ( tEf * ( -1 + tVf ) * ( -1 + tNum + 2 * std::pow( tNum, 2 ) ) - tEm * ( 1 + tVf - 2 * tNum ) * ( -1 + tNuf + 2 * std::pow( tNuf, 2 ) ) ) );    // 1st row
                mConst( 0, 1 ) = ( tEm * ( -( tEf * ( 1 + tNum ) * ( ( -1 + tVf ) * tNum + 2 * tVf * ( -1 + tNum ) * tNuf ) ) + tEm * ( -1 + tVf ) * tNum * ( -1 + tNuf + 2 * std::pow( tNuf, 2 ) ) ) ) / ( ( 1 + tNum ) * ( tEf * ( -1 + tVf ) * ( -1 + tNum + 2 * std::pow( tNum, 2 ) ) - tEm * ( 1 + tVf - 2 * tNum ) * ( -1 + tNuf + 2 * std::pow( tNuf, 2 ) ) ) );
                mConst( 0, 2 ) = mConst( 0, 1 );
                // second row
                mConst( 1, 0 ) = mConst( 0, 1 );
                mConst( 1, 1 ) = -( ( tEm * ( -1 + tNum ) * ( std::pow( tEf, 2 ) * ( -1 + tVf ) * std::pow( 1 + tNum, 2 ) * ( -3 - 2 * tVf + 4 * ( 1 + tVf ) * tNum ) + std::pow( tEm, 2 ) * ( -1 + tVf ) * ( 1 + 2 * tVf ) * std::pow( 1 + tNuf, 2 ) * ( -1 + 2 * tNuf ) + 2 * tEm * tEf * ( 1 + tNum ) * ( 1 + tNuf ) * ( 2 - 2 * tNum - 3 * tNuf + tVf * tNuf + 4 * tNum * tNuf - 2 * std::pow( tVf, 2 ) * ( -1 + tNum + tNuf ) ) ) ) / ( ( 1 + tNum ) * ( tEf * ( -1 + tVf ) * ( -3 + tNum + 4 * std::pow( tNum, 2 ) ) - tEm * ( -1 + tVf * ( -3 + 4 * tNum ) ) * ( 1 + tNuf ) ) * ( tEf * ( -1 + tVf ) * ( -1 + tNum + 2 * std::pow( tNum, 2 ) ) - tEm * ( 1 + tVf - 2 * tNum ) * ( -1 + tNuf + 2 * std::pow( tNuf, 2 ) ) ) ) );
                mConst( 1, 2 ) = -( ( tEm * ( pow( tEf, 2 ) * ( -1 + tVf ) * pow( 1 + tNum, 2 ) * ( tVf * pow( 1 - 2 * tNum, 2 ) + ( 3 - 4 * tNum ) * tNum ) + std::pow( tEm, 2 ) * ( -1 + tVf ) * ( -tNum + tVf * ( -1 + 2 * tNum ) ) * pow( 1 + tNuf, 2 ) * ( -1 + 2 * tNuf ) - 2 * tEm * tEf * ( 1 + tNum ) * ( 1 + tNuf ) * ( pow( tVf, 2 ) * ( -1 + 2 * tNum ) * ( -1 + tNum + tNuf ) + tVf * ( -1 + tNum + 5 * tNuf - 7 * tNum * tNuf ) + tNum * ( 2 - 2 * tNum - 3 * tNuf + 4 * tNum * tNuf ) ) ) ) / ( ( 1 + tNum ) * ( tEf * ( -1 + tVf ) * ( -3 + tNum + 4 * pow( tNum, 2 ) ) - tEm * ( -1 + tVf * ( -3 + 4 * tNum ) ) * ( 1 + tNuf ) ) * ( tEf * ( -1 + tVf ) * ( -1 + tNum + 2 * pow( tNum, 2 ) ) - tEm * ( 1 + tVf - 2 * tNum ) * ( -1 + tNuf + 2 * pow( tNuf, 2 ) ) ) ) );
                // third row
                mConst( 2, 0 ) = mConst( 0, 2 );
                mConst( 2, 1 ) = mConst( 1, 2 );
                mConst( 2, 2 ) = mConst( 1, 1 );
                // shear terms - order  xy, xz, yz (older version had yz, xz, xy)
                mConst( 5, 5 ) = ( tEm * ( tEf * ( 3 + tVf - 4 * tNum ) * ( 1 + tNum ) - tEm * ( -1 + tVf ) * ( 1 + tNuf ) ) ) / ( 2. * ( 1 + tNum ) * ( tEf * ( -1 + tVf ) * ( -3 + tNum + 4 * pow( tNum, 2 ) ) - tEm * ( -1 + tVf * ( -3 + 4 * tNum ) ) * ( 1 + tNuf ) ) );
                mConst( 4, 4 ) = ( tEm * ( -( tEf * ( 1 + tVf ) * ( 1 + tNum ) ) + tEm * ( -1 + tVf ) * ( 1 + tNuf ) ) ) / ( 2. * ( 1 + tNum ) * ( tEf * ( -1 + tVf ) * ( 1 + tNum ) - tEm * ( 1 + tVf ) * ( 1 + tNuf ) ) );
                mConst( 3, 3 ) = mConst( 4, 4 );
            }
            else    // Low aspect ratio, particles
            {
                // Eshelby Tensor
                this->set_eshelby_tensor( tAspectRatio, tNum );

                // Matrix and fiber material stiffness matrices
                this->set_isotopic_tensor( tEm, tNum, mConstMatrix );
                this->set_isotopic_tensor( tEf, tNuf, mConstFiber );

                // build an identity matrix
                Matrix< DDRMat > I;
                eye( 6, 6, I );

                // Dilute concentration factor
                // Adil = [I + S.C1^-1.(C2 - C1)]^-1
                Matrix< DDRMat > tAdil = inv( I + mEshelbyTensor * inv( mConstMatrix ) * ( mConstFiber - mConstMatrix ) );

                // Mori-Tanaka concentration tensor
                // AMT = Adil.[(1 - tVf)*I + tVf*Adil]^-1
                Matrix< DDRMat > tAMT = tAdil * inv( ( 1 - tVf ) * I + tVf * tAdil );

                // Get the effective stiffness matrix
                // CeffMT = C1 + tVf*(C2 - C1).AMT
                mConst = mConstMatrix + tVf * ( mConstFiber - mConstMatrix ) * tAMT;

                // Exchange C44 with C66
                std::swap( mConst( 3, 3 ), mConst( 5, 5 ) );
            }

            real lx = std::cos( theta2 ) * std::cos( theta1 );
            real ly = std::sin( theta1 );
            real lz = std::sin( theta2 ) * std::cos( theta1 );

            real rx = -std::cos( theta2 ) * std::sin( theta1 );
            real ry = std::cos( theta1 );
            real rz = -std::sin( theta2 ) * std::sin( theta1 );

            real tx = -std::sin( theta2 );
            real ty = 0.0;
            real tz = std::cos( theta2 );

            // first row
            mRotation( 0, 0 ) = lx * lx;
            mRotation( 0, 1 ) = ly * ly;
            mRotation( 0, 2 ) = lz * lz;
            mRotation( 0, 3 ) = ly * lz;
            mRotation( 0, 4 ) = lx * lz;
            mRotation( 0, 5 ) = lx * ly;
            // second row
            mRotation( 1, 0 ) = rx * rx;
            mRotation( 1, 1 ) = ry * ry;
            mRotation( 1, 2 ) = rz * rz;
            mRotation( 1, 3 ) = ry * rz;
            mRotation( 1, 4 ) = rx * rz;
            mRotation( 1, 5 ) = rx * ry;
            // third row
            mRotation( 2, 0 ) = tx * tx;
            mRotation( 2, 1 ) = ty * ty;
            mRotation( 2, 2 ) = tz * tz;
            mRotation( 2, 3 ) = ty * tz;
            mRotation( 2, 4 ) = tx * tz;
            mRotation( 2, 5 ) = tx * ty;
            // fourth row
            mRotation( 3, 0 ) = 2 * lx * rx;
            mRotation( 3, 1 ) = 2 * ly * ry;
            mRotation( 3, 2 ) = 2 * lz * rz;
            mRotation( 3, 3 ) = ly * rz + lz * ry;
            mRotation( 3, 4 ) = lz * rx + lx * rz;
            mRotation( 3, 5 ) = lx * ry + ly * rx;
            // fifth row
            mRotation( 4, 0 ) = 2 * lx * tx;
            mRotation( 4, 1 ) = 2 * ly * ty;
            mRotation( 4, 2 ) = 2 * lz * tz;
            mRotation( 4, 3 ) = ty * lz + tz * ly;
            mRotation( 4, 4 ) = tz * lx + tx * lz;
            mRotation( 4, 5 ) = tx * ly + ty * lx;
            // sixth row
            mRotation( 5, 0 ) = 2 * rx * tx;
            mRotation( 5, 1 ) = 2 * ry * ty;
            mRotation( 5, 2 ) = 2 * rz * tz;
            mRotation( 5, 3 ) = ry * tz + rz * ty;
            mRotation( 5, 4 ) = rz * tx + rx * tz;
            mRotation( 5, 5 ) = rx * ty + ry * tx;

            mConst = trans( mRotation ) * mConst * mRotation;
        }

        //--------------------------------------------------------------------------------------------------------------

        void
        CM_Struc_Linear_MoriTanaka::set_eshelby_tensor( real const & tAspectRatio, real const & v )
        {
            moris::real tAspectRatio2 = tAspectRatio * tAspectRatio;

            real g = 1.0;
            if ( tAspectRatio == 1. )
            {
                // Eshelby Tensor
                mEshelbyTensor( 0, 0 ) = ( 7 - 5 * v ) / ( 15 - 15 * v );
                mEshelbyTensor( 0, 1 ) = ( 1 - 5 * v ) / ( 15. * ( -1 + v ) );
                mEshelbyTensor( 0, 2 ) = mEshelbyTensor( 0, 1 );

                mEshelbyTensor( 1, 0 ) = mEshelbyTensor( 0, 1 );
                mEshelbyTensor( 1, 1 ) = mEshelbyTensor( 0, 0 );
                mEshelbyTensor( 1, 2 ) = mEshelbyTensor( 0, 1 );

                mEshelbyTensor( 2, 0 ) = mEshelbyTensor( 0, 1 );
                mEshelbyTensor( 2, 1 ) = mEshelbyTensor( 2, 0 );
                mEshelbyTensor( 2, 2 ) = mEshelbyTensor( 0, 0 );

                mEshelbyTensor( 3, 3 ) = ( 2 * ( -4 + 5 * v ) ) / ( 15. * ( -1 + v ) );
                mEshelbyTensor( 4, 4 ) = mEshelbyTensor( 3, 3 );
                mEshelbyTensor( 5, 5 ) = mEshelbyTensor( 3, 3 );

                return;
            }

            if ( tAspectRatio < 1. )
            {
                g = ( tAspectRatio * ( std::acos( tAspectRatio ) - tAspectRatio * std::pow( 1 - tAspectRatio2, 0.5 ) ) ) / std::pow( 1 - tAspectRatio2, 1.5 );
            }

            if ( tAspectRatio > 1. )
            {
                g = ( tAspectRatio * ( tAspectRatio * std::pow( tAspectRatio2 - 1, 0.5 ) - std::acosh( tAspectRatio ) ) ) / std::pow( tAspectRatio2 - 1, 1.5 );
            }

            // Eshelby Tensor
            mEshelbyTensor( 0, 0 ) = ( 1 + ( -1 + 3 * tAspectRatio2 ) / ( -1 + tAspectRatio2 ) - 2 * v ) / ( 2. * ( 1 - v ) ) - ( ( 1 + ( 3 * tAspectRatio2 ) / ( -1 + tAspectRatio2 ) - 2 * v ) * g ) / ( 2. * ( 1 - v ) );
            mEshelbyTensor( 0, 1 ) = -( 1 + 1 / ( -1 + tAspectRatio2 ) - 2 * v ) / ( 2. * ( 1 - v ) ) + ( ( 1 + 3 / ( 2. * ( -1 + tAspectRatio2 ) ) - 2 * v ) * g ) / ( 2. * ( 1 - v ) );
            mEshelbyTensor( 0, 2 ) = mEshelbyTensor( 0, 1 );

            mEshelbyTensor( 1, 0 ) = -tAspectRatio2 / ( 2. * ( -1 + tAspectRatio2 ) * ( 1 - v ) ) + ( ( -1 + ( 3 * tAspectRatio2 ) / ( -1 + tAspectRatio2 ) + 2 * v ) * g ) / ( 4. * ( 1 - v ) );
            mEshelbyTensor( 1, 1 ) = ( 3 * tAspectRatio2 ) / ( 8. * ( -1 + tAspectRatio2 ) * ( 1 - v ) ) + ( ( 1 - 9 / ( 4. * ( -1 + tAspectRatio2 ) ) - 2 * v ) * g ) / ( 4. * ( 1 - v ) );
            mEshelbyTensor( 1, 2 ) = tAspectRatio2 / ( 8. * ( -1 + tAspectRatio2 ) * ( 1 - v ) ) - ( ( 1 + 3 / ( 4. * ( -1 + tAspectRatio2 ) ) - 2 * v ) * g ) / ( 4. * ( 1 - v ) );

            mEshelbyTensor( 2, 0 ) = mEshelbyTensor( 1, 0 );
            mEshelbyTensor( 2, 1 ) = mEshelbyTensor( 1, 2 );
            mEshelbyTensor( 2, 2 ) = mEshelbyTensor( 1, 1 );

            mEshelbyTensor( 3, 3 ) = tAspectRatio2 / ( 4. * ( -1 + tAspectRatio2 ) * ( 1 - v ) ) + ( ( 1 - 3 / ( 4. * ( -1 + tAspectRatio2 ) ) - 2 * v ) * g ) / ( 2. * ( 1 - v ) );
            mEshelbyTensor( 4, 4 ) = ( 1 - ( 1 + tAspectRatio2 ) / ( -1 + tAspectRatio2 ) - 2 * v ) / ( 2. * ( 1 - v ) ) - ( ( 1 - ( 3 * ( 1 + tAspectRatio2 ) ) / ( -1 + tAspectRatio2 ) - 2 * v ) * g ) / ( 4. * ( 1 - v ) );
            mEshelbyTensor( 5, 5 ) = mEshelbyTensor( 4, 4 );
        }

        //--------------------------------------------------------------------------------------------------------------

        void
        CM_Struc_Linear_MoriTanaka::set_isotopic_tensor( const real& aEmod,
                const real&                                          aNu,
                Matrix< DDRMat >&                                    aIsotropicConst )
        {
            const real tPre = aEmod / ( 1.0 + aNu ) / ( 1.0 - 2.0 * aNu );

            aIsotropicConst( 0, 0 ) = tPre * ( 1.0 - aNu );
            aIsotropicConst( 0, 1 ) = tPre * aNu;
            aIsotropicConst( 0, 2 ) = tPre * aNu;
            aIsotropicConst( 1, 0 ) = tPre * aNu;
            aIsotropicConst( 1, 1 ) = tPre * ( 1.0 - aNu );
            aIsotropicConst( 1, 2 ) = tPre * aNu;
            aIsotropicConst( 2, 0 ) = tPre * aNu;
            aIsotropicConst( 2, 1 ) = tPre * aNu;
            aIsotropicConst( 2, 2 ) = tPre * ( 1.0 - aNu );
            aIsotropicConst( 3, 3 ) = tPre * ( 1.0 - 2.0 * aNu ) / 2.0;
            aIsotropicConst( 4, 4 ) = tPre * ( 1.0 - 2.0 * aNu ) / 2.0;
            aIsotropicConst( 5, 5 ) = tPre * ( 1.0 - 2.0 * aNu ) / 2.0;
        }

        //--------------------------------------------------------------------------------------------------------------

        void
        CM_Struc_Linear_MoriTanaka::set_function_pointers()
        {
            CM_Struc_Linear::set_function_pointers();

            switch ( mSpaceDim )
            {
                case 2:
                {
                    switch ( mPlaneType )
                    {
                        case Model_Type::PLANE_STRESS:
                        {
                            mRotation.set_size( 3, 3, 0.0 );
                            mConstPrime.set_size( 3, 3, 0.0 );
                            mRotationDer.set_size( 3, 3, 0.0 );
                            break;
                        }
                        default:
                        {
                            MORIS_ERROR( false,
                                    "Mori Tanaka in 2d requires "
                                    "plane stress" );
                        }
                    }
                    break;
                }
                case 3:
                {
                    mRotation.set_size( 6, 6, 0.0 );
                    mConstPrime.set_size( 3, 3, 0.0 );
                    mRotationDer.set_size( 6, 6, 0.0 );
                    break;
                }
                default:
                {
                    MORIS_ERROR( false, "Mori Tanaka implemented for 2d and 3d only" );
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void
        CM_Struc_Linear_MoriTanaka::eval_dFluxdDOF( const Cell< MSI::Dof_Type >& aDofTypes )
        {
            // call the parent contribution
            CM_Struc_Linear::eval_dFluxdDOF( aDofTypes );

            // get the dof type as a uint
            const uint tDofType = static_cast< uint >( aDofTypes( 0 ) );

            // get the dof type index
            const uint tDofIndex = mGlobalDofTypeMap( tDofType );

            // if elastic modulus depends on dof type
            if ( mPropThetaIp->check_dof_dependency( aDofTypes ) )
            {
                // update consitutive matrix and rotation tensor
                this->eval_const();

                // FIXME: hard wired to plane stress - mRotationDer needs to go into seperate routine

                // get rotation angle
                const real& theta1 = mPropThetaIp->val()( 0 );

                // compute derivative of ration matrix wrt in-plane angle
                real ly  = std::sin( theta1 );
                real lx2 = std::cos( 2.0 * theta1 );
                real ly2 = std::sin( 2.0 * theta1 );

                // first row
                mRotationDer( 0, 0 ) = -ly2;
                mRotationDer( 0, 1 ) = ly2;
                mRotationDer( 0, 2 ) = lx2;
                // second row
                mRotationDer( 1, 0 ) = ly2;
                mRotationDer( 1, 1 ) = -ly2;
                mRotationDer( 1, 2 ) = 2.0 * ly * ly - 1.0;
                // third row
                mRotationDer( 2, 0 ) = 4.0 * ly * ly - 2.0;
                mRotationDer( 2, 1 ) = 2.0 - 4.0 * ly * ly;
                mRotationDer( 2, 2 ) = -2.0 * ly2;

                // compute derivative with indirect dependency through properties
                mdFluxdDof( tDofIndex ) +=
                        ( trans( mRotationDer ) * mConstPrime * mRotation              //
                                + trans( mRotation ) * mConstPrime * mRotationDer )    //
                        * this->strain() * mPropThetaIp->dPropdDOF( aDofTypes );
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void
        CM_Struc_Linear_MoriTanaka::eval_dTestTractiondDOF(
                const Cell< MSI::Dof_Type >& aDofTypes,
                const Matrix< DDRMat >&      aNormal,
                const Matrix< DDRMat >&      aJump,
                const Cell< MSI::Dof_Type >& aTestDofTypes )
        {
            CM_Struc_Linear::eval_dTestTractiondDOF( aDofTypes, aNormal, aJump, aTestDofTypes );

            // get test dof type index
            const uint tTestDofIndex = mDofTypeMap( static_cast< uint >( aTestDofTypes( 0 ) ) );

            // get the dof type index
            const uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // if test traction wrt displacement
            if ( aTestDofTypes( 0 ) == mDofDispl )
            {

                // if elastic modulus depends on dof type
                if ( mPropThetaIp->check_dof_dependency( aDofTypes ) )
                {
                    // update consitutive matrix and rotation tensor
                    this->eval_const();

                    // FIXME: hard wired to plane stress - mRotationDer needs to go into seperate routine

                    // get rotation angle
                    const real& theta1 = mPropThetaIp->val()( 0 );

                    // compute derivative of ration matrix wrt in-plane angle
                    real ly  = std::sin( theta1 );
                    real lx2 = std::cos( 2.0 * theta1 );
                    real ly2 = std::sin( 2.0 * theta1 );

                    // first row
                    mRotationDer( 0, 0 ) = -ly2;
                    mRotationDer( 0, 1 ) = ly2;
                    mRotationDer( 0, 2 ) = lx2;
                    // second row
                    mRotationDer( 1, 0 ) = ly2;
                    mRotationDer( 1, 1 ) = -ly2;
                    mRotationDer( 1, 2 ) = 2.0 * ly * ly - 1.0;
                    // third row
                    mRotationDer( 2, 0 ) = 4.0 * ly * ly - 2.0;
                    mRotationDer( 2, 1 ) = 2.0 - 4.0 * ly * ly;
                    mRotationDer( 2, 2 ) = -2.0 * ly2;

                    // flatten the normal
                    Matrix< DDRMat > tFlatNormal;
                    this->flatten_normal( aNormal, tFlatNormal );

                    // compute test traction wrt displacement
                    mdTestTractiondDof( tTestDofIndex )( tDofIndex ) =
                            trans( this->testStrain() )
                            * ( trans( mRotationDer ) * mConstPrime * mRotation + trans( mRotation ) * mConstPrime * mRotationDer )
                            * trans( tFlatNormal ) * aJump * mPropThetaIp->dPropdDOF( aDofTypes );
                }
            }
        }

    }    // namespace fem
}    // namespace moris
