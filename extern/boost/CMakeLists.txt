# Settings for finding correct Boost libraries.
set( Boost_USE_STATIC_LIBS      FALSE )
set( Boost_USE_MULTITHREADED    TRUE )
set( Boost_USE_STATIC_RUNTIME   FALSE )
# set( Boost_ADDITIONAL_VERSIONS  "${BOOST_VERSION}" )
# set( Boost_COMPILER             "-gcc" )

# Search for Boost libraries.
find_package( Boost ${BOOST_VERSION}  REQUIRED
    COMPONENTS program_options)
               

# Make found targets globally available.
if ( Boost_FOUND )
    set_target_properties(Boost::program_options
      PROPERTIES IMPORTED_GLOBAL TRUE )
endif ()
