cmake_minimum_required(VERSION 3.12)


project(CVRP
		VERSION 1.0.0
		DESCRIPTION "PPM + Huffman text compressor"
		HOMEPAGE_URL "lucassguedes.github.io"
		)

set(CMAKE_C_STANDARD 17) 
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./bin")
add_compile_options(-g -Wall -lGL -lGLU -lglut -lGLEW -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl -lXinerama -lXcursor)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

file(GLOB SOURCES src/*.c)

add_executable(ppm_huffman ${SOURCES})

target_link_libraries(ppm_huffman m)



target_include_directories(ppm_huffman PUBLIC
						   include
)


