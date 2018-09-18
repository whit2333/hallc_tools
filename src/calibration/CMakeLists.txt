cmake_minimum_required (VERSION 3.1)

find_package(hcana REQUIRED)
find_package(Podd 1.6 REQUIRED)
include(PoddCMakeEnv)

###########################
# ExpMonitor library
add_library(hcCalibration SHARED 
  src/THcPShHit.cxx
  src/THcPShowerCalib.cxx
  src/THcPShTrack.cxx
  #src/hcCalibration.cxx
  src/CalorimeterCalibration.cxx
  src/ShowerCalibrator.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/hcCalibrationDict.cxx
  )

set(lib_HEADERS 
  #include/calibration/hcCalibration.h
  include/calibration/THcPShHit.h
  include/calibration/THcPShowerCalib.h
  include/calibration/THcPShTrack.h
  include/calibration/CalorimeterCalibration.h
  include/calibration/ShowerCalibrator.h
  )

#get_target_property(insane_include_dir InSANE::InSANEnew_xsec INTERFACE_INCLUDE_DIRECTORIES)

root_generate_dictionary(hcCalibrationDict 
  #-I${insane_include_dir} 
  -I${CMAKE_CURRENT_SOURCE_DIR}/include
  -I${CMAKE_CURRENT_SOURCE_DIR} ${lib_HEADERS} 
  LINKDEF include/LinkDef.h 
  OPTIONS -p)
add_custom_target(hcCalibration_ROOTDICTS DEPENDS ${lib_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/hcCalibrationDict.cxx)
add_dependencies(hcCalibration hcCalibration_ROOTDICTS)

target_link_libraries(hcCalibration
  PUBLIC ${CXX_FILESYSTEM_LIB}
  PUBLIC nlohmann_json::nlohmann_json
  #  PUBLIC fmt::fmt
  #  PUBLIC InSANE::InSANEbase
  #  PUBLIC InSANE::InSANEnew_xsec
)
target_include_directories(hcCalibration
    PUBLIC 
        $<INSTALL_INTERFACE:include/runplan>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/calibration>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC ${EIGEN3_INCLUDE_DIRS}
)
target_compile_features(hcCalibration PUBLIC cxx_std_17)

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

################################
# experiment_monitor executable
#add_executable(experiment_monitor src/experiment_monitor.cxx)
#target_compile_features(experiment_monitor PUBLIC cxx_std_17)
#target_include_directories(experiment_monitor
#  PRIVATE include    
#  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
#  )
#target_link_libraries(experiment_monitor
#  PRIVATE nlohmann_json::nlohmann_json
#  PUBLIC ${CXX_FILESYSTEM_LIB}
#  #PUBLIC InSANEbase
#  )
#

##################################

# hc_cal_calib
add_executable(hc_cal_calib src/pcal_calib.cxx)

target_compile_features(hc_cal_calib PUBLIC cxx_std_17)
target_include_directories(hc_cal_calib
  PRIVATE include    
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
target_link_libraries(hc_cal_calib
  PUBLIC hcCalibration
  PRIVATE nlohmann_json::nlohmann_json
  #PUBLIC InSANEbase
  PUBLIC ${CXX_FILESYSTEM_LIB}
  PRIVATE hcana::HallC
)

# shms_cal_calib
add_executable(shms_cal_calib src/shms_cal_calib.cxx)

target_compile_features(shms_cal_calib PUBLIC cxx_std_17)
target_include_directories(shms_cal_calib
  PRIVATE include    
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
target_link_libraries(shms_cal_calib
  PUBLIC hcCalibration
  PRIVATE nlohmann_json::nlohmann_json
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

install(DIRECTORY include/calibration
  DESTINATION include
  )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hcCalibrationDict_rdict.pcm
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

install(TARGETS hcCalibration hc_cal_calib shms_cal_calib
    EXPORT RunPlannerTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
