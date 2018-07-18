# Geometry Engine Dependencies --------------------------------------------
# -------------------------------------------------------------------------

# Check if GEN has already been included
if(DEFINED GEN_CONFIGURED_ONCE)
    return()
endif()

set(GEN_CONFIGURED_ONCE "YES")

# Add GEN to the source directory list
list(APPEND MORIS_SRC_DIRS ${GEN})

# Include libraries needed by GEN
# N/A
include(share/cmake/MatthewCMake/LNA_Depends.cmake) #> headers
include(share/cmake/MatthewCMake/DLA_Depends.cmake)

set(GEN_TPL_DEPENDENCIES
    ${LNA_TPL_DEPENDENCIES}
    ${DLA_TPL_DEPENDENCIES} )
