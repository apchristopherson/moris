/*
 * fn_PRM_XTK_Parameters.hpp
 *
 *  Created on: March 10, 2020
 *      Author: doble
 */

#ifndef PROJECTS_PRM_SRC_FN_PRM_XTK_PARAMETERS_HPP_
#define PROJECTS_PRM_SRC_FN_PRM_XTK_PARAMETERS_HPP_

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
    ParameterList create_xtk_parameter_list()
    {
        ParameterList tParameterList;

        // decomposition and decomposition related parameters
        tParameterList.insert( "decompose", true );
        tParameterList.insert( "decomposition_type", "conformal" );

        // enrichment and enrichment related parameters
        tParameterList.insert( "enrich", false );
        tParameterList.insert( "basis_rank","node" );
        tParameterList.insert( "enrich_mesh_indices","0");

        // ghost stabilization and ghost related parameters
        tParameterList.insert( "ghost_stab", false );

        // multigrid
        tParameterList.insert( "multigrid", false );

        // contact sandbox
        tParameterList.insert( "contact_sandbox", false );
        tParameterList.insert( "potential_phases_in_contact", "" );
        tParameterList.insert( "bb_epsilon", 0.1 );

        // verbose - should be replaced by the severity level of the logger
        tParameterList.insert( "verbose", false );

        // if to deactivate empty sets - used only if outputting ig mesh as well, set to true only for debugging
        tParameterList.insert( "deactivate_empty_sets", false );

        // print enriched integration mesh
        tParameterList.insert( "print_enriched_ig_mesh", false );

        // write XTK exodus mesh
        tParameterList.insert( "exodus_output_XTK_ig_mesh", false );

        // add ghost blocks to XTK exodus mesh
        tParameterList.insert( "exodus_output_XTK_ghost_mesh", false );

        // enriched integration mesh options
        tParameterList.insert( "high_to_low_dbl_side_sets", false );

        // print memory usage
        tParameterList.insert( "print_memory", false );

        return tParameterList;
    }
//------------------------------------------------------------------------------

    }/* end_namespace_prm */
}/* end_namespace_moris */

#endif /* PROJECTS_PRM_SRC_FN_PRM_MSI_PARAMETERS_HPP_ */
