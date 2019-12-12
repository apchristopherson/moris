# MSI Dependencies --------------------------------------------------------
# -------------------------------------------------------------------------

# Check if MSI has already been included
if(DEFINED MSI_CONFIGURED_ONCE)
    return()
endif()

set(MSI_CONFIGURED_ONCE "YES")

# Add MSI to the source directory list
list(APPEND MORIS_SOURCE_DIRS ${FEM}/${MSI})

# Include libraries needed by MSI
set(MSI_TPL_DEPENDENCIES
    ""
    )

# Make sure needed moris libraries are built
include(${MORIS_DEPENDS_DIR}/LINALG_Depends.cmake)
include(${MORIS_DEPENDS_DIR}/COM_Depends.cmake)
include(${MORIS_DEPENDS_DIR}/INT_Depends.cmake)
include(${MORIS_DEPENDS_DIR}/DLA_Depends.cmake)
include(${MORIS_DEPENDS_DIR}/HMR_Depends.cmake)
#include(${MORIS_DEPENDS_DIR}/MDL_Depends.cmake)

# needed includes for test
include(${MORIS_DEPENDS_DIR}/ALG_Depends.cmake)
include(${MORIS_DEPENDS_DIR}/NLA_Depends.cmake)
