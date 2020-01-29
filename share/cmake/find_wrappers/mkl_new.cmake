# -------------------------------------------------------------------------
# MKL libraries -----------------------------------------------------------
# -------------------------------------------------------------------------

if(NOT MKL_FOUND_ONCE)
    find_package(MKL)
    if(MKL_FOUND)
        #set(MKL_FOUND_ONCE TRUE CACHE INTERNAL "MKL was found.")
        set(MKL_FOUND_ONCE TRUE)
        
        #set(MORIS_MKL_INCLUDE_DIRS ${MKL_INCLUDE_DIRS}
        #	CACHE PATH "MKL include directories.")
        #set(MORIS_MKL_LIBRARIES ${MKL_LIBRARIES}
        #	CACHE INTERNAL "MKL libraries.")
        #set(MORIS_MKL_DEFINITIONS "-DMORIS_HAVE_MKL"
        #	CACHE INTERNAL "Moris preprocessor definitions for MKL.")
        set(MORIS_MKL_INCLUDE_DIRS ${MKL_INCLUDE_DIRS})
        set(MORIS_MKL_DEFINITIONS "-DMORIS_HAVE_MKL")
        set(MORIS_MKL_LIBRARIES MKL::mkl)
        #set(MORIS_MKL_LIBRARIES ${MKL_LIBRARIES})
        
        mark_as_advanced(MORIS_MKL_INCLUDE_DIRS
        	MORIS_MKL_LIBRARIES
        	MORIS_MKL_DEFINITIONS )
    endif()
    message(STATUS "MKL_LIBRARIES: ${MKL_LIBRARIES}")
endif()

if(NOT TARGET ${MORIS}::mkl)
	add_library(${MORIS}::mkl INTERFACE IMPORTED GLOBAL)
	target_link_libraries(${MORIS}::mkl INTERFACE ${MORIS_MKL_LIBRARIES})
endif()

#if(MORIS_USE_MKL)
#    add_definitions("-DMORIS_HAVE_MKL")
#endif()
#include_directories(${MKL_INCLUDE_DIRS})
#set(MORIS_MKL_LIBS ${MKL_LIBRARIES})
#set(MORIS_ACML_LAPACK_MKL_OPENBLAS_LIBS ${MKL_LIBRARIES})
