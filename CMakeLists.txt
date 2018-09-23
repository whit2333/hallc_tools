cmake_minimum_required (VERSION 3.1)
project (RunPlanner VERSION 0.0.1)

include(GNUInstallDirs)
include(cmake/os.cmake)

find_package(ROOT COMPONENTS Core REQUIRED CONFIG)
include(${ROOT_USE_FILE})
include(cmake/root.cmake)

#add_subdirectory(src/3rd_party)
#list(APPEND CMAKE_PREFIX_PATH /usr/local/share/eigen3/cmake)
#list(APPEND CMAKE_PREFIX_PATH /usr/local/share/eigen3)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})
find_package (Eigen3 3.3 REQUIRED NO_MODULE)

find_package(nlohmann_json CONFIG)
if(NOT nlohmann_json_FOUND)
  message(FATAL_ERROR " to build all 3rd party run the command:\n\
  bash ${CMAKE_SOURCE_DIR}/src/3rd_party/build_all_3rd_party.sh ${CMAKE_INSTALL_PREFIX}\n\
   nlohmann json missing : https://github.com/nlohmann/json.git")
  find_package(nlohmann_json REQUIRED)
endif(NOT nlohmann_json_FOUND)

#https://github.com/fmtlib/fmt.git
find_package(fmt REQUIRED)
find_package(range-v3 REQUIRED)

add_subdirectory(src/runplan)
add_subdirectory(src/monitor)
add_subdirectory(src/calibration)
add_subdirectory(replay)

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

