#ifndef MORIS_CL_GEN_PROPERTY_HPP_
#define MORIS_CL_GEN_PROPERTY_HPP_

#include "cl_GEN_Field.hpp"
#include "st_GEN_Property_Parameters.hpp"

namespace moris
{
    namespace ge
    {
        class Property : virtual public Field
        {
        private:
            Property_Parameters mParameters;

        public:

            /**
             * Constructor for property, needs to know about other fields that it depends on.
             *
             * @param aFieldDependencies This property's dependencies
             */
            Property(Property_Parameters aParameters);

            /**
             * Copy constructor
             *
             * @param aProperty Property to copy
             */
            Property(std::shared_ptr<Property> aProperty);

            /**
             * Gets the PDV type that this property defines.
             *
             * @return PDV type
             */
            PDV_Type get_pdv_type();

            /**
             * Gets if this property's PDV type is defined on the interpolation mesh.
             *
             * @return mesh type switch
             */
            bool is_interpolation_pdv();

            /**
             * Gets the mesh set indices where this property's PDV is defined.
             *
             * @return Mesh set indices
             */
            Matrix<DDUMat> get_pdv_mesh_set_indices();

            /**
             * Gets the mesh set names where this property's PDV is defined.
             *
             * @return Mesh set names
             */
            Cell<std::string> get_pdv_mesh_set_names();

        };
    }
}

#endif /* MORIS_CL_Property_HPP_ */
