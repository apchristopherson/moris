/*
 * cl_Ascii.hpp
 *
 *  Created on: Nov 10, 2020
 *      Author: schmidt
 */

#ifndef PROJECTS_MRS_IOS_SRC_HDF5_TOOLS_HPP_
#define PROJECTS_MRS_IOS_SRC_HDF5_TOOLS_HPP_

#include "typedefs.hpp"
#include "cl_Cell.hpp"

namespace moris
{
    //------------------------------------------------------------------------------

    enum class FileMode
    {
            NEW,
            OPEN_RDONLY,
            OPEN_RDWR
    };

    //------------------------------------------------------------------------------

    /**
     * this function tests if a file exists
     * @param aPath
     * @return
     */
    bool file_exists( const std::string & aPath );

    class Ascii
    {
            //------------------------------------------------------------------------------
        protected:
            //------------------------------------------------------------------------------

            std::string         mPath;
            const FileMode       mMode;
            Cell< std::string >       mBuffer;

            bool                 mChangedSinceLastSave = false;

            //------------------------------------------------------------------------------
        public:
            //------------------------------------------------------------------------------

            Ascii(
                    const std::string   & aPath,
                    const enum FileMode & aMode );

            //------------------------------------------------------------------------------

            ~Ascii();

            //------------------------------------------------------------------------------

            // save the buffer to the file
            bool save();

            //------------------------------------------------------------------------------

            void print( const std::string & aLine );

            //------------------------------------------------------------------------------

            /**
             * return the number of lines
             */
            moris::uint length() const;

            //------------------------------------------------------------------------------

            std::string & line( const moris::uint aLineNumber );

            //------------------------------------------------------------------------------

            const std::string & line( const moris::uint aLineNumber ) const;

            //------------------------------------------------------------------------------
        private:
            //------------------------------------------------------------------------------

            void load_buffer();

            //------------------------------------------------------------------------------
    };

    //------------------------------------------------------------------------------
}

#endif /* PROJECTS_MRS_IOS_SRC_HDF5_TOOLS_HPP_ */


