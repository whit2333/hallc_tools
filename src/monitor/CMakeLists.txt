cmake_minimum_required (VERSION 3.1)

find_package(hcana REQUIRED)
find_package(Podd 1.6 REQUIRED)
include(PoddCMakeEnv)

find_package(spdlog REQUIRED)

SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
find_package( Boost 1.50 COMPONENTS system thread regex REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

############################
## ExpMonitor library
#add_library(ExpMonitor SHARED 
#  src/ExperimentMonitor.cxx
#  ${CMAKE_CURRENT_BINARY_DIR}/ExpMonitorDict.cxx
#  )
#
#set(lib_HEADERS 
#  include/monitor/ExperimentMonitor.h
#  )
#
#root_generate_dictionary(ExpMonitorDict 
#  -I${CMAKE_CURRENT_SOURCE_DIR} ${lib_HEADERS} 
#  LINKDEF include/LinkDef.h 
#  OPTIONS -p)
#add_custom_target(ExpMonitor_ROOTDICTS DEPENDS ${lib_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/ExpMonitorDict.cxx)
#add_dependencies(ExpMonitor ExpMonitor_ROOTDICTS)
#
#target_include_directories(ExpMonitor
#    PUBLIC 
#        $<INSTALL_INTERFACE:include/runplan>    
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/monitor>
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
#    PUBLIC ${InSANE_INCLUDE_DIRS}
#    PUBLIC ${EIGEN3_INCLUDE_DIRS}
#)
#target_compile_features(ExpMonitor PUBLIC cxx_std_17)
#target_link_libraries(ExpMonitor
#  PUBLIC RunPlan ${CXX_FILESYSTEM_LIB}
#  PUBLIC fmt::fmt
#)

###########################
# Scandalizer library
add_library(Scandalizer SHARED 
  src/ExperimentMonitor.cxx
  src/DetectorDisplay.cxx
  src/DisplayServer.cxx
  src/DisplayPlots.cxx
  src/PostProcessors.cxx
  src/EventDisplays.cxx
  src/MonitoringDisplay.cxx
  src/SpectrometerMonitor.cxx
  src/YieldMonitors.cxx
  src/ScriptHelpers.cxx

  ${CMAKE_CURRENT_BINARY_DIR}/ScandalizerDict.cxx
  )

set(lib_HEADERS 
  include/scandalizer/PostProcessors.h
  include/scandalizer/ScriptHelpers.h
  include/scandalizer/SpectrometerMonitor.h
  include/scandalizer/YieldMonitors.h
  include/monitor/ExperimentMonitor.h
  include/monitor/DetectorDisplay.h
  include/monitor/DisplayServer.h
  include/monitor/DisplayPlots.h
  include/monitor/EventDisplays.h
  include/monitor/MonitoringDisplay.h
  )

root_generate_dictionary(ScandalizerDict 
  -I${CMAKE_CURRENT_SOURCE_DIR} 
  -I${CMAKE_CURRENT_SOURCE_DIR}/include
  $<BUILD_INTERFACE:-I${CMAKE_SOURCE_DIR}/src/epics/include>
  -I${PODD_INCLUDE_DIR}
  ${lib_HEADERS} 
  LINKDEF include/LinkDef.h 
  OPTIONS -p)
add_custom_target(Scandalizer_ROOTDICTS DEPENDS ${lib_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/ScandalizerDict.cxx)
add_dependencies(Scandalizer Scandalizer_ROOTDICTS)

target_link_libraries(Scandalizer
  PUBLIC simple_epics RunPlan ${CXX_FILESYSTEM_LIB}
  PUBLIC fmt::fmt
  PUBLIC Podd::Podd
  PUBLIC Podd::Decode
  PUBLIC hcana::HallC
)
target_include_directories(Scandalizer
  PRIVATE 
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/epics/include>
    PUBLIC 
        $<INSTALL_INTERFACE:include/scandalizer>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/scandalizer>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC ${EIGEN3_INCLUDE_DIRS}
    PUBLIC ${PODD_INCLUDE_DIR}
    PUBLIC ${HCANA_INCLUDE_DIR}
)
target_compile_features(Scandalizer PUBLIC cxx_std_17)

################################
# experiment_monitor executable
add_executable(experiment_monitor src/experiment_monitor.cxx)
target_compile_features(experiment_monitor PUBLIC cxx_std_17)
target_include_directories(experiment_monitor
  PRIVATE include    
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
target_link_libraries(experiment_monitor
  PRIVATE nlohmann_json::nlohmann_json
  PUBLIC ${CXX_FILESYSTEM_LIB}
  #PUBLIC InSANEbase
  )

##################################
# spectrometer_settings executable
add_executable(hcspec src/spectrometer_settings.cxx)
target_compile_features(hcspec PUBLIC cxx_std_17)
target_include_directories(hcspec
  PRIVATE include    
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
target_link_libraries(hcspec
  PRIVATE nlohmann_json::nlohmann_json
  PUBLIC fmt::fmt
  #PUBLIC InSANEbase
  PUBLIC ${CXX_FILESYSTEM_LIB}
  PRIVATE hcana::HallC
)

#file(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR} / include / CSV/*)
#install(FILES ${headers}
#  DESTINATION include/CSV
#  )
#install(DIRECTORY include/runplan
#  DESTINATION include
#  )


include(GNUInstallDirs)

install(DIRECTORY include/scandalizer
  DESTINATION include
  )
install(DIRECTORY include/monitor
  DESTINATION include
  )
install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/ScandalizerDict_rdict.pcm
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

install(TARGETS Scandalizer experiment_monitor hcspec
    EXPORT RunPlannerTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)


################################
## experiment_monitor executable
#add_executable(chat_client
#  tests/async_tcp_client.cpp)
#add_executable(chat_server  tests/server.cpp)
##target_compile_features(chat PUBLIC cxx_std_17)
#target_include_directories(chat_client
#  PRIVATE ${Boost_INCLUDE_DIR}
#  PRIVATE /usr/include/boost
#  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/chat
#  PRIVATE include    
#  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
#  )
#target_link_libraries(chat_client
#  PRIVATE ${Boost_LIBRARIES}
#  PRIVATE nlohmann_json::nlohmann_json
#  PUBLIC ${CXX_FILESYSTEM_LIB}
#  #PUBLIC InSANEbase
#  )
#target_include_directories(chat_server
#  PRIVATE ${Boost_INCLUDE_DIR}
#  PRIVATE /usr/include/boost
#  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/chat
#  PRIVATE include    
#  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
#  )
#target_link_libraries(chat_server
#  PRIVATE ${Boost_LIBRARIES}
#  PRIVATE nlohmann_json::nlohmann_json
#  PUBLIC ${CXX_FILESYSTEM_LIB}
#  #PUBLIC InSANEbase
#  )
