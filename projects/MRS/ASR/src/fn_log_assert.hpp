#ifndef MORIS_ASSERT_FN_LOG_ASSERT_HPP_
#define MORIS_ASSERT_FN_LOG_ASSERT_HPP_

// C++ header files.
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <cstdio>
#include <string>


// MORIS header files.
#include "core.hpp"
#include "cl_Logger.hpp"


// ----------------------------------------------------------------------------

namespace moris
{
namespace assert
{
    /**
     * Overloaded moris::assert::error.
     *
     * @param[in] msg Error message.
     */
    //template< typename Exception = std::runtime_error >
    void
    error( std::string const & msg )
    {
        MORIS_LOG_ERROR( "*** Error: ");  MORIS_LOG_ERROR( msg.c_str() );

        throw;
    }

    /**
     * Log error message with global logger.
     *
     * @param[in] location  Location where error was raised.
     * @param[in] task      Task where error was raised.
     * @param[in] check     Check that raised assertion.
     * @param[in] exception Exception raised by check.
     */
    //template< typename Exception >
    void
    error( std::string const & location,
           std::string const & task,
           std::string const & check,
           std::runtime_error const & exception )
    {
        MORIS_LOG_ERROR ( "*** ---------------------------------------------------------------------------\n");
        MORIS_LOG_ERROR ( "*** \n"                                                                           );
        MORIS_LOG_ERROR ( "*** Moris encountered an error. If you are not able to resolve this issue\n"      );
        MORIS_LOG_ERROR ( "*** using the information listed below, you can ask for help at\n"                );
        MORIS_LOG_ERROR ( "***\n"                                                                            );
        MORIS_LOG_ERROR ( "***     kurt.maute@colorado.edu\n"                                                );
        MORIS_LOG_ERROR ( "***\n"                                                                            );
        MORIS_LOG_ERROR ( "*** Remember to include the error message listed below and, if possible,\n"       );
        MORIS_LOG_ERROR ( "*** include a *minimal* running example to reproduce the error.\n"                );
        MORIS_LOG_ERROR ( "***\n"                                                                            );
        MORIS_LOG_ERROR ( "*** ---------------------------------------------------------------------------\n");
        MORIS_LOG_ERROR ( "***\n"                                                                            );
        MORIS_LOG_ERROR ( "*** Error:   Unable to "); MORIS_LOG_ERROR( task.c_str() ); MORIS_LOG_ERROR (".\n");
        MORIS_LOG_ERROR ( "*** Reason:  ");           MORIS_LOG_ERROR( check.c_str()); MORIS_LOG_ERROR ("\n");
        std::istringstream exception_msg( exception.what() );
        std::string exception_line;
        while ( std::getline( exception_msg, exception_line ) )
        {
            MORIS_LOG_ERROR("***          "); MORIS_LOG_ERROR(exception_line.c_str()); MORIS_LOG_ERROR("\n");
        }
        MORIS_LOG_ERROR( "*** Where:   This error was encountered inside "); MORIS_LOG_ERROR( location.c_str() ); MORIS_LOG_ERROR(".\n");
        MORIS_LOG_ERROR( "***  Version: 1.0\n");
        MORIS_LOG_ERROR( "***\n");
        MORIS_LOG_ERROR( "*** ---------------------------------------------------------------------------\n");

        throw exception;
    }

    /**
     * Overloaded moris::assert::moris_assert.
     *
     * @param[in] file      File where assertion was raised.
     * @param[in] line      Line where assertion was raised.
     * @param[in] function  Function where assertion was raised.
     * @param[in] check     Check that raised assertion.
     * @param[in] exception Exception raised by check.
     */
    //template<typename Exception>
    void
    moris_assert( std::string        const & file,
                  moris::size_t      const & line,
                  std::string        const & function,
                  std::string        const & check,
                  std::runtime_error const & exception )
    {
        std::stringstream location;
        location << file << " (line " << line << ")";

        std::stringstream task;
        task << "complete call to function " << function << "()";

        std::stringstream reason;
        reason << "Assertion " << check << " failed.";

        moris::assert::error( location.str(), task.str(), reason.str(), exception );
    }

    /**
     * Overloaded moris::assert::moris_assert.
     *
     * @param[in] file      File where assertion was raised.
     * @param[in] line      Line where assertion was raised.
     * @param[in] function  Function where assertion was raised.
     * @param[in] check     Check that raised assertion.
     * @param[in] msg       Error message to build exception.
     */
    template<typename ... Args>
    inline
    void
    moris_assert( const std::string   & file,
                  const moris::size_t & line,
                  const std::string   & function,
                  const std::string   & check,
                  const Args ...        aArgs )
    {
        // Determine size of string
        auto tSize = snprintf( nullptr, 0, aArgs ... );

        // create char pointer with size of string length + 1 for \0
        std::unique_ptr< char[] > tMsg( new char[ tSize + 1 ]);

        // write string into buffered char pointer
        snprintf( tMsg.get(), tSize + 1, aArgs ... );

        moris::assert::moris_assert( file, line, function, check, std::runtime_error( std::string(tMsg.get(), tMsg.get() +tSize).c_str() ) );
    }
}    // namespace assert
}    // namespace moris

#endif /* MORIS_ASSERT_FN_LOG_ASSERT_HPP_ */
