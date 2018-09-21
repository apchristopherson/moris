/*
 * cl_HMR_BSpline_Mesh_Base.hpp
 *
 *  Created on: Jun 12, 2018
 *      Author: messe
 */

#ifndef SRC_HMR_CL_HMR_BSPLINE_MESH_BASE_HPP_
#define SRC_HMR_CL_HMR_BSPLINE_MESH_BASE_HPP_

#include "cl_HMR_Mesh_Base.hpp" //HMR/src
#include "cl_HMR_Element.hpp" //HMR/src
#include "cl_HMR_BSpline.hpp" //HMR/src
namespace moris
{
    namespace hmr
    {
//------------------------------------------------------------------------------

    /**
     * \brief Base class for Bspline Mesh
     */
        class BSpline_Mesh_Base : public Mesh_Base
        {

// ----------------------------------------------------------------------------
        public:
// ----------------------------------------------------------------------------

            bool
            test_sanity();

// ----------------------------------------------------------------------------
        protected:
// ----------------------------------------------------------------------------
            //! number of children per basis
            const uint mNumberOfChildrenPerBasis;

            //! max number of elements connected to bass
            const uint mNumberOfElementsPerBasis;

            //! Cell containing all nodes this proc knows about
            Cell< Basis* > mAllCoarsestBasisOnProc;

            //! number of basis used by this proc
            luint mNumberOfBasis = 0;

            //! number of elements used by this proc
            //luint mNumberOfElements = 0;

            //! number of all basis (including unused on padding)
            luint mNumberOfAllBasis = 0;

            //! number of basis on coarsest level
            luint mNumberOfCoarsestBasisOnProc[ 3 ] = { 0, 0, 0 };

            //! Lookup table containing offset for node IDs
            luint mBasisLevelOffset[ gMaxNumberOfLevels ];

            //! a container that remembers the number of basis per level
            luint mNumberOfBasisPerLevel[ gMaxNumberOfLevels ] = { 0 };

            //! counts the number of basis used and owned
            //luint mNumberOfOwnedBasis = 0;


            luint mNumberOfActiveBasisOnProc = 0;

            Cell< Basis* > mActiveBasisOnProc;

// ----------------------------------------------------------------------------
        public:
// ----------------------------------------------------------------------------

            /**
             * Default Mesh constructor
             *
             * @param[in] aParameters         container of user defined settings
             * @param[in] aBackgroundMesh   pointer to background mesh
             * @param[in] aOrder            polynomial degree of mesh
             */
            BSpline_Mesh_Base (
                    const Parameters       * aParameters,
                    Background_Mesh_Base   * aBackgroundMesh,
                    const uint             & aOrder,
                    const uint             & aActivationPattern );

// ----------------------------------------------------------------------------

            /**
             * Virtual destructor. Does nothing.
             */
            virtual ~BSpline_Mesh_Base(){};

// ----------------------------------------------------------------------------

            /**
             * This function is called by the constructor, but can also be called
             * after the B-Spline mesh is generated, and the background mesh is
             * refined.
             *
             * @return void
             *
             */
            void
            update_mesh();
// ----------------------------------------------------------------------------

            /**
             * Saves the basis to a VTK file. Useful for debugging.
             *
             * @param[in] string aFilePath file where mesh is to be stored
             *
             * @return void
             */
            void
            save_to_vtk( const std::string & aFilePath );

// ----------------------------------------------------------------------------

            /**
             * returns how many children a basis has
             */
            auto
            get_number_of_children_per_basis() const
            -> decltype ( mNumberOfChildrenPerBasis )
            {
                return mNumberOfChildrenPerBasis;
            }

// ----------------------------------------------------------------------------

            /**
             * returns an active basis by a position in the memory
             */
            Basis*
            get_active_basis( const luint& aIndex )
            {
                return mActiveBasisOnProc( aIndex );
            }

// ----------------------------------------------------------------------------

            /**
             * returns the number of active basis owned
             * and shared by current proc
             */
            auto
            get_number_of_active_basis_on_proc() const
            -> decltype ( mNumberOfActiveBasisOnProc )
            {
                return mNumberOfActiveBasisOnProc;
            }

// ----------------------------------------------------------------------------

            /*
             * A function that tests if each basis is uniquely generated.
             * Returns false otherwise.
             *
             * @return bool
             */
            bool
            test_for_double_basis();

// ----------------------------------------------------------------------------

            /**
             * recalculates the domain indices based on flagged basis
             */
            void
            calculate_basis_indices( const Matrix< IdMat > & aCommTable );

// ----------------------------------------------------------------------------
        protected:
// ----------------------------------------------------------------------------

            /**
             * creates a basis depending on polynomial order and dimension
             *
             * @param[in]   aIJK        ijk position of node
             * @param[in]   aLevel      level on which basis exists
             * @param[in]   aOwner      owner of basis
             */
            virtual Basis*
            create_basis(
                    const luint * aIJK,
                    const  uint & aLevel,
                    const  uint & aOwner ) = 0;

// ----------------------------------------------------------------------------
            /**
             * Returns the pointer to a basis on the coarsest level. 2D case.
             */
            Basis*
            get_coarsest_basis_by_ij(
                    const luint & aI,
                    const luint & aJ );

// ----------------------------------------------------------------------------

            /**
             * Returns the pointer to a basis on the coarsest level. 3D case.
             */
            Basis*
            get_coarsest_basis_by_ijk(
                const luint & aI,
                const luint & aJ,
                const luint & aK );

// ----------------------------------------------------------------------------

            /**
             * calculates domain wide unique node ID (1D case)
             * Useful for debugging.
             *
             * @param[in]  aLevel    level of node
             * @param[in]  aI        proc local i-position of node
             *
             * @return uint          domain wide unique ID
             */
            virtual luint
            calculate_basis_id(
                    const uint  & aLevel,
                    const luint & aI ) = 0;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            /**
             * calculates domain wide unique node ID (2D case)
             * Useful for debugging.
             *
             * @param[in]  aLevel    level of node
             * @param[in]  aI        proc local i-position of node
             * @param[in]  aJ        proc local j-position of node
             * @return uint          domain wide unique ID
             */
            virtual luint
            calculate_basis_id(
                    const uint  & aLevel,
                    const luint & aI,
                    const luint & aJ ) = 0;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            /**
             * calculates domain wide unique node ID (3D case)
             * Useful for debugging.
             *
             * @param[in]  aLevel    level of node
             * @param[in]  aI        proc local i-position of node
             * @param[in]  aJ        proc local j-position of node
             * @param[in]  aK        proc local k-position of node
             * @return uint          domain wide unique ID
             */
            virtual luint
            calculate_basis_id(
                    const uint  & aLevel,
                    const luint & aI,
                    const luint & aJ,
                    const luint & aK ) = 0;

// ----------------------------------------------------------------------------

            /**
             * makes sure that if a basis is flagged, it is also flagged
             * on any other proc
             */
            void
            synchronize_flags( const Matrix< IdMat > & aCommTable );

// ----------------------------------------------------------------------------
        private:
// ----------------------------------------------------------------------------

            /**
             * creates the lookup table needed for basis IDs
             */

            void
            calculate_basis_level_offset();

// ----------------------------------------------------------------------------
            /**
             * creates B-Splines for this mesh
             *
             * @return void
             */
            void
            create_basis();

// ----------------------------------------------------------------------------

            /**
             * creates basis on coarsest level
             *
             * @return void
             */
            void
            create_basis_on_level_zero();

// ----------------------------------------------------------------------------

            /**
             * tells elements on coarsest level which basis they have
             *
             * @return void
             */
            void
            link_basis_to_elements_on_level_zero();

// ----------------------------------------------------------------------------

            /**
             * calculates XZY coordinates for each basis
             *
             * @return void
             */
            virtual void
            calculate_basis_coordinates() = 0;

// ----------------------------------------------------------------------------

            /**
             * Loops over all elements and stores basis in
             * mAllBasisOnProc
             */
            void
            collect_basis();

// ----------------------------------------------------------------------------

            /**
             * Provides a Cell of basis that live on a specified level
             *
             * @param[ in    ]  aLevel   level to be investigated
             * @param[ inout ]  aBasis   cell containing found basis
             */
            void
            collect_basis_from_level(
                            const uint     & aLevel,
                            Cell< Basis* > & aBasis );

// ----------------------------------------------------------------------------

            void
            process_level( const uint & aLevel );

// ----------------------------------------------------------------------------

            /**
             * Provides a cell of all basis on current level.
             * Also:
             *     - resets general purpose flahgs
             *     - determine if basis is used by this proc
             *     - creates basis to element connectivity
             *     - determines basis ownership
             *     - determines basis neighbors for relevant basis
             */
            void
            preprocess_basis_from_level(
                    const uint                       & aLevel,
                    Cell< Background_Element_Base* > & aBackgroundElements,
                    Cell< Basis* >                   & aBasis);

// ----------------------------------------------------------------------------

            /**
             * identifies basis that are flagged for refinement
             */
            void
            determine_basis_state( Cell< Basis* > & aBasis );

// ----------------------------------------------------------------------------

            /**
             * Links B-Splines to parents. Needed for testing.
             */
            void
            link_basis_to_parents();

// ----------------------------------------------------------------------------

            /*void
            use_only_basis_in_frame(); */

// ----------------------------------------------------------------------------

            void
            calculate_basis_ids();

// ----------------------------------------------------------------------------
            //void
            //synchronize_active_basis_in_aura();

            //void
            //synchronize_refined_basis_in_aura();

// ----------------------------------------------------------------------------

            void
            collect_active_basis();

// ----------------------------------------------------------------------------

            void
            delete_unused_basis(
                    const uint                       & aLevel,
                    Cell< Background_Element_Base* > & aBackgroundElements,
                    Cell< Basis* >                   & aBasis );

        };
//------------------------------------------------------------------------------
    } /* namespace hmr */
} /* namespace moris */

#endif /* SRC_HMR_CL_HMR_BSPLINE_MESH_BASE_HPP_ */
