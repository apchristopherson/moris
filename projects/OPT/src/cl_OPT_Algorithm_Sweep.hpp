#ifndef MORIS_CL_OPT_ALGORITHM_SWEEP_HPP_
#define MORIS_CL_OPT_ALGORITHM_SWEEP_HPP_

#include "core.hpp"
#include "cl_OPT_Algorithm.hpp"
#include "HDF5_Tools.hpp"

namespace moris
{
    namespace opt
    {
        class Algorithm_Sweep : public Algorithm
        {
        public:

            /**
             * Constructor
             */
            Algorithm_Sweep(ParameterList aParameterList);

            /**
             * Destructor
             */
            ~Algorithm_Sweep();

            /**
             * @brief MORIS interface for solving of optimization problem using
             *        GCMMA
             *
             * @param[in] aOptProb Object of type Problem containing relevant
             *            data regarding ADVs, the objective and constraints
             */
            void solve(std::shared_ptr<Problem> aOptProb );

        private:
            bool mSave; // If saving the results of the sweep to an hdf5 file
            bool mUpdateObjectives; // whether or not to compute new objectives when requested
            bool mUpdateConstraints; // whether or not to compute new constraints when requested
            bool mUpdateObjectiveGradients; // "                 " objective gradients
            bool mUpdateConstraintGradients; // "                 " constraint gradients
            hid_t mFileID; // Fild id for hdf5 file

            /**
             * Outputs the optimization problem at the current ADVs (objective and constraints)
             *
             * @param aEvaluationName The name to be printed/saved after the optimization variable type
             */
            void output_objectives_constraints(std::string aEvaluationName);

            /**
             * Evaluates the objective gradients at the current ADVs and outputs them
             *
             * @param aEvaluationName The name to be printed/saved after the optimization variable type
             */
            Matrix<DDRMat> evaluate_objective_gradients(std::string aEvaluationName);

            /**
             * Evaluates the constraint gradients at the current ADVs and outputs them
             *
             * @param aEvaluationName The name to be printed/saved after the optimization variable type
             */
            Matrix<DDRMat> evaluate_constraint_gradients(std::string aEvaluationName);

            /**
             * Outputs the given optimization variables based on printing/saving options
             *
             * @param aVariables Matrix of optimization variables
             * @param aFullEvaluationName Full name to be output to the screen/hdf5
             */
            void output_variables(Matrix<DDRMat> aVariables, std::string aFullEvaluationName);

        };
    }
}

#endif /* MORIS_CL_OPT_ALGORITHM_SWEEP_HPP_ */

