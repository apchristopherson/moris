//
// Created by christopherson on 5/19/20.
//

#ifndef MORIS_CL_GEN_PDV_PROPERTY_HPP
#define MORIS_CL_GEN_PDV_PROPERTY_HPP

#include "cl_GEN_Pdv.hpp"
#include "cl_GEN_Property.hpp"

namespace moris
{
    namespace ge
    {
        class Pdv_Property : public Pdv
        {

        private:
            std::shared_ptr<Property> mProperty;

        public:
            /**
             * Constructor
             *
             * @param aPropertyPointer a GEN property pointer
             */
            Pdv_Property(std::shared_ptr<Property> aPropertyPointer);

            /**
             * Get the PDV value
             *
             * @param aNodeIndex Node index
             * @param aCoordinates Coordinate values
             * @return Current value of this PDV
             */
            real get_value(uint aNodeIndex, const Matrix<DDRMat>& aCoordinates);

            /**
             * Get the PDV sensitivity with respect to ADVs
             *
             * @param aNodeIndex Node index
             * @param aCoordinates Coordinate values
             * @return Matrix of sensitivities to be returned
             */
            Matrix<DDRMat> get_sensitivities(uint aNodeIndex, const Matrix<DDRMat>& aCoordinates);

            /**
             * Gets the IDs of ADVs which this PDV depends on.
             *
             * @return ADV IDs
             */
            Matrix<DDSMat> get_determining_adv_ids();

        };
    }
}

#endif //MORIS_CL_GEN_PDV_PROPERTY_HPP
