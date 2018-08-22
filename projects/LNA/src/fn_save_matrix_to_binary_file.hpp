/*
 * fn_save_matrix_to_binary_file.hpp
 *
 *  Created on: Jul 18, 2018
 *      Author: messe
 */

#ifndef SRC_LINALG_FN_SAVE_MATRIX_TO_BINARY_FILE_HPP_
#define SRC_LINALG_FN_SAVE_MATRIX_TO_BINARY_FILE_HPP_

#include <fstream>
#include <iostream>
#include <string>

// MORIS header files.
#include "typedefs.hpp" //MRS/COR/src
#include "cl_Mat.hpp" //LNA/src

namespace moris
{

//------------------------------------------------------------------------------

    /**
     * stores a matrix to a binary file
     * @param[ in ] aFilePath    string to file path
     * @param[ in ] aMatrix      matrix that is to be stored
     */
    template< typename T >
    void
    save_matrix_to_binary_file(
            const Mat< T >     & aMatrix,
            const std::string  & aFilePath )
    {

        // size of buffer in bit
        const uint tSizeOfBuffer = 512 * 1024 * 8;

        // samples in buffer to write
        const uint tNumberOfSamplesInBuffer = tSizeOfBuffer / sizeof( T );

        // get number of rows of input matrix
        uint tNumberOfRows = aMatrix.n_rows();

        // get number of cols of input matrix
        uint tNumberOfCols = aMatrix.n_cols();

        // output file object
        std::ofstream tFile;

        // open output file
        tFile.open( aFilePath, std::ios::binary );

        // throw error if file can not be opened
        if( ! tFile )
        {
            std::string tMessage = "Could not create file " + aFilePath + " ." ;
            throw std::runtime_error(tMessage);
        }

        // write number of rows to file
        tFile.write( ( char* ) &tNumberOfRows, sizeof( uint ) );

        // write number of cols to file
        tFile.write( ( char* ) &tNumberOfCols, sizeof( uint ) );

        // create buffer
        T tBuffer[tNumberOfSamplesInBuffer];

        // counter for buffer data
        uint tCount = 0;

        // loop over all columns
        for ( uint j=0; j<tNumberOfCols; ++j )
        {
            // loop over all rows
            for (uint i=0; i<tNumberOfRows; ++i )
            {
                // write data in buffer
                tBuffer[ tCount++ ] = aMatrix( i, j );

                // check if buffer is full
                if( tCount == tNumberOfSamplesInBuffer )
                {
                    // write buffer to output file
                    tFile.write( ( char* ) & tBuffer, tSizeOfBuffer );

                    // reset counter
                    tCount = 0;
                }
            }
        }

        // write last incomplete buffer to output file
        if ( tCount != 0 )
        {
            for ( uint k=0; k<tCount; ++k )
            {
                tFile.write( ( char* ) &tBuffer[ k ], sizeof( T ) );
            }
        }

        // close output file
        tFile.close();
    }

//------------------------------------------------------------------------------
} /* namespace moris */

#endif /* SRC_LINALG_FN_SAVE_MATRIX_TO_BINARY_FILE_HPP_ */
