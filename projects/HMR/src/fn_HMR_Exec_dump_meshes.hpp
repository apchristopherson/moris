/*
 * fn_HMR_Exec_dump_meshes.hpp
 *
 *  Created on: Nov 13, 2018
 *      Author: messe
 */

#ifndef PROJECTS_HMR_SRC_FN_HMR_EXEC_DUMP_MESHES_HPP_
#define PROJECTS_HMR_SRC_FN_HMR_EXEC_DUMP_MESHES_HPP_

#include <string>

#include "assert.hpp"
#include "typedefs.hpp"
#include "cl_HMR.hpp"
#include "cl_HMR_Arguments.hpp"

namespace moris
{
    namespace hmr
    {
//--------------------------------------------------------------------------------

        void
        dump_meshes(
                const Arguments & aArguments,
                const Paramfile & aParamfile,
                HMR * aHMR )
        {
            // test if an output database path is given
            if( aParamfile.get_output_db_path().size() > 0 )
            {
                aHMR->save_to_hdf5( aParamfile.get_output_db_path() );
            }

            // test if coefficient path is given
            if( aParamfile.get_coefficient_db_path().size() > 0 )
            {
                aHMR->save_coeffs_to_hdf5_file( aParamfile.get_coefficient_db_path() );
            }

            // loop over all output meshes
            for( uint m=0; m<aParamfile.get_number_of_meshes(); ++m )
            {
                // test if path is given
                if ( aParamfile.get_mesh_path( m ).size() > 0 )
                {
                    // get orde rof mesh
                    uint tOrder = aParamfile.get_mesh_order( m );

                    uint tIndex;

                    // get index of mesh order
                    if( tOrder <= 2 )
                    {
                        tIndex = aHMR->get_mesh_index(
                                 tOrder,
                                 aHMR->get_parameters()->get_lagrange_output_pattern() );
                    }
                    else
                    {
                        tIndex = aHMR->get_mesh_index(
                                tOrder,
                                aHMR->get_parameters()->get_refined_output_pattern() );
                    }

                    // dump mesh
                    aHMR->save_to_exodus(
                        tIndex,
                        aParamfile.get_mesh_path( m ),
                        aArguments.get_timestep() );
                }
            }

        }

//--------------------------------------------------------------------------------
    }
}

#endif /* PROJECTS_HMR_SRC_FN_HMR_EXEC_DUMP_MESHES_HPP_ */
