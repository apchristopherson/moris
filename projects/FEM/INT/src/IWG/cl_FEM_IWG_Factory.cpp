#include <memory>
#include "assert.hpp"

//FEM/INT/src
#include "cl_FEM_IWG_Factory.hpp"
#include "cl_FEM_IWG_L2.hpp"

//#include "cl_FEM_IWG_Helmholtz_Bulk.hpp"
//#include "cl_FEM_IWG_Helmholtz_Bulk2.hpp"
//#include "cl_FEM_IWG_Helmholtz_Interface.hpp"
//#include "cl_FEM_IWG_Hamilton_Jacobi_Bulk.hpp"
//#include "cl_FEM_IWG_Hamilton_Jacobi_Bulk2.hpp"
//#include "cl_FEM_IWG_LSNormal_Bulk.hpp"
//#include "cl_FEM_IWG_Olsson_CLS_Bulk.hpp"
//#include "cl_FEM_IWG_Olsson_CLS_Interface.hpp"
#include "cl_FEM_IWG_Hamilton_Jacobi_Bulk_Test.hpp"
//Diffusion
#include "cl_FEM_IWG_Diffusion_Bulk.hpp"
#include "cl_FEM_IWG_Diffusion_Dirichlet_Nitsche.hpp"
#include "cl_FEM_IWG_Diffusion_Neumann.hpp"
#include "cl_FEM_IWG_Diffusion_Robin.hpp"
#include "cl_FEM_IWG_Diffusion_Radiation.hpp"
#include "cl_FEM_IWG_Diffusion_Interface.hpp"
#include "cl_FEM_IWG_Diffusion_Virtual_Work_Ghost.hpp"
//Advection
#include "cl_FEM_IWG_Advection_Bulk.hpp"
//Elasticity
#include "cl_FEM_IWG_Isotropic_Struc_Linear_Bulk.hpp"
#include "cl_FEM_IWG_Isotropic_Struc_Linear_Dirichlet.hpp"
#include "cl_FEM_IWG_Isotropic_Struc_Linear_Neumann.hpp"
#include "cl_FEM_IWG_Isotropic_Struc_Linear_Interface.hpp"
#include "cl_FEM_IWG_Isotropic_Struc_Linear_Virtual_Work_Ghost.hpp"
//Incompressible solid
#include "cl_FEM_IWG_Isotropic_Struc_Linear_Pressure_Bulk.hpp"
#include "cl_FEM_IWG_Isotropic_Struc_Linear_Pressure_Dirichlet.hpp"
//Incompressible fluid
#include "cl_FEM_IWG_Incompressible_NS_Velocity_Bulk.hpp"
#include "cl_FEM_IWG_Incompressible_NS_Pressure_Bulk.hpp"
#include "cl_FEM_IWG_Incompressible_NS_Convective_Velocity_Ghost.hpp"
#include "cl_FEM_IWG_Incompressible_NS_Velocity_Dirichlet_Nitsche.hpp"
#include "cl_FEM_IWG_Incompressible_NS_Pressure_Dirichlet_Nitsche.hpp"
#include "cl_FEM_IWG_Incompressible_NS_Pressure_Neumann.hpp"
#include "cl_FEM_IWG_Incompressible_NS_Velocity_Interface.hpp"
#include "cl_FEM_IWG_Incompressible_NS_Pressure_Interface.hpp"
//Compressible Fluid
#include "cl_FEM_IWG_Compressible_NS_Density_Bulk.hpp"
#include "cl_FEM_IWG_Compressible_NS_Velocity_Bulk.hpp"
#include "cl_FEM_IWG_Compressible_NS_Temperature_Bulk.hpp"
#include "cl_FEM_IWG_Compressible_NS_Advective_Energy_Flux_Boundary.hpp"
#include "cl_FEM_IWG_Compressible_NS_Advective_Momentum_Flux_Boundary.hpp"
#include "cl_FEM_IWG_Compressible_NS_Mass_Flux_Neumann.hpp"
#include "cl_FEM_IWG_Compressible_NS_Traction_Neumann.hpp"
#include "cl_FEM_IWG_Compressible_NS_Heat_Flux_Neumann.hpp"
#include "cl_FEM_IWG_Compressible_NS_Velocity_Dirichlet_Nitsche.hpp"
#include "cl_FEM_IWG_Compressible_NS_Temperature_Dirichlet_Nitsche.hpp"
//Fluid structure interface
#include "cl_FEM_IWG_FS_Struc_Interface.hpp"
//Time continuity
#include "cl_FEM_IWG_Time_Continuity_Dof.hpp"
//Turbulence
#include "cl_FEM_IWG_Spalart_Allmaras_Turbulence_Bulk.hpp"
#include "cl_FEM_IWG_Spalart_Allmaras_Turbulence_Dirichlet.hpp"
#include "cl_FEM_IWG_Spalart_Allmaras_Turbulence_Interface.hpp"
//Contact
#include "cl_FEM_IWG_Isotropic_Struc_Linear_Contact_Penalty.hpp"
#include "cl_FEM_IWG_Isotropic_Struc_Linear_Contact_Nitsche.hpp"
//Ghost
#include "cl_FEM_IWG_Ghost_Normal_Field.hpp"

namespace moris
{
    namespace fem
    {
        //------------------------------------------------------------------------------

        std::shared_ptr< IWG > IWG_Factory::create_IWG( IWG_Type aIWGType )
        {
            switch( aIWGType )
            {

                //------------------------------------------------------------------------------

                case IWG_Type::L2 :
                    return std::make_shared< IWG_L2 >();

                case IWG_Type::HJTEST :
                    return std::make_shared< IWG_Hamilton_Jacobi_Bulk_Test >();

                    //                case IWG_Type::HELMHOLTZ :
                    //                    return std::make_shared< IWG_Helmholtz_Bulk >();
                    //
                    //                case IWG_Type::HJ :
                    //                    return std::make_shared< IWG_Hamilton_Jacobi_Bulk2 >();
                    //
                    //                case IWG_Type::LSNORMAL :
                    //                    return std::make_shared< IWG_LSNormal_Bulk >();
                    //
                    //                case IWG_Type::OLSSON :
                    //                    return std::make_shared< IWG_Olsson_CLS_Bulk >();

                    //------------------------------------------------------------------------------

                case IWG_Type::SPATIALDIFF_BULK :
                    return std::make_shared< IWG_Diffusion_Bulk >();

                case IWG_Type::SPATIALDIFF_DIRICHLET_SYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Diffusion_Dirichlet_Nitsche >( -1 );

                case IWG_Type::SPATIALDIFF_DIRICHLET_UNSYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Diffusion_Dirichlet_Nitsche >( 1 );

                case IWG_Type::SPATIALDIFF_NEUMANN :
                    return std::make_shared< IWG_Diffusion_Neumann >();

                case IWG_Type::SPATIALDIFF_ROBIN :
                    return std::make_shared< IWG_Diffusion_Robin >();

                case IWG_Type::SPATIALDIFF_RADIATION :
                    return std::make_shared< IWG_Diffusion_Radiation >();

                case IWG_Type::SPATIALDIFF_INTERFACE_SYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Diffusion_Interface >( 1 );

                case IWG_Type::SPATIALDIFF_INTERFACE_UNSYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Diffusion_Interface >( -1 );

                case IWG_Type::SPATIALDIFF_VW_GHOST :
                    return std::make_shared< IWG_Diffusion_Virtual_Work_Ghost >();

                    //------------------------------------------------------------------------------

                case IWG_Type::ADVECTION_BULK :
                    return std::make_shared< IWG_Advection_Bulk >();

                case IWG_Type::STRUC_LINEAR_BULK :
                    return std::make_shared< IWG_Isotropic_Struc_Linear_Bulk >();

                    //------------------------------------------------------------------------------

                case IWG_Type::STRUC_LINEAR_DIRICHLET_SYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Isotropic_Struc_Linear_Dirichlet >( -1 );

                case IWG_Type::STRUC_LINEAR_DIRICHLET_UNSYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Isotropic_Struc_Linear_Dirichlet >( 1 );

                case IWG_Type::STRUC_LINEAR_INTERFACE_SYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Isotropic_Struc_Linear_Interface >( -1 );

                case IWG_Type::STRUC_LINEAR_INTERFACE_UNSYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Isotropic_Struc_Linear_Interface >( 1 );

                case IWG_Type::STRUC_LINEAR_NEUMANN :
                    return std::make_shared< IWG_Isotropic_Struc_Linear_Neumann >();

                case IWG_Type::STRUC_LINEAR_VW_GHOST :
                    return std::make_shared< IWG_Isotropic_Struc_Linear_Virtual_Work_Ghost >();

                case IWG_Type::STRUC_LINEAR_PRESSURE_BULK :
                    return std::make_shared< IWG_Isotropic_Struc_Linear_Pressure_Bulk >();

                case IWG_Type::STRUC_LINEAR_PRESSURE_DIRICHLET_SYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Isotropic_Struc_Linear_Pressure_Dirichlet >( -1 );

                case IWG_Type::STRUC_LINEAR_PRESSURE_DIRICHLET_UNSYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Isotropic_Struc_Linear_Pressure_Dirichlet >( 1 );

                case IWG_Type::STRUC_LINEAR_CONTACT_SYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Isotropic_Struc_Linear_Contact_Nitsche >( -1 );

                case IWG_Type::STRUC_LINEAR_CONTACT_UNSYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Isotropic_Struc_Linear_Contact_Nitsche >( 1 );

                case IWG_Type::STRUC_LINEAR_CONTACT_PENALTY :
                    return std::make_shared< IWG_Isotropic_Struc_Linear_Contact_Penalty >();

                    //------------------------------------------------------------------------------

                case IWG_Type::INCOMPRESSIBLE_NS_VELOCITY_BULK :
                    return std::make_shared< IWG_Incompressible_NS_Velocity_Bulk >();

                case IWG_Type::INCOMPRESSIBLE_NS_PRESSURE_BULK :
                    return std::make_shared< IWG_Incompressible_NS_Pressure_Bulk >();

                case IWG_Type::INCOMPRESSIBLE_NS_CONVECTIVE_VELOCITY_GHOST :
                    return std::make_shared< IWG_Incompressible_NS_Convective_Velocity_Ghost >();

                case IWG_Type::INCOMPRESSIBLE_NS_VELOCITY_DIRICHLET_SYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Incompressible_NS_Velocity_Dirichlet_Nitsche >( 1 );

                case IWG_Type::INCOMPRESSIBLE_NS_VELOCITY_DIRICHLET_UNSYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Incompressible_NS_Velocity_Dirichlet_Nitsche >( -1 );

                case IWG_Type::INCOMPRESSIBLE_NS_PRESSURE_DIRICHLET_SYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Incompressible_NS_Pressure_Dirichlet_Nitsche >( 1 );

                case IWG_Type::INCOMPRESSIBLE_NS_PRESSURE_DIRICHLET_UNSYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Incompressible_NS_Pressure_Dirichlet_Nitsche >( -1 );

                case IWG_Type::INCOMPRESSIBLE_NS_IMPOSED_PRESSURE :
                    return std::make_shared< IWG_Incompressible_NS_Pressure_Neumann >();

                case IWG_Type::INCOMPRESSIBLE_NS_VELOCITY_INTERFACE_SYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Incompressible_NS_Velocity_Interface >( 1 );

                case IWG_Type::INCOMPRESSIBLE_NS_VELOCITY_INTERFACE_UNSYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Incompressible_NS_Velocity_Interface >( -1 );

                case IWG_Type::INCOMPRESSIBLE_NS_PRESSURE_INTERFACE_SYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Incompressible_NS_Pressure_Interface >( 1 );

                case IWG_Type::INCOMPRESSIBLE_NS_PRESSURE_INTERFACE_UNSYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Incompressible_NS_Pressure_Interface >( -1 );

                    //------------------------------------------------------------------------------

                case IWG_Type::COMPRESSIBLE_NS_DENSITY_BULK :
                    return std::make_shared< IWG_Compressible_NS_Density_Bulk >();

                case IWG_Type::COMPRESSIBLE_NS_VELOCITY_BULK :
                    return std::make_shared< IWG_Compressible_NS_Velocity_Bulk >();

                case IWG_Type::COMPRESSIBLE_NS_TEMPERATURE_BULK :
                    return std::make_shared< IWG_Compressible_NS_Temperature_Bulk >();

                case IWG_Type::COMPRESSIBLE_NS_ADVECTIVE_MOMENTUM_FLUX:
                    return std::make_shared< IWG_Compressible_NS_Advective_Momentum_Flux_Boundary >();

                case IWG_Type::COMPRESSIBLE_NS_ADVECTIVE_ENERGY_FLUX:
                    return std::make_shared< IWG_Compressible_NS_Advective_Energy_Flux_Boundary >();

                case IWG_Type::COMPRESSIBLE_NS_MASS_FLUX_NEUMANN :
                    return std::make_shared< IWG_Compressible_NS_Mass_Flux_Neumann >();

                case IWG_Type::COMPRESSIBLE_NS_TRACTION_NEUMANN :
                    return std::make_shared< IWG_Compressible_NS_Traction_Neumann >();

                case IWG_Type::COMPRESSIBLE_NS_HEAT_FLUX_NEUMANN :
                    return std::make_shared< IWG_Compressible_NS_Heat_Flux_Neumann >();

                case IWG_Type::COMPRESSIBLE_NS_VELOCITY_DIRICHLET_SYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Compressible_NS_Velocity_Dirichlet_Nitsche >( 1 );

                case IWG_Type::COMPRESSIBLE_NS_VELOCITY_DIRICHLET_UNSYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Compressible_NS_Velocity_Dirichlet_Nitsche >( -1 );

                case IWG_Type::COMPRESSIBLE_NS_TEMPERATURE_DIRICHLET_SYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Compressible_NS_Temperature_Dirichlet_Nitsche >( 1 );

                case IWG_Type::COMPRESSIBLE_NS_TEMPERATURE_DIRICHLET_UNSYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Compressible_NS_Temperature_Dirichlet_Nitsche >( -1 );

                    //------------------------------------------------------------------------------

                case IWG_Type::FS_STRUC_INTERFACE :
                    return std::make_shared< IWG_FS_Struc_Interface >();

                case IWG_Type::TIME_CONTINUITY_DOF :
                    return std::make_shared< IWG_Time_Continuity_Dof >();

                case IWG_Type::SPALART_ALLMARAS_TURBULENCE_BULK :
                    return std::make_shared< IWG_Spalart_Allmaras_Turbulence_Bulk >();

                case IWG_Type::SPALART_ALLMARAS_TURBULENCE_DIRICHLET_SYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Spalart_Allmaras_Turbulence_Dirichlet >( 1 );

                case IWG_Type::SPALART_ALLMARAS_TURBULENCE_DIRICHLET_UNSYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Spalart_Allmaras_Turbulence_Dirichlet >( -1 );

                case IWG_Type::SPALART_ALLMARAS_TURBULENCE_INTERFACE_SYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Spalart_Allmaras_Turbulence_Interface >( 1 );

                case IWG_Type::SPALART_ALLMARAS_TURBULENCE_INTERFACE_UNSYMMETRIC_NITSCHE :
                    return std::make_shared< IWG_Spalart_Allmaras_Turbulence_Interface >( -1 );

                case IWG_Type::GHOST_NORMAL_FIELD :
                    return std::make_shared< IWG_Ghost_Normal_Field >();

                default:
                    MORIS_ERROR( false, " IWG_Factory::create_IWGs - IWG type specified is not defined. " );
                    return nullptr;
            }
        }
        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */