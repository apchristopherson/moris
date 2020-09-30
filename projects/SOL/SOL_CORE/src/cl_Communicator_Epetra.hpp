/*
 * cl_DLA_Communicator_Epetra.hpp
 *
 *  Created on: May 16, 2018
 *      Author: schmidt
 */
#ifndef SRC_DISTLINALG_CL_COMMUNICATOR_EPETRA_HPP_
#define SRC_DISTLINALG_CL_COMMUNICATOR_EPETRA_HPP_

#include "Epetra_ConfigDefs.h"

#ifdef MORIS_HAVE_PARALLEL
 #include "Epetra_MpiComm.h"
 #include <mpi.h>
#else
#include "Epetra_SerialComm.h"
#endif

namespace moris
{
class Communicator_Epetra
{
private:
protected:
    Epetra_Comm   *mEpetraComm;

public:
    /** Constructor */
    Communicator_Epetra();

    /** Destructor */
     ~Communicator_Epetra();
    
    /** Get the pointer to the Epetra Communicator */
    //Epetra_Comm* get_epetra_comm() { return mEpetraComm; }
    Epetra_Comm* get_epetra_comm() { return mEpetraComm; }
};
}
#endif /* SRC_DISTLINALG_CL_COMMUNUCATOR_EPETRA_HPP_ */
