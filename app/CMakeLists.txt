add_executable(app main.cpp)
target_link_libraries(app
  PUBLIC aditum
  PUBLIC Boost::program_options
  )
