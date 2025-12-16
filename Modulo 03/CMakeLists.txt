cmake_minimum_required(VERSION 3.18)
project(PAVIC_LAB_2025 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(OpenCV REQUIRED)
find_package(OpenMP REQUIRED)

# CUDA is only compatible with MSVC on Windows, not MinGW
# For MinGW builds, we use CPU fallback
set(HAVE_CUDA FALSE)
if(MSVC)
    find_package(CUDAToolkit QUIET)
    if(CUDAToolkit_FOUND)
        enable_language(CUDA)
        set(HAVE_CUDA TRUE)
        message(STATUS "CUDA found: ${CUDAToolkit_VERSION}")
    endif()
endif()

if(NOT HAVE_CUDA)
    message(STATUS "Using CPU fallback for CUDA operations (MinGW or CUDA not found)")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

# Core sources
set(SOURCES
    src/main.cpp
    src/ImageProcessor.cpp
    src/SequentialFilter.cpp
    src/ParallelFilter.cpp
    src/MultithreadFilter.cpp
    src/GUI.cpp
    src/WebcamCapture.cpp
    src/FilterUtils.cpp
    src/PerformanceMetrics.cpp
)

# Add CUDA source if available, otherwise use stub
if(HAVE_CUDA)
    list(APPEND SOURCES src/CUDAFilter.cu)
else()
    list(APPEND SOURCES src/CUDAFilter.cpp)
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    OpenMP::OpenMP_CXX
)

# Windows: linkar com comdlg32 para diálogo de arquivo
if(WIN32)
    target_link_libraries(${PROJECT_NAME} comdlg32)
endif()

if(HAVE_CUDA)
    target_link_libraries(${PROJECT_NAME} CUDA::cudart)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
        CUDA_ARCHITECTURES "50;60;70;75;80;86;89"
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE PAVIC_HAVE_CUDA=1)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE PAVIC_HAVE_CUDA=0)
endif()

# Benchmark executable
set(BENCHMARK_SOURCES
    src/Benchmark.cpp
    src/ImageProcessor.cpp
    src/SequentialFilter.cpp
    src/ParallelFilter.cpp
    src/MultithreadFilter.cpp
    src/FilterUtils.cpp
    src/PerformanceMetrics.cpp
)
if(HAVE_CUDA)
    list(APPEND BENCHMARK_SOURCES src/CUDAFilter.cu)
else()
    list(APPEND BENCHMARK_SOURCES src/CUDAFilter.cpp)
endif()

add_executable(Benchmark ${BENCHMARK_SOURCES})

target_link_libraries(Benchmark
    ${OpenCV_LIBS}
    OpenMP::OpenMP_CXX
)

if(HAVE_CUDA)
    target_link_libraries(Benchmark CUDA::cudart)
    set_target_properties(Benchmark PROPERTIES CUDA_ARCHITECTURES "50;60;70;75;80;86;89")
    target_compile_definitions(Benchmark PRIVATE PAVIC_HAVE_CUDA=1)
else()
    target_compile_definitions(Benchmark PRIVATE PAVIC_HAVE_CUDA=0)
endif()

# Copy resources if present
if(EXISTS ${CMAKE_SOURCE_DIR}/resources)
    file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Create results directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/results)
