/*
 * cl_GE_Factory.hpp
 *
 *  Created on: Dec 28, 2018
 *      Author: sonne
 */
#ifndef SRC_DISTLINALG_CL_GE_FACTORY_HPP_
#define SRC_DISTLINALG_CL_GE_FACTORY_HPP_

#include <memory>
#include <iostream>

#include "cl_GE_Analytical.hpp"
#include "cl_GE_Discrete.hpp"
#include "cl_GE_Enums.hpp"
#include "cl_GE_Geometry.hpp"
#include "cl_GE_SDF.hpp"

#include "assert.hpp"

namespace moris
{
    namespace ge
    {

    class Ge_Factory
    {
    	public:
    		Ge_Factory(){

    		};
    		~Ge_Factory(){
    		};

    	    /**
    	     * @brief factory member function building GE types
    	     *
    	     * @param[in]  aFlagType    - determines the type of geometry
    	     * .
    	     * @param[out] tFlagPointer - GE pointer to base class.
    	     *
    	     */
    		Geometry* set_geometry_type(enum type aGeomType = type::ANALYTIC)
    		{
    			Geometry* tGeomPointer;
    			switch(aGeomType)
    			{
    			case(type::ANALYTIC):
    					tGeomPointer = new Analytical();
    					break;
                case(type::DISCRETE):
                        tGeomPointer = new Discrete();
                        break;
    			default:
    					MORIS_ERROR(false, "Ge_Factory::set_geometry_type() please input a valid geometry type");
    					break;
    			}
    		return tGeomPointer;
    		}

        private:
            int flagType = 0;

        protected:
    };
    } /* namespace gen */
} /* namespace moris */

#endif /* SRC_DISTLINALG_CL_GE_FACTORY_HPP_ */
