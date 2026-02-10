cmake_minimum_required(VERSION 3.18.1)

if (NOT DEFINED MODULE_NAME)
    message(FATAL_ERROR "MODULE_NAME is not set")
else ()
    project(${MODULE_NAME})
endif ()

message("Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 20)

set(LINKER_FLAGS "-ffixed-x18 -Wl,--hash-style=both")
set(C_FLAGS "-Werror=format -fdata-sections -ffunction-sections")
set(CXX_FLAGS "${CXX_FLAGS} -fno-exceptions -fno-rtti")

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(C_FLAGS "${C_FLAGS} -O2 -fvisibility=hidden -fvisibility-inlines-hidden")
    set(LINKER_FLAGS "${LINKER_FLAGS} -Wl,-exclude-libs,ALL -Wl,--gc-sections -Wl,--strip-all")
else ()
    set(C_FLAGS "${C_FLAGS} -O0")
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_FLAGS} ${CXX_FLAGS}")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LINKER_FLAGS}")

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        xdl/include
        imgui
        imgui/backends
        menu
        il2cpp
        il2cpp/src
        service
)

aux_source_directory(xdl xdl-src)
aux_source_directory(imgui imgui-src)
aux_source_directory(imgui/backends imgui-backends-src)
aux_source_directory(menu menu-src)
aux_source_directory(il2cpp il2cpp-src)

add_library(${MODULE_NAME} SHARED
        main.cpp
        loader.cpp
        ${xdl-src}
        ${imgui-src}
        ${imgui-backends-src}
        ${menu-src}
        ${il2cpp-src})

find_library(EGL_LIB EGL)
find_library(android android)
find_library(LZ_LIB z)

if (${ANDROID_PLATFORM_LEVEL} LESS 12)
    message(FATAL_ERROR "OpenGL 2 is not supported before API level 11")
    return()
elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
    add_definitions("-DDYNAMIC_ES3")
    find_library(OPENGL_LIB GLESv2)
else ()
    find_library(OPENGL_LIB GLESv3)
endif ()

add_subdirectory(dobby)

add_library(service_static STATIC IMPORTED)

set_target_properties(service_static PROPERTIES IMPORTED_LOCATION
    ${CMAKE_CURRENT_SOURCE_DIR}/service/${ANDROID_ABI}/libservice.a
)

target_link_libraries(${MODULE_NAME} log dobby_static service_static ${android} ${OPENGL_LIB} ${EGL_LIB} ${LZ_LIB})

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
            COMMAND ${CMAKE_STRIP} --strip-all --remove-section=.comment "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${MODULE_NAME}.so")
endif ()

