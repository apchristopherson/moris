/*
 * cl_Equation_Object.cpp
 *
 *  Created on: Jul 14, 2018
 *      Author: schmidt
 */

#include "cl_Equation_Object.hpp"
#include "cl_Solver_Factory.hpp" // DLA/src
#include "cl_Solver_Input.hpp"

namespace moris
{
    namespace MSI
    {
    //FIXME will be deleted soon
    void Equation_Object::get_pdofs_values()
    {
        // pdof values of this element
        Mat< real > tPdofValues;

        moris::Mat< moris::real> tTMatrix;

        this->build_PADofMap( tTMatrix );

        mLin->extract_my_values( mUniqueAdofList.length(), mUniqueAdofList, 0, tPdofValues);

        //tPdofValues = trans( tTMatrix ) * tPdofValues;

        // fixme: check if transposed or not
        tPdofValues = tTMatrix * tPdofValues;
        tPdofValues.print("Pdofs");

    }
    //FIXME will be deleted soon
    void Equation_Object::set_solver( std::shared_ptr< Linear_Solver > aLin)
    {
       mLin = aLin;
    }
}
}
