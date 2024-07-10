cmake_minimum_required(VERSION 3.24)

project(cgull)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

include(FetchContent)

FetchContent_Declare(
	incbin
	GIT_REPOSITORY "https://github.com/graphitemaster/incbin.git"
	GIT_TAG "main"
	GIT_SHALLOW TRUE
)

FetchContent_Declare(
	raylib
	URL "https://github.com/raysan5/raylib/archive/refs/tags/5.0.zip"
	URL_HASH SHA256=701f92f0646fbf1cec5ecc3bbc9d27d6511a3c9163c1ef8a679b15474ce854f5
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
	raylib-cpp
	URL "https://github.com/RobLoach/raylib-cpp/archive/refs/tags/v5.0.2.zip"
	URL_HASH SHA256=754cbf61a3b67867fa0db83069daafc39157a2cfdf17661acddac6cf372c546e
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
	rocket
	GIT_REPOSITORY "https://github.com/rocket/rocket.git"
	GIT_TAG "59be1f138c983a7b88bd94cc049db32431ed1ba2"
	GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(incbin raylib raylib-cpp rocket)

# Shader Minifier runs only on Windows
if(WIN32)
	FetchContent_Declare(
		shader-minifier
		URL "https://github.com/laurentlb/shader-minifier/releases/download/1.4.0/shader_minifier.exe"
		URL_HASH SHA256=7a4bd1605a0e3d3cc501265a439be2888c4865c3694518f7ba41200f5808d610
		DOWNLOAD_NO_EXTRACT TRUE
	)

	FetchContent_MakeAvailable(shader-minifier)
endif()

# Since I modify glfwSetTime I need to disable raylib frame time control
target_compile_definitions(raylib PRIVATE SUPPORT_CUSTOM_FRAME_CONTROL=1)

# I will literally go insane if I have to fight with MSVC/vcxproj any more, just add the sources directly
set(rocket_SOURCES
	"${rocket_SOURCE_DIR}/lib/device.c"
	"${rocket_SOURCE_DIR}/lib/tcp.c"
	"${rocket_SOURCE_DIR}/lib/track.c")

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE src/main.cpp $<$<PLATFORM_ID:Windows>:data.c> ${rocket_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/src"
	"${incbin_SOURCE_DIR}"
	"${raylib_BINARY_DIR}/raylib/include"
	"${raylib-cpp_SOURCE_DIR}/include"
	"${rocket_SOURCE_DIR}/lib"
)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:SYNC_PLAYER>)

# Build the Rocket .xml data into its binary representation
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/sync.bin"
	COMMAND python3 "${CMAKE_SOURCE_DIR}/rocket2bin.py" "${CMAKE_SOURCE_DIR}/sync.xml" "${CMAKE_CURRENT_BINARY_DIR}/sync.bin"
	DEPENDS "${CMAKE_SOURCE_DIR}/sync.xml"
)
add_custom_target(sync-data DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/sync.bin")
add_dependencies(${PROJECT_NAME} sync-data)

# Build the incbin binary needed for embedding with MSVC
if(WIN32)
	add_executable(incbin-bin ${incbin_SOURCE_DIR}/incbin.c)
	add_dependencies(${PROJECT_NAME} incbin-bin)
	add_custom_command(OUTPUT data.c
		DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/sync.bin"
		COMMAND $<TARGET_FILE:incbin-bin> ${CMAKE_SOURCE_DIR}/src/main.cpp ${CMAKE_SOURCE_DIR}/src/scenes/City.h
	)
endif()

# Minify shaders on Windows
if(WIN32)
	add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/src/scenes/City.min.fs
		DEPENDS ${CMAKE_SOURCE_DIR}/src/scenes/City.fs
		COMMAND ${shader-minifier_SOURCE_DIR}/shader_minifier.exe -o ${CMAKE_SOURCE_DIR}/src/scenes/City.min.fs --format text --preserve-externals ${CMAKE_SOURCE_DIR}/src/scenes/City.fs
	)
	add_custom_target(minified-shaders DEPENDS ${CMAKE_SOURCE_DIR}/src/scenes/City.min.fs)
	add_dependencies(${PROJECT_NAME} minified-shaders)
endif()