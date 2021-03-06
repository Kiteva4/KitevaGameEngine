cmake_minimum_required(VERSION 3.8)
project(KGECore)

set(HDRS
    include/core.h
    include/graphic/KGEVulkanApp.h
    include/graphic/KGEVulkan.h
    include/graphic/KGEVulkanCore.h
    include/graphic/VulkanWindowControl/GLFWWindowControl.h
    include/graphic/VulkanWindowControl/IVulkanWindowControl.h
    include/graphic/VulkanWindowControl/LinuxXCBWindowControl.h
    include/graphic/VulkanWindowControl/MacOSWindowControl.h
    include/graphic/VulkanWindowControl/WindowsWindowControl.h
    include/graphic/VulkanCoreModules/KGEVkInstance.h
    include/graphic/VulkanCoreModules/KGEVkSurface.h
    include/graphic/VulkanCoreModules/KGEVkDevice.h
    include/graphic/VulkanCoreModules/KGEVkRenderPass.h
    include/graphic/VulkanCoreModules/KGEVkSwapChain.h
    include/graphic/VulkanCoreModules/KGEVkCommandBuffer.h
    include/graphic/VulkanCoreModules/KGEVkGraphicsPipeline.h
    include/graphic/VulkanCoreModules/KGEVkPipelineLayout.h
    include/graphic/VulkanCoreModules/KGEVkCommandPool.h
    include/graphic/VulkanCoreModules/KGEVkUniformBufferWorld.h
    include/graphic/VulkanCoreModules/KGEVkUniformBufferModels.h
    include/graphic/VulkanCoreModules/KGEVkDescriptorPool.h
    include/graphic/VulkanCoreModules/KGEVkDescriptorSetLayout.h
    include/graphic/VulkanCoreModules/KGEVkSampler.h
    include/graphic/VulkanCoreModules/KGEVkDescriptorSet.h
    include/graphic/VulkanCoreModules/KGEVkUboModels.h
    include/graphic/VulkanCoreModules/KGEVkSynchronization.h
    include/graphic/VulkanCoreModules/KGEVkReportCallBack.h
    include/stb/stb_image.h
    include/application/KGEAppData.h
    )

set(SRCS
    src/core.cpp
    src/graphic/KGEVulkanApp.cpp
    src/graphic/KGEVulkan.cpp
    src/graphic/KGEVulkanCore.cpp
    src/graphic/VulkanWindowControl/GLFWWindowControl.cpp
    src/graphic/VulkanWindowControl/LinuxXCBWindowControl.cpp
    src/graphic/VulkanWindowControl/MacOSWindowControl.cpp
    src/graphic/VulkanWindowControl/WindowsWindowControl.cpp
    src/graphic/VulkanCoreModules/KGEVkInstance.cpp
    src/graphic/VulkanCoreModules/KGEVkSurface.cpp
    src/graphic/VulkanCoreModules/KGEVkDevice.cpp
    src/graphic/VulkanCoreModules/KGEVkRenderPass.cpp
    src/graphic/VulkanCoreModules/KGEVkSwapChain.cpp
    src/graphic/VulkanCoreModules/KGEVkCommandBuffer.cpp
    src/graphic/VulkanCoreModules/KGEVkGraphicsPipeline.cpp
    src/graphic/VulkanCoreModules/KGEVkPipelineLayout.cpp
    src/graphic/VulkanCoreModules/KGEVkCommandPool.cpp
    src/graphic/VulkanCoreModules/KGEVkUniformBufferWorld.cpp
    src/graphic/VulkanCoreModules/KGEVkUniformBufferModels.cpp
    src/graphic/VulkanCoreModules/KGEVkDescriptorPool.cpp
    src/graphic/VulkanCoreModules/KGEVkDescriptorSetLayout.cpp
    src/graphic/VulkanCoreModules/KGEVkSampler.cpp
    src/graphic/VulkanCoreModules/KGEVkDescriptorSet.cpp
    src/graphic/VulkanCoreModules/KGEVkUboModels.cpp
    src/graphic/VulkanCoreModules/KGEVkSynchronization.cpp
    src/graphic/VulkanCoreModules/KGEVkReportCallBack.cpp
    src/application/KGEAppData.cpp
    )

add_library(${PROJECT_NAME} STATIC ${SRCS} ${HDRS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "lib")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".a")

include_directories(${PROJECT_SOURCE_DIR}/include/)
include_directories(${PROJECT_SOURCE_DIR}/src/)

include_directories(${CMAKE_SOURCE_DIR}/external)
#include_directories(${CMAKE_SOURCE_DIR}/external/glm/glm)
include_directories(${CMAKE_SOURCE_DIR}/external/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/engine/lib/include)

add_definitions(-DUNICODE)

target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${GLFW_LIBRARIES})

#VULKAN
if(WIN32)
  find_path(Vulkan_INCLUDE_DIR
    NAMES vulkan/vulkan.h
    PATHS
      "$ENV{VULKAN_SDK}/Include"
    )

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_library(Vulkan_LIBRARY
      NAMES vulkan-1
      PATHS
        "$ENV{VULKAN_SDK}/Lib"
        "$ENV{VULKAN_SDK}/Bin"
        )
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    find_library(Vulkan_LIBRARY
      NAMES vulkan-1
      PATHS
        "$ENV{VULKAN_SDK}/Lib32"
        "$ENV{VULKAN_SDK}/Bin32"
        NO_SYSTEM_ENVIRONMENT_PATH
        )
  endif()
else()
    find_path(Vulkan_INCLUDE_DIR
      NAMES vulkan/vulkan.h
      PATHS
        "$ENV{VULKAN_SDK}/include")
    find_library(Vulkan_LIBRARY
      NAMES vulkan
      PATHS
        "$ENV{VULKAN_SDK}/lib")
endif()

set(Vulkan_LIBRARIES ${Vulkan_LIBRARY})
set(Vulkan_INCLUDE_DIRS ${Vulkan_INCLUDE_DIR})

#include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vulkan
  DEFAULT_MSG
  Vulkan_LIBRARY Vulkan_INCLUDE_DIR)

mark_as_advanced(Vulkan_INCLUDE_DIR Vulkan_LIBRARY)

if(Vulkan_FOUND AND NOT TARGET Vulkan::Vulkan)
  add_library(Vulkan::Vulkan UNKNOWN IMPORTED)
  set_target_properties(Vulkan::Vulkan PROPERTIES
    IMPORTED_LOCATION "${Vulkan_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}")
endif()
#VULKAN_END

include_directories(${Vulkan_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} 
    ${Vulkan_LIBRARIES}
    )
target_include_directories(${PROJECT_NAME}
    PUBLIC ${Vulkan_INCLUDE_DIRS})


IF (NOT WIN32)
# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PKG_X11_XCB QUIET x11-xcb)

SET(X11_XCB_DEFINITIONS ${PKG_X11_XCB_CFLAGS})

FIND_PATH(X11_XCB_INCLUDE_DIR NAMES X11/Xlib-xcb.h HINTS ${PKG_X11_XCB_INCLUDE_DIRS})
FIND_LIBRARY(X11_XCB_LIBRARIES NAMES X11-xcb HINTS ${PKG_X11_XCB_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(X11_XCB DEFAULT_MSG X11_XCB_LIBRARIES X11_XCB_INCLUDE_DIR)

MARK_AS_ADVANCED(X11_XCB_INCLUDE_DIR X11_XCB_LIBRARIES)
ENDIF (NOT WIN32)
#X11_END

#XCB
IF (UNIX AND NOT APPLE)
find_package(PkgConfig)

if(NOT XCB_FIND_COMPONENTS)
    set(XCB_FIND_COMPONENTS xcb)
endif()

include(FindPackageHandleStandardArgs)
set(XCB_FOUND true)
set(XCB_INCLUDE_DIRS "")
set(XCB_LIBRARIES "")
foreach(comp ${XCB_FIND_COMPONENTS})
    # component name
    string(TOUPPER ${comp} compname)
    string(REPLACE "-" "_" compname ${compname})
    # header name
    string(REPLACE "xcb-" "" headername xcb/${comp}.h)
    # library name
    set(libname ${comp})

    pkg_check_modules(PC_${comp} QUIET ${comp})

    find_path(${compname}_INCLUDE_DIR NAMES ${headername}
        HINTS
        ${PC_${comp}_INCLUDEDIR}
        ${PC_${comp}_INCLUDE_DIRS}
        )

    find_library(${compname}_LIBRARY NAMES ${libname}
        HINTS
        ${PC_${comp}_LIBDIR}
        ${PC_${comp}_LIBRARY_DIRS}
        )

    find_package_handle_standard_args(${comp}
        FOUND_VAR ${comp}_FOUND
        REQUIRED_VARS ${compname}_INCLUDE_DIR ${compname}_LIBRARY)
    mark_as_advanced(${compname}_INCLUDE_DIR ${compname}_LIBRARY)

    list(APPEND XCB_INCLUDE_DIRS ${${compname}_INCLUDE_DIR})
    list(APPEND XCB_LIBRARIES ${${compname}_LIBRARY})

    if(NOT ${comp}_FOUND)
        set(XCB_FOUND false)
    endif()
endforeach()

list(REMOVE_DUPLICATES XCB_INCLUDE_DIRS)

include_directories(${XCB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}
    ${XCB_LIBRARIES}
    )

ENDIF (UNIX AND NOT APPLE)
#XCB_END

# If ANDROID is ON, turn on cross-compiling for itset(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(ANDROID)
    set(CMAKE_SYSTEM_NAME "Android")
    set(CMAKE_SYSTEM_VERSION "7")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DWIN32_LEAN_AND_MEAN)
    set(DisplayServer Win32)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (SAMPLES_WSI_SELECTION STREQUAL "XCB")
        if (NOT BUILD_WSI_XCB_SUPPORT)
            message( FATAL_ERROR "Selected XCB for samples build but not building Xcb support" )
        endif()
        add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
    elseif (SAMPLES_WSI_SELECTION STREQUAL "XLIB")
        if (NOT BUILD_WSI_XLIB_SUPPORT)
            message( FATAL_ERROR "Selected XLIB for samples build but not building Xlib support" )
        endif()
        add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)
    elseif (SAMPLES_WSI_SELECTION STREQUAL "WAYLAND")
        if (NOT BUILD_WSI_WAYLAND_SUPPORT)
            message( FATAL_ERROR "Selected Wayland for samples build but not building Wayland support" )
        endif()
        add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR)
    endif()
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()
