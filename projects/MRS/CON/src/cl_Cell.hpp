#ifndef MORIS_CONTAINERS_CL_CELL_HPP_
#define MORIS_CONTAINERS_CL_CELL_HPP_

// C++ header files.
#include <vector>
#include <algorithm> // for unique
#include <iostream>

// MORIS library header files.
#include "typedefs.hpp" // COR/src
#include "assert.hpp"

namespace moris
{
    //------------------------------------------------------------------
    template< typename T >
    class Cell
    {
        private:

            /**
             * MORIS cell
             */
            std::vector< T > mCell;

        public:

            /**
             * moris::Cell constructor
             */
            Cell() = default;

            //------------------------------------------------------------------
            /**
             * moris::Cell constructor
             *
             * @param[in] aCell A Cell
             */

            Cell(
                    std::initializer_list< T > const & aCell )
            : mCell( aCell.begin(), aCell.end() )
            {
            }

            //------------------------------------------------------------------

            /**
             * moris::Cell constructor
             *
             * @param[in] aSize Size of the Cell to be initialized
             * @param[in] aValue Value of the elements of the initialized Cell
             */

            template< typename A >
            Cell(
                    moris::uint const aSize,
                    A           const aValue )
            : mCell( aSize, aValue )
            {
                MORIS_CHECK_MEMORY(sizeof(T)*aSize < MORIS_MAX_CELL_CAPACITY,
                         "Cell::Cell: Maximum allowable capacity exceeded: %f MB.\n",sizeof(T)*aSize/1e6);
            }

            //------------------------------------------------------------------

            Cell(
                    moris::uint const aSize )
            : mCell( aSize )
            {
                MORIS_CHECK_MEMORY(sizeof(T)*aSize < MORIS_MAX_CELL_CAPACITY,
                         "Cell::Cell: Maximum allowable capacity exceeded: %f MB.\n",sizeof(T)*aSize/1e6);
            }

            //------------------------------------------------------------------

            /**
             * moris::Cell destructor
             */

            ~Cell() = default; // 'default' tells the compiler to automatically
            // delete the underlying Cell

            //------------------------------------------------------------------

            /**
             * @brief Operator to replace the contents of the container
             *
             * @param[in] val Contents to be copied into the container
             */

            const moris::Cell< T > &
            operator=(moris::Cell<T> const & val)
            {
                mCell = val.data();

                return *this;
            }

            //------------------------------------------------------------------

            /**
             * @brief Operator to replace the contents of the container
             *
             * @param[in] val Replacement Cell
             *
             * @return Assignment.
             */

            template< typename A >
            const moris::Cell< T > &
            operator=(
                    const A & val )
            {
                mCell = val;

                return *this;
            }

            //------------------------------------------------------------------

            /**
             * Replaces the contents of the container
             *
             * @param[in] aCount number of entries
             * @param[in] aval Value to set as contents of container
             */

            void
            assign(
                    moris::size_t const aCount,
                    T             const & aval)
            {
                mCell.assign( aCount, aval );
            }

            //------------------------------------------------------------------

            /**
             * @brief Operator for accessing a specified Cell element.
             *
             * @param[in] i_index Position of an element in the Cell.The function
             *            at() automatically checks whether an element is within the
             *            bounds of valid elements in the container, throwing an
             *            out_of_range exception if it is not. This is in contrast
             *            with member operator[], that does not check against bounds
             *            and hence is faster.
             */

            auto
            operator()(
                    moris::size_t const i_index )
            -> decltype(
#ifdef DEBUG
                    ( mCell.at( i_index ) )
#else
                    ( mCell[ i_index ] )
#endif
            )
            {
#ifdef DEBUG
                return( mCell.at( i_index ) );
#else
                return( mCell[ i_index ] );
#endif
            }

            //------------------------------------------------------------------

            /**
             * const version of above
             */

            auto
            operator()(
                    moris::size_t const i_index ) const
            -> decltype(
#ifdef DEBUG
                    ( mCell.at( i_index ) )
#else
                    ( mCell[ i_index ] )
#endif
            )
            {
#ifdef DEBUG
                return( mCell.at( i_index ) );
#else
                return( mCell[ i_index ] );
#endif
            }

            //------------------------------------------------------------------

            /**
             * Direct access to the underlying array.
             *
             * @return Direct access to the underlying array.
             */

            std::vector< T > const &
            data() const
            {
                return mCell;
            }

            std::vector< T > &
            data()
            {
                return mCell;
            }

            //------------------------------------------------------------------

            /**
             * Direct access to the memory of the underlying array.
             *
             * @return Pointer to memory of the underlying array.
             */

            T*
            memptr() const
            {
                return mCell.data();
            }

            T*
            memptr()
            {
                return mCell.data();
            }

            //------------------------------------------------------------------

            /**
             * Checks whether the Cell container is empty.
             *
             * @return bool value indicating whether the Cell container is empty.
             */

            bool
            empty()
            {
                return mCell.empty();
            }

            //------------------------------------------------------------------

            /**
             * Returns the last element of the Cell.
             *
             * @return the last element of the Cell.
             */

            auto
            back()
            ->decltype( mCell.back() )
            {
                return mCell.back();
            }

            //------------------------------------------------------------------

            /**
             * Returns the number of elements in the Cell.
             *
             * @return Number of elements in the Cell.
             */

            auto
            size()
            ->decltype (mCell.size() )
            {
                return mCell.size();
            }

            //------------------------------------------------------------------

            /**
             * const version of above
             */

            auto
            size() const
            ->decltype (mCell.size() )
            {
                return mCell.size();
            }

            //------------------------------------------------------------------

            /**
             * Returns the maximum possible number of elements in the Cell.
             *
             * @return Maximum possible number of elements in the Cell.
             */

            auto
            max_size()
            ->decltype (mCell.max_size() )
            {
                return mCell.max_size();
            }

            //------------------------------------------------------------------

            /**
             * @brief Increase the capacity of the container to a value that's
             *        greater or equal to new_cap
             *
             * @param[in] new_cap The new capacity of the Cell
             */

            void
            reserve( moris::size_t const & new_cap )
            {
                MORIS_CHECK_MEMORY(sizeof(T)*new_cap < MORIS_MAX_CELL_CAPACITY,
                         "Cell::reserve: Maximum allowable capacity exceeded: %f MB.\n",sizeof(T)*new_cap/1e6);

               mCell.reserve( new_cap );
            }

            //------------------------------------------------------------------

            /**
             * @brief Returns the number elements that can be held in currently
             *        allocated storage.
             *
             * @return Number elements that can be held in currently allocated storage.
             */

            auto
            capacity() const
            ->decltype (mCell.capacity() )
            {
                return mCell.capacity();
            }


            //------------------------------------------------------------------

            /**
             * @brief Requests the removal of unused capacity
             */
            void
            shrink_to_fit()
            {
                // check that resize on large cells does not indicate overly conservative initial size allocation
                MORIS_CHECK_MEMORY(
                        sizeof(T)*this->capacity() > MORIS_CELL_RESIZE_CHECK_LIMIT * MORIS_MAX_CELL_CAPACITY ?
                                this->size() > MORIS_CELL_RESIZE_FRACTION_LIMIT * this->capacity() : true,
                                "Cell::shrink_to_fit: Shrink to less than 1 percent of large matrix - reduce initial allocation\n");

                mCell.shrink_to_fit();
            }

            //------------------------------------------------------------------

            /**
             * Clears the contents of the Cell.
             */
            void
            clear()
            {
                mCell.clear();
            }

            //------------------------------------------------------------------

            /**
             * @brief Inserts elements at the specified location in the container
             *
             * @param[in] pos Iterator at which the content will be inserted.
             * @param[in] value Element value to insert
             */

            void
            insert(
                    moris::size_t const & pos,
                    T             const & value)
            {
                mCell.insert( mCell.begin() + pos, value );
            }

            //------------------------------------------------------------------

            /**
             * Appends a moris::Cell, similar to how push_back appends a single value.
             *
             * @param[in] aCell Cell to be appended
             */

            void
            append(
                    moris::Cell < T > const & aCell )
            {
                mCell.insert( mCell.end(), aCell.data().begin(), aCell.data().end() );
            }

            //------------------------------------------------------------------

            /**
             * @brief Removes specified element from the container
             *
             * @param[in] pos Position (not index) of element to be remove
             */

            void
            erase(
                    moris::size_t const & pos )
            {
                mCell.erase( mCell.begin() + pos );
            }

            //------------------------------------------------------------------

            /**
             * @brief Removes specified element from the container lying in the range
             *        [ pos1, pos2 ).
             *
             * @param[in] pos1 Position(not index) of first element to be removed
             * @param[in] pos2 Position(not index) of last element to be removed + 1
             *
             */

            void
            erase_range(
                    moris::size_t const & pos1,
                    moris::size_t const & pos2 )
            {
                mCell.erase( mCell.begin() + pos1, mCell.begin() + pos2 );
            }

            //------------------------------------------------------------------

            /**
             * @brief Appends the given element value to the end of the container
             *
             * @param[in] value The value of the element to append
             */

            void
            push_back(
                    T const & value )
            {
                mCell.push_back( value );
            }

            //------------------------------------------------------------------

            /**
             * @brief removes the last element of the container
             */

            void
            pop_back()
            {
                mCell.pop_back();
            }

            //------------------------------------------------------------------

            /**
             * @brief Resizes the container to contain count elements
             *
             * @param[in] aCount The new size of the Cell
             * @param[in] aValue The value to initialize the new elements with
             */

            void
            resize(
                    moris::size_t const & aCount,
                    T                     aValue = T() )
            {
                MORIS_CHECK_MEMORY(sizeof(T)*aCount < MORIS_MAX_CELL_CAPACITY,
                         "Cell::resize: Maximum allowable capacity exceeded: %f MB.\n",sizeof(T)*aCount/1e6);

                // check that resize on large cells does not indicate overly conservative initial size allocation
                MORIS_CHECK_MEMORY(
                        sizeof(T)*this->capacity() > MORIS_CELL_RESIZE_CHECK_LIMIT * MORIS_MAX_CELL_CAPACITY ?
                                aCount > MORIS_CELL_RESIZE_FRACTION_LIMIT * this->capacity() : true,
                                "Cell::resize: Resize to less than 1 percent of large matrix - reduce initial allocation");

                mCell.resize( aCount, aValue );
            }

            //------------------------------------------------------------------

            /**
             * @brief Returns an iterator to the first element
             */
            auto
            begin()
            -> decltype( mCell.begin() )
            {
                return mCell.begin();
            }

            //------------------------------------------------------------------

            /**
             * @brief Returns an iterator to the first element
             */

            auto
            end()
            -> decltype( mCell.end() )
            {
                return mCell.end();
            }

            //------------------------------------------------------------------

            /**
             * @brief Returns a const iterator to the first element
             */

            auto
            cbegin() const
            -> decltype( mCell.cbegin() )
            {
                return mCell.cbegin();
            }

            //------------------------------------------------------------------

            /**
             * @brief Returns an iterator to the first element
             */

            auto
            cend() const
            -> decltype( mCell.cend() )
            {
                return mCell.cend();
            }
    };

    //------------------------------------------------------------------

    // Free functions
    template< typename T >
    void
    unique( Cell< T > & aCell )
    {
        // get ref to data
        std::vector< T > & tVec = aCell.data();

        // sort data
        std::sort( tVec.begin(), tVec.end() );

        // trim vector
        tVec.erase( std::unique( tVec.begin(), tVec.end() ), tVec.end() );
    }

    //------------------------------------------------------------------

    //https://stackoverflow.com/questions/25921706/creating-a-vector-of-indices-of-a-sorted-vector
    // extended to create a unique with the first index of unique values
    template< typename T >
    Cell<moris::moris_index>
    unique_index( Cell< T > & aCell )
    {
        std::vector<T> x = aCell.data();

        std::vector<int> y(x.size());
        std::size_t n(0);
        std::generate(std::begin(y), std::end(y), [&]{ return n++; });

        std::sort(  std::begin(y),
                std::end(y),
                [&](int i1, int i2) { return x[i1] < x[i2]; } );

        Cell<moris::moris_index> tUniqueInds;
        for(moris::uint  i = 0; i < aCell.size(); i++)
        {
            if(i == 0)
            {
                tUniqueInds.push_back( y[i] );
            }

            else if(aCell(y[i-1]) != aCell(y[i]) )
            {
                tUniqueInds.push_back( y[i] );
            }
        }

        return tUniqueInds;

    }

    //------------------------------------------------------------------

    /*!
     * Iterates through cell and prints each cell.
     * Will only work on data types that allow std::cout calls
     */
    template< typename T >
    void
    print(
            Cell< T > const & aCell,
            std::string aStr = "Cell")
    {
        std::cout<<"Cell Name: "<<aStr<<"\n";
        std::cout<<"Number of entries = "<<aCell.size()<<"\n";
        for(moris::uint  i = 0; i <aCell.size(); i++)
        {
            std::cout<<aCell(i)<<"\n";
        }

        std::cout<<std::endl;
    }

    //------------------------------------------------------------------

    inline
    moris::Cell<char>
    string_to_char(moris::Cell<std::string>& strings);

}

#endif /* MORIS_CONTAINERS_CL_Cell_HPP_ */
