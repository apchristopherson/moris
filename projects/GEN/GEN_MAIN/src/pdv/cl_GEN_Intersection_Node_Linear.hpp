#ifndef MORIS_CL_GEN_INTERSECTION_NODE_LINEAR_HPP
#define MORIS_CL_GEN_INTERSECTION_NODE_LINEAR_HPP

#include "cl_GEN_Intersection_Node.hpp"

namespace moris
{
    namespace ge
    {
        class Geometry;

        class Intersection_Node_Linear : public Intersection_Node
        {

        public:

            /**
             * Constructor
             *
             * @param aFirstNodeIndex Index of the first parent of this node
             * @param aSecondNodeIndex Index of the second parent of this node
             * @param aFirstNodeCoordinates Coordinates of the first parent of this node
             * @param aSecondNodeCoordinates Coordinates of the second parent of this node
             * @param aInterfaceGeometry Geometry that intersects the parent to create this child
             * @param aIsocontourThreshold Threshold for determining the intersection location of this node
             * @param aIsocontourTolerance Tolerance for determining interface parent nodes based on geometry value
             * @param aIntersectionTolerance Tolerance for determining interface parent nodes with intersection distance
             */
            Intersection_Node_Linear(
                    uint                      aFirstNodeIndex,
                    uint                      aSecondNodeIndex,
                    const Matrix<DDRMat>&     aFirstNodeCoordinates,
                    const Matrix<DDRMat>&     aSecondNodeCoordinates,
                    std::shared_ptr<Geometry> aInterfaceGeometry,
                    real                      aIsocontourThreshold = 0.0,
                    real                      aIsocontourTolerance = 0.0,
                    real                      aIntersectionTolerance = 0.0);

        private:

            /**
             * Gets the sensitivity of this node's local coordinate within its parent edge with respect to the field
             * values on each of its ancestors.
             *
             * @return Local coordinate sensitivity
             */
            real get_dcoordinate_dfield_from_ancestor(uint aAncestorIndex);

            /**
             * Interpolate and return the local coordinates of this intersection node. Used to clean up constructor.
             *
             * @param aFirstNodeIndex Index of the first parent of this node
             * @param aSecondNodeIndex Index of the second parent of this node
             * @param aFirstNodeCoordinates Coordinates of the first parent of this node
             * @param aSecondNodeCoordinates Coordinates of the second parent of this node
             * @param aInterfaceGeometry Geometry that intersects the parent to create this node
             * @param aIsocontourThreshold Threshold for determining the intersection location of this node
             * @return Local coordinates
             */
            real get_local_coordinate(
                    uint                      aFirstNodeIndex,
                    uint                      aSecondNodeIndex,
                    const Matrix<DDRMat>&     aFirstNodeCoordinates,
                    const Matrix<DDRMat>&     aSecondNodeCoordinates,
                    std::shared_ptr<Geometry> aInterfaceGeometry,
                    real                      aIsocontourThreshold);

        };
    }
}

#endif //MORIS_CL_GEN_INTERSECTION_NODE_LINEAR_HPP
