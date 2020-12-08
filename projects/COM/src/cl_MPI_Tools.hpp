/*
 * cl_MPI_Tools.hpp
 *
 *  Created on: Jun 27, 2017
 *      Author: ktdoble
 */

#ifndef SRC_TOOLS_CL_MPI_TOOLS_HPP_
#define SRC_TOOLS_CL_MPI_TOOLS_HPP_

#include <stdlib.h>
#include <mpi.h>

#include "typedefs.hpp"
#include "cl_Cell.hpp"

#include "cl_Matrix.hpp"

#include "cl_Communication_Tools.hpp"

namespace moris
{
    /* ---------------------------------------------------------------------
     * Gather cells of type T of all processors on root processor (default: 0)
     * Note: cells need to have same size
     *
     * @param aBuffer input cell
     * @param aResult output cell (only valid on root processor)
     */
    template <typename T>
    inline
    void gather(
            moris::Cell<T> & aBuffer,
            moris::Cell<T> & aResult,
            int              aRootProc = 0)
    {
        // get processor rank and size
        int tProcRank = 0;
        int tProcSize = 0;

        MPI_Comm_rank(moris::get_comm(), &tProcRank);
        MPI_Comm_size(moris::get_comm(), &tProcSize);

        // get data type of cell
        MPI_Datatype tType = get_comm_datatype ( ( T ) 0 );

        // allocate proper size for root processor
        int tSizeofBuffer = aBuffer.size();
        if (tProcRank == aRootProc)
        {
            aResult.resize(tProcSize * tSizeofBuffer, 0);
        }

        // check for equal size of cells across all processors
        MORIS_ASSERT( max_all( tSizeofBuffer ) == tSizeofBuffer,
                "gather - cell size differs across processors.\n");

        // mpi call
        MPI_Gather(
                aBuffer.data().data(),
                tSizeofBuffer,
                tType,
                aResult.data().data(),
                tSizeofBuffer,
                tType,
                aRootProc,
                moris::get_comm());
    }

    /* ---------------------------------------------------------------------
     * Scatter cells of type T form root processor  (default: 0) to all processors
     * Note: cells need to have same size
     *
     * @param aBuffer input cell (only valid on root processor)
     * @param aResult output cell (needs to be size correctly externally)
     */
    template <typename T>
    inline
    void scatter(
            moris::Cell<T> & aBuffer,
            moris::Cell<T> & aResult,
            int              aRootProc = 0)
    {
        // get processor rank and size
        int tProcRank = 0;
        int tProcSize = 0;

        MPI_Comm_rank(moris::get_comm(), &tProcRank);
        MPI_Comm_size(moris::get_comm(), &tProcSize);

        // get data type of cell
        MPI_Datatype tType = get_comm_datatype ( ( T ) 0 );

        // get size of receive buffer (needs to be set externally)
        int tSizeofBuffer = aResult.size();

        // check for equal size of cells across all processors
        MORIS_ASSERT( max_all( tSizeofBuffer ) == tSizeofBuffer,
                "scatter - cell size differs across processors.\n");

        MPI_Scatter(
                aBuffer.data().data(),
                tSizeofBuffer,
                tType,
                aResult.data().data(),
                tSizeofBuffer,
                tType,
                aRootProc,
                moris::get_comm());
    }

    // ---------------------------------------------------------------------
    // FIXME: the following routine need proper commenting and error checking
    // ---------------------------------------------------------------------

    /**
     * Requirement: Column major matrix
     */
    template <typename Size_T_Matrix>
    MPI_Request nonblocking_send(moris::Matrix<Size_T_Matrix> const & aSendingMatrix,
            size_t aNumRows,
            size_t aNumColumns,
            int aReceivingProc,
            int aTag)
    {
        int tNumToSend = aSendingMatrix.numel();
        MPI_Request tRequest;

        //    std::cout<<"NONBLOCKING SEND FROM "<<par_rank()<<" TO "<<aReceivingProc<<" WITH TAG "<<aTag<<" | Size: "<<aNumRows<<", "<<aNumColumns<<std::endl;

        MPI_Isend(aSendingMatrix.data(), tNumToSend, moris::get_comm_datatype(aSendingMatrix(0,0)), aReceivingProc, aTag, moris::get_comm(),&tRequest);

        return tRequest;

    }

    inline
    bool
    sent_message_exists(int aOtherProc,
            int aTag,
            MPI_Status & aStatus)
    {

        int flag = 1000;
        MPI_Iprobe(aOtherProc, aTag, moris::get_comm(), &flag, &aStatus);

        //    std::cout<<"Flag = "<<flag<< " | aOtherProc = "<<aOtherProc<<"| my rank = "<<moris::par_rank()<<" | Tag = "<<aTag<<std::endl;

        if(flag)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    template <typename Size_T_Matrix>
    inline
    void receive(moris::Matrix<Size_T_Matrix> & aReceivingMatrix,
            size_t aNumRows,
            int aSendingProc,
            int aTag)
    {

        MPI_Status tStatus;
        MPI_Probe(aSendingProc, aTag, moris::get_comm(), &tStatus);

        //    MORIS_ERROR(tExists,"Trying to receive a message that does not exists");

        int tNumSent = 0;
        MPI_Get_count(&tStatus, moris::get_comm_datatype(aReceivingMatrix(0,0)), &tNumSent);


        size_t tNumColumns = tNumSent / aNumRows;

        //    std::cout<<"RECEIVE FROM FROM "<<aSendingProc<<" ON "<<par_rank()<<" WITH TAG "<<aTag<<" | Size: "<<aNumRows<<", "<<tNumColumns<<std::endl;

        // Resize the matrix
        aReceivingMatrix.resize(aNumRows, tNumColumns);

        MPI_Recv(aReceivingMatrix.data(), tNumSent, moris::get_comm_datatype(aReceivingMatrix(0,0)), aSendingProc, aTag, moris::get_comm(), &tStatus);

    }

    template <typename Size_T_Matrix>
    inline
    void receive_col_known(moris::Matrix<Size_T_Matrix> & aReceivingMatrix,
            size_t aNumCols,
            int aSendingProc,
            int aTag)
    {
        MPI_Status tStatus;
        bool tExists = sent_message_exists(aSendingProc,aTag,tStatus);

        MORIS_ERROR(tExists,"Trying to receive a message that does not exists");

        int tNumSent = 0;
        MPI_Get_count(&tStatus, moris::get_comm_datatype(aReceivingMatrix(0,0)), &tNumSent);

        size_t tNumRows = tNumSent / aNumCols;

        // Resize the matrix
        aReceivingMatrix.resize(tNumRows, aNumCols);

        MPI_Recv(aReceivingMatrix.data(), tNumSent, moris::get_comm_datatype(aReceivingMatrix(0,0)), aSendingProc, aTag, moris::get_comm(), &tStatus);
    }
}

#endif /* SRC_TOOLS_CL_MPI_TOOLS_HPP_ */
