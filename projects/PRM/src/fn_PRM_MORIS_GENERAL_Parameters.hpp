/*
 * fn_PRM_MORIS_GENERAL_Parameters.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: schmidt
 */

#ifndef PROJECTS_PRM_SRC_FN_PRM_MORIS_GENERAL_PARAMETERS_HPP_
#define PROJECTS_PRM_SRC_FN_PRM_MORIS_GENERAL_PARAMETERS_HPP_

#include <string>
#include <cstdio>

#include "assert.hpp"
//#include "cl_Communication_Tools.hpp"
#include "typedefs.hpp"
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
#include "cl_XML_Parser.hpp"

#include "cl_Param_List.hpp"

namespace moris
{
    namespace prm
    {

//------------------------------------------------------------------------------

    // creates a parameter list with default inputs
    ParameterList create_moris_general_parameter_list()
    {
        ParameterList tParameterList;

        return tParameterList;
    }

    //------------------------------------------------------------------------------

    void create_refinement_parameterlist( ParameterList & aParameterlist )
    {
        aParameterlist.insert( "field_names" , "" );
        aParameterlist.insert( "levels_of_refinement" , "" );
        aParameterlist.insert( "refinement_pattern" , "" );
    }

    //------------------------------------------------------------------------------

    void create_remeshing_parameterlist( ParameterList & aParameterlist )
    {
        aParameterlist.insert( "mode" , "" );

    }

    //------------------------------------------------------------------------------

    void create_mapping_parameterlist( ParameterList & aParameterlist )
    {

    }

    //------------------------------------------------------------------------------
//------------------------------------------------------------------------------

    }/* end_namespace_prm */
}/* end_namespace_moris */

#endif /* PROJECTS_PRM_SRC_FN_PRM_MORIS_GENERAL_PARAMETERS_HPP_ */