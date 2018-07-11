# -----------------------------------------------------------------------------
# Boost libraries and includes ------------------------------------------------
# -----------------------------------------------------------------------------

# CMake is case-sensitive.
# Use find_package(Boost ...) but not find_package(BOOST ...).
# @see [Properly configuring boost in CMake]
# (http://stackoverflow.com/questions/23782799/properly-configuring-boost-in-cmake)
find_package(Boost 1.54.0 REQUIRED filesystem system log log_setup thread serialization timer)

if(Boost_FOUND)
    message(STATUS "Boost_FOUND: ${Boost_FOUND}")
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")

    message(STATUS "Boost_FILESYSTEM_FOUND: ${Boost_FILESYSTEM_FOUND}")
    message(STATUS "Boost_FILESYSTEM_LIBRARY: ${Boost_FILESYSTEM_LIBRARY}")

    message(STATUS "Boost_SYSTEM_FOUND: ${Boost_SYSTEM_FOUND}")
    message(STATUS "Boost_SYSTEM_LIBRARY: ${Boost_SYSTEM_LIBRARY}")

    message(STATUS "Boost_LOG_FOUND: ${Boost_LOG_FOUND}")
    message(STATUS "Boost_LOG_LIBRARY: ${Boost_LOG_LIBRARY}")

    message(STATUS "Boost_LOG_SETUP_FOUND: ${Boost_LOG_SETUP_FOUND}")
    message(STATUS "Boost_LOG_SETUP_LIBRARY: ${Boost_LOG_SETUP_LIBRARY}")

    message(STATUS "Boost_SERIALIZATION_FOUND: ${Boost_SERIALIZATION_FOUND}")
    message(STATUS "Boost_SERIALIZATION_LIBRARY: ${Boost_SERIALIZATION_LIBRARY}")

    message(STATUS "Boost_TIMER_FOUND: ${Boost_TIMER_FOUND}")
    message(STATUS "Boost_TIMER_LIBRARY: ${Boost_TIMER_LIBRARY}")

    message(STATUS "Boost_VERSION: ${Boost_VERSION}")
    message(STATUS "Boost_LIB_VERSION: ${Boost_LIB_VERSION}")
    message(STATUS "Boost_MAJOR_VERSION: ${Boost_MAJOR_VERSION}")
    message(STATUS "Boost_MINOR_VERSION: ${Boost_MINOR_VERSION}")
    message(STATUS "Boost_SUBMINOR_VERSION: ${Boost_SUBMINOR_VERSION}")

    list(APPEND MORIS_INCDIRS ${Boost_INCLUDE_DIRS})
    list(APPEND MORIS_LDFLAGS ${Boost_LIBRARY_DIRS})
    list(APPEND MORIS_LDLIBS ${Boost_LIBRARIES})

    if(Boost_LOG_FOUND)
        # @note[chvillanuevap@gmail.com] Fix Boost.Log library linking issue.
        # @see [Boost logger linking issue]
        # (http://stackoverflow.com/questions/18881602/boost-logger-linking-issue)
        list(APPEND MORIS_DEFINITIONS "-DBOOST_LOG_DYN_LINK")

        # @note[chvillanuevap@gmail.com]
        # Fix Boost.Preprocessor library variadic template issue in Clang.
        # Needed to overload macros.
        # Note this may not work with other compilers.
        # In that scenario, we need to revisit this definition.
        # @see [[Boost-users] Problem with variadic BOOST_PP_TUPLE_REM in Clang]
        # (https://groups.google.com/forum/#!topic/boost-list/YTXqzIjMUrg)
        list(APPEND MORIS_DEFINITIONS "-DBOOST_PP_VARIADICS")
        
        # @note[chvillanuevap@gmail.com]
        # Boost.Log requires this library.
        # Otherwise, the following error is produced at linking time:
        # undefined reference to symbol 'pthread_rwlock_wrlock@@GLIBC_2.2.5'
        # You can link either '-lpthread' or 'pthread'.
        list(APPEND MORIS_LDLIBS "-lpthread")
    endif()
endif()