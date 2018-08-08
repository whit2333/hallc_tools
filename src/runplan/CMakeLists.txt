cmake_minimum_required (VERSION 3.1)

add_library(RunPlan SHARED 
  src/arrange_tables.cxx
  src/arrange_tables2.cxx
  src/RunStatus.cxx
  src/sidis_settings_rate.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/RunPlanDict.cxx
  )

set(lib_HEADERS 
  include/runplan/arrange_tables.h
  include/runplan/sidis_settings_rate.h
  include/runplan/RunStatus.h
  )

get_target_property(insane_include_dir InSANE::InSANEnew_xsec INTERFACE_INCLUDE_DIRECTORIES)

# build root dictionaries
root_generate_dictionary(RunPlanDict 
  #-I${insane_include_dir} 
  -I${CMAKE_CURRENT_SOURCE_DIR} ${lib_HEADERS} 
  LINKDEF include/LinkDef.h 
  OPTIONS -p)
add_custom_target(RunPlan_ROOTDICTS DEPENDS ${lib_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/RunPlanDict.cxx)
add_dependencies(RunPlan RunPlan_ROOTDICTS)

target_include_directories(RunPlan
    PUBLIC 
        $<INSTALL_INTERFACE:include/runplan>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/runplan>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC ${InSANE_INCLUDE_DIRS}
    PUBLIC ${EIGEN3_INCLUDE_DIRS}
)
target_compile_features(RunPlan PUBLIC cxx_std_17)

target_link_libraries(RunPlan
  PUBLIC fmt::fmt
  PUBLIC InSANE::InSANEbase
  PUBLIC InSANE::InSANEnew_xsec
)


install(DIRECTORY include/runplan
  DESTINATION include
  )

include(GNUInstallDirs)
install(TARGETS RunPlan
    EXPORT RunPlannerTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

