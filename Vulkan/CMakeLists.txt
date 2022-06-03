set(PROJECT_NAME Vulkan)

################################################################################
# Source groups
################################################################################

set(Headers
  "../externals/imggui/imconfig.h"
  "../externals/imggui/imgui.h"
  "../externals/imggui/imgui_impl_glfw.h"
  "../externals/imggui/imgui_impl_vulkan.h"
  "../externals/imggui/imgui_internal.h"
  "../externals/imggui/imstb_rectpack.h"
  "../externals/imggui/imstb_textedit.h"
  "../externals/imggui/imstb_truetype.h"
  "Mesh.h"
  "Utilites.h"
  "VulkanRenderer.h"
)

set(Sources
  "../externals/imggui/imgui.cpp"
  "../externals/imggui/imgui_demo.cpp"
  "../externals/imggui/imgui_draw.cpp"
  "../externals/imggui/imgui_impl_glfw.cpp"
  "../externals/imggui/imgui_impl_vulkan.cpp"
  "../externals/imggui/imgui_tables.cpp"
  "../externals/imggui/imgui_widgets.cpp"
  "Main.cpp"
  "Mesh.cpp"
  "VulkanRenderer.cpp"
)


set(ALL_FILES
  ${Headers}
  ${Sources}
)

################################################################################
# Target
################################################################################

add_executable(${PROJECT_NAME} ${ALL_FILES} )

set_target_properties(${PROJECT_NAME} PROPERTIES
  #OUTPUT_NAME Game
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/Release
  VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

################################################################################
# Flags
################################################################################

if(MSVC)
  #VS multithread compilation flag
  target_compile_options(${PROJECT_NAME} PRIVATE /MP)
endif()

#target_compile_definitions(${PROJECT_NAME} PRIVATE _AFXDLL)
message( ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/.
  ${CMAKE_CURRENT_SOURCE_DIR}/../externals/imggui
  ${CMAKE_CURRENT_SOURCE_DIR}/../externals/GLFW/GLFW/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../externals/glm/glm
  C:/VulkanSDK/1.3.204.1/Include

)

target_link_libraries(${PROJECT_NAME}
${CMAKE_CURRENT_SOURCE_DIR}/../externals/GLFW/GLFW/lib-vc2022/glfw3.lib
C:/VulkanSDK/1.3.204.1/Lib32/vulkan-1.lib
)