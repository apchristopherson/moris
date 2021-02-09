#include "st_MTK_Mesh_Pair.hpp"
#include "cl_MTK_Integration_Mesh.hpp"
#include "cl_MTK_Interpolation_Mesh.hpp"

namespace moris
{
    namespace mtk
    {

        //--------------------------------------------------------------------------------------------------------------

        Mesh_Pair::Mesh_Pair(const Mesh_Pair& aMeshPair)
                : mInterpolationMesh(aMeshPair.mInterpolationMesh)
                , mIntegrationMesh(aMeshPair.mIntegrationMesh)
                , mIsOwned(false)
        {
        }

        //--------------------------------------------------------------------------------------------------------------

        Mesh_Pair::~Mesh_Pair()
        {
            if (mIsOwned)
            {
                delete mInterpolationMesh;
                delete mIntegrationMesh;
            }
        }

        //--------------------------------------------------------------------------------------------------------------

    }
}