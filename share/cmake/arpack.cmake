# -------------------------------------------------------------------------
# ARPACK libraries --------------------------------------------------------
# -------------------------------------------------------------------------

if(NOT ARPACK_FOUND_ONCE)
    find_package(ARPACK)
    
    if(ARPACK_FOUND)
        set(ARPACK_FOUND_ONCE TRUE CACHE INTERNAL "ARPACK was found.")
    endif()
endif()

message(STATUS "ARPACK_LIBRARIES: ${ARPACK_LIBRARIES}")

# list(APPEND MORIS_LDLIBS ${ARPACK_LIBRARIES})

set(MORIS_ARPACK_LIBS ${ARPACK_LIBRARIES})
