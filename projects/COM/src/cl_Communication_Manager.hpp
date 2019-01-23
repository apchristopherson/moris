/*
 * cl_MPI_Comm_Manager.hpp
 *
 *  Created on: Mar 21, 2018
 *      Author: doble
 */

#ifndef SRC_MPI_CL_COMMUNICATION_MANAGER_HPP_
#define SRC_MPI_CL_COMMUNICATION_MANAGER_HPP_

#include <mpi.h>
#include <string>
#include "cl_Cell.hpp" // CON/src

#include "assert.hpp"

namespace moris
{
    class Comm_Manager
    {
    public:

        // Default constructor
        Comm_Manager(){};

        // Constructor which sets the communicator to default to MPI COMM WORLD
        Comm_Manager(int *argc,
                     char ***argv);

        /*
        * Constructor which sets the communictor to the provided aComm. aComm in this case
        * should be the broadest communicator for MORIS.
        *
        * @param[in] aComm - broadest communicator for MORIS
        */
        Comm_Manager(MPI_Comm & aComm);


        /*
         * Default Destructor
         */
        ~Comm_Manager();


        /*
         * Initialize the communicator
         */
        void
        initialize(int *argc,
                   char ***argv);

        /*
         * Get the global communicator, this may not be necessarily MPI_COMM_WORLD.
         */
        MPI_Comm
        get_global_comm();

        /*
         * Returns the communicator at the given index
         * @param[in] aCommIndex - A communicator index
         */
        MPI_Comm
        get_comm(size_t aCommIndex);


        /*
         * Add a communictor with its name to the moris communicator pool
         * @param[in]  aNewComm - New Communicator to add
         * @param[in]  aCommName - Name of this communicator
         * @param[out] Communicator index in mComm
         */
        size_t
        add_communicator(MPI_Comm & aNewComm,
                         std::string & aCommName);

        /*
         * Finalizes the mpi
         */
        void
        finalize();


        /*
         * return the path of the running executable
         */
        const std::string &
        get_exec_path();

    private:
        // A moris cell of communicators
        Cell<MPI_Comm> mComm;
        Cell<std::string> mCommName;

        // path to running executable
        std::string mMorisExec;
    };


}


#endif /* SRC_MPI_CL_COMMUNICATION_MANAGER_HPP_ */
