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
    inline
    ParameterList create_moris_general_parameter_list()
    {
        ParameterList tParameterList;

        return tParameterList;
    }

    //------------------------------------------------------------------------------
    inline
    void create_refinement_parameterlist( ParameterList & aParameterlist )
    {
        aParameterlist.insert( "field_names" , "" );
        aParameterlist.insert( "levels_of_refinement" , "" );
        aParameterlist.insert( "refinement_pattern" , "" );

        aParameterlist.insert( "refinement_function_name" , "" );

        aParameterlist.insert( "remeshing_copy_old_pattern_to_pattern" , "" );
    }

    //------------------------------------------------------------------------------
    inline
    void create_remeshing_parameterlist( ParameterList & aParameterlist )
    {
        // Remeshing mode. Options are "ab_initio", "former"
        aParameterlist.insert( "mode" , "" );
        aParameterlist.insert( "remeshing_refinement_pattern" , "" );
        aParameterlist.insert( "refinement_function_name" , "" );

        // mode "ab_initio"
        aParameterlist.insert( "remeshing_field_names" , "" );
        aParameterlist.insert( "remeshing_levels_of_refinement" , "" );
        aParameterlist.insert( "remeshing_refinement_pattern" , "" );

        // mode "based_on_previous"
        aParameterlist.insert( "remeshing_maximum_refinement_level" , "" );
        aParameterlist.insert( "remeshing_minimum_refinement_level" , "" );

        aParameterlist.insert( "remeshing_copy_old_pattern_to_pattern" , "" );

        // minimum refinement level per pattern and level
        // input: pattern, level, iter, level, iter,level, iter, ... ; pattern, level, iter, level, iter,level, iter, ...
        aParameterlist.insert( "minimum_refinement_level" , "" );






        // modde "previous"


    }

    //------------------------------------------------------------------------------
    inline
    void create_mapping_parameterlist( ParameterList & aParameterlist )
    {

    }

    //------------------------------------------------------------------------------
//------------------------------------------------------------------------------

    }/* end_namespace_prm */
}/* end_namespace_moris */

#endif /* PROJECTS_PRM_SRC_FN_PRM_MORIS_GENERAL_PARAMETERS_HPP_ */
