cmake_minimum_required (VERSION 3.1)

add_library(csvplan SHARED 
  src/CSVSettings.cxx
  )


target_include_directories(csvplan
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        PUBLIC ${InSANE_INCLUDE_DIRS}
        PUBLIC ${EIGEN3_INCLUDE_DIRS}
)
target_compile_features(csvplan PUBLIC cxx_std_17)

target_link_libraries(csvplan
  PUBLIC InSANEbase
)

#file(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR}/include/CSV/*)
#install(FILES ${headers}
#  DESTINATION include/CSV
#  )

install(DIRECTORY include/CSV
  DESTINATION include
  )


include(GNUInstallDirs)
install(TARGETS csvplan
    EXPORT RunPlannerTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

