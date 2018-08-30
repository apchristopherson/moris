/*
 * cl_XTK_Child_Mesh_Modification_Template.hpp
 *
 *  Created on: Jun 21, 2018
 *      Author: ktdoble
 */

#ifndef SRC_XTK_CL_XTK_CHILD_MESH_MODIFICATION_TEMPLATE_HPP_
#define SRC_XTK_CL_XTK_CHILD_MESH_MODIFICATION_TEMPLATE_HPP_

#include "assert/fn_xtk_assert.hpp"
#include "linalg/cl_XTK_Matrix.hpp"
#include "xtk/cl_XTK_Enums.hpp"

namespace xtk
{
template<typename Real, typename Integer, typename Real_Matrix, typename Integer_Matrix>
class Mesh_Modification_Template
{
public:
    Mesh_Modification_Template()
    {

    }

    Mesh_Modification_Template( Integer                             aParentElemInd,
                                Integer                             aElemToReplace,
                                moris::Matrix<Integer, Integer_Matrix> const & aNodeInds,
                                moris::Matrix<Integer, Integer_Matrix> const & aParentEdgeInds,
                                moris::Matrix<Integer, Integer_Matrix> const & aParentEdgeRanks,
                                moris::Matrix<Integer, Integer_Matrix> const & aParentFaceInds,
                                moris::Matrix<Integer, Integer_Matrix> const & aParentFaceRanks,
                                enum TemplateType                   aTemplateType,
                                Integer                             aPermutationId = 0)
    {
        mElemIndToReplace = aElemToReplace;
        mParentElemInd    = aParentElemInd;
        mNodeInds         = aNodeInds.copy();
        mParentEdgeInds   = aParentEdgeInds.copy();
        mParentEdgeRanks  = aParentEdgeRanks.copy();
        mParentFaceInds   = aParentFaceInds.copy();
        mParentFaceRanks  = aParentFaceRanks.copy();
        template_catalog(aPermutationId,aTemplateType);
    }

    // Number of elements to replace and number of new elements in the template
    Integer                     mNumNewElem;
    Integer                     mNumElemToReplace;
    Integer                     mElemIndToReplace;

    // Parent Entity's parent information
    // This is the information relative to the parent this template is created from
    Integer                     mParentElemInd;
    moris::Matrix<Integer, Integer_Matrix> mParentEdgeInds;
    moris::Matrix<Integer, Integer_Matrix> mParentEdgeRanks;
    moris::Matrix<Integer, Integer_Matrix> mParentFaceInds;
    moris::Matrix<Integer, Integer_Matrix> mParentFaceRanks;

    // Node indices in the template
    moris::Matrix<Integer, Integer_Matrix> mNodeInds;

    // Element to Node Connectivity
    moris::Matrix<Integer, Integer_Matrix> mNewElementToNode;

    // Parent's of an entity ordered by ordinal relative to
    // (these are the inheritance of the new elements created by this template)
    moris::Matrix<Integer, Integer_Matrix> mNewParentEdgeRanks;
    moris::Matrix<Integer, Integer_Matrix> mNewParentEdgeOrdinals;
    moris::Matrix<Integer, Integer_Matrix> mNewParentFaceRanks;
    moris::Matrix<Integer, Integer_Matrix> mNewParentFaceOrdinals;
    moris::Matrix<Integer, Integer_Matrix> mNewElementInterfaceSides;

    // Local coordinate relative to parent coordinate (Note: this has not been implemented yet)
    moris::Matrix<Real,Real_Matrix>      mNodeLocalCoordinates;

private:
    void
    template_catalog(Integer const & aPermutationId,
                     enum TemplateType aTemplateType)
    {
        switch(aTemplateType)
        {
        case(TemplateType::REGULAR_SUBDIVISION_HEX8):
        {
            hex_8_reg_sub_template();
            break;
        }
        case(TemplateType::TET_4):
         {
             tet4_template();
             break;
         }
        case(TemplateType::HIERARCHY_TET4_3N):
         {
            hierarchy_tet4_3N(aPermutationId);
             break;
         }
        case(TemplateType::HIERARCHY_TET4_4Na):
         {
            hierarchy_tet4_4na(aPermutationId);
             break;
         }
        case(TemplateType::HIERARCHY_TET4_4Nb):
         {
            hierarchy_tet4_4nb(aPermutationId);
             break;
         }
        case(TemplateType::HIERARCHY_TET4_4Nc):
         {
            hierarchy_tet4_4nc(aPermutationId);
             break;
         }
        default :
            std::cout<<"Template not found in the catalog"<<std::endl;
            break;
        }
    }

    void
    hex_8_reg_sub_template()
    {
        XTK_ASSERT(mNodeInds.n_cols() == 15, "For a Hex8 regular subdivision template, there must be 15 node inds.");
        mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({
                                                      {0, 8, 1,  14},
                                                      {1, 8, 5,  14},
                                                      {4, 5, 8,  14},
                                                      {0, 4, 8,  14},
                                                      {1, 9, 2,  14},
                                                      {2, 9, 6,  14},
                                                      {5, 6, 9,  14},
                                                      {1, 5, 9,  14},
                                                      {2, 10, 3, 14},
                                                      {2, 6, 10, 14},
                                                      {6, 7, 10, 14},
                                                      {3, 10, 7, 14},
                                                      {0, 3, 11, 14},
                                                      {3, 7, 11, 14},
                                                      {4, 11, 7, 14},
                                                      {0, 11, 4, 14},
                                                      {0, 1, 12, 14},
                                                      {1, 2, 12, 14},
                                                      {2, 3, 12, 14},
                                                      {0, 12, 3, 14},
                                                      {4, 13, 5, 14},
                                                      {5, 13, 6, 14},
                                                      {6, 13, 7, 14},
                                                      {4, 7, 13, 14}});

        mNumNewElem = 24;
        mNumElemToReplace = 1;
        mNewParentEdgeRanks    = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 1, 3, 3, 3}, {2, 2, 1, 3, 3, 3}, {1, 2, 2, 3, 3, 3}, {1, 2, 2, 3, 3, 3}, {2, 2, 1, 3, 3, 3}, {2, 2, 1, 3, 3, 3}, {1, 2, 2, 3, 3, 3}, {1, 2, 2, 3, 3, 3}, {2, 2, 1, 3, 3, 3}, {1, 2, 2, 3, 3, 3}, {1, 2, 2, 3, 3, 3}, {2, 2, 1, 3, 3, 3}, {1, 2, 2, 3, 3, 3}, {1, 2, 2, 3, 3, 3}, {2, 2, 1, 3, 3, 3}, {2, 2, 1, 3, 3, 3}, {1, 2, 2, 3, 3, 3}, {1, 2, 2, 3, 3, 3}, {1, 2, 2, 3, 3, 3}, {2, 2, 1, 3, 3, 3}, {2, 2, 1, 3, 3, 3}, {2, 2, 1, 3, 3, 3}, {2, 2, 1, 3, 3, 3}, {1, 2, 2, 3, 3, 3}});
        mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 0, 0, 0}, {0, 0, 9, 0, 0, 0}, {4, 0, 0, 0, 0, 0}, {8, 0, 0, 0, 0, 0}, {1, 1, 1, 0, 0, 0}, {1, 1, 10, 0, 0, 0}, {5, 1, 1, 0, 0, 0}, {9, 1, 1, 0, 0, 0}, {2, 2, 2, 0, 0, 0}, {10, 2, 2, 0, 0, 0}, {6, 2, 2, 0, 0, 0}, {2, 2, 11, 0, 0, 0}, {3, 3, 3, 0, 0, 0}, {11, 3, 3, 0, 0, 0}, {3, 3, 7, 0, 0, 0}, {3, 3, 8, 0, 0, 0}, {0, 4, 4, 0, 0, 0}, {1, 4, 4, 0, 0, 0}, {2, 4, 4, 0, 0, 0}, {4, 4, 3, 0, 0, 0}, {5, 5, 4, 0, 0, 0}, {5, 5, 5, 0, 0, 0}, {5, 5, 6, 0, 0, 0}, {7, 5, 5, 0, 0, 0}});
        mNewParentFaceRanks    = moris::Matrix<Integer, Integer_Matrix>({{3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}, {3, 3, 3, 2}});
        mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 3}, {0, 0, 0, 3}, {0, 0, 0, 3}, {0, 0, 0, 3}, {0, 0, 0, 4}, {0, 0, 0, 4}, {0, 0, 0, 4}, {0, 0, 0, 4}, {0, 0, 0, 5}, {0, 0, 0, 5}, {0, 0, 0, 5}, {0, 0, 0, 5}});
        mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>(24,1,std::numeric_limits<Integer>::max());
    }

    void
    tet4_template()
    {

        mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0,1,2,3}});
        mNumNewElem = 1;
        mNumElemToReplace = 0;
        mNewParentEdgeRanks       = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 1, 1, 1, 1}});
        mNewParentEdgeOrdinals    = moris::Matrix<Integer, Integer_Matrix>({{0, 1, 2, 3, 4, 5}});
        mNewParentFaceRanks       = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 2, 2}});
        mNewParentFaceOrdinals    = moris::Matrix<Integer, Integer_Matrix>({{0, 1, 2, 3}});
        mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>(1,1,std::numeric_limits<Integer>::max());

    }


    void
    hierarchy_tet4_3N(Integer const & aPermutation)
    {

        switch(aPermutation)
        {
        case(320):
        {
            // Permutation 320
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 6}, {4, 1, 5, 6}, {1, 2, 5, 6}, {1, 3, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 2, 2, 2}, {1, 1, 2, 2, 2, 2}, {1, 1, 1, 2, 1, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 2, 3, 0, 2}, {0, 3, 3, 0, 0, 2}, {1, 2, 3, 0, 2, 2}, {4, 5, 1, 0, 3, 2}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 2, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 2, 3}, {0, 0, 0, 3}, {0, 2, 0, 3}, {0, 2, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {2}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }

        case(32):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 6}, {4, 1, 5, 6}, {1, 2, 5, 6}, {1, 3, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 2, 2, 2}, {1, 1, 2, 2, 2, 2}, {1, 1, 1, 2, 1, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 0, 3, 0}, {2, 2, 2, 3, 3, 0}, {5, 3, 2, 3, 0, 0}, {1, 4, 5, 3, 0, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 2, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 2}, {3, 0, 0, 2}, {0, 0, 0, 2}, {3, 0, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {2}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});

            break;
        }


        case(203):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 6}, {4, 1, 5, 6}, {1, 2, 5, 6}, {1, 3, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 2, 2, 2}, {1, 1, 2, 2, 2, 2}, {1, 1, 1, 2, 1, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 2, 2, 3}, {3, 0, 0, 2, 2, 3}, {4, 0, 0, 2, 3, 3}, {5, 1, 4, 2, 2, 3}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 2, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 3, 0}, {2, 0, 0, 0}, {0, 3, 0, 0}, {2, 3, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {2}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(251):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 6}, {4, 1, 5, 6}, {1, 2, 5, 6}, {1, 3, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 2, 2, 2}, {1, 1, 2, 2, 2, 2}, {1, 1, 1, 2, 1, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 5, 2, 3, 2}, {1, 1, 1, 3, 3, 2}, {4, 5, 1, 3, 2, 2}, {0, 3, 4, 3, 2, 2}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 2, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 2, 1}, {3, 0, 0, 1}, {0, 2, 0, 1}, {3, 2, 0, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {2}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }

        case(512):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 6}, {4, 1, 5, 6}, {1, 2, 5, 6}, {1, 3, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 2, 2, 2}, {1, 1, 2, 2, 2, 2}, {1, 1, 1, 2, 1, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 1, 5, 2, 1}, {2, 3, 3, 2, 2, 1}, {0, 1, 3, 2, 1, 1}, {3, 4, 0, 2, 5, 1}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 2, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 1, 3}, {2, 0, 0, 3}, {0, 1, 0, 3}, {2, 1, 0, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {2}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(125):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 6}, {4, 1, 5, 6}, {1, 2, 5, 6}, {1, 3, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 2, 2, 2}, {1, 1, 2, 2, 2, 2}, {1, 1, 1, 2, 1, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 2, 2, 1, 1, 3}, {5, 2, 2, 1, 1, 3}, {3, 2, 2, 1, 3, 3}, {4, 0, 3, 1, 1, 3}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 2, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 3, 2}, {1, 0, 0, 2}, {0, 3, 0, 2}, {1, 3, 0, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {2}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(140):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 6}, {4, 1, 5, 6}, {1, 2, 5, 6}, {1, 3, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 2, 2, 2}, {1, 1, 2, 2, 2, 2}, {1, 1, 1, 2, 1, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 4, 1, 3, 1}, {0, 0, 0, 3, 3, 1}, {3, 4, 0, 3, 1, 1}, {2, 5, 3, 3, 1, 1}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 2, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 1, 0}, {3, 0, 0, 0}, {0, 1, 0, 0}, {3, 1, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {2}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});

            break;
        }

        case(401):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 6}, {4, 1, 5, 6}, {1, 2, 5, 6}, {1, 3, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 2, 2, 2}, {1, 1, 2, 2, 2, 2}, {1, 1, 1, 2, 1, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 3, 0, 4, 1, 0}, {1, 3, 3, 1, 1, 0}, {2, 0, 3, 1, 0, 0}, {5, 3, 2, 1, 4, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 2, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 3}, {1, 0, 0, 3}, {0, 0, 0, 3}, {1, 0, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {2}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});

            break;
        }
        case(14):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 6}, {4, 1, 5, 6}, {1, 2, 5, 6}, {1, 3, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 2, 2, 2}, {1, 1, 2, 2, 2, 2}, {1, 1, 1, 2, 1, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 1, 1, 0, 0, 3}, {4, 1, 1, 0, 0, 3}, {5, 1, 1, 0, 3, 3}, {3, 2, 5, 0, 0, 3}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 2, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 3, 1}, {0, 0, 0, 1}, {0, 3, 0, 1}, {0, 3, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {2}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(453):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 6}, {4, 1, 5, 6}, {1, 2, 5, 6}, {1, 3, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 2, 2, 2}, {1, 1, 2, 2, 2, 2}, {1, 1, 1, 2, 1, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 2, 5, 4, 0, 1}, {3, 2, 2, 0, 0, 1}, {2, 5, 2, 0, 1, 1}, {0, 1, 2, 0, 4, 1}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 2, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 1, 2}, {0, 0, 0, 2}, {0, 1, 0, 2}, {0, 1, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {2}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(534):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 6}, {4, 1, 5, 6}, {1, 2, 5, 6}, {1, 3, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 2, 2, 2}, {1, 1, 2, 2, 2, 2}, {1, 1, 1, 2, 1, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 3, 5, 1, 2}, {4, 0, 0, 1, 1, 2}, {0, 3, 0, 1, 2, 2}, {1, 2, 0, 1, 5, 2}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 2, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 2, 0}, {1, 0, 0, 0}, {0, 2, 0, 0}, {1, 2, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {2}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(345):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 6}, {4, 1, 5, 6}, {1, 2, 5, 6}, {1, 3, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 2, 2, 2}, {1, 1, 2, 2, 2, 2}, {1, 1, 1, 2, 1, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 1, 4, 3, 2, 0}, {5, 1, 1, 2, 2, 0}, {1, 4, 1, 2, 0, 0}, {2, 0, 1, 2, 3, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 2, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 1}, {2, 0, 0, 1}, {0, 0, 0, 1}, {2, 0, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {2}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});

            break;
        }

        case(230):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 6}, {1, 4, 5, 6}, {2, 1, 5, 6}, {3, 1, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 2, 2}, {1, 2, 1, 2, 2, 2}, {1, 1, 1, 1, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 0, 3, 2, 2}, {0, 0, 0, 3, 3, 2}, {4, 0, 3, 2, 3, 2}, {1, 4, 5, 2, 3, 2}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 3, 2, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 2, 0, 0}, {3, 0, 0, 0}, {0, 0, 2, 0}, {3, 0, 2, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {1}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(302):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 6}, {1, 4, 5, 6}, {2, 1, 5, 6}, {3, 1, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 2, 2}, {1, 2, 1, 2, 2, 2}, {1, 1, 1, 1, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 3, 2, 3, 0}, {2, 3, 3, 2, 2, 0}, {1, 3, 0, 0, 2, 0}, {5, 1, 4, 3, 2, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 3, 2, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 3}, {2, 0, 0, 3}, {0, 0, 0, 3}, {2, 0, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {1}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(23):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 6}, {1, 4, 5, 6}, {2, 1, 5, 6}, {3, 1, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 2, 2}, {1, 2, 1, 2, 2, 2}, {1, 1, 1, 1, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 2, 2, 0, 0, 3}, {3, 2, 2, 0, 0, 3}, {5, 2, 2, 3, 0, 3}, {4, 5, 1, 0, 0, 3}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 3, 2, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 0, 2}, {0, 0, 0, 2}, {0, 0, 3, 2}, {0, 0, 3, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {1}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(521):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 6}, {1, 4, 5, 6}, {2, 1, 5, 6}, {3, 1, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 2, 2}, {1, 2, 1, 2, 2, 2}, {1, 1, 1, 1, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 3, 1, 5, 2}, {1, 3, 3, 1, 1, 2}, {0, 3, 2, 2, 1, 2}, {4, 0, 3, 5, 1, 2}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 3, 2, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 0, 3}, {1, 0, 0, 3}, {0, 0, 2, 3}, {1, 0, 2, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {1}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(152):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 6}, {1, 4, 5, 6}, {2, 1, 5, 6}, {3, 1, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 2, 2}, {1, 2, 1, 2, 2, 2}, {1, 1, 1, 1, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 5, 2, 3, 1, 1}, {2, 2, 2, 3, 3, 1}, {3, 2, 5, 1, 3, 1}, {0, 3, 4, 1, 3, 1}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 3, 2, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 1, 0, 2}, {3, 0, 0, 2}, {0, 0, 1, 2}, {3, 0, 1, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {1}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(215):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 6}, {1, 4, 5, 6}, {2, 1, 5, 6}, {3, 1, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 2, 2}, {1, 2, 1, 2, 2, 2}, {1, 1, 1, 1, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 1, 1, 2, 2, 3}, {5, 1, 1, 2, 2, 3}, {4, 1, 1, 3, 2, 3}, {3, 4, 0, 2, 2, 3}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 3, 2, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 0, 1}, {2, 0, 0, 1}, {0, 0, 3, 1}, {2, 0, 3, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {1}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(410):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 6}, {1, 4, 5, 6}, {2, 1, 5, 6}, {3, 1, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 2, 2}, {1, 2, 1, 2, 2, 2}, {1, 1, 1, 1, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 1, 3, 0, 4, 1}, {0, 3, 3, 0, 0, 1}, {2, 3, 1, 1, 0, 1}, {3, 2, 5, 4, 0, 1}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 3, 2, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 1, 0, 3}, {0, 0, 0, 3}, {0, 0, 1, 3}, {0, 0, 1, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {1}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
           break;
        }
        case(41 ):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 6}, {1, 4, 5, 6}, {2, 1, 5, 6}, {3, 1, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 2, 2}, {1, 2, 1, 2, 2, 2}, {1, 1, 1, 1, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 4, 1, 3, 0, 0}, {1, 1, 1, 3, 3, 0}, {5, 1, 4, 0, 3, 0}, {2, 5, 3, 0, 3, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 3, 2, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 1}, {3, 0, 0, 1}, {0, 0, 0, 1}, {3, 0, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {1}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(104):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 6}, {1, 4, 5, 6}, {2, 1, 5, 6}, {3, 1, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 2, 2}, {1, 2, 1, 2, 2, 2}, {1, 1, 1, 1, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 0, 1, 1, 3}, {4, 0, 0, 1, 1, 3}, {3, 0, 0, 3, 1, 3}, {5, 3, 2, 1, 1, 3}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 3, 2, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 3, 0, 0}, {1, 0, 0, 0}, {0, 0, 3, 0}, {1, 0, 3, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {1}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(543):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 6}, {1, 4, 5, 6}, {2, 1, 5, 6}, {3, 1, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 2, 2}, {1, 2, 1, 2, 2, 2}, {1, 1, 1, 1, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 4, 0, 2, 5, 1}, {3, 0, 0, 2, 2, 1}, {0, 0, 4, 1, 2, 1}, {2, 0, 1, 5, 2, 1}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 3, 2, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 1, 0, 0}, {2, 0, 0, 0}, {0, 0, 1, 0}, {2, 0, 1, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {1}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(354):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 6}, {1, 4, 5, 6}, {2, 1, 5, 6}, {3, 1, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 2, 2}, {1, 2, 1, 2, 2, 2}, {1, 1, 1, 1, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 5, 1, 0, 3, 2}, {4, 1, 1, 0, 0, 2}, {1, 1, 5, 2, 0, 2}, {0, 1, 2, 3, 0, 2}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 3, 2, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 2, 0, 1}, {0, 0, 0, 1}, {0, 0, 2, 1}, {0, 0, 2, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {1}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }
        case(435):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 6}, {1, 4, 5, 6}, {2, 1, 5, 6}, {3, 1, 2, 6}});
            mNumNewElem = 4;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 2, 2}, {1, 2, 1, 2, 2, 2}, {1, 1, 1, 1, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 3, 2, 1, 4, 0}, {5, 2, 2, 1, 1, 0}, {2, 2, 3, 0, 1, 0}, {1, 2, 0, 4, 1, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 3, 2, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 2}, {1, 0, 0, 2}, {0, 0, 0, 2}, {1, 0, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {1}, {std::numeric_limits<Integer>::max()}, {std::numeric_limits<Integer>::max()}});
            break;
        }

        default:
        {
            std::cout<<"WARNING INVALID PERMUTATION"<<std::endl;
            break;
        }
        }
    }

    void
    hierarchy_tet4_4na(Integer const & aPermutationId)
    {
        switch(aPermutationId)
        {
         case(5420):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 7, 6}, {4, 3, 6, 7}, {4, 3, 7, 5}, {1, 2, 7, 5}, {2, 4, 7, 5}, {4, 2, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 3, 2, 2}, {1, 2, 3, 2, 1, 2}, {1, 2, 1, 1, 2, 2}, {1, 3, 2, 2, 2, 2}, {1, 2, 3, 2, 1, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 2, 5, 4, 0, 1}, {0, 0, 0, 0, 2, 1}, {0, 2, 0, 3, 2, 2}, {1, 1, 5, 2, 3, 2}, {0, 0, 1, 3, 3, 2}, {0, 1, 0, 0, 4, 1}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {3, 3, 3, 2}, {2, 2, 3, 3}, {2, 3, 2, 2}, {2, 3, 3, 3}, {2, 2, 3, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 2, 1, 2}, {2, 2, 2, 0}, {3, 2, 2, 2}, {3, 2, 2, 1}, {3, 2, 2, 2}, {0, 1, 2, 2}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {2}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {2}});

             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 7, 6}, {4, 3, 6, 7}, {4, 3, 7, 5}, {1, 2, 7, 5}, {2, 4, 7, 5}, {4, 2, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 3, 2, 2}, {1, 2, 3, 2, 1, 2}, {1, 2, 1, 1, 2, 2}, {1, 3, 2, 2, 2, 2}, {1, 2, 3, 2, 1, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 2, 5, 4, 0, 1}, {0, 0, 0, 0, 2, 1}, {0, 2, 0, 3, 2, 2}, {1, 1, 5, 2, 3, 2}, {0, 0, 1, 3, 3, 2}, {0, 1, 0, 0, 4, 1}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {3, 3, 3, 2}, {2, 2, 3, 3}, {2, 3, 2, 2}, {2, 3, 3, 3}, {2, 2, 3, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 1, 2}, {0, 0, 0, 0}, {3, 2, 0, 0}, {3, 0, 2, 1}, {3, 0, 0, 0}, {0, 1, 0, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {2}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {2}});

             break;
         }
         case(5240):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 7, 6}, {4, 3, 6, 7}, {4, 3, 7, 5}, {1, 2, 7, 5}, {2, 4, 7, 5}, {4, 2, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 3, 2, 2}, {1, 2, 3, 2, 1, 2}, {1, 2, 1, 1, 2, 2}, {1, 3, 2, 2, 2, 2}, {1, 2, 3, 2, 1, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 5, 2, 3, 2}, {0, 3, 3, 0, 1, 2}, {0, 1, 0, 0, 4, 1}, {3, 2, 5, 4, 0, 1}, {0, 0, 2, 0, 0, 1}, {0, 2, 0, 3, 2, 2}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {3, 3, 3, 2}, {2, 2, 3, 3}, {2, 3, 2, 2}, {2, 3, 3, 3}, {2, 2, 3, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 2, 1}, {0, 0, 0, 3}, {0, 1, 0, 0}, {0, 0, 1, 2}, {0, 0, 0, 0}, {3, 2, 0, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {2}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {2}});

             break;
         }
         case(425 ):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 7, 6}, {4, 3, 6, 7}, {4, 3, 7, 5}, {1, 2, 7, 5}, {2, 4, 7, 5}, {4, 2, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 3, 2, 2}, {1, 2, 3, 2, 1, 2}, {1, 2, 1, 1, 2, 2}, {1, 3, 2, 2, 2, 2}, {1, 2, 3, 2, 1, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 3, 0, 4, 1, 0}, {5, 1, 1, 0, 3, 0}, {5, 3, 0, 2, 2, 3}, {3, 0, 0, 2, 2, 3}, {5, 0, 0, 2, 2, 3}, {5, 0, 0, 1, 4, 0}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {3, 3, 3, 2}, {2, 2, 3, 3}, {2, 3, 2, 2}, {2, 3, 3, 3}, {2, 2, 3, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 3}, {0, 0, 0, 1}, {2, 3, 0, 0}, {2, 0, 3, 0}, {2, 0, 0, 0}, {1, 0, 0, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {2}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {2}});
             break;
         }
         case(3501):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 7, 6}, {4, 3, 6, 7}, {4, 3, 7, 5}, {1, 2, 7, 5}, {2, 4, 7, 5}, {4, 2, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 3, 2, 2}, {1, 2, 3, 2, 1, 2}, {1, 2, 1, 1, 2, 2}, {1, 3, 2, 2, 2, 2}, {1, 2, 3, 2, 1, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 3, 5, 1, 2}, {1, 1, 1, 0, 0, 2}, {1, 0, 0, 3, 0, 0}, {2, 2, 3, 0, 3, 0}, {1, 0, 2, 3, 3, 0}, {1, 2, 0, 1, 5, 2}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {3, 3, 3, 2}, {2, 2, 3, 3}, {2, 3, 2, 2}, {2, 3, 3, 3}, {2, 2, 3, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 2, 0}, {0, 0, 0, 1}, {3, 0, 0, 0}, {3, 0, 0, 2}, {3, 0, 0, 0}, {1, 2, 0, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {2}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {2}});
             break;
         }
         case(1503):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 7, 6}, {4, 3, 6, 7}, {4, 3, 7, 5}, {1, 2, 7, 5}, {2, 4, 7, 5}, {4, 2, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 3, 2, 2}, {1, 2, 3, 2, 1, 2}, {1, 2, 1, 1, 2, 2}, {1, 3, 2, 2, 2, 2}, {1, 2, 3, 2, 1, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 1, 5, 2, 1}, {3, 2, 2, 0, 3, 1}, {3, 3, 0, 0, 0, 3}, {4, 1, 1, 0, 0, 3}, {3, 0, 1, 0, 0, 3}, {3, 1, 0, 2, 5, 1}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {3, 3, 3, 2}, {2, 2, 3, 3}, {2, 3, 2, 2}, {2, 3, 3, 3}, {2, 2, 3, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 1, 3}, {0, 0, 0, 2}, {0, 3, 0, 0}, {0, 0, 3, 1}, {0, 0, 0, 0}, {2, 1, 0, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {2}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {2}});

             break;
         }
         case(3051):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 7, 6}, {4, 3, 6, 7}, {4, 3, 7, 5}, {1, 2, 7, 5}, {2, 4, 7, 5}, {4, 2, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 3, 2, 2}, {1, 2, 3, 2, 1, 2}, {1, 2, 1, 1, 2, 2}, {1, 3, 2, 2, 2, 2}, {1, 2, 3, 2, 1, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 0, 3, 0}, {1, 3, 3, 0, 2, 0}, {1, 2, 0, 1, 5, 2}, {4, 0, 3, 5, 1, 2}, {1, 0, 0, 1, 1, 2}, {1, 0, 0, 3, 0, 0}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {3, 3, 3, 2}, {2, 2, 3, 3}, {2, 3, 2, 2}, {2, 3, 3, 3}, {2, 2, 3, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 2}, {0, 0, 0, 3}, {1, 2, 0, 0}, {1, 0, 2, 0}, {1, 0, 0, 0}, {3, 0, 0, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {2}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {2}});
             break;
         }
         case(1053):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 7, 6}, {4, 3, 6, 7}, {4, 3, 7, 5}, {1, 2, 7, 5}, {2, 4, 7, 5}, {4, 2, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 3, 2, 2}, {1, 2, 3, 2, 1, 2}, {1, 2, 1, 1, 2, 2}, {1, 3, 2, 2, 2, 2}, {1, 2, 3, 2, 1, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 1, 1, 0, 0, 3}, {3, 0, 0, 0, 1, 3}, {3, 1, 0, 2, 5, 1}, {2, 3, 1, 5, 2, 1}, {3, 0, 3, 2, 2, 1}, {3, 3, 0, 0, 0, 3}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {3, 3, 3, 2}, {2, 2, 3, 3}, {2, 3, 2, 2}, {2, 3, 3, 3}, {2, 2, 3, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 3, 1}, {0, 0, 0, 0}, {2, 1, 0, 0}, {2, 0, 1, 3}, {2, 0, 0, 0}, {0, 3, 0, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {2}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {2}});

             break;
         }
         case(4312):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 7, 6}, {4, 3, 6, 7}, {4, 3, 7, 5}, {1, 2, 7, 5}, {2, 4, 7, 5}, {4, 2, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 3, 2, 2}, {1, 2, 3, 2, 1, 2}, {1, 2, 1, 1, 2, 2}, {1, 3, 2, 2, 2, 2}, {1, 2, 3, 2, 1, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 1, 4, 3, 2, 0}, {2, 2, 2, 0, 1, 0}, {2, 1, 0, 3, 1, 1}, {0, 0, 4, 1, 3, 1}, {2, 0, 0, 3, 3, 1}, {2, 0, 0, 2, 3, 0}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {3, 3, 3, 2}, {2, 2, 3, 3}, {2, 3, 2, 2}, {2, 3, 3, 3}, {2, 2, 3, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 1}, {0, 0, 0, 2}, {3, 1, 0, 0}, {3, 0, 1, 0}, {3, 0, 0, 0}, {2, 0, 0, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {2}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {2}});
             break;
         }
         case(2314):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 7, 6}, {4, 3, 6, 7}, {4, 3, 7, 5}, {1, 2, 7, 5}, {2, 4, 7, 5}, {4, 2, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 3, 2, 2}, {1, 2, 3, 2, 1, 2}, {1, 2, 1, 1, 2, 2}, {1, 3, 2, 2, 2, 2}, {1, 2, 3, 2, 1, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 2, 3, 0, 2}, {4, 0, 0, 0, 3, 2}, {4, 3, 0, 1, 1, 3}, {5, 2, 2, 1, 1, 3}, {4, 0, 2, 1, 1, 3}, {4, 2, 0, 0, 3, 2}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {3, 3, 3, 2}, {2, 2, 3, 3}, {2, 3, 2, 2}, {2, 3, 3, 3}, {2, 2, 3, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 2, 3}, {0, 0, 0, 0}, {1, 3, 0, 0}, {1, 0, 3, 2}, {1, 0, 0, 0}, {0, 2, 0, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {2}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {2}});
             break;
         }
         case(4132):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 7, 6}, {4, 3, 6, 7}, {4, 3, 7, 5}, {1, 2, 7, 5}, {2, 4, 7, 5}, {4, 2, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 3, 2, 2}, {1, 2, 3, 2, 1, 2}, {1, 2, 1, 1, 2, 2}, {1, 3, 2, 2, 2, 2}, {1, 2, 3, 2, 1, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 4, 1, 3, 1}, {2, 3, 3, 0, 0, 1}, {2, 0, 0, 2, 3, 0}, {5, 1, 4, 3, 2, 0}, {2, 0, 1, 2, 2, 0}, {2, 1, 0, 3, 1, 1}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {3, 3, 3, 2}, {2, 2, 3, 3}, {2, 3, 2, 2}, {2, 3, 3, 3}, {2, 2, 3, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 1, 0}, {0, 0, 0, 3}, {2, 0, 0, 0}, {2, 0, 0, 1}, {2, 0, 0, 0}, {3, 1, 0, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {2}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {2}});
             break;
         }
         case(2134):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 7, 6}, {4, 3, 6, 7}, {4, 3, 7, 5}, {1, 2, 7, 5}, {2, 4, 7, 5}, {4, 2, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 3, 2, 2}, {1, 2, 3, 2, 1, 2}, {1, 2, 1, 1, 2, 2}, {1, 3, 2, 2, 2, 2}, {1, 2, 3, 2, 1, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 2, 2, 1, 1, 3}, {4, 1, 1, 0, 2, 3}, {4, 2, 0, 0, 3, 2}, {0, 3, 2, 3, 0, 2}, {4, 0, 3, 0, 0, 2}, {4, 3, 0, 1, 1, 3}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {3, 3, 3, 2}, {2, 2, 3, 3}, {2, 3, 2, 2}, {2, 3, 3, 3}, {2, 2, 3, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 3, 2}, {0, 0, 0, 1}, {0, 2, 0, 0}, {0, 0, 2, 3}, {0, 0, 0, 0}, {1, 3, 0, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {2}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {2}});

             break;
         }
         case(245 ):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 7, 6}, {4, 3, 6, 7}, {4, 3, 7, 5}, {1, 2, 7, 5}, {2, 4, 7, 5}, {4, 2, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 1, 1, 2, 2}, {1, 2, 2, 3, 2, 2}, {1, 2, 3, 2, 1, 2}, {1, 2, 1, 1, 2, 2}, {1, 3, 2, 2, 2, 2}, {1, 2, 3, 2, 1, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 2, 2, 3}, {5, 2, 2, 0, 0, 3}, {5, 0, 0, 1, 4, 0}, {1, 3, 0, 4, 1, 0}, {5, 0, 3, 1, 1, 0}, {5, 3, 0, 2, 2, 3}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 2, 2}, {3, 3, 3, 2}, {2, 2, 3, 3}, {2, 3, 2, 2}, {2, 3, 3, 3}, {2, 2, 3, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 3, 0}, {0, 0, 0, 2}, {1, 0, 0, 0}, {1, 0, 0, 3}, {1, 0, 0, 0}, {2, 3, 0, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {2}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {2}});
             break;
         }
         case(4502):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 7, 6}, {3, 4, 6, 7}, {3, 4, 7, 5}, {2, 1, 7, 5}, {4, 2, 7, 5}, {2, 4, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 3, 2}, {1, 3, 2, 1, 2, 2}, {1, 1, 2, 2, 1, 2}, {1, 2, 3, 2, 2, 2}, {1, 3, 2, 1, 2, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 4, 0, 2, 5, 1}, {2, 2, 2, 0, 0, 1}, {2, 0, 0, 0, 3, 0}, {1, 4, 1, 3, 0, 0}, {2, 1, 0, 3, 3, 0}, {2, 0, 1, 5, 2, 1}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {3, 3, 3, 2}, {2, 3, 2, 3}, {2, 2, 3, 2}, {2, 3, 3, 3}, {2, 3, 2, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 1, 0, 0}, {0, 0, 0, 2}, {3, 0, 0, 0}, {3, 0, 0, 1}, {3, 0, 0, 0}, {2, 0, 1, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {1}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {1}});
             break;
         }
         case(4052):
        {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 7, 6}, {3, 4, 6, 7}, {3, 4, 7, 5}, {2, 1, 7, 5}, {4, 2, 7, 5}, {2, 4, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 3, 2}, {1, 3, 2, 1, 2, 2}, {1, 1, 2, 2, 1, 2}, {1, 2, 3, 2, 2, 2}, {1, 3, 2, 1, 2, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 4, 1, 3, 0, 0}, {2, 3, 3, 1, 0, 0}, {2, 0, 1, 5, 2, 1}, {3, 4, 0, 2, 5, 1}, {2, 0, 0, 2, 2, 1}, {2, 0, 0, 0, 3, 0}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {3, 3, 3, 2}, {2, 3, 2, 3}, {2, 2, 3, 2}, {2, 3, 3, 3}, {2, 3, 2, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 1}, {0, 0, 0, 3}, {2, 0, 1, 0}, {2, 1, 0, 0}, {2, 0, 0, 0}, {3, 0, 0, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {1}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {1}});
             break;
        }
         case(1243):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 7, 6}, {3, 4, 6, 7}, {3, 4, 7, 5}, {2, 1, 7, 5}, {4, 2, 7, 5}, {2, 4, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 3, 2}, {1, 3, 2, 1, 2, 2}, {1, 1, 2, 2, 1, 2}, {1, 2, 3, 2, 2, 2}, {1, 3, 2, 1, 2, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 1, 1, 2, 2, 3}, {3, 2, 2, 1, 0, 3}, {3, 0, 1, 4, 0, 1}, {0, 1, 3, 0, 4, 1}, {3, 3, 0, 0, 0, 1}, {3, 0, 3, 2, 2, 3}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {3, 3, 3, 2}, {2, 3, 2, 3}, {2, 2, 3, 2}, {2, 3, 3, 3}, {2, 3, 2, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 0, 1}, {0, 0, 0, 2}, {0, 0, 1, 0}, {0, 1, 0, 3}, {0, 0, 0, 0}, {2, 0, 3, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {1}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {1}});
             break;
         }
         case(2504):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 7, 6}, {3, 4, 6, 7}, {3, 4, 7, 5}, {2, 1, 7, 5}, {4, 2, 7, 5}, {2, 4, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 3, 2}, {1, 3, 2, 1, 2, 2}, {1, 1, 2, 2, 1, 2}, {1, 2, 3, 2, 2, 2}, {1, 3, 2, 1, 2, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 3, 1, 5, 2}, {4, 1, 1, 3, 0, 2}, {4, 0, 3, 0, 0, 3}, {3, 2, 2, 0, 0, 3}, {4, 2, 0, 0, 0, 3}, {4, 0, 2, 5, 1, 2}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {3, 3, 3, 2}, {2, 3, 2, 3}, {2, 2, 3, 2}, {2, 3, 3, 3}, {2, 3, 2, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 0, 3}, {0, 0, 0, 1}, {0, 0, 3, 0}, {0, 3, 0, 2}, {0, 0, 0, 0}, {1, 0, 2, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {1}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {1}});
             break;
         }
         case(2054):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 7, 6}, {3, 4, 6, 7}, {3, 4, 7, 5}, {2, 1, 7, 5}, {4, 2, 7, 5}, {2, 4, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 3, 2}, {1, 3, 2, 1, 2, 2}, {1, 1, 2, 2, 1, 2}, {1, 2, 3, 2, 2, 2}, {1, 3, 2, 1, 2, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 2, 2, 0, 0, 3}, {4, 0, 0, 2, 0, 3}, {4, 0, 2, 5, 1, 2}, {1, 2, 3, 1, 5, 2}, {4, 3, 0, 1, 1, 2}, {4, 0, 3, 0, 0, 3}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {3, 3, 3, 2}, {2, 3, 2, 3}, {2, 2, 3, 2}, {2, 3, 3, 3}, {2, 3, 2, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 0, 2}, {0, 0, 0, 0}, {1, 0, 2, 0}, {1, 2, 0, 3}, {1, 0, 0, 0}, {0, 0, 3, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {1}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {1}});

             break;
         }
         case(5310):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 7, 6}, {3, 4, 6, 7}, {3, 4, 7, 5}, {2, 1, 7, 5}, {4, 2, 7, 5}, {2, 4, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 3, 2}, {1, 3, 2, 1, 2, 2}, {1, 1, 2, 2, 1, 2}, {1, 2, 3, 2, 2, 2}, {1, 3, 2, 1, 2, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 5, 1, 0, 3, 2}, {0, 0, 0, 1, 0, 2}, {0, 0, 1, 1, 3, 1}, {2, 5, 2, 3, 1, 1}, {0, 2, 0, 3, 3, 1}, {0, 0, 2, 3, 0, 2}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {3, 3, 3, 2}, {2, 3, 2, 3}, {2, 2, 3, 2}, {2, 3, 3, 3}, {2, 3, 2, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 2, 0, 1}, {0, 0, 0, 0}, {3, 0, 1, 0}, {3, 1, 0, 2}, {3, 0, 0, 0}, {0, 0, 2, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {1}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {1}});
             break;
         }
         case(5130):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 7, 6}, {3, 4, 6, 7}, {3, 4, 7, 5}, {2, 1, 7, 5}, {4, 2, 7, 5}, {2, 4, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 3, 2}, {1, 3, 2, 1, 2, 2}, {1, 1, 2, 2, 1, 2}, {1, 2, 3, 2, 2, 2}, {1, 3, 2, 1, 2, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 5, 2, 3, 1, 1}, {0, 3, 3, 2, 0, 1}, {0, 0, 2, 3, 0, 2}, {4, 5, 1, 0, 3, 2}, {0, 1, 0, 0, 0, 2}, {0, 0, 1, 1, 3, 1}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {3, 3, 3, 2}, {2, 3, 2, 3}, {2, 2, 3, 2}, {2, 3, 3, 3}, {2, 3, 2, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 1, 0, 2}, {0, 0, 0, 3}, {0, 0, 2, 0}, {0, 2, 0, 1}, {0, 0, 0, 0}, {3, 0, 1, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {1}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {1}});

             break;
         }
         case(135 ):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 7, 6}, {3, 4, 6, 7}, {3, 4, 7, 5}, {2, 1, 7, 5}, {4, 2, 7, 5}, {2, 4, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 3, 2}, {1, 3, 2, 1, 2, 2}, {1, 1, 2, 2, 1, 2}, {1, 2, 3, 2, 2, 2}, {1, 3, 2, 1, 2, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 0, 1, 1, 3}, {5, 1, 1, 0, 0, 3}, {5, 0, 0, 3, 2, 0}, {2, 0, 3, 2, 3, 0}, {5, 3, 0, 2, 2, 0}, {5, 0, 3, 1, 1, 3}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {3, 3, 3, 2}, {2, 3, 2, 3}, {2, 2, 3, 2}, {2, 3, 3, 3}, {2, 3, 2, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 3, 0, 0}, {0, 0, 0, 1}, {2, 0, 0, 0}, {2, 0, 0, 3}, {2, 0, 0, 0}, {1, 0, 3, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {1}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {1}});

             break;
         }
         case(315 ):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 7, 6}, {3, 4, 6, 7}, {3, 4, 7, 5}, {2, 1, 7, 5}, {4, 2, 7, 5}, {2, 4, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 3, 2}, {1, 3, 2, 1, 2, 2}, {1, 1, 2, 2, 1, 2}, {1, 2, 3, 2, 2, 2}, {1, 3, 2, 1, 2, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 3, 2, 3, 0}, {5, 2, 2, 3, 0, 0}, {5, 0, 3, 1, 1, 3}, {4, 0, 0, 1, 1, 3}, {5, 0, 0, 1, 1, 3}, {5, 0, 0, 3, 2, 0}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {3, 3, 3, 2}, {2, 3, 2, 3}, {2, 2, 3, 2}, {2, 3, 3, 3}, {2, 3, 2, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 3}, {0, 0, 0, 2}, {1, 0, 3, 0}, {1, 3, 0, 0}, {1, 0, 0, 0}, {2, 0, 0, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {1}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {1}});

             break;
         }
         case(3421):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 7, 6}, {3, 4, 6, 7}, {3, 4, 7, 5}, {2, 1, 7, 5}, {4, 2, 7, 5}, {2, 4, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 3, 2}, {1, 3, 2, 1, 2, 2}, {1, 1, 2, 2, 1, 2}, {1, 2, 3, 2, 2, 2}, {1, 3, 2, 1, 2, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 3, 2, 1, 4, 0}, {1, 1, 1, 2, 0, 0}, {1, 0, 2, 2, 3, 2}, {0, 3, 0, 3, 2, 2}, {1, 0, 0, 3, 3, 2}, {1, 0, 0, 4, 1, 0}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {3, 3, 3, 2}, {2, 3, 2, 3}, {2, 2, 3, 2}, {2, 3, 3, 3}, {2, 3, 2, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 2}, {0, 0, 0, 1}, {3, 0, 2, 0}, {3, 2, 0, 0}, {3, 0, 0, 0}, {1, 0, 0, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {1}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {1}});
             break;
         }
         case(3241):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 7, 6}, {3, 4, 6, 7}, {3, 4, 7, 5}, {2, 1, 7, 5}, {4, 2, 7, 5}, {2, 4, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 3, 2}, {1, 3, 2, 1, 2, 2}, {1, 1, 2, 2, 1, 2}, {1, 2, 3, 2, 2, 2}, {1, 3, 2, 1, 2, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 0, 3, 2, 2}, {1, 3, 3, 0, 0, 2}, {1, 0, 0, 4, 1, 0}, {5, 3, 2, 1, 4, 0}, {1, 2, 0, 1, 1, 0}, {1, 0, 2, 2, 3, 2}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {3, 3, 3, 2}, {2, 3, 2, 3}, {2, 2, 3, 2}, {2, 3, 3, 3}, {2, 3, 2, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 2, 0, 0}, {0, 0, 0, 3}, {1, 0, 0, 0}, {1, 0, 0, 2}, {1, 0, 0, 0}, {3, 0, 2, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {1}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {1}});
             break;
         }
         case(1423):
         {
             mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 7, 6}, {3, 4, 6, 7}, {3, 4, 7, 5}, {2, 1, 7, 5}, {4, 2, 7, 5}, {2, 4, 7, 6}});
             mNumNewElem = 6;
             mNumElemToReplace = 1;
             mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 2, 2, 1, 2}, {1, 2, 2, 2, 3, 2}, {1, 3, 2, 1, 2, 2}, {1, 1, 2, 2, 1, 2}, {1, 2, 3, 2, 2, 2}, {1, 3, 2, 1, 2, 2}});
             mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 1, 3, 0, 4, 1}, {3, 0, 0, 3, 0, 1}, {3, 0, 3, 2, 2, 3}, {5, 1, 1, 2, 2, 3}, {3, 1, 0, 2, 2, 3}, {3, 0, 1, 4, 0, 1}});
             mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 3, 2}, {3, 3, 3, 2}, {2, 3, 2, 3}, {2, 2, 3, 2}, {2, 3, 3, 3}, {2, 3, 2, 3}});
             mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 1, 0, 3}, {0, 0, 0, 0}, {2, 0, 3, 0}, {2, 3, 0, 1}, {2, 0, 0, 0}, {0, 0, 1, 0}});
             mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{std::numeric_limits<Integer>::max()}, {1}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {1}});
             break;
         }
         default :
             std::cout<<"Template not found in the catalog"<<std::endl;
             break;

        }

    }

    void
    hierarchy_tet4_4nb(Integer const & aPermutationId)
    {
        switch(aPermutationId)
        {
        case(4250):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 6, 7}, {0, 3, 6, 7}, {3, 5, 6, 7}, {5, 1, 6, 7}, {1, 2, 6, 7}, {2, 4, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 3, 0, 4, 0}, {1, 2, 3, 4, 1, 0}, {5, 2, 2, 1, 1, 0}, {5, 2, 2, 1, 4, 0}, {3, 2, 2, 4, 0, 0}, {0, 3, 2, 0, 0, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 3}, {1, 0, 0, 3}, {1, 0, 0, 2}, {1, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {1}, {2}, {std::numeric_limits<Integer>::max()}, {1}});
            break;
        }
        case(2450):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 6, 7}, {0, 3, 6, 7}, {3, 5, 6, 7}, {5, 1, 6, 7}, {1, 2, 6, 7}, {2, 4, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 3, 2, 0}, {3, 4, 0, 2, 2, 0}, {5, 1, 4, 2, 2, 0}, {5, 1, 1, 2, 2, 0}, {1, 4, 1, 2, 3, 0}, {0, 0, 4, 3, 3, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 0}, {2, 0, 0, 0}, {2, 0, 0, 1}, {2, 0, 0, 1}, {3, 0, 0, 1}, {3, 0, 0, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {1}, {2}, {std::numeric_limits<Integer>::max()}, {1}});

            break;
        }
        case(4205):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 6, 7}, {0, 3, 6, 7}, {3, 5, 6, 7}, {5, 1, 6, 7}, {1, 2, 6, 7}, {2, 4, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 2, 2, 1, 4, 0}, {3, 2, 2, 4, 0, 0}, {0, 3, 2, 0, 0, 0}, {0, 3, 3, 0, 4, 0}, {1, 2, 3, 4, 1, 0}, {5, 2, 2, 1, 1, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 3}, {0, 0, 0, 3}, {1, 0, 0, 3}, {1, 0, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {1}, {2}, {std::numeric_limits<Integer>::max()}, {1}});
            break;
        }
        case(2405):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 6, 7}, {0, 3, 6, 7}, {3, 5, 6, 7}, {5, 1, 6, 7}, {1, 2, 6, 7}, {2, 4, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 1, 1, 2, 2, 0}, {1, 4, 1, 2, 3, 0}, {0, 0, 4, 3, 3, 0}, {0, 0, 0, 3, 2, 0}, {3, 4, 0, 2, 2, 0}, {5, 1, 4, 2, 2, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 1}, {3, 0, 0, 1}, {3, 0, 0, 0}, {3, 0, 0, 0}, {2, 0, 0, 0}, {2, 0, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {1}, {2}, {std::numeric_limits<Integer>::max()}, {1}});
            break;
        }
        case(5031):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 6, 7}, {0, 3, 6, 7}, {3, 5, 6, 7}, {5, 1, 6, 7}, {1, 2, 6, 7}, {2, 4, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 3, 3, 1, 5, 0}, {2, 0, 3, 5, 2, 0}, {3, 0, 0, 2, 2, 0}, {3, 0, 0, 2, 5, 0}, {4, 0, 0, 5, 1, 0}, {1, 3, 0, 1, 1, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 3}, {2, 0, 0, 3}, {2, 0, 0, 0}, {2, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {1}, {2}, {std::numeric_limits<Integer>::max()}, {1}});
            break;
        }
        case(5013):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 6, 7}, {0, 3, 6, 7}, {3, 5, 6, 7}, {5, 1, 6, 7}, {1, 2, 6, 7}, {2, 4, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 2, 5, 0}, {4, 0, 0, 5, 1, 0}, {1, 3, 0, 1, 1, 0}, {1, 3, 3, 1, 5, 0}, {2, 0, 3, 5, 2, 0}, {3, 0, 0, 2, 2, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 3}, {1, 0, 0, 3}, {2, 0, 0, 3}, {2, 0, 0, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {1}, {2}, {std::numeric_limits<Integer>::max()}, {1}});
            break;
        }
        case(531 ):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 6, 7}, {0, 3, 6, 7}, {3, 5, 6, 7}, {5, 1, 6, 7}, {1, 2, 6, 7}, {2, 4, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 1, 3, 0, 0}, {4, 5, 1, 0, 0, 0}, {3, 2, 5, 0, 0, 0}, {3, 2, 2, 0, 0, 0}, {2, 5, 2, 0, 3, 0}, {1, 1, 5, 3, 3, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 2}, {0, 0, 0, 2}, {3, 0, 0, 2}, {3, 0, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {1}, {2}, {std::numeric_limits<Integer>::max()}, {1}});
            break;
        }
        case(513 ):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 6, 7}, {0, 3, 6, 7}, {3, 5, 6, 7}, {5, 1, 6, 7}, {1, 2, 6, 7}, {2, 4, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 2, 2, 0, 0, 0}, {2, 5, 2, 0, 3, 0}, {1, 1, 5, 3, 3, 0}, {1, 1, 1, 3, 0, 0}, {4, 5, 1, 0, 0, 0}, {3, 2, 5, 0, 0, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 2}, {3, 0, 0, 2}, {3, 0, 0, 1}, {3, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {1}, {2}, {std::numeric_limits<Integer>::max()}, {1}});
            break;
        }
        case(3124):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 6, 7}, {0, 3, 6, 7}, {3, 5, 6, 7}, {5, 1, 6, 7}, {1, 2, 6, 7}, {2, 4, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 1, 1, 0, 3, 0}, {5, 1, 1, 3, 2, 0}, {2, 3, 1, 2, 2, 0}, {2, 3, 3, 2, 3, 0}, {0, 1, 3, 3, 0, 0}, {4, 1, 1, 0, 0, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 1}, {2, 0, 0, 1}, {2, 0, 0, 3}, {2, 0, 0, 3}, {0, 0, 0, 3}, {0, 0, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {1}, {2}, {std::numeric_limits<Integer>::max()}, {1}});

            break;
        }
        case(1342):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 6, 7}, {0, 3, 6, 7}, {3, 5, 6, 7}, {5, 1, 6, 7}, {1, 2, 6, 7}, {2, 4, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 2, 3, 1, 0}, {5, 3, 2, 1, 1, 0}, {4, 0, 3, 1, 1, 0}, {4, 0, 0, 1, 1, 0}, {0, 3, 0, 1, 3, 0}, {2, 2, 3, 3, 3, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 2}, {1, 0, 0, 2}, {1, 0, 0, 0}, {1, 0, 0, 0}, {3, 0, 0, 0}, {3, 0, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {1}, {2}, {std::numeric_limits<Integer>::max()}, {1}});

            break;
        }
        case(1324):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 6, 7}, {0, 3, 6, 7}, {3, 5, 6, 7}, {5, 1, 6, 7}, {1, 2, 6, 7}, {2, 4, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 0, 1, 1, 0}, {0, 3, 0, 1, 3, 0}, {2, 2, 3, 3, 3, 0}, {2, 2, 2, 3, 1, 0}, {5, 3, 2, 1, 1, 0}, {4, 0, 3, 1, 1, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 0}, {3, 0, 0, 0}, {3, 0, 0, 2}, {3, 0, 0, 2}, {1, 0, 0, 2}, {1, 0, 0, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {1}, {2}, {std::numeric_limits<Integer>::max()}, {1}});

            break;
        }
        case(3142):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 6, 7}, {0, 3, 6, 7}, {3, 5, 6, 7}, {5, 1, 6, 7}, {1, 2, 6, 7}, {2, 4, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 2, 1, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2, 3, 0}, {0, 1, 3, 3, 0, 0}, {4, 1, 1, 0, 0, 0}, {4, 1, 1, 0, 3, 0}, {5, 1, 1, 3, 2, 0}, {2, 3, 1, 2, 2, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 3}, {0, 0, 0, 3}, {0, 0, 0, 1}, {0, 0, 0, 1}, {2, 0, 0, 1}, {2, 0, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {1}, {2}, {std::numeric_limits<Integer>::max()}, {1}});

            break;
        }
        case(5024):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 6, 7}, {3, 0, 6, 7}, {5, 3, 6, 7}, {1, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 0, 5, 1, 0}, {3, 0, 0, 2, 5, 0}, {2, 0, 3, 2, 2, 0}, {2, 3, 3, 5, 2, 0}, {1, 3, 0, 1, 5, 0}, {4, 0, 0, 1, 1, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 0}, {2, 0, 0, 0}, {2, 0, 0, 3}, {2, 0, 0, 3}, {1, 0, 0, 3}, {1, 0, 0, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {2}, {1}, {std::numeric_limits<Integer>::max()}, {2}});

            break;
        }
        case(524 ):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 6, 7}, {3, 0, 6, 7}, {5, 3, 6, 7}, {1, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 1, 1, 0, 0, 0}, {1, 1, 5, 3, 0, 0}, {2, 5, 2, 3, 3, 0}, {2, 2, 2, 0, 3, 0}, {3, 2, 5, 0, 0, 0}, {4, 5, 1, 0, 0, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 1}, {3, 0, 0, 1}, {3, 0, 0, 2}, {3, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {2}, {1}, {std::numeric_limits<Integer>::max()}, {2}});
            break;
        }
        case(5042):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 6, 7}, {3, 0, 6, 7}, {5, 3, 6, 7}, {1, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 5, 2, 0}, {1, 3, 0, 1, 5, 0}, {4, 0, 0, 1, 1, 0}, {4, 0, 0, 5, 1, 0}, {3, 0, 0, 2, 5, 0}, {2, 0, 3, 2, 2, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 3}, {1, 0, 0, 3}, {1, 0, 0, 0}, {1, 0, 0, 0}, {2, 0, 0, 0}, {2, 0, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {2}, {1}, {std::numeric_limits<Integer>::max()}, {2}});

            break;
        }
        case(542 ):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 6, 7}, {3, 0, 6, 7}, {5, 3, 6, 7}, {1, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 2, 0, 3, 0}, {3, 2, 5, 0, 0, 0}, {4, 5, 1, 0, 0, 0}, {4, 1, 1, 0, 0, 0}, {1, 1, 5, 3, 0, 0}, {2, 5, 2, 3, 3, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 1}, {0, 0, 0, 1}, {3, 0, 0, 1}, {3, 0, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {2}, {1}, {std::numeric_limits<Integer>::max()}, {2}});

            break;
        }
        case(3150):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 6, 7}, {3, 0, 6, 7}, {5, 3, 6, 7}, {1, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 3, 3, 0, 0}, {2, 3, 1, 2, 3, 0}, {5, 1, 1, 2, 2, 0}, {5, 1, 1, 3, 2, 0}, {4, 1, 1, 0, 3, 0}, {0, 1, 3, 0, 0, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 3}, {2, 0, 0, 3}, {2, 0, 0, 1}, {2, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {2}, {1}, {std::numeric_limits<Integer>::max()}, {2}});

            break;
        }
        case(1350):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 6, 7}, {3, 0, 6, 7}, {5, 3, 6, 7}, {1, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 1, 3, 0}, {4, 0, 3, 1, 1, 0}, {5, 3, 2, 1, 1, 0}, {5, 2, 2, 1, 1, 0}, {2, 2, 3, 3, 1, 0}, {0, 3, 0, 3, 3, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 2}, {1, 0, 0, 2}, {3, 0, 0, 2}, {3, 0, 0, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {2}, {1}, {std::numeric_limits<Integer>::max()}, {2}});

            break;
        }
        case(3105):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 6, 7}, {3, 0, 6, 7}, {5, 3, 6, 7}, {1, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 1, 1, 3, 2, 0}, {4, 1, 1, 0, 3, 0}, {0, 1, 3, 0, 0, 0}, {0, 3, 3, 3, 0, 0}, {2, 3, 1, 2, 3, 0}, {5, 1, 1, 2, 2, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 3}, {0, 0, 0, 3}, {2, 0, 0, 3}, {2, 0, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {2}, {1}, {std::numeric_limits<Integer>::max()}, {2}});
            break;
        }
        case(1305):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 6, 7}, {3, 0, 6, 7}, {5, 3, 6, 7}, {1, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 2, 2, 1, 1, 0}, {2, 2, 3, 3, 1, 0}, {0, 3, 0, 3, 3, 0}, {0, 0, 0, 1, 3, 0}, {4, 0, 3, 1, 1, 0}, {5, 3, 2, 1, 1, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 2}, {3, 0, 0, 2}, {3, 0, 0, 0}, {3, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {2}, {1}, {std::numeric_limits<Integer>::max()}, {2}});
            break;
        }
        case(4231):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 6, 7}, {3, 0, 6, 7}, {5, 3, 6, 7}, {1, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 3, 3, 4, 1, 0}, {0, 3, 2, 0, 4, 0}, {3, 2, 2, 0, 0, 0}, {3, 2, 2, 4, 0, 0}, {5, 2, 2, 1, 4, 0}, {1, 2, 3, 1, 1, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 3}, {0, 0, 0, 3}, {0, 0, 0, 2}, {0, 0, 0, 2}, {1, 0, 0, 2}, {1, 0, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {2}, {1}, {std::numeric_limits<Integer>::max()}, {2}});
            break;
        }
        case(2431):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 6, 7}, {3, 0, 6, 7}, {5, 3, 6, 7}, {1, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 1, 2, 3, 0}, {5, 1, 4, 2, 2, 0}, {3, 4, 0, 2, 2, 0}, {3, 0, 0, 2, 2, 0}, {0, 0, 4, 3, 2, 0}, {1, 4, 1, 3, 3, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 1}, {2, 0, 0, 1}, {2, 0, 0, 0}, {2, 0, 0, 0}, {3, 0, 0, 0}, {3, 0, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {2}, {1}, {std::numeric_limits<Integer>::max()}, {2}});
            break;
        }
        case(4213):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 6, 7}, {3, 0, 6, 7}, {5, 3, 6, 7}, {1, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 2, 2, 4, 0, 0}, {5, 2, 2, 1, 4, 0}, {1, 2, 3, 1, 1, 0}, {1, 3, 3, 4, 1, 0}, {0, 3, 2, 0, 4, 0}, {3, 2, 2, 0, 0, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 2}, {1, 0, 0, 2}, {1, 0, 0, 3}, {1, 0, 0, 3}, {0, 0, 0, 3}, {0, 0, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {2}, {1}, {std::numeric_limits<Integer>::max()}, {2}});
            break;
        }
        case(2413):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 6, 7}, {3, 0, 6, 7}, {5, 3, 6, 7}, {1, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 1, 2, 2, 2, 3}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 2, 2, 0}, {0, 0, 4, 3, 2, 0}, {1, 4, 1, 3, 3, 0}, {1, 1, 1, 2, 3, 0}, {5, 1, 4, 2, 2, 0}, {3, 4, 0, 2, 2, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}, {2, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 0}, {3, 0, 0, 0}, {3, 0, 0, 1}, {3, 0, 0, 1}, {2, 0, 0, 1}, {2, 0, 0, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {2}, {1}, {std::numeric_limits<Integer>::max()}, {2}});

            break;
        }
        default :
        {
            std::cout<<"Template not found in the catalog"<<std::endl;
            break;
        }
        }
    }

    void
    hierarchy_tet4_4nc(Integer const & aPermutationId)
    {

        switch(aPermutationId)
        {
        case(4520):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 7}, {0, 3, 5, 7}, {5, 3, 6, 7}, {1, 2, 6, 7}, {2, 4, 5, 7}, {2, 5, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 1, 2, 3, 2, 2}, {1, 2, 1, 1, 2, 2}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 3, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 3, 0, 4, 0}, {1, 2, 3, 4, 1, 0}, {2, 5, 2, 0, 1, 1}, {3, 2, 5, 4, 0, 1}, {0, 3, 2, 0, 0, 0}, {2, 2, 2, 0, 0, 1}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 3, 2, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 3}, {1, 0, 0, 3}, {0, 1, 0, 2}, {0, 0, 1, 2}, {0, 0, 0, 3}, {0, 0, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {1}});
            break;
        }
        case(2540):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 7}, {0, 3, 5, 7}, {5, 3, 6, 7}, {1, 2, 6, 7}, {2, 4, 5, 7}, {2, 5, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 1, 2, 3, 2, 2}, {1, 2, 1, 1, 2, 2}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 3, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 3, 2, 0}, {3, 4, 0, 2, 2, 0}, {4, 5, 1, 0, 2, 2}, {1, 1, 5, 2, 3, 2}, {0, 0, 4, 3, 3, 0}, {4, 1, 1, 3, 0, 2}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 3, 2, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 0}, {2, 0, 0, 0}, {0, 2, 0, 1}, {3, 0, 2, 1}, {3, 0, 0, 0}, {0, 0, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {1}});
            break;
        }
        case(4025):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 7}, {0, 3, 5, 7}, {5, 3, 6, 7}, {1, 2, 6, 7}, {2, 4, 5, 7}, {2, 5, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 1, 2, 3, 2, 2}, {1, 2, 1, 1, 2, 2}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 3, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 2, 2, 1, 4, 0}, {3, 2, 2, 4, 0, 0}, {2, 0, 3, 0, 0, 0}, {1, 3, 0, 4, 1, 0}, {5, 2, 2, 1, 1, 0}, {2, 3, 3, 1, 0, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 3, 2, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 3}, {1, 0, 0, 3}, {1, 0, 0, 2}, {0, 0, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {1}});

            break;
        }
        case(2045):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 7}, {0, 3, 5, 7}, {5, 3, 6, 7}, {1, 2, 6, 7}, {2, 4, 5, 7}, {2, 5, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 1, 2, 3, 2, 2}, {1, 2, 1, 1, 2, 2}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 3, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 1, 1, 2, 2, 0}, {1, 4, 1, 2, 3, 0}, {4, 0, 0, 0, 3, 3}, {3, 0, 0, 2, 2, 3}, {5, 1, 4, 2, 2, 0}, {4, 0, 0, 2, 0, 3}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 3, 2, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 1}, {3, 0, 0, 1}, {0, 3, 0, 0}, {2, 0, 3, 0}, {2, 0, 0, 1}, {0, 0, 0, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {1}});

            break;
        }
        case(5301):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 7}, {0, 3, 5, 7}, {5, 3, 6, 7}, {1, 2, 6, 7}, {2, 4, 5, 7}, {2, 5, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 1, 2, 3, 2, 2}, {1, 2, 1, 1, 2, 2}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 3, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 3, 3, 1, 5, 0}, {2, 0, 3, 5, 2, 0}, {0, 3, 0, 0, 2, 2}, {4, 0, 3, 5, 1, 2}, {1, 3, 0, 1, 1, 0}, {0, 0, 0, 1, 0, 2}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 3, 2, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 3}, {2, 0, 0, 3}, {0, 2, 0, 0}, {1, 0, 2, 0}, {1, 0, 0, 3}, {0, 0, 0, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {1}});

            break;
        }
        case(5103):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 7}, {0, 3, 5, 7}, {5, 3, 6, 7}, {1, 2, 6, 7}, {2, 4, 5, 7}, {2, 5, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 1, 2, 3, 2, 2}, {1, 2, 1, 1, 2, 2}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 3, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 2, 5, 0}, {4, 0, 0, 5, 1, 0}, {0, 1, 3, 0, 1, 1}, {2, 3, 1, 5, 2, 1}, {3, 0, 0, 2, 2, 0}, {0, 3, 3, 2, 0, 1}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 3, 2, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 0}, {1, 0, 0, 0}, {0, 1, 0, 3}, {2, 0, 1, 3}, {2, 0, 0, 0}, {0, 0, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {1}});
            break;
        }
        case(351 ):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 7}, {0, 3, 5, 7}, {5, 3, 6, 7}, {1, 2, 6, 7}, {2, 4, 5, 7}, {2, 5, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 1, 2, 3, 2, 2}, {1, 2, 1, 1, 2, 2}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 3, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 1, 3, 0, 0}, {4, 5, 1, 0, 0, 0}, {5, 3, 2, 0, 0, 0}, {2, 2, 3, 0, 3, 0}, {1, 1, 5, 3, 3, 0}, {5, 2, 2, 3, 0, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 3, 2, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 2}, {3, 0, 0, 2}, {3, 0, 0, 1}, {0, 0, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {1}});

            break;
        }
        case(153 ):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 7}, {0, 3, 5, 7}, {5, 3, 6, 7}, {1, 2, 6, 7}, {2, 4, 5, 7}, {2, 5, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 1, 2, 3, 2, 2}, {1, 2, 1, 1, 2, 2}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 3, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 2, 2, 0, 0, 0}, {2, 5, 2, 0, 3, 0}, {5, 1, 1, 0, 3, 3}, {4, 1, 1, 0, 0, 3}, {3, 2, 5, 0, 0, 0}, {5, 1, 1, 0, 0, 3}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 3, 2, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 2}, {3, 0, 0, 2}, {0, 3, 0, 1}, {0, 0, 3, 1}, {0, 0, 0, 2}, {0, 0, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {1}});

            break;
        }
        case(3412):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 7}, {0, 3, 5, 7}, {5, 3, 6, 7}, {1, 2, 6, 7}, {2, 4, 5, 7}, {2, 5, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 1, 2, 3, 2, 2}, {1, 2, 1, 1, 2, 2}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 3, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2, 3, 0}, {0, 1, 3, 3, 0, 0}, {1, 4, 1, 0, 0, 0}, {5, 1, 4, 3, 2, 0}, {2, 3, 1, 2, 2, 0}, {1, 1, 1, 2, 0, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 3, 2, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 3}, {0, 0, 0, 3}, {0, 0, 0, 1}, {2, 0, 0, 1}, {2, 0, 0, 3}, {0, 0, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {1}});


            break;
        }
        case(3214):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 7}, {0, 3, 5, 7}, {5, 3, 6, 7}, {1, 2, 6, 7}, {2, 4, 5, 7}, {2, 5, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 1, 2, 3, 2, 2}, {1, 2, 1, 1, 2, 2}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 3, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 1, 1, 0, 3, 0}, {5, 1, 1, 3, 2, 0}, {1, 2, 3, 0, 2, 2}, {0, 3, 2, 3, 0, 2}, {4, 1, 1, 0, 0, 0}, {1, 3, 3, 0, 0, 2}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 3, 2, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 1}, {2, 0, 0, 1}, {0, 2, 0, 3}, {0, 0, 2, 3}, {0, 0, 0, 1}, {0, 0, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {1}});
            break;
        }
        case(1432):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 7}, {0, 3, 5, 7}, {5, 3, 6, 7}, {1, 2, 6, 7}, {2, 4, 5, 7}, {2, 5, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 1, 2, 3, 2, 2}, {1, 2, 1, 1, 2, 2}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 3, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 2, 3, 1, 0}, {5, 3, 2, 1, 1, 0}, {3, 4, 0, 0, 1, 1}, {0, 0, 4, 1, 3, 1}, {2, 2, 3, 3, 3, 0}, {3, 0, 0, 3, 0, 1}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 3, 2, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 2}, {1, 0, 0, 2}, {0, 1, 0, 0}, {3, 0, 1, 0}, {3, 0, 0, 2}, {0, 0, 0, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {1}});
            break;
        }
        case(1234):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 5, 7}, {0, 3, 5, 7}, {5, 3, 6, 7}, {1, 2, 6, 7}, {2, 4, 5, 7}, {2, 5, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 2, 1, 3}, {1, 1, 2, 1, 2, 3}, {1, 1, 2, 3, 2, 2}, {1, 2, 1, 1, 2, 2}, {1, 2, 1, 2, 2, 3}, {1, 2, 2, 2, 3, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 0, 1, 1, 0}, {0, 3, 0, 1, 3, 0}, {3, 2, 2, 0, 3, 3}, {5, 2, 2, 1, 1, 3}, {4, 0, 3, 1, 1, 0}, {3, 2, 2, 1, 0, 3}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}, {2, 3, 2, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 0}, {3, 0, 0, 0}, {0, 3, 0, 2}, {1, 0, 3, 2}, {1, 0, 0, 0}, {0, 0, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{2}, {std::numeric_limits<Integer>::max()}, {2}, {std::numeric_limits<Integer>::max()}, {1}, {1}});
            break;
        }
        case(5402):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 7}, {3, 0, 5, 7}, {3, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 5, 7}, {5, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 2, 1, 2, 3, 2}, {1, 1, 2, 2, 1, 2}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 3, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 5, 2, 0}, {1, 3, 0, 1, 5, 0}, {0, 0, 4, 1, 0, 1}, {3, 4, 0, 2, 5, 1}, {2, 0, 3, 2, 2, 0}, {0, 0, 0, 0, 2, 1}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 3}, {1, 0, 0, 3}, {0, 0, 1, 0}, {2, 1, 0, 0}, {2, 0, 0, 3}, {0, 0, 0, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {2}});

            break;
        }
        case(452 ):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 7}, {3, 0, 5, 7}, {3, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 5, 7}, {5, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 2, 1, 2, 3, 2}, {1, 1, 2, 2, 1, 2}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 3, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 2, 2, 0, 3, 0}, {3, 2, 5, 0, 0, 0}, {5, 1, 4, 0, 0, 0}, {1, 4, 1, 3, 0, 0}, {2, 5, 2, 3, 3, 0}, {5, 1, 1, 0, 3, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 1}, {3, 0, 0, 1}, {3, 0, 0, 2}, {0, 0, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {2}});

            break;
        }
        case(5204):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 7}, {3, 0, 5, 7}, {3, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 5, 7}, {5, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 2, 1, 2, 3, 2}, {1, 1, 2, 2, 1, 2}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 3, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 0, 0, 5, 1, 0}, {3, 0, 0, 2, 5, 0}, {0, 3, 2, 2, 0, 2}, {1, 2, 3, 1, 5, 2}, {4, 0, 0, 1, 1, 0}, {0, 3, 3, 0, 1, 2}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 0}, {2, 0, 0, 0}, {0, 0, 2, 3}, {1, 2, 0, 3}, {1, 0, 0, 0}, {0, 0, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {2}});

            break;
        }
        case( 254):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 7}, {3, 0, 5, 7}, {3, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 5, 7}, {5, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 2, 1, 2, 3, 2}, {1, 1, 2, 2, 1, 2}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 3, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{4, 1, 1, 0, 0, 0}, {1, 1, 5, 3, 0, 0}, {5, 2, 2, 3, 0, 3}, {3, 2, 2, 0, 0, 3}, {4, 5, 1, 0, 0, 0}, {5, 2, 2, 0, 0, 3}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 1}, {3, 0, 0, 1}, {0, 0, 3, 2}, {0, 3, 0, 2}, {0, 0, 0, 1}, {0, 0, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {2}});
            break;
        }
        case(3510):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 7}, {3, 0, 5, 7}, {3, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 5, 7}, {5, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 2, 1, 2, 3, 2}, {1, 1, 2, 2, 1, 2}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 3, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 3, 3, 3, 0, 0}, {2, 3, 1, 2, 3, 0}, {1, 1, 5, 2, 0, 2}, {4, 5, 1, 0, 3, 2}, {0, 1, 3, 0, 0, 0}, {1, 1, 1, 0, 0, 2}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 3}, {2, 0, 0, 3}, {0, 0, 2, 1}, {0, 2, 0, 1}, {0, 0, 0, 3}, {0, 0, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {2}});

            break;
        }
        case(1530):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 7}, {3, 0, 5, 7}, {3, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 5, 7}, {5, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 2, 1, 2, 3, 2}, {1, 1, 2, 2, 1, 2}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 3, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 1, 3, 0}, {4, 0, 3, 1, 1, 0}, {3, 2, 5, 1, 0, 1}, {2, 5, 2, 3, 1, 1}, {0, 3, 0, 3, 3, 0}, {3, 2, 2, 0, 3, 1}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 0}, {1, 0, 0, 0}, {0, 0, 1, 2}, {3, 1, 0, 2}, {3, 0, 0, 0}, {0, 0, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {2}});

            break;
        }
        case(3015):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 7}, {3, 0, 5, 7}, {3, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 5, 7}, {5, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 2, 1, 2, 3, 2}, {1, 1, 2, 2, 1, 2}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 3, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 1, 1, 3, 2, 0}, {4, 1, 1, 0, 3, 0}, {1, 3, 0, 0, 0, 0}, {2, 0, 3, 2, 3, 0}, {5, 1, 1, 2, 2, 0}, {1, 3, 3, 0, 2, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 3}, {2, 0, 0, 3}, {2, 0, 0, 1}, {0, 0, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {2}});
            break;
        }
        case(1035):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 7}, {3, 0, 5, 7}, {3, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 5, 7}, {5, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 2, 1, 2, 3, 2}, {1, 1, 2, 2, 1, 2}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 3, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{5, 2, 2, 1, 1, 0}, {2, 2, 3, 3, 1, 0}, {3, 0, 0, 3, 0, 3}, {4, 0, 0, 1, 1, 3}, {5, 3, 2, 1, 1, 0}, {3, 0, 0, 0, 1, 3}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 2}, {3, 0, 0, 2}, {0, 0, 3, 0}, {1, 3, 0, 0}, {1, 0, 0, 2}, {0, 0, 0, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {2}});


            break;
        }
        case(4321):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 7}, {3, 0, 5, 7}, {3, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 5, 7}, {5, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 2, 1, 2, 3, 2}, {1, 1, 2, 2, 1, 2}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 3, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 3, 3, 4, 1, 0}, {0, 3, 2, 0, 4, 0}, {2, 2, 3, 0, 0, 0}, {5, 3, 2, 1, 4, 0}, {1, 2, 3, 1, 1, 0}, {2, 2, 2, 0, 1, 0}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 0, 0, 3}, {0, 0, 0, 3}, {0, 0, 0, 2}, {1, 0, 0, 2}, {1, 0, 0, 3}, {0, 0, 0, 2}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {2}});
            break;
        }
        case(2341):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 7}, {3, 0, 5, 7}, {3, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 5, 7}, {5, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 2, 1, 2, 3, 2}, {1, 1, 2, 2, 1, 2}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 3, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{1, 1, 1, 2, 3, 0}, {5, 1, 4, 2, 2, 0}, {4, 0, 3, 2, 0, 2}, {0, 3, 0, 3, 2, 2}, {1, 4, 1, 3, 3, 0}, {4, 0, 0, 0, 3, 2}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 1}, {2, 0, 0, 1}, {0, 0, 2, 0}, {3, 2, 0, 0}, {3, 0, 0, 1}, {0, 0, 0, 0}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {2}});

            break;
        }
        case(4123):
        {

            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 7}, {3, 0, 5, 7}, {3, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 5, 7}, {5, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 2, 1, 2, 3, 2}, {1, 1, 2, 2, 1, 2}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 3, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 2, 2, 4, 0, 0}, {5, 2, 2, 1, 4, 0}, {2, 3, 1, 1, 0, 1}, {0, 1, 3, 0, 4, 1}, {3, 2, 2, 0, 0, 0}, {2, 3, 3, 0, 0, 1}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{0, 0, 0, 2}, {1, 0, 0, 2}, {0, 0, 1, 3}, {0, 1, 0, 3}, {0, 0, 0, 2}, {0, 0, 0, 3}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {2}});

            break;
        }
        case(2143):
        {
            mNewElementToNode = moris::Matrix<Integer, Integer_Matrix>({{0, 4, 5, 7}, {3, 0, 5, 7}, {3, 5, 6, 7}, {2, 1, 6, 7}, {4, 2, 5, 7}, {5, 2, 6, 7}});
            mNumNewElem = 6;
            mNumElemToReplace = 1;
            mNewParentEdgeRanks = moris::Matrix<Integer, Integer_Matrix>({{1, 2, 2, 1, 2, 3}, {1, 2, 1, 2, 1, 3}, {1, 2, 1, 2, 3, 2}, {1, 1, 2, 2, 1, 2}, {1, 1, 2, 2, 2, 3}, {1, 2, 2, 3, 2, 2}});
            mNewParentEdgeOrdinals = moris::Matrix<Integer, Integer_Matrix>({{3, 0, 0, 2, 2, 0}, {0, 0, 4, 3, 2, 0}, {4, 1, 1, 3, 0, 3}, {5, 1, 1, 2, 2, 3}, {3, 4, 0, 2, 2, 0}, {4, 1, 1, 0, 2, 3}});
            mNewParentFaceRanks = moris::Matrix<Integer, Integer_Matrix>({{2, 3, 3, 2}, {2, 3, 3, 2}, {3, 3, 2, 2}, {2, 2, 3, 2}, {2, 3, 3, 2}, {3, 3, 3, 2}});
            mNewParentFaceOrdinals = moris::Matrix<Integer, Integer_Matrix>({{2, 0, 0, 0}, {3, 0, 0, 0}, {0, 0, 3, 1}, {2, 3, 0, 1}, {2, 0, 0, 0}, {0, 0, 0, 1}});
            mNewElementInterfaceSides = moris::Matrix<Integer, Integer_Matrix>({{1}, {std::numeric_limits<Integer>::max()}, {1}, {std::numeric_limits<Integer>::max()}, {2}, {2}});
            break;
        }
        default :
        {
            std::cout<<"Template not found in the catalog"<<std::endl;
            break;
        }
        }
    }





};

}
#endif /* SRC_XTK_CL_XTK_CHILD_MESH_MODIFICATION_TEMPLATE_HPP_ */