
#ifndef PROJECTS_HMR_SRC_CL_HMR_ARGUMENTS_HPP_
#define PROJECTS_HMR_SRC_CL_HMR_ARGUMENTS_HPP_
#include <string>

#include "../../../HMR/src/cl_HMR_State.hpp"

namespace moris
{
    namespace hmr
    {
//--------------------------------------------------------------------------------
        class Arguments
        {
            std::string mParameterPath  = "";
            State       mState;
            double      mTimestep = 0.0;
            bool        mMapWhileRefine = false;

//--------------------------------------------------------------------------------
        public:
//--------------------------------------------------------------------------------

            Arguments(
                    int  & argc,
                    char * argv[] );

//---------------------------------------------------------------------------------

            void
            print_usage();

//---------------------------------------------------------------------------------

            void
            print_help();

//---------------------------------------------------------------------------------

            /**
             * return the run state of the executable
             */
            State
            get_state() const
            {
                return mState;
            }

//---------------------------------------------------------------------------------

            /**
             * return the parameter path
             */
            const std::string &
            get_parameter_path() const
            {
                return mParameterPath;
            }

//---------------------------------------------------------------------------------

            /**
             * return the timestep that is written into the exodus file
             */
            const double
            get_timestep() const
            {
                return mTimestep;
            }

//---------------------------------------------------------------------------------

            /**
             * returns true if user wants to call refinement and mapping at the same time
             */
            bool
            map_while_refine() const
            {
                return mMapWhileRefine;
            }
        };
//---------------------------------------------------------------------------------
    }
}

#endif /* PROJECTS_HMR_SRC_CL_HMR_ARGUMENTS_HPP_ */
