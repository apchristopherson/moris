/**
 * cl_XTK_Subphase_Group.hpp  
 * 
 *  Created on: Jan  18, 2022 
 *      Author: Nils Wunsch
 */
#ifndef SRC_cl_XTK_Subphase_Group
#define SRC_cl_XTK_Subphase_Group

#include "cl_Cell.hpp"
#include "cl_MTK_Cell.hpp"

using namespace moris;
namespace xtk
{
    // ----------------------------------------------------------------------------------

    class Subphase_Group
    {
        // ----------------------------------------------------------------------------------
      private:
        // index for Subphase_Group
        moris_index mSubphaseGroupIndex;

        // associated Bspline Element
        moris_index mBsplineCellIndex;

        // index of bulk phase the SPG lives on
        moris_index mBulkPhaseIndex;

        // List of subphases in group
        moris::Cell< moris_index > mSubphaseIndicesInGroup;

        // List of ig cells in group
        moris::Cell< moris_index > mIgCellIndicesInGroup;
        bool mIgCellIndicesSet = false;

        // side ordinals of the basis (B-spline) cell through which the SPG is connected to neighboring SPGs
        moris::Cell< moris_index > mLigamentSideOrdinals;
        bool mLigamentSideOrdinalsSet = false;

        // ----------------------------------------------------------------------------------

      public:
        Subphase_Group(
                moris_index                aSubphaseGroupIndex,
                moris_index                aBsplineCellIndex,
                moris::Cell< moris_index > aSubphaseIndicesInGroup )
        {
            mSubphaseGroupIndex =     aSubphaseGroupIndex;
            mBsplineCellIndex =       aBsplineCellIndex;
            mSubphaseIndicesInGroup = aSubphaseIndicesInGroup;
        }

        ~Subphase_Group(){}

        moris_index
        get_index() const
        {
            return mSubphaseGroupIndex;
        }

        void 
        set_ligament_side_ordinals( moris::Cell< moris_index > aLigamentSideOrdinals )
        { 
            mLigamentSideOrdinals = aLigamentSideOrdinals;
            mLigamentSideOrdinalsSet = true;
        }

        const moris::Cell< moris_index > &
        get_ligament_side_ordinals() const
        {
            MORIS_ASSERT( mLigamentSideOrdinalsSet, "Subphase_Group::get_ligament_side_ordinals() - Side ordinals have not been set yet." );
            return mLigamentSideOrdinals;
        }

        void 
        set_ig_cell_indices( moris::Cell< moris_index > aIgCellIndicesInGroup )
        {
            MORIS_ASSERT( aIgCellIndicesInGroup.size() > 0, "Subphase_Group::set_ig_cell_indices() - passing empty list IG cells" );
            mIgCellIndicesInGroup = aIgCellIndicesInGroup;
            mIgCellIndicesSet = true;
        }

        const moris::Cell< moris_index > &
        get_ig_cell_indices_in_group() const
        {
            MORIS_ASSERT( mIgCellIndicesSet, "Subphase_Group::get_ig_cell_indices_in_group() - IG cell indices have not been set yet." );
            return mIgCellIndicesInGroup;
        }

        const moris::Cell< moris_index > &
        get_SP_indices_in_group() const
        {
            return mSubphaseIndicesInGroup;
        }
    };

    // ----------------------------------------------------------------------------------

    struct Bspline_Mesh_Info
    {
        // counter storing the maximum number of B-spline cells and subphase groups in mesh
        moris_index mMaxBsplineCellIndex = -1;
        moris_index mMaxSpgIndex = -1;

        // store for all (Lagrange) extraction cells in which (B-spline) basis cell they live
        moris::Cell< moris_index > mExtractionCellToBsplineCell;

        // input: extraction cell index, SPG index local to extraction cell || output: list of Subphases on IP cell associated with SPG
        moris::Cell< moris::Cell< moris::Cell< moris_index > > > mExtractionCellToSubPhase;

        // store which (Lagrange) extraction cells sit in a given (B-spline) basis cell
        // input: index of (B-spline) basis cell || output: list of (Lagrange) extraction cells (or their indices)
        moris::Cell< moris::Cell< mtk::Cell* > >  mExtractionCellsInBsplineCells; // TODO: needed?
        moris::Cell< moris::Cell< moris_index > > mExtractionCellsIndicesInBsplineCells;

        // store which Subphase groups sit in a given (B-spline) basis cells
        // input: index of (B-spline) basis cell || output: list of SPGs living in it (or their indices)
        moris::Cell< moris::Cell< moris_index > > mSPGsIndicesInBsplineCells;

        // Subphase Groups
        moris::Cell< Subphase_Group* > mSubphaseGroups;

        // SP to SPG map
        moris::Cell< moris_index > mSpToSpgMap;

        // // store which bulk-phases are present in each basis (B-spline) cell
        // // moris::Cell< moris::Cell< moris_index > > mBulkPhasesInBsplineCells; // TODO: needed?

        // ----------------------------------------------------------------------------------

        ~Bspline_Mesh_Info()
        {
            this->delete_subphase_groups();
        }

        void
        delete_subphase_groups()
        {
            // delete subphase groups in Cell
            for ( auto iSPG : mSubphaseGroups )
            {
                delete iSPG;
            }

            // delete memory for cell
            mSubphaseGroups.clear();
        }

        // ----------------------------------------------------------------------------------

        /**
         * @brief free unused memory
         * 
         */
        void 
        finalize()
        {
            mSubphaseGroups.shrink_to_fit();
        }

        // ----------------------------------------------------------------------------------

        moris_index
        get_num_Bspline_cells() const
        {
            return mMaxBsplineCellIndex + 1;
        }

        uint
        get_num_SPGs() const
        {
            return (uint) mMaxSpgIndex + 1;
        }

        // ----------------------------------------------------------------------------------

        moris_index 
        get_bspline_cell_index_for_extraction_cell( moris_index aExtractionCellIndex )
        {
            return mExtractionCellToBsplineCell( aExtractionCellIndex );
        }
        
        // ----------------------------------------------------------------------------------

        moris::Cell< moris_index > const&
        get_SPG_indices_in_bspline_cell( moris_index aBsplineCellIndex )
        {
            MORIS_ASSERT( (uint) aBsplineCellIndex < mExtractionCellsIndicesInBsplineCells.size(), 
                "Bspline_Mesh_Info::get_SPG_indices_in_bspline_cell() - aBsplineCellIndex out of bounds" );
            return mSPGsIndicesInBsplineCells( aBsplineCellIndex );
        }

        // ----------------------------------------------------------------------------------

        uint
        get_num_SPGs_associated_with_extraction_cell( moris_index aExtractionCellIndex )
        {
            // get the underlying B-spline cell's index
            moris_index tBsplineCellIndex = mExtractionCellToBsplineCell( aExtractionCellIndex );

            // get and return the number of SPGs in the B-spline cell
            return mSPGsIndicesInBsplineCells( tBsplineCellIndex ).size();
        }

        // ----------------------------------------------------------------------------------

        const moris::Cell< moris_index > &
        get_SPG_indices_associated_with_extraction_cell( moris_index aExtractionCellIndex )
        {
            // get the underlying B-spline cell's index
            moris_index tBsplineCellIndex = mExtractionCellToBsplineCell( aExtractionCellIndex );

            // get and return the number of SPGs in the B-spline cell
            return mSPGsIndicesInBsplineCells( tBsplineCellIndex );
        }

        // ----------------------------------------------------------------------------------

        void
        add_subphase_group_to_bspline_cell( 
                moris::Cell< moris_index > aSPsInGroup,           // TODO: is it a problem to pass this Cell by reference?
                moris_index                aBsplineElementIndex )
        {
            // track SPG indices and get new one
            mMaxSpgIndex++;

            // create a new SPG and commit it to the B-spline mesh info
            Subphase_Group * tNewSPG = new Subphase_Group( mMaxSpgIndex, aBsplineElementIndex, aSPsInGroup );
            mSubphaseGroups.push_back( tNewSPG );
        }

        // ----------------------------------------------------------------------------------

        void
        add_ig_cell_indices_to_last_admitted_subphase_group( moris::Cell< moris_index > aIgCellIndicesInGroup ) // TODO: is it a problem to pass this Cell by reference?
        {
            // add ig cells to last admitted SPG
            mSubphaseGroups( mMaxSpgIndex )->set_ig_cell_indices( aIgCellIndicesInGroup );
        }

        // ----------------------------------------------------------------------------------

        void
        add_subphase_group_to_last_admitted_bspline_cell( moris::Cell< moris_index > & aSPsInGroup )
        {
            // check  
            MORIS_ASSERT( mMaxBsplineCellIndex > -1, 
                "Bspline_Mesh_Info::add_subphase_group_to_last_admitted_bspline_cell() - no B-spline cells have been admitted yet. Unable to add SPG." );

            // track SPG indices and get new one
            mMaxSpgIndex++;

            // create and commit a new SPG to the B-spline mesh info
            Subphase_Group * tNewSPG = new Subphase_Group( mMaxSpgIndex, mMaxBsplineCellIndex, aSPsInGroup );
            mSubphaseGroups.push_back( tNewSPG );
        }

        // ----------------------------------------------------------------------------------

        bool
        admit_extraction_cell_group( moris::Cell< mtk::Cell * > & tExtractionCellsInBsplineCell )
        {
            // check if list L-to-B-map is initialized
            MORIS_ASSERT( &mExtractionCellToBsplineCell != nullptr && mExtractionCellToBsplineCell.size() > 0, 
                "Bspline_Mesh_Info::admit_extraction_cell_group() -mExtractionCellToBsplineCell has not been initialized." );

            // get size of extraction cell group
            moris::size_t tNumCellsInGroup = tExtractionCellsInBsplineCell.size();
            MORIS_ASSERT( tNumCellsInGroup > 0, 
                "Bspline_Mesh_Info::admit_extraction_cell_group() - empty group cannot be admitted." );

            // get first extraction Cell's index
            moris_index tFirstCellIndex = tExtractionCellsInBsplineCell( 0 )->get_index();
            MORIS_ASSERT( tFirstCellIndex >= 0, 
                "Bspline_Mesh_Info::admit_extraction_cell_group() - trying extraction to admit cell with negative index." );

            // check if Cell index is unknown
            bool tCreateNewBsplineCell = ( mExtractionCellToBsplineCell( (uint) tFirstCellIndex ) == -1 );

            if ( tCreateNewBsplineCell )
            {
                // allocate new B-spline Cell Index
                mMaxBsplineCellIndex++;

                // create List 
                moris::Cell< moris_index > tExtractionCellIndices( tNumCellsInGroup );

                // go over all cells to be admitted
                for ( moris::size_t iCell = 0; iCell < tNumCellsInGroup; iCell++)
                {              
                    // get extraction Cell's index
                    moris_index tCellIndex = tExtractionCellsInBsplineCell( iCell )->get_index();
                    MORIS_ASSERT( tCellIndex >= 0, 
                        "Bspline_Mesh_Info::admit_extraction_cell_group() - trying to admit extraction cell with negative index." );

                    // convert moris_index to uint
                    uint tPosCellIndex = (uint) tCellIndex;

                    // give B-spline cell index to extraction cell
                    mExtractionCellToBsplineCell( tPosCellIndex ) = mMaxBsplineCellIndex;

                    // give extraction cell index to B-spline cell
                    tExtractionCellIndices( iCell ) = tCellIndex;
                }

                // add to B-spline cell to Lagrange cell map
                mExtractionCellsIndicesInBsplineCells.push_back( tExtractionCellIndices );
            }

            // return whether new B-spline Cell was created
            return tCreateNewBsplineCell;
        }

        // ----------------------------------------------------------------------------------

        void
        set_ligament_side_ordinals_of_last_admitted_subphase_group( moris::Cell< bool > aActiveLigamentSideOrdinals )
        {
            // initialize list of side ordinals with correct size
            moris::Cell< moris_index > tLigamentSideOrdinals( 6 );

            // intialize counter for number of side ordinals
            uint tNumSideOrds = 0;

            // go through side ordinals, see whats active and add to list of ordinals
            for ( moris::size_t iOrd = 0; iOrd < aActiveLigamentSideOrdinals.size(); iOrd++ )
            {
                if( aActiveLigamentSideOrdinals( iOrd ) )
                {
                    tLigamentSideOrdinals( tNumSideOrds ) = iOrd;
                    tNumSideOrds++;
                }
            }

            // check that SPGs have been comitted
            MORIS_ASSERT( mMaxSpgIndex > -1, 
                "Bspline_Mesh_Info::set_ligament_side_ordinals_of_last_admitted_subphase_group() - No SPGs have been admitted yet. Unable to add ligaments." );

            // commit information to corresponding subphase group
            mSubphaseGroups( mMaxSpgIndex )->set_ligament_side_ordinals( tLigamentSideOrdinals );
        }

        // ----------------------------------------------------------------------------------

        void
        create_SP_to_SPG_map( luint tTotNumSPs )
        {
            // initialize map with correct size
            mSpToSpgMap.resize( tTotNumSPs, -1 );

            // go over SPGs and get their SP indices, then list them in the map
            for ( luint iSPG = 0; iSPG < mSubphaseGroups.size(); iSPG++ )
            {
                // get list of SP indices in SPG
                const moris::Cell< moris_index > & tSpIndicesInGroup = mSubphaseGroups( iSPG )->get_SP_indices_in_group();

                // get the SPG's index
                const moris_index tSpgIndex = mSubphaseGroups( iSPG )->get_index();

                // loop over the SPs and set their SPG indices in the map
                for ( moris::size_t iSP = 0; iSP < tSpIndicesInGroup.size(); iSP++ )
                {
                    mSpToSpgMap( tSpIndicesInGroup( iSP ) ) = tSpgIndex;
                }
            }
        }

        // ----------------------------------------------------------------------------------
    };

    // ----------------------------------------------------------------------------------

}// namespace xtk

#endif /* cl_XTK_Subphase_Group.hpp */