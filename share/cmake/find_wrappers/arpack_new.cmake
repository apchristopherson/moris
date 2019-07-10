# -------------------------------------------------------------------------
# ARPACK libraries --------------------------------------------------------
# -------------------------------------------------------------------------

if(NOT ARPACK_FOUND_ONCE)
    find_package(ARPACK)
    
    if(ARPACK_FOUND)
        #set(ARPACK_FOUND_ONCE TRUE CACHE INTERNAL "ARPACK was found.")
        set(ARPACK_FOUND_ONCE TRUE)
        
        #set(MORIS_ARPACK_LIBRARIES ${ARPACK_LIBRARIES}
        #	CACHE INTERNAL "ARPACK libraries.")
        set(MORIS_ARPACK_LIBRARIES ARPACK::arpack)
        
        mark_as_advanced(MORIS_ARPACK_LIBRARIES)
    endif()
    
    message(STATUS "ARPACK_LIBRARIES: ${ARPACK_LIBRARIES}")
endif()

if(NOT TARGET ${MORIS}::arpack)
	add_library(${MORIS}::arpack INTERFACE IMPORTED GLOBAL)
	target_link_libraries(${MORIS}::arpack INTERFACE ${MORIS_ARPACK_LIBRARIES})
endif()
