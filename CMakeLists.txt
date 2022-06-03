cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(VulkanSolution CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_PARALLEL_ENABLE ON)

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################

if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME x86)
endif()
message(${CMAKE_VS_PLATFORM_NAME} architecture in use)

################################################################################
# Global configuration types
################################################################################

set(CMAKE_CONFIGURATION_TYPES
    Debug
    Release
    CACHE STRING "" FORCE
)

################################################################################
# Use solution folders feature
################################################################################

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Sub-projects
################################################################################

#Main Projects
ADD_SUBDIRECTORY(Vulkan)
