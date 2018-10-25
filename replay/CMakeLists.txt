cmake_minimum_required(VERSION 3.6)
project(HallCReplay 
  VERSION 0.0.1 
  LANGUAGES CXX
  #DESCRIPTION "A better experiment replay" # cmake >=3.9.2
  #HOMEPAGE_URL https://github.com/whit2333/EXPERIMENT_replay # only cmake >3.12
  )

set(HALLC_REPLAY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/submodules/hallc_replay
    CACHE PATH "replay directory to use (submodule)")

configure_file(cmake/hallc_replay_config.h.in      hallc_replay_config.h @ONLY)
configure_file(cmake/make_hallc_replay_symlinks.in make_hallc_replay_symlinks @ONLY)
configure_file(cmake/make_hallc_replay.in make_hallc_replay @ONLY)
configure_file(cmake/hallc_config.in               hallc_config @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hallc_replay_config.h
  DESTINATION include
  )

install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/hallc_config
  ${CMAKE_CURRENT_BINARY_DIR}/make_hallc_replay_symlinks
  ${CMAKE_CURRENT_BINARY_DIR}/make_hallc_replay
  DESTINATION bin
  PERMISSIONS Group_READ WORLD_READ WORLD_EXECUTE OWNER_EXECUTE OWNER_WRITE OWNER_READ
  )

add_library(HallCReplay INTERFACE)
add_library(${PROJECT_NAME}::HallCReplay ALIAS HallCReplay)
target_sources(HallCReplay INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/hallc_replay_config.h>
  $<INSTALL_INTERFACE:include/hallc_replay_config.h>
  )
target_include_directories(HallCReplay INTERFACE
    $<BUILD_INTERFACE:include>
    $<INSTALL_INTERFACE:include>
)

include(GNUInstallDirs)
install(TARGETS HallCReplay
  EXPORT hallc_replay-export
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# -------------------------------------
include(CMakePackageConfigHelpers)

install(EXPORT hallc_replay-export
  FILE HallCReplayTargets.cmake
  NAMESPACE HallCReplay::
  DESTINATION lib/HallCReplay
  )

set(TARGETS_INSTALL_PATH lib/HallCReplay/HallCReplayTargets.cmake)

CONFIGURE_PACKAGE_CONFIG_FILE(
  cmake/HallCReplayConfig.cmake.in  
  ${CMAKE_CURRENT_BINARY_DIR}/HallCReplayConfig.cmake
  INSTALL_DESTINATION lib
  PATH_VARS TARGETS_INSTALL_PATH
  )

  write_basic_package_version_file("HallCReplayConfigVersion.cmake"
    VERSION ${VERSION}
    COMPATIBILITY SameMajorVersion
    )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/HallCReplayConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/HallCReplayConfigVersion.cmake
  DESTINATION lib/HallCReplay
  )

