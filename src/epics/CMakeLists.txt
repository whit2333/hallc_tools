cmake_minimum_required (VERSION 3.5)
# --------------------------------------------------------------------------
# EPICS (https://github.com/mdavidsaver/cmake4epics)
find_package(EPICS COMPONENTS ca) # Com is implied
#find_package(EPICSHostArch)
##find_package(EPICSTools)
#include(TestBigEndian)
#test_big_endian(target_endian)
#message(STATUS "Target big endian=${target_endian}")
#message(STATUS "Best guess at EPICS OS class and target")
#
#find_package(EPICSHostArch)
#
#message(STATUS "EPICS_HOST_CLASS=${EPICS_HOST_CLASS}")
#message(STATUS "EPICS_HOST_CLASSES=${EPICS_HOST_CLASSES}")
#message(STATUS "EPICS_HOST_COMPILER=${EPICS_HOST_COMPILER}")
#message(STATUS "EPICS_HOST_ARCHS=${EPICS_HOST_ARCHS}")
#
#find_package(EPICSTargetArch)
#
#message(STATUS "EPICS_TARGET_CLASS=${EPICS_TARGET_CLASS}")
#message(STATUS "EPICS_TARGET_CLASSES=${EPICS_TARGET_CLASSES}")
#message(STATUS "EPICS_TARGET_COMPILER=${EPICS_TARGET_COMPILER}")
#message(STATUS "EPICS_TARGET_ARCHS=${EPICS_TARGET_ARCHS}")
#
#message(STATUS "Find EPICS Base")
#
#find_package(EPICS COMPONENTS 
#  ca IOC database pvAccess pvData pvaClient pvDatabase pvAccessCA nt)
#
if(EPICS_FOUND)
message(STATUS "EPICS_FOUND=${EPICS_FOUND}")
message(STATUS "EPICS_BASE_DIR=${EPICS_BASE_DIR}")
message(STATUS "EPICS_INCLUDE_DIRS=${EPICS_INCLUDE_DIRS}")
message(STATUS "EPICS_TARGET_ARCH=${EPICS_TARGET_ARCH}")
message(STATUS "EPICS_LIBRARIES=${EPICS_LIBRARIES}")
message(STATUS "EPICS_Com_LIBRARY=${EPICS_Com_LIBRARY}")
message(STATUS "EPICS_ca_LIBRARY=${EPICS_ca_LIBRARY}")
message(STATUS "EPICS_pvAccessCA_LIBRARY=${EPICS_pvAccessCA_LIBRARY}")
message(STATUS "EPICS_pvData_LIBRARY=${EPICS_pvData_LIBRARY}")
message(STATUS "EPICS_pvaClient_LIBRARY=${EPICS_pvaClient_LIBRARY}")
message(STATUS "EPICS_IOC_LIBRARIES=${EPICS_IOC_LIBRARIES}")
message(STATUS "EPICSBase_VERSION=${EPICSBase_VERSION}")
message(STATUS "EPICSBase_VERSION_STRING=${EPICSBase_VERSION_STRING}")
message(STATUS "EPICS_DEFINITIONS=${EPICS_DEFINITIONS}")

find_package(hcana REQUIRED)
find_package(Podd 1.6 REQUIRED)
include(PoddCMakeEnv)

find_package(spdlog REQUIRED)

# --------------------------------------------------------
# Because epics' build system is really stupid
add_library(simple_epics SHARED
  src/PVList.cxx
  src/PVBuffer.cxx
  src/PVBufferList.cxx
  )

target_include_directories(simple_epics
  PUBLIC ${EPICS_INCLUDE_DIRS}
  PRIVATE ${EPICS_INCLUDE_DIRS}
  INTERFACE ${EPICS_INCLUDE_DIRS}
  PRIVATE include
  PRIVATE include/simple_epics
  PRIVATE include/cppitertools
)
target_compile_definitions(simple_epics
  PUBLIC ${EPICS_DEFINITIONS}
  PRIVATE ${EPICS_DEFINITIONS}
  INTERFACE ${EPICS_DEFINITIONS}
)
target_link_libraries(simple_epics
  PUBLIC ${EPICS_ca_LIBRARY}
  ${EPICS_Com_LIBRARY}
  ${EPICS_BASE_DIR}/lib/${EPICS_TARGET_ARCH}/libpvAccess.so
  ${EPICS_BASE_DIR}/lib/${EPICS_TARGET_ARCH}/libpvAccessCA.so
  ${EPICS_BASE_DIR}/lib/${EPICS_TARGET_ARCH}/libpvData.so
  PUBLIC fmt::fmt
  PUBLIC Podd::Podd
)
set_target_properties(simple_epics
  PROPERTIES
  LINKER_LANGUAGE CXX
  CXX_STANDARD_REQUIRED 17
)
target_compile_features(simple_epics
    PUBLIC cxx_auto_type
    PUBLIC cxx_trailing_return_types
    PRIVATE cxx_variadic_templates
    PRIVATE cxx_std_17
    )

install(DIRECTORY include/simple_epics
  DESTINATION include
  )
include(GNUInstallDirs)
install(TARGETS simple_epics
    EXPORT RunPlannerTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)


## ----------------------------------------------
## Tests
#
## test_line
#set(test_name test_PVGetList)
#add_executable(${test_name} tests/${test_name}.cxx)
#target_include_directories(${test_name} PUBLIC
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
#  PUBLIC src/3rd_party/imgui
#  PUBLIC ${EPICS_INCLUDE_DIRS}
#  )
#target_link_libraries(${test_name} 
#  PRIVATE epics  
#  PRIVATE ImGui  
#  PRIVATE Catch
#  ${EPICS_LIBRARIES}
#  )
#add_test(NAME ${test_name} COMMAND ${test_name})


endif(EPICS_FOUND)
