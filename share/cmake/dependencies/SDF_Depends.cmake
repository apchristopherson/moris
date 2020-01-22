# SDF Dependencies ------------------------------
# -------------------------------------------------------------------------

# Check if SDF has already been included
if(DEFINED SDF_CONFIGURED_ONCE)
    return()
endif()

set(SDF_CONFIGURED_ONCE "YES")

# Add SDF to the source directory list
list(APPEND MORIS_SOURCE_DIRS ${GEN}/${GEN_MAIN}/${SDF})

# Third party libraries needed by SDF library
set(SDF_TPL_DEPENDENCIES
    ""
    )

# Moris packages needed by SDF
include(${MORIS_DEPENDS_DIR}/LINALG_Depends.cmake)
include(${MORIS_DEPENDS_DIR}/COM_Depends.cmake)
include(${MORIS_DEPENDS_DIR}/MTK_Depends.cmake)
