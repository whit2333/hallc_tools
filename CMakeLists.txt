cmake_minimum_required (VERSION 3.1)
project (RunPlanner VERSION 0.0.1)

find_package(ROOT COMPONENTS Core)
include(${ROOT_USE_FILE})

list(APPEND CMAKE_PREFIX_PATH /usr/local/share/eigen3/cmake)
list(APPEND CMAKE_PREFIX_PATH /usr/local/share/eigen3)
find_package (Eigen3 3.3 REQUIRED NO_MODULE)
find_package(InSANE REQUIRED)

find_package(fmt REQUIRED)


add_subdirectory(src/runplan)

# -------------------------
# install library config
include(CMakePackageConfigHelpers)

# Install and export targets
install(EXPORT RunPlannerTargets
    FILE RunPlannerTargets.cmake
    NAMESPACE RunPlanner::
    DESTINATION lib/RunPlanner
    )


set(TARGETS_INSTALL_PATH lib/RunPlanner/RunPlannerTargets.cmake)
CONFIGURE_PACKAGE_CONFIG_FILE(
  cmake/RunPlannerConfig.cmake.in  
  ${CMAKE_CURRENT_BINARY_DIR}/RunPlannerConfig.cmake
  INSTALL_DESTINATION lib
  PATH_VARS TARGETS_INSTALL_PATH
  )

  write_basic_package_version_file("RunPlannerConfigVersion.cmake"
    VERSION ${VERSION}
    COMPATIBILITY SameMajorVersion
    )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/RunPlannerConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/RunPlannerConfigVersion.cmake
  DESTINATION lib/RunPlanner
  )

