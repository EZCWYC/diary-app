cmake_minimum_required(VERSION 3.14)
project(DiaryApp VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)

include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
    TIMEOUT 60
)
FetchContent_MakeAvailable(glfw)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
FetchContent_Declare(
    glad
    URL https://github.com/libigl/libigl-glad/archive/refs/heads/master.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    TIMEOUT 30
)
FetchContent_MakeAvailable(glad)

find_package(OpenGL REQUIRED)

FetchContent_Declare(
    eui_neo
    GIT_REPOSITORY https://github.com/sudoevolve/EUI-NEO.git
    GIT_TAG main
    SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/eui_neo-src
    UPDATE_DISCONNECTED TRUE
)
FetchContent_GetProperties(eui_neo)
if(NOT eui_neo_POPULATED)
    FetchContent_Populate(eui_neo)
endif()

find_package(OpenSSL REQUIRED)

set(CORE_SOURCES
    ${eui_neo_SOURCE_DIR}/core/image.cpp
    ${eui_neo_SOURCE_DIR}/core/network.cpp
    ${eui_neo_SOURCE_DIR}/core/platform.cpp
    ${eui_neo_SOURCE_DIR}/core/text.cpp
)

set(DIARY_SOURCES
    diary/encrypt.cpp
    diary/diary_data.cpp
)

set(APP_ICON_RESOURCE)
if(WIN32)
    set(APP_ICON_ICO "${eui_neo_SOURCE_DIR}/assets/icon.ico")
    set(APP_ICON_RC "${CMAKE_CURRENT_BINARY_DIR}/eui_app_icon.rc")
    if(EXISTS "${APP_ICON_ICO}")
        file(TO_NATIVE_PATH "${APP_ICON_ICO}" APP_ICON_ICO_NATIVE)
        string(REPLACE "\\" "\\\\" APP_ICON_ICO_ESCAPED "${APP_ICON_ICO_NATIVE}")
        file(WRITE "${APP_ICON_RC}" "IDI_APP_ICON ICON \"${APP_ICON_ICO_ESCAPED}\"\n")
        set(APP_ICON_RESOURCE "${APP_ICON_RC}")
    endif()
endif()

add_executable(diary
    ${eui_neo_SOURCE_DIR}/main.cpp
    app/diary.cpp
    ${CORE_SOURCES}
    ${DIARY_SOURCES}
    ${APP_ICON_RESOURCE}
)

target_include_directories(diary PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${eui_neo_SOURCE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${glfw_SOURCE_DIR}/include
)

target_link_libraries(diary PRIVATE
    glfw
    glad
    OpenGL::GL
    OpenSSL::SSL
    OpenSSL::Crypto
    crypt32
    ws2_32
)

if(MSVC)
    target_compile_options(diary PRIVATE /utf-8 $<$<NOT:$<CONFIG:Debug>>:/O1> /GS- /sdl-)
    target_link_options(diary PRIVATE /OPT:REF /OPT:ICF /ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS)
else()
    target_compile_options(diary PRIVATE -Os)
    target_link_options(diary PRIVATE -Wl,--gc-sections -s)
endif()

if(WIN32)
    target_link_libraries(diary PRIVATE winmm urlmon shell32)
endif()

if(TARGET CURL::libcurl)
    target_link_libraries(diary PRIVATE CURL::libcurl)
    target_compile_definitions(diary PRIVATE EUI_HAS_CURL=1)
endif()

add_custom_command(TARGET diary POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${eui_neo_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:diary>/assets
)
