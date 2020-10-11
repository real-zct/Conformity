include(${CMAKE_SOURCE_DIR}/cmake/add_FetchContent_MakeAvailable.cmake)
include(FetchContent)
# Testing library
FetchContent_Declare(
catch2
GIT_REPOSITORY https://github.com/catchorg/Catch2.git
GIT_TAG        v2.9.1
)


FetchContent_MakeAvailable(catch2)
# Adds Catch2::Catch2
message("fetched")
# Tests need to be added as executables first
add_executable(testlib testlib.cpp)
add_executable(aditumtest aditumTests.cpp)

# I'm using C++17 in the test
target_compile_features(testlib PRIVATE cxx_std_17)
target_compile_features(aditumtest PRIVATE cxx_std_17)

# Should be linked to the main library, as well as the Catch2 testing library
target_link_libraries(testlib PRIVATE
  aditum
  networkit
  sfmt
  absl::flat_hash_set
  Catch2::Catch2)

target_link_libraries(aditumtest PRIVATE
  aditum
  networkit
  sfmt
  absl::flat_hash_set
  Catch2::Catch2)


# If you register a test, then ctest and make test will run it.
# You can also run examples and check the output, as well.
add_test(NAME testlibtest COMMAND testlib) # Command can be a target
add_test(NAME aditumtest COMMAND aditumtest)
