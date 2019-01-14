/*
 * cl_HMR_File.hpp
 *
 *  Created on: Jun 29, 2018
 *      Author: messe
 */

//https://support.hdfgroup.org/HDF5/doc1.8/RM/RM_H5S.html

#ifndef SRC_HMR_CL_HMR_FILE_HPP_
#define SRC_HMR_CL_HMR_FILE_HPP_

#include <string>

#include "cl_HMR_Background_Mesh_Base.hpp" //HMR/src
#include "cl_HMR_Parameters.hpp" //HMR/src
// HD5 c-interface
#include "hdf5.h"

#include "typedefs.hpp" //COR/src
#include "cl_Matrix.hpp" //LINALG/src
#include "linalg_typedefs.hpp"
#include "fn_sum.hpp" //LINALG/src

namespace moris
{
    namespace hmr
    {

//-------------------------------------------------------------------------------
        class File
        {

            //! file ID for HDF5
            hid_t  mFileID;

            //! error handler
            herr_t mStatus;

//-------------------------------------------------------------------------------
        public :
//-------------------------------------------------------------------------------

            // default constructor
            File(){}

//-------------------------------------------------------------------------------
            // default destructor
            ~File(){}

//-------------------------------------------------------------------------------

            /** creates a new HDF5 file
             *
             * @param[ in ] aPath   path to file
             */
            void
            create( const std::string & aPath );

//-------------------------------------------------------------------------------

            /**
             * opens an existing HDF5 file
             *
             * @param[ in ] aPath   path to file
             */
            void
            open( const std::string & aPath );

//-------------------------------------------------------------------------------

            /**
             * stores the contents of aParameters into the file
             *
             * @param[ in ] aParameters   settings object
             */

            void
            save_settings( const Parameters * aParameters );

//-------------------------------------------------------------------------------

            /**
             * reads the settings from the file and writes them into the object
             */
            void
            load_settings( Parameters * aParameters );

//-------------------------------------------------------------------------------

            /**
             * stores the refinement pattern of the current proc into the file
             *
             * @param[ in ]  aMesh     pointer to background mesh
             */
            void
            save_refinement_pattern( Background_Mesh_Base * aMesh );

//-------------------------------------------------------------------------------

            /**
             * laods the refinement pattern of the current proc
             * and initializes a new mesh object
             *
             * @param[ inout ] aMesh           aMesh   pointer to background mesh
             * @param[ in ]    aMode           false: input, true: output
             *
             */
            void
            load_refinement_pattern(
                    Background_Mesh_Base * aMesh,
                    const bool             aMode );

//-------------------------------------------------------------------------------

            /**
             * closes the interface to the file
             */
            void
            close();

//-------------------------------------------------------------------------------
        private:
//-------------------------------------------------------------------------------

            /**
             * adds the proc number to the filename
             */
            std::string
            parralize_filename( const std::string & aPath );

//-------------------------------------------------------------------------------
        };

//-------------------------------------------------------------------------------

        /**
         * free function needed by loading constructor
         */
        Parameters *
        create_hmr_parameters_from_hdf5_file( const std::string & aPath );

//-------------------------------------------------------------------------------
    } /* namespace hmr */

} /* namespace moris */

#endif /* SRC_HMR_CL_HMR_FILE_HPP_ */
