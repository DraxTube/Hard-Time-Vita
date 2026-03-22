cmake_minimum_required(VERSION 3.10)

if(DEFINED ENV{VITASDK})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
else()
  message(FATAL_ERROR "VITASDK not set")
endif()

project(HardTime C CXX)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VITA_APP_NAME "Hard Time")
set(VITA_TITLEID  "HTIM00001")
set(VITA_VERSION  "01.00")

# vitaSDK toolchain does NOT automatically add arm-vita-eabi/include to -I flags
# (it uses --sysroot but CMake strips those from explicit -I).
# We add it directly here so psp2/ headers are found.
include_directories(BEFORE SYSTEM $ENV{VITASDK}/arm-vita-eabi/include)

set(SOURCES
    src/main.cpp        src/values.cpp      src/functions.cpp
    src/render3d.cpp    src/ai.cpp          src/anims.cpp
    src/players.cpp     src/data.cpp        src/gameplay.cpp
    src/menus.cpp       src/particles.cpp   src/crimes.cpp
    src/promos.cpp      src/moves.cpp       src/missions.cpp
    src/texts.cpp       src/costume.cpp     src/editor.cpp
    src/bullets.cpp     src/world_props.cpp src/screen_fade.cpp
    src/vita_audio.cpp
    third_party/ds3_loader.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    third_party
    $ENV{VITASDK}/arm-vita-eabi/include/vitaGL
    $ENV{VITASDK}/arm-vita-eabi/include/glm
)

target_link_libraries(${PROJECT_NAME}
    vitaGL vitashark SceShaccCgExt taihen_stub
    SceKernelDmacMgr_stub SceDisplay_stub
    SceGxm_stub
    SceSysmodule_stub SceCtrl_stub SceTouch_stub
    SceAudio_stub SceAudioIn_stub SceIme_stub
    SceNet_stub SceNetCtl_stub ScePower_stub
    SceCommonDialog_stub SceAppMgr_stub SceAppUtil_stub
    SceMotion_stub SceShaccCg_stub mathneon m z c
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
)
