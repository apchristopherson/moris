/*
 * cl_MTK_Integration_Mesh.hpp
 *
 *  Created on: Apr 15, 2019
 *      Author: doble
 */

#ifndef PROJECTS_MTK_SRC_CL_MTK_INTEGRATION_MESH_HPP_
#define PROJECTS_MTK_SRC_CL_MTK_INTEGRATION_MESH_HPP_

#include <string>
#include "assert.hpp"

#include "cl_MTK_Mesh_Core.hpp"
#include "cl_MTK_Cell_Cluster.hpp"
#include "cl_MTK_Side_Cluster.hpp"
#include "cl_MTK_Double_Side_Cluster.hpp"
#include "cl_Matrix.hpp"
#include "cl_MTK_Block.hpp"

namespace moris
{
    namespace mtk
    {
        class Integration_Mesh : public virtual Mesh
        {
            protected:
                moris::Cell< moris::mtk::Set * > mListofBlocks;
                moris::Cell< moris::mtk::Set * > mListofSideSets;
                moris::Cell< moris::mtk::Set * > mListofDoubleSideSets;

                moris::Cell< moris::mtk::Set * > mListOfAllSets;    

                map< std::string, moris_index >  mSetNameToIndexMap;

                // set by color
                moris_index mMaxColor;
                moris::Cell<moris::Cell<moris::mtk::Set *>> mBlockSetToColor;
                moris::Cell<moris::Cell<moris::mtk::Set *>> mSideSetToColor;
                moris::Cell<moris::Cell<moris::mtk::Set *>> mDoubleSideSetToColor;
                moris::Cell<moris::Cell<moris::mtk::Set *>> mAllSetToColor;

            public:
                Integration_Mesh(){};
                // Functions only valid for integration meshes

                ~Integration_Mesh();

                //##############################################
                // Cell Cluster Access
                //##############################################

                /*
                 * Get a cell cluster related to an interpolation
                 * cell
                 */
                virtual
                Cell_Cluster const &
                get_cell_cluster(Cell const & aInterpCell) const = 0;

                //##############################################
                // MTK Set Access
                //##############################################

                // ----------------------------------------------------------------------------

                virtual
                moris::uint 
                get_num_sets() const;

                // ----------------------------------------------------------------------------
                /*!
                * @brief Get mesh set by name
                * @param[in] aSetLabel Set label
                */
                moris::mtk::Set * 
                get_set_by_name( std::string aSetLabel ) const;

                // ----------------------------------------------------------------------------
                /*!
                * @brief Get set name by set index
                * @param[in] aIndex Set index
                */
                moris::mtk::Set * 
                get_set_by_index( moris_index aIndex ) const;

                // ----------------------------------------------------------------------------
                /*!
                * @brief Get set index by set name
                * @param[in] aSetLabel Set label
                * @return Set index
                */
                moris_index 
                get_set_index_by_name( std::string aSetLabel );
                
                // ----------------------------------------------------------------------------
                /*!
                * @brief Get block sets with color
                * @param[in] aColor Set color
                */
                moris::Cell<moris::mtk::Set*> const &
                get_block_sets_with_color(moris_index const & aColor);

                // ----------------------------------------------------------------------------
                /*!
                 * @brief Get block set names with color
                 * @param[in] aColor    Set color
                 * @param[in] aSetNames List of set names
                 */
                void
                get_block_set_names_with_color(
                        moris_index const        & aColor,
                        moris::Cell<std::string> & aSetNames);

                // ----------------------------------------------------------------------------
                /*!
                * @brief Get side sets with color
                * @param[in] aColor Set color
                */
                moris::Cell<moris::mtk::Set*> const &
                get_side_sets_with_color(moris_index const & aColor);

                // ----------------------------------------------------------------------------
                /*!
                * @brief Get double side sets with color
                * @param[in] aColor Set color
                */
                moris::Cell<moris::mtk::Set*> const &
                get_double_side_sets_with_color(moris_index const & aColor);

                // ----------------------------------------------------------------------------
                /*!
                * @brief Get all sets with color
                * @param[in] aColor Set color
                */
                moris::Cell<moris::mtk::Set*> const &
                get_all_sets_with_color(moris_index const & aColor);

                // ----------------------------------------------------------------------------
                /*!
                * @brief Print sets by colors
                */
                void
                print_sets_by_colors();
                 
                // ----------------------------------------------------------------------------
                  
                /*
                 * Get block set names
                 */
                virtual
                moris::Cell<std::string>
                get_block_set_names() const = 0;

                /*!
                 * Returns the label
                 */
                virtual
                std::string
                get_block_set_label(moris_index aBlockSetOrdinal) const;

                // ----------------------------------------------------------------------------

                /*!
                 * Returns the index given a label
                 */
                virtual
                moris_index
                get_block_set_index(std::string aBlockSetLabel) const;

                // ----------------------------------------------------------------------------
                /*
                 * Get number of blocks
                 */
                moris::uint
                get_num_blocks() const;

                // ----------------------------------------------------------------------------
                /*
                 * Get number of blocks
                 * Sometimes num side set * 2. Ask Keenan
                 */
                moris::uint
                get_num_side_set() const;

                // ----------------------------------------------------------------------------
                /*
                 * Get number of blocks
                 * Sometimes num side set * 2. Ask Keenan
                 */
                moris::uint
                get_num_double_side_set() const;

                // ----------------------------------------------------------------------------

                /*
                 * Get cell clusters within a block set
                 */
                virtual
                moris::Cell<Cluster const *>
                get_cell_clusters_in_set(moris_index aBlockSetOrdinal) const = 0;

                //##############################################
                // Side Set Cluster Access
                //##############################################
                /*!
                 * Get side clusters within a side set
                 */
                virtual
                moris::Cell<Cluster const *>
                get_side_set_cluster(moris_index aSideSetOrdinal) const = 0;

                // ----------------------------------------------------------------------------

                /*!
                 * get number of side sets
                 */
                virtual
                uint
                get_num_side_sets() const = 0;

                // ----------------------------------------------------------------------------

                /*!
                 * Returns the label
                 */
                virtual
                std::string
                get_side_set_label(moris_index aSideSetOrdinal) const = 0;

                // ----------------------------------------------------------------------------

                /*!
                 * Returns the index given a label
                 */
                virtual
                moris_index
                get_side_set_index(std::string aSideSetLabel) const  = 0;

                // ----------------------------------------------------------------------------

                //##############################################
                // Double Side Set Cluster Access
                //##############################################

                /*!
                 * Returns the number of double sided side sets in the mesh
                 */
                virtual
                uint
                get_num_double_sided_sets() const  = 0;

                // ----------------------------------------------------------------------------

                /*!
                 * Returns the label
                 */
                virtual
                std::string
                get_double_sided_set_label(moris_index aSideSetOrdinal) const = 0;

                // ----------------------------------------------------------------------------

                /*!
                 * Returns the index given a label
                 */
                virtual
                moris_index
                get_double_sided_set_index(std::string aDoubleSideSetLabel) const;

                // ----------------------------------------------------------------------------

                /*!
                 * Returns the double side clusters in the side set
                 */
                virtual
                moris::Cell<Cluster const*>
                get_double_side_set_cluster(moris_index aSideSetOrdinal) const  = 0;

                // ----------------------------------------------------------------------------

            protected:

                void collect_all_sets();

                void
                setup_set_to_color();

                // ----------------------------------------------------------------------------

        };
    }
}


#endif /* PROJECTS_MTK_SRC_CL_MTK_INTEGRATION_MESH_HPP_ */
