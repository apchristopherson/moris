#ifndef MORIS_CL_XTK_DECOMPOSITION_ALGORITHM_HPP_
#define MORIS_CL_XTK_DECOMPOSITION_ALGORITHM_HPP_

#include "cl_XTK_Integration_Mesh_Generator.hpp"

namespace xtk
{

enum class Decomposition_Algorithm_Type
{
    REGULAR_TEMPLATE_NONCONFORMING,
    EDGE_BASED
};

class Integration_Mesh_Generation_Data;
class Integration_Mesh_Generator;
class Decomposition_Data;
class Cut_Integration_Mesh;


class Decomposition_Algorithm
{
    public:
    moris_index                                         mNumNewCells;
    moris::Cell<moris::Cell<moris::moris_index>>        mNewCellToVertexConnectivity;
    moris::Cell<moris::moris_index>                     mNewCellChildMeshIndex;
    moris::Cell<moris::moris_index>                     mNewCellCellIndexToReplace;
    moris::Cell<std::shared_ptr<moris::mtk::Cell_Info>> mNewCellCellInfo;
    

    public:
    Decomposition_Algorithm(){}
    
    // set of 
    void perform(Integration_Mesh_Generation_Data *  aMeshGenerationData,
                 Decomposition_Data*                 aDecompositionData,
                 Cut_Integration_Mesh*               aCutIntegrationMesh,
                 moris::mtk::Mesh*                   aBackgroundMesh,
                 Integration_Mesh_Generator*         aMeshGenerator);

    virtual enum Decomposition_Algorithm_Type get_algorithm_type() const = 0;

    virtual moris_index get_signature() const = 0;

    virtual 
    void
    perform_impl_vertex_requests(Integration_Mesh_Generation_Data* aMeshGenerationData,
                                 Decomposition_Data*               aDecompositionData,
                                 Cut_Integration_Mesh*             aCutIntegrationMesh,
                                 moris::mtk::Mesh*                 aBackgroundMesh,
                                 Integration_Mesh_Generator*       aMeshGenerator) = 0;
    virtual
    void
    perform_impl_generate_mesh(Integration_Mesh_Generation_Data* aMeshGenerationData,
                               Decomposition_Data*               aDecompositionData,
                               Cut_Integration_Mesh*             aCutIntegrationMesh,
                               moris::mtk::Mesh*                 aBackgroundMesh,
                               Integration_Mesh_Generator*       aMeshGenerator) = 0;
};

}




#endif