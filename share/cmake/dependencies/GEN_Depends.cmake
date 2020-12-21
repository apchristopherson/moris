# Geometry Engine Dependencies --------------------------------------------
# -------------------------------------------------------------------------

# Check if GEN has already been included
if(DEFINED GEN_CONFIGURED_ONCE)
    return()
endif()

set(GEN_CONFIGURED_ONCE "YES")

# Add GEN to the source directory list
list(APPEND MORIS_SOURCE_DIRS ${GEN}/${GEN_MAIN})

# Third party libraries used directly by GEN
set(GEN_TPL_DEPENDENCIES
	""
	)

# Make sure needed moris libraries are built
include(${MORIS_DEPENDS_DIR}/GEN_CORE_Depends.cmake)
include(${MORIS_DEPENDS_DIR}/LINALG_Depends.cmake)
include(${MORIS_DEPENDS_DIR}/MTK_Depends.cmake)
include(${MORIS_DEPENDS_DIR}/MSI_Depends.cmake)
include(${MORIS_DEPENDS_DIR}/PRM_Depends.cmake)
include(${MORIS_DEPENDS_DIR}/WRK_Depends.cmake)
