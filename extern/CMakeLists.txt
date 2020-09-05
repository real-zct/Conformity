set(NETWORKIT_BUILD_TESTS OFF)
# add networkit directory
add_subdirectory(networkit)

# add boost directory
add_subdirectory(boost)

# add the random generator
add_subdirectory(sfmt)

# add abseil library
add_subdirectory(abseil-cpp)


