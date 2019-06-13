# -------------------------------------------------------------------------
# SNOPT libraries ---------------------------------------------------------
# -------------------------------------------------------------------------

if(NOT SNOPT_FOUND_ONCE)
    find_package(SNOPT)
    
    if(SNOPT_FOUND)
        set(SNOPT_FOUND_ONCE TRUE CACHE INTERNAL "SNOPT was found.")
    endif()
    message(STATUS "SNOPT_LIBRARIES: ${SNOPT_LIBRARIES}")
endif()

link_directories(${SNOPT_LIBRARY_DIRS})
set(MORIS_SNOPT_LIBS ${SNOPT_LIBRARIES})