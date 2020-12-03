/*
 * cl_MSI_Dof_Type_Enums.hpp
 *
 *  Created on: Jul 23, 2018
 *      Author: schmidt
 */
#ifndef SRC_FEM_CL_DOF_TYPE_ENUMS_HPP_
#define SRC_FEM_CL_DOF_TYPE_ENUMS_HPP_

#include "cl_Map.hpp"

namespace moris
{
    namespace MSI
    {
        enum class Dof_Type
        {
            UX,          //< X-Displacement
            UY,          //< Y-Displacement
            UZ,          //< Z-Displacement
            TEMP,        //< Temperature degree of freedom
            L2,          //< Least Squares type
            MAPPING_DOF, //< L2 Mapping
            LS1,         //< Level set
            LS2,         //< Level set
            NLSX,        //< X-Level set normal
            NLSY,        //< Y-Level set normal
            NLSZ,        //< Z-Level set normal
            THETA,       //< Heat method temperature
            PHID,        //< Heat method distance field
            PHISD,       //< Heat method signed distance field
            VX,          //< X-Velocity
            VY,          //< Y-Velocity
            VZ,          //< Z-Velocity
            P,           //< Pressure
            RHO,         //< Density
            VISCOSITY,   //< Turbulence viscosity
            UNDEFINED,   //< Undefined
            END_ENUM     //
        };

        //------------------------------------------------------------------------------

        map< std::string, enum MSI::Dof_Type > get_msi_dof_type_map()
        {
            map< std::string, enum MSI::Dof_Type > tMSIDofTypeMap;

            tMSIDofTypeMap["UX"]          = MSI::Dof_Type::UX;
            tMSIDofTypeMap["UY"]          = MSI::Dof_Type::UY;
            tMSIDofTypeMap["UZ"]          = MSI::Dof_Type::UZ;
            tMSIDofTypeMap["TEMP"]        = MSI::Dof_Type::TEMP;
            tMSIDofTypeMap["L2"]          = MSI::Dof_Type::L2;
            tMSIDofTypeMap["MAPPING_DOF"] = MSI::Dof_Type::MAPPING_DOF;
            tMSIDofTypeMap["LS1"]         = MSI::Dof_Type::LS1;
            tMSIDofTypeMap["LS2"]         = MSI::Dof_Type::LS2;
            tMSIDofTypeMap["NLSX"]        = MSI::Dof_Type::NLSX;
            tMSIDofTypeMap["NLSY"]        = MSI::Dof_Type::NLSY;
            tMSIDofTypeMap["NLSZ"]        = MSI::Dof_Type::NLSZ;
            tMSIDofTypeMap["THETA"]       = MSI::Dof_Type::THETA;
            tMSIDofTypeMap["PHID"]        = MSI::Dof_Type::PHID;
            tMSIDofTypeMap["PHISD"]       = MSI::Dof_Type::PHISD;
            tMSIDofTypeMap["VX"]          = MSI::Dof_Type::VX;
            tMSIDofTypeMap["VY"]          = MSI::Dof_Type::VY;
            tMSIDofTypeMap["VZ"]          = MSI::Dof_Type::VZ;
            tMSIDofTypeMap["P"]           = MSI::Dof_Type::P;
            tMSIDofTypeMap["RHO"]         = MSI::Dof_Type::RHO;
            tMSIDofTypeMap["VISCOSITY"]   = MSI::Dof_Type::VISCOSITY;
            tMSIDofTypeMap["UNDEFINED"]   = MSI::Dof_Type::UNDEFINED;

            return tMSIDofTypeMap;
        }

        //------------------------------------------------------------------------------

        map< enum MSI::Dof_Type, std::string > get_dof_type_name_map()
        {
            map< enum MSI::Dof_Type, std::string  > tMSIDofTypeMap;

            tMSIDofTypeMap[MSI::Dof_Type::UX]          = "UX";
            tMSIDofTypeMap[MSI::Dof_Type::UY]          = "UY";
            tMSIDofTypeMap[MSI::Dof_Type::UZ]          = "UZ";
            tMSIDofTypeMap[MSI::Dof_Type::TEMP]        = "TEMP";
            tMSIDofTypeMap[MSI::Dof_Type::L2]          = "L2";
            tMSIDofTypeMap[MSI::Dof_Type::MAPPING_DOF] = "MAPPING_DOF";
            tMSIDofTypeMap[MSI::Dof_Type::LS1]         = "LS1";
            tMSIDofTypeMap[MSI::Dof_Type::LS2]         = "LS2";
            tMSIDofTypeMap[MSI::Dof_Type::NLSX]        = "NLSX";
            tMSIDofTypeMap[MSI::Dof_Type::NLSY]        = "NLSY";
            tMSIDofTypeMap[MSI::Dof_Type::NLSZ]        = "NLSZ";
            tMSIDofTypeMap[MSI::Dof_Type::THETA]       = "THETA";
            tMSIDofTypeMap[MSI::Dof_Type::PHID]        = "PHID";
            tMSIDofTypeMap[MSI::Dof_Type::PHISD]       = "PHISD";
            tMSIDofTypeMap[MSI::Dof_Type::VX]          = "VX";
            tMSIDofTypeMap[MSI::Dof_Type::VY]          = "VY";
            tMSIDofTypeMap[MSI::Dof_Type::VZ]          = "VZ";
            tMSIDofTypeMap[MSI::Dof_Type::P]           = "P";
            tMSIDofTypeMap[MSI::Dof_Type::RHO]         = "RHO";
            tMSIDofTypeMap[MSI::Dof_Type::VISCOSITY]   = "VISCOSITY";
            tMSIDofTypeMap[MSI::Dof_Type::UNDEFINED]   = "UNDEFINED";

            return tMSIDofTypeMap;
        }
    }
}

#endif /* SRC_FEM_CL_DOF_TYPE_ENUMS_HPP_ */
