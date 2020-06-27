#ifndef MORIS_CL_WRK_PERFORMER_HPP
#define MORIS_CL_WRK_PERFORMER_HPP

#include "cl_Matrix.hpp"

namespace moris
{
    namespace wrk
    {
        class Performer
        {

        public:

            /**
             * Constructor
             */
            Performer();

            /**
             * Return the number of fields that can be used for refinement
             *
             * @return Number of fields for refinement
             */
            virtual uint get_num_refinement_fields();

            /**
             * Gets a flag to determine if refinement should continue
             *
             * @param aFieldIndex The index of a field
             * @param aRefinementNumber The current refinement step being performed
             * @return If refinement is needed for this field
             */
            virtual bool refinement_needed(uint aFieldIndex,
                                           uint aRefinementNumber);

            /**
             * Returns fields so that HMR can perform refinement based on the data from this performer
             *
             * @param aFieldIndex Index of the field
             * @param aNodeIndex Index of the node
             * @param aCoordinates Coordinates of the node
             */
            virtual real get_field_value(uint aFieldIndex,
                                         uint aNodeIndex,
                                         const Matrix<DDRMat>& aCoordinates);

            /**
             * Gets the index of an HMR user-defined refinement function for the given field index
             *
             * @param aFieldIndex Index of the field
             * @param aRefinementNumber The current refinement step being performed
             * @return User-defined function index, or -1 to use default refinement
             */
            virtual sint get_refinement_function_index(uint aFieldIndex,
                                                       uint aRefinementNumber);

        };
    }
}

#endif //MORIS_CL_WRK_PERFORMER_HPP
