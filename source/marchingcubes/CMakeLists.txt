
set(target marchingcubes-painters)
message(STATUS "Example ${target}")


# External libraries

# ...

# Includes

include_directories(
    BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}
)


# Libraries

set(libs
    ${GLEXAMPLES_DEPENDENCY_LIBRARIES}
)


# Compiler definitions



# for compatibility between glm 0.9.4 and 0.9.5
add_definitions("-DGLM_FORCE_RADIANS")


# Sources

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/")
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/")

set(sources
    ${source_path}/AddChunksStage.cpp
    ${source_path}/Chunk.cpp
    ${source_path}/ChunkFactory.cpp
    ${source_path}/LookUpData.cpp
    ${source_path}/ManageChunksStage.cpp
    ${source_path}/MarchingCubes.cpp
    ${source_path}/MarchingCubesPipeline.cpp
    ${source_path}/RenderStage.cpp
    ${source_path}/TerrainModificationStage.cpp
    ${source_path}/TerrainTypeStage.cpp
    ${source_path}/plugin.cpp
)

set(api_includes
    ${include_path}/AddChunksStage.h
    ${include_path}/Chunk.h
    ${include_path}/ChunkFactory.h
    ${include_path}/LookUpData.h
    ${include_path}/ManageChunksStage.h
    ${include_path}/MarchingCubes.h
    ${include_path}/MarchingCubesPipeline.h
    ${include_path}/RenderStage.h
    ${include_path}/TerrainModificationStage.h
    ${include_path}/TerrainType.h
    ${include_path}/TerrainTypeStage.h
    ${include_path}/vec3_hash.h
)

# Group source files
set(header_group "Header Files (API)")
set(source_group "Source Files")
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$"
    ${header_group} ${api_includes})
source_group_by_path(${source_path} "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$"
    ${source_group} ${sources})


# Build library

add_library(${target} SHARED ${api_includes} ${sources})

target_link_libraries(${target} ${libs})

target_compile_options(${target} PRIVATE ${DEFAULT_COMPILE_FLAGS})

set_target_properties(${target}
    PROPERTIES
    LINKER_LANGUAGE              CXX
    FOLDER                      "${IDE_FOLDER}"
    COMPILE_DEFINITIONS_DEBUG   "${DEFAULT_COMPILE_DEFS_DEBUG}"
    COMPILE_DEFINITIONS_RELEASE "${DEFAULT_COMPILE_DEFS_RELEASE}"
    LINK_FLAGS_DEBUG            "${DEFAULT_LINKER_FLAGS_DEBUG}"
    LINK_FLAGS_RELEASE          "${DEFAULT_LINKER_FLAGS_RELEASE}"
    DEBUG_POSTFIX               "d${DEBUG_POSTFIX}"
    INCLUDE_PATH                ${include_path})


# Deployment

# Plugin library (release)
install(TARGETS ${target}
    RUNTIME DESTINATION ${INSTALL_PLUGINS}       CONFIGURATIONS Release
    LIBRARY DESTINATION ${INSTALL_PLUGINS}       CONFIGURATIONS Release
    ARCHIVE DESTINATION ${INSTALL_LIB}
)

# Plugin library (debug)
install(TARGETS ${target}
    RUNTIME DESTINATION ${INSTALL_PLUGINS_DEBUG} CONFIGURATIONS Debug
    LIBRARY DESTINATION ${INSTALL_PLUGINS_DEBUG} CONFIGURATIONS Debug
)
