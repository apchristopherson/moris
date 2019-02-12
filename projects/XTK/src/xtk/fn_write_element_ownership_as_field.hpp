/*
 * fn_write_element_ownership_as_field.hpp
 *
 *  Created on: May 24, 2018
 *      Author: ktdoble
 */

#ifndef SRC_MESH_FN_WRITE_ELEMENT_OWNERSHIP_AS_FIELD_HPP_
#define SRC_MESH_FN_WRITE_ELEMENT_OWNERSHIP_AS_FIELD_HPP_

#include "cl_MTK_Mesh.hpp"
#include "cl_XTK_Background_Mesh.hpp"
#include "cl_XTK_Cut_Mesh.hpp"
#include "cl_Cell.hpp"
namespace xtk
{

/*
 * Write the elemental owner data as a field in the output mesh, this function writes this data on the output mesh taking into account the
 * children elements created in the XTK decomposition. For visualization purposes the elemental field needs to be a moris::real type number rather
 * than an moris::size_t. The field with the name "aOwnerFieldName" needs to be declared in the output options prior to
 * call to get_output_mesh in the XTK model.
 */

void
write_element_ownership_as_field(std::string aOwnerFieldName,
                                 Background_Mesh & aXTKMesh,
                                 Cut_Mesh & aCutMesh,
                                 moris::mtk::Mesh & aOutputMesh
                                 )
{
//    // Get the background mesh
//    moris::mtk::Mesh const & tBackgroundMesh = aXTKMesh.get_mesh_data();
//
//    // Initialize Data (using output mesh because this knows the total number of elements
//    moris::size_t tNumElementsOutput = aOutputMesh.get_num_entities(EntityRank::ELEMENT);
//    Cell<moris::real> tOwnerData(tNumElementsOutput);
//
//    // Iterate through background mesh elements (i here corresponds to elemental index)
//    for(moris::size_t i = 0; i<tBackgroundMesh.get_num_entities(EntityRank::ELEMENT); i++)
//    {
//        // Process that owns the element
//        moris::size_t tElementOwner = tBackgroundMesh.get_entity_parallel_owner_rank(i, EntityRank::ELEMENT);
//
//        // Get the element Id (needed to translate between background and output mesh)
//        moris::size_t tElementId = tBackgroundMesh.get_glb_entity_id_from_entity_loc_index(i,EntityRank::ELEMENT);
//
//        // Check to see if this element has any children
//        if(aXTKMesh.entity_has_children(i,EntityRank::ELEMENT))
//        {
//            // The location of the child mesh in the cut mesh
//            moris::size_t tChildMeshIndex = aXTKMesh.child_mesh_index(i,EntityRank::ELEMENT);
//
//            // Retrieve all the element Ids of the children
//            moris::Matrix< moris::IndexMat > const & tElementIds = aCutMesh.get_element_ids(tChildMeshIndex);
//
//            //Iterate through children elements and ask the output mesh for the indices using ids.
//            // The index is then used to place the data in the correct location of tOwnerData.
//            for(moris::size_t j = 0; j<tElementIds.n_cols(); j++ )
//            {
//                // Get element index in output mesh using element Id
//
//                moris::size_t tElementIndex = aOutputMesh.get_loc_entity_index_from_entity_glb_id(tElementIds(0,j), EntityRank::ELEMENT);
//
//                // Add to owner data
//                tOwnerData(tElementIndex) = (moris::real) tElementOwner;
//
//            }
//        }
//
//        // No children elements case
//        else
//        {
//
//            // Get element index in output mesh using element Id
//            moris::size_t tElementIndex = aOutputMesh.get_loc_entity_index_from_entity_glb_id(tElementId, EntityRank::ELEMENT);
//
//            // Add to owner data
//            tOwnerData(tElementIndex) = (moris::real) tElementOwner;
//
//        }
//
//    }
//
//    // Write the data to the mesh
//    aOutputMesh.add_mesh_field_data_loc_indices(aOwnerFieldName, EntityRank::ELEMENT, tOwnerData);
}

}




#endif /* SRC_MESH_FN_WRITE_ELEMENT_OWNERSHIP_AS_FIELD_HPP_ */
