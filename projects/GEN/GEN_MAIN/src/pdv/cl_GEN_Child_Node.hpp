#ifndef MORIS_CL_GEN_CHILD_NODE_HPP
#define MORIS_CL_GEN_CHILD_NODE_HPP

#include "cl_Matrix.hpp"
#include "cl_XTK_Basis_Function.hpp"

namespace moris
{
    namespace ge
    {
        // Forward declaration of Field class
        class Field;

        class Child_Node
        {

        protected:
            Matrix<DDUMat>       mAncestorNodeIndices;
            Cell<Matrix<DDRMat>> mAncestorNodeCoordinates;
            Matrix<DDRMat>       mBasisValues;

        private:
            Matrix<DDRMat>       mLocalCoordinatesInAncestor;
            Matrix<DDRMat>       mJoinedSensitivities;

        public:

            /**
             * Constructor
             *
             * @param aAncestorNodeIndices Node indices of the ancestors of this child node
             * @param aAncestorNodeCoordinates Coordinates of the ancestors of this child node
             * @param aBasisFunction Basis function of the ancestor topology
             * @param aLocalCoordinatesInAncestor Local coordinate of this child inside of the ancestor element
             */
            Child_Node(
                    Matrix<DDUMat>             aAncestorNodeIndices,
                    Cell<Matrix<DDRMat>>       aAncestorNodeCoordinates,
                    const xtk::Basis_Function& aBasisFunction,
                    Matrix<DDRMat>             aLocalCoordinatesInAncestor);

            /**
             * Get the field value on the child node based on values from its ancestors.
             *
             * @param aField Field pointer, referenced during call from field class
             * @return Field value
             */
            virtual real interpolate_field_value(Field* aField);

            /**
             * Joins the field sensitivities on the child node based on its ancestors.
             *
             * @param aField Field pointer, referenced during call from field class
             * @return Field sensitivities
             */
            virtual const Matrix<DDRMat>& join_field_sensitivities(Field* aField);

            /**
             * Joins the depending ADV IDs on the child node based on its ancestors.
             *
             * @param aField Field pointer, referenced during call from field class
             * @return Field ADV IDs
             */
            virtual Matrix<DDSMat> join_determining_adv_ids(Field* aField);

            friend class Geometry_Engine;
        };
    }
}

#endif //MORIS_CL_GEN_CHILD_NODE_HPP
