/*
 * cl_MSI_Design_Variable_Interface.cpp
 *
 *  Created on: Jan 10, 20120
 *      Author: schmidt
 */
#include "cl_MSI_Design_Variable_Interface.hpp"

#include "cl_SOL_Dist_Vector.hpp"

#include "cl_MSI_Equation_Model.hpp"

#include "fn_trans.hpp"

namespace moris
{
    namespace MSI
    {

//-------------------------------------------------------------------------------------------------------
        void Design_Variable_Interface::set_requested_IQIs( const moris::Cell< std::string > & aRequestedIQIs )
        {
            mModel->set_requested_IQI_names(aRequestedIQIs);
        }

//-------------------------------------------------------------------------------------------------------

        Matrix<DDRMat> Design_Variable_Interface::get_dQidu()
        {
            Matrix<DDRMat> tSensitivities;
            mModel->get_dQidu()->extract_copy(tSensitivities);

            //print( tSensitivities, "tSensitivities");
            return trans(tSensitivities);
        }


    }
}
