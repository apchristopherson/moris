/*
 * cl_HMR_BSpline_Element.hpp
 *
 *  Created on: Jun 12, 2018
 *      Author: messe
 */

#ifndef SRC_HMR_CL_HMR_BSPLINE_ELEMENT_HPP_
#define SRC_HMR_CL_HMR_BSPLINE_ELEMENT_HPP_


#include "typedefs.hpp" //COR/src
#include "cl_HMR_Background_Element_Base.hpp" //HMR/src
#include "cl_HMR_Element.hpp" //HMR/src
#include "cl_HMR_BSpline.hpp" //HMR/src


namespace moris
{
    namespace hmr
    {
//------------------------------------------------------------------------------

        /**
         * \brief BSplinee Element templated against
         *
         * uint N: number of dimensions (1, 2, or 3)
         * uint B: number of basis
         */
        template< uint N, uint B >
        class BSpline_Element : public Element
        {
            //! pointer to nodes
            Basis*     mBasis[ B ] = { nullptr };

// -----------------------------------------------------------------------------
            public:
// -----------------------------------------------------------------------------

            /**
             * default Lagrange Element constructor
             */
            BSpline_Element( Background_Element_Base* aElement) :
                Element( aElement )

            {

            }

//------------------------------------------------------------------------------

            /**
             * default destructor
             */
            ~BSpline_Element()
            {
            }

//------------------------------------------------------------------------------

            /**
             * get pointer to basis
             *
             * @param[in]    aIndex   element local index of basis
             *
             * @return Basis* pointer to Lagrange node
             *
             */
            Basis*
            get_basis( const uint& aIndex )
            {
                return mBasis[ aIndex ];
            }

//------------------------------------------------------------------------------

            /**
             * set pointer of basis to specified index and object
             *
             * @param[in]    aIndex  element local index of node
             * @param[in]    aBasis  pointer to Lagrange node
             *
             * @return void
             *
             */
            void
            insert_basis(
                    const  uint  & aIndex,
                    Basis * aBasis )
            {
                mBasis[ aIndex ] = aBasis;
            }

// -----------------------------------------------------------------------------

            /**
             * Creates all basis on the coarsest level.
             * Called by Bspline mesh create_basis_on_level_zero().
             *
             * @param[inout] aAllElementsOnProc   cell containing all Bspline
             *                                    elements including the aura
             * @param[inout] aBasisCounter        counter to keep track of
             *                                    how many basis were generated
             * @return void
             */
            void
            create_basis_on_level_zero(
                    moris::Cell< Element * > & aAllElementsOnProc,
                    luint                    & aBasisCounter );
//------------------------------------------------------------------------------

            /**
             * Creates nodes for children of refined elements.
             * Called by B-Spline mesh.
             *
             * @param[inout] aAllElementsOnProc   cell containing all Lagrange
             *                                    elements including the aura
             * @param[inout] aBasisCounter        counter to keep track of
             *                                    how many nodes were generated
             * @return void
             */
            void
            create_basis_for_children(
                    moris::Cell< Element * > & aAllElementsOnProc,
                    luint                    & aBasisCounter );

// -----------------------------------------------------------------------------

            /**
             * for debugging
             *
             * @return void
             */
            void
            print_connectivity()
            {
                std::fprintf( stdout,
                        "connectivity of element %4lu ( ID %4lu, parent %4lu ):\n",
                        ( long unsigned int ) mElement->get_domain_index(),
                        ( long unsigned int ) mElement->get_domain_id(),
                        ( long unsigned int ) mElement->get_parent()->get_domain_id() );
                for( uint k=0; k<B; ++k )
                {
                    // get node
                    Basis* tNode = this->get_basis( k );
                    std::fprintf( stdout,
                            "    %2u :  Basis %lu , ID %lu, MEM %lu \n",
                            ( unsigned int ) k,
                            ( long unsigned int ) tNode->get_domain_index(),
                            ( long unsigned int ) tNode->get_domain_id(),
                            ( long unsigned int ) tNode->get_memory_index());
                }
                std::fprintf( stdout, "\n" );
            }

//------------------------------------------------------------------------------

            /**
             * string needed for gmsh output
             *
             * @return std::string
             *
             */
            std::string
            get_gmsh_string();

//------------------------------------------------------------------------------

            /**
             * VTK ID needed for VTK output
             *
             * @return uint
             */
            uint
            get_vtk_type();

//------------------------------------------------------------------------------

            /**
             * node IDs needed for VTK output
             *
             * @param[out] moris::Mat<luint>
             *
             * @return void
             *
             */
            void
            get_basis_indices_for_vtk( Mat<luint> & aNodes );

//------------------------------------------------------------------------------

            /**
             * returns the ijk position of a given basis
             *
             * @param[in]  aBasisNumber   element local number of basis
             * @param[out] aIJK           proc local ijk position of this basis
             *
             * @return void
             *
             */
            void
            get_ijk_of_basis(
                    const uint & aBasisNumber,
                    luint      * aIJK );

//------------------------------------------------------------------------------

            /**
             * Links each basis of an element with neighbor basis.
             *
             * @param[inout] aAllElementsOnProc   cell containing all Bspline
             *                                    elements including the aura
             * @return void
             */
            void
            link_basis_with_neighbors(
                    moris::Cell< Element* > & aAllElementsOnProc );

//------------------------------------------------------------------------------

            /**
             * Refines the element.
             */
            void
            refine( moris::Cell< Element* > & aAllElementsOnProc,
                    luint            & aBasisCounter);

//------------------------------------------------------------------------------

            /**
             * Returns the geometry type of the element
             */
            mtk::Geometry_Type
            get_geometry_type() const ;

//------------------------------------------------------------------------------
            protected:
//------------------------------------------------------------------------------

            /**
             * create new node at position
             *
             * @param[in]    aBasisNumber   element local index of new basis
             *
             * @return void
             */
            void
            create_basis( const uint & aBasisNumber );

//------------------------------------------------------------------------------

            /**
             * refines the basis of an element
             *
             * @param[in] aIndex    Index of basis to  refine
             *
             * @return void
             */
            void
            refine_basis( const uint & aBasisNumber,
                          luint      & aBasisCounter );
//------------------------------------------------------------------------------

            /**
             * returns the interpolation order of this element
             */
            mtk::Interpolation_Order
            get_interpolation_order() const;

//------------------------------------------------------------------------------

            /**
             * returns a Mat with the basis coords
             */
            Mat< real >
            get_vertex_coords() const ;

//------------------------------------------------------------------------------
        };
//------------------------------------------------------------------------------

        template< uint N, uint B >
        std::string
        BSpline_Element< N, B >::get_gmsh_string()
        {
            std::string aString = "GMSH not implemented for this element";
            return aString;
        }

//------------------------------------------------------------------------------

        template< uint N, uint B >
        uint
        BSpline_Element< N, B >::get_vtk_type()
        {
            // this element has no VTK id
            return 0;
        }

//------------------------------------------------------------------------------

        template< uint N, uint B >
        void
        BSpline_Element< N, B >::get_basis_indices_for_vtk( Mat<luint> & aBasis )
        {
            // do nothing
        }


//------------------------------------------------------------------------------
        template< uint N, uint B >
        void
        BSpline_Element< N, B >::create_basis(
                const uint & aBasisNumber )
        {
            MORIS_ERROR( false, "Don't know how to create B-Spline.");
        }

//------------------------------------------------------------------------------

        template< uint N, uint B >
        void
        BSpline_Element< N, B >::create_basis_on_level_zero(
                moris::Cell< Element* > & aAllElementsOnProc,
                luint            & aBasisCounter )
        {
            MORIS_ERROR( false, "Don't know how to create B-Splines on level zero.");
        }

//------------------------------------------------------------------------------


        template< uint N, uint B >
        void
        BSpline_Element< N, B >::create_basis_for_children(
                moris::Cell< Element * > & aAllElementsOnProc,
                luint             & aBasisCounter )
        {
            MORIS_ERROR( false, "Don't know how to create B-Splines for children.");
        }

//------------------------------------------------------------------------------

        template< uint N, uint B >
        void
        BSpline_Element< N, B >::get_ijk_of_basis(
                const uint & aBasisNumber,
                luint      * aIJK )
        {
            MORIS_ERROR( false, "Don't know how to get ijk of basis.");
        }

//------------------------------------------------------------------------------

        template< uint N, uint B >
        void
        BSpline_Element< N, B >::link_basis_with_neighbors(
                moris::Cell< Element* > & aAllElementsOnProc )
        {
            MORIS_ERROR( false,
                    "Link basis with neighbors not available for this element.");
        }

//------------------------------------------------------------------------------

        template< uint N, uint B >
        void
        BSpline_Element< N, B >::refine_basis(
                const uint & aBasisNumber,
                     luint & aBasisCounter)
        {
            MORIS_ERROR( false, "refine_basis() not available for this element.");
        }

//------------------------------------------------------------------------------

        template< uint N, uint B >
        void
        BSpline_Element< N, B >::refine(
                moris::Cell< Element* > & aAllElementsOnProc,
                           luint & aBasisCounter )
        {
            MORIS_ERROR( false, "refine() not available for this element.");
        }

//------------------------------------------------------------------------------

        template< uint N, uint B >
        mtk::Geometry_Type
        BSpline_Element< N, B >::get_geometry_type() const
        {
            MORIS_ERROR( false, "get_geometry_type() not available for this element.");
            return mtk::Geometry_Type::UNDEFINED;
        }

//------------------------------------------------------------------------------

        template< uint N, uint B >
        mtk::Interpolation_Order
        BSpline_Element< N, B >::get_interpolation_order() const
        {
            MORIS_ERROR( false, "get_interpolation_order() not available for this element.");
            return mtk::Interpolation_Order::UNDEFINED;
        }

//------------------------------------------------------------------------------

        template< uint N, uint B >
        Mat< real >
        BSpline_Element< N, B >::get_vertex_coords() const
        {
            Mat<real> aCoords( B, N );
            for( uint k=0; k<B; ++k )
            {
                const real * tXYZ = mBasis[ k ]->get_xyz();

                for( uint i=0; i<N; ++i )
                {
                    aCoords( k, i ) = tXYZ[ i ];
                }
            }
            return aCoords;
        }

//------------------------------------------------------------------------------

    } /* namespace hmr */
} /* namespace moris */

#endif /* SRC_HMR_CL_HMR_BSPLINE_ELEMENT_HPP_ */
