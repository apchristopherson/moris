# Optimization Dependencies -----------------------------------------------
# -------------------------------------------------------------------------

# Check if OPT has already been included
if(DEFINED OPT_CONFIGURED_ONCE)
    return()
endif()

set(OPT_CONFIGURED_ONCE "YES")

# Add OPT to the source directory list
list(APPEND MORIS_SOURCE_DIRS ${OPT})

# Include libraries needed by OPT
set(OPT_TPL_DEPENDENCIES
    ${ACML_LAPACK_MKL}
    "gcmma"
    "snopt"
    )

# Make sure needed moris libraries are built
include(${MORIS_DEPENDS_DIR}/LNA_Depends.cmake)

# Include third party libraries indirectly needed by OPT
list(APPEND OPT_TPL_DEPENDENCIES
    ${LNA_TPL_DEPENDENCIES}
    )