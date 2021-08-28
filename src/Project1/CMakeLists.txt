cmake_minimum_required (VERSION 3.6)
project (g4gP1)

include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/../../includes")
link_directories ("${CMAKE_CURRENT_SOURCE_DIR}/../../lib")

include_directories ("${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB_RECURSE G4G1_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/*.c)
# On Windows, we're not going to worry about CRT secure warnings.
if (MSVC)
	set(CMAKE_CXX_FLAGS "$(CMAKE_CXX_FLAGS) /EHsc")
endif()

add_executable(g4g1 ${G4G1_SOURCE_FILES} always_copy_data.h)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT g4g1)
target_link_libraries(g4g1 glfw3 opengl32 )
if (MSVC)
	set_target_properties(g4g1 PROPERTIES LINK_FLAGS "/ignore:4098 /ignore:4099")
	set_property(TARGET g4g1 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Debug)
endif()

add_custom_target(ALWAYS_COPY_DATA COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_SOURCE_DIR}/always_copy_data.h)
add_dependencies(g4g1 ALWAYS_COPY_DATA)

add_custom_command(TARGET g4g1 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../../data $<TARGET_FILE_DIR:g4g1>/data)