//
// Created by christopherson on 5/18/20.
//

#ifndef MORIS_CL_Field_HPP
#define MORIS_CL_Field_HPP

#include "cl_Matrix.hpp"

namespace moris
{
    namespace ge
    {
        class Field
        {
        protected:
            Cell<real*> mFieldVariables;

        private:
            Cell<bool> mActiveVariables;
            Matrix<DDUMat> mADVIndices;
            Matrix<DDRMat> mConstantParameters;

        protected:

            /**
             * Constructor, sets the pointers to advs and constant parameters for evaluations
             *
             * @param aADVs Reference to the full advs
             * @param aFieldVariableIndices Indices of geometry variables to be filled by the ADVs
             * @param aADVIndices The indices of the ADV vector to fill in the geometry variables
             * @param aConstantParameters The constant parameters not filled by ADVs
             */
            Field(Matrix<DDRMat>& aADVs,
                  Matrix<DDUMat> aFieldVariableIndices,
                  Matrix<DDUMat> aADVIndices,
                  Matrix<DDRMat> aConstantParameters);

            /**
             * Constructor for only constant parameters
             *
             * @param aConstantParameters The parameters that define this field
             */
            Field(Matrix<DDRMat> aConstantParameters);

        public:

            /**
             * Destructor
             */
            ~Field();

            /**
             * Get the indices of the ADVs which this field depends on
             *
             * @return Vector of ADV indices
             */
            Matrix<DDUMat> get_adv_indices();

            /**
             * Given a node index or coordinate, returns the field value
             *
             * @param aIndex Node index
             * @param aCoordinates Vector of coordinate values
             * @return Field value
             */
            virtual real evaluate_field_value(      uint            aIndex,
                                              const Matrix<DDRMat>& aCoordinates) = 0;

            /**
             * Given a node index or coordinate, returns a matrix of relevant sensitivities
             *
             * @param aIndex Node index
             * @param aCoordinates Vector of coordinate values
             * @param aSensitivity Matrix of sensitivities
             */
            void evaluate_sensitivity(      uint            aIndex,
                                      const Matrix<DDRMat>& aCoordinates,
                                            Matrix<DDRMat>& aSensitivities);

        private:

            /**
             * Given a node coordinate @param aCoordinates, the function returns a matrix of all sensitivities
             *
             * @param aCoordinates Vector of coordinate values
             * @param aSensitivity Matrix of sensitivities
             */
            virtual void evaluate_all_sensitivities(      uint            aIndex,
                                                    const Matrix<DDRMat>& aCoordinates,
                                                          Matrix<DDRMat>& aSensitivities) = 0;

        };
    }
}


#endif //MORIS_CL_Field_HPP
