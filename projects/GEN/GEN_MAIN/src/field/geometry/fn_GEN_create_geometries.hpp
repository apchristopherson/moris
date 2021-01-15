#ifndef MORIS_FN_CREATE_GEOMETRY_HPP
#define MORIS_FN_CREATE_GEOMETRY_HPP

#include "cl_GEN_Geometry.hpp"
#include "cl_Param_List.hpp"
#include "cl_MTK_Mesh_Core.hpp"
#include "fn_Exec_load_user_library.hpp"

namespace moris
{
    namespace ge
    {
        /**
         * Higher-level call for creating a cell of geometries, which also creates multigeometries.
         *
         * @tparam Vector_Type Type of vector where ADVs are stored
         * @param aGeometryParameterLists Parameter lists for creating geometry classes
         * @param aADVs ADV Vector
         * @param aLibrary Pointer to library for loading user-defined functions
         * @return Pointers to created Geometry classes
         */
        template <typename Vector_Type>
        Cell<std::shared_ptr<Geometry>> create_geometries(
                Cell<ParameterList>         aGeometryParameterLists,
                Vector_Type&                aADVs,
                std::shared_ptr<Library_IO> aLibrary = nullptr);

        /**
         * Creates an instance of the specified Geometry class and returns a shared pointer to it.
         *
         * @tparam Vector_Type Type of vector where ADVs are stored
         * @param aGeometryParameterList Parameter list for creating a geometry class
         * @param aADVs ADV Vector
         * @param aLibrary Pointer to library for loading user-defined functions
         * @return Pointer to specific Geometry class
         */
        template <typename Vector_Type>
        std::shared_ptr<Geometry> create_geometry(
                ParameterList               aGeometryParameterList,
                Vector_Type&                aADVs,
                std::shared_ptr<Library_IO> aLibrary = nullptr,
                std::shared_ptr<Geometry>   aGeometry = nullptr,
                uint                        aIndex = 0);
    }
}

#endif //MORIS_FN_CREATE_GEOMETRY_HPP