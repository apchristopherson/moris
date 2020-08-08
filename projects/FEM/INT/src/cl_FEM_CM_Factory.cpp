#include "assert.hpp"
//FEM/INT/src
#include "cl_FEM_CM_Factory.hpp"
#include "cl_FEM_CM_Diffusion_Linear_Isotropic.hpp"
#include "cl_FEM_CM_Diffusion_Linear_Isotropic_Phase_Change.hpp"
#include "cl_FEM_CM_Struc_Linear_Isotropic.hpp"
#include "cl_FEM_CM_Fluid_Incompressible.hpp"
#include "cl_FEM_CM_Fluid_Turbulence.hpp"
#include "cl_FEM_CM_Fluid_Compressible_Ideal.hpp"
#include "cl_FEM_CM_Fluid_Compressible_VanDerWaals.hpp"

namespace moris
{
    namespace fem
    {
        //------------------------------------------------------------------------------
        std::shared_ptr< Constitutive_Model > CM_Factory::create_CM( fem::Constitutive_Type aConstitutiveType )
        {

            switch( aConstitutiveType )
            {
                case Constitutive_Type::DIFF_LIN_ISO :
                    return std::make_shared< CM_Diffusion_Linear_Isotropic >();

                case Constitutive_Type::DIFF_LIN_ISO_PC :
                    return std::make_shared< CM_Diffusion_Linear_Isotropic_Phase_Change >();

                case  Constitutive_Type::STRUC_LIN_ISO :
                    return std::make_shared< CM_Struc_Linear_Isotropic >();

                case Constitutive_Type::FLUID_INCOMPRESSIBLE :
                    return std::make_shared< CM_Fluid_Incompressible >();

                case Constitutive_Type::FLUID_TURBULENCE :
                    return std::make_shared< CM_Fluid_Turbulence >();

                case Constitutive_Type::FLUID_COMPRESSIBLE_IDEAL :
                    return std::make_shared< CM_Fluid_Compressible_Ideal >();

                case Constitutive_Type::FLUID_COMPRESSIBLE_VDW :
                    return std::make_shared< CM_Fluid_Compressible_VdW >();

                default:
                    MORIS_ERROR( false, " CM_Factory::create_CM - No constitutive type specified. " );
                    return nullptr;
            }
        }
        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */
