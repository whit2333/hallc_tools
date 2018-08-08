cmake_minimum_required (VERSION 3.1)

find_package(hcana REQUIRED)
find_package(Podd 1.6 REQUIRED)
include(PoddCMakeEnv)


#add_library(ExpMonitor SHARED 
#  )
#
#target_include_directories(ExpMonitor
#  PUBLIC 
#    $<INSTALL_INTERFACE:include>    
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#  PRIVATE
#    ${CMAKE_CURRENT_SOURCE_DIR}/src
#  )
#target_include_directories(experiment_monitor
#  PUBLIC 
#    $<INSTALL_INTERFACE:include>    
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#  PRIVATE
#    ${CMAKE_CURRENT_SOURCE_DIR}/src
#  )
#target_compile_features(ExpMonitor PUBLIC cxx_std_17)
#target_link_libraries(ExpMonitor
#  #PUBLIC fmt::fmt
#  #PUBLIC InSANEbase
#)

add_executable(experiment_monitor src/experiment_monitor.cxx)
target_compile_features(experiment_monitor PUBLIC cxx_std_17)
target_include_directories(experiment_monitor
  PRIVATE include    
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
target_link_libraries(experiment_monitor
  PRIVATE nlohmann_json::nlohmann_json
  #PUBLIC InSANEbase
  )

add_executable(spectrometer_settings src/spectrometer_settings.cxx)
target_compile_features(spectrometer_settings PUBLIC cxx_std_17)
target_include_directories(spectrometer_settings
  PRIVATE include    
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
target_link_libraries(spectrometer_settings
  PRIVATE nlohmann_json::nlohmann_json
  #PUBLIC InSANEbase
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
install(TARGETS experiment_monitor spectrometer_settings
    EXPORT RunPlannerTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

