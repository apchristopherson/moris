/*
 * cl_GEN_Field_User_Defined.hpp
 *
 *  Created on: Nov 5, 2019
 *      Author: sonne
 */

#ifndef PROJECTS_GEN_SRC_NEW_FIELD_CL_GEN_FIELD_USER_DEFINED_HPP_
#define PROJECTS_GEN_SRC_NEW_FIELD_CL_GEN_FIELD_USER_DEFINED_HPP_

#include "typedefs.hpp"
#include "cl_Matrix.hpp"
#include "cl_Cell.hpp"

#include "cl_FEM_Field_Interpolator.hpp"

#include "cl_GEN_Field.hpp"


namespace moris{
namespace ge{

//typedef std::function< Matrix< DDRMat > ( moris::Cell< Matrix< DDRMat > >                & aCoeff,
//                                          moris::Cell< moris::fem::Field_Interpolator* > & aDofFI,
//                                          moris::Cell< moris::fem::Field_Interpolator* > & aDvFI,
//                                          moris::fem::Geometry_Interpolator              * aGeometryInterpolator ) > PropertyFunc;

typedef std::function< real ( Matrix< DDRMat > const & aCoeff,
                              Matrix< DDRMat > const & aParam ) > PropertyFunction_noFieldInterp;


class GEN_Field_User_Defined    :   public GEN_Field    //fixme: this should take in a property at construction and use the property to perform evaluations
{
private:
    PropertyFunction_noFieldInterp mValFunc;

    Matrix< DDRMat >               mCoeffList;
//    moris::Cell< real >            mParamList;
    //------------------------------------------------------------------------------

public:
    GEN_Field_User_Defined(  )
    {    }

    GEN_Field_User_Defined( PropertyFunction_noFieldInterp aValFunc,
                            Matrix< DDRMat >               aCoeff ) :
                                mValFunc( aValFunc ),
                                mCoeffList( aCoeff )
    {    }

    ~GEN_Field_User_Defined(  )
    {
    }
    //------------------------------------------------------------------------------
    void set_coeff_list( Matrix< DDRMat > & aCoeff )
    {
        mCoeffList = aCoeff;
    }
    //------------------------------------------------------------------------------
    real eval_function( Matrix< DDRMat > const & aParam )
    {
        return mValFunc( mCoeffList, aParam );
    }

};

}   // end ge namespace
}   // end moris namespace



#endif /* PROJECTS_GEN_SRC_NEW_FIELD_CL_GEN_FIELD_USER_DEFINED_HPP_ */
