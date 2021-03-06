cmake_minimum_required(VERSION 3.8)
project(camera)
set(TOPDIR /home/citos-android/data/myandroid)
set(PRODUCT nav132)
set(TARGET_OUT ${TOPDIR}/out/target/product/${PRODUCT})
set(CMAKE_SYSTEM Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CROSS_COMPILE ${TOPDIR}/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-)
set(CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)
set(CMAKE_FIND_ROOT_PATH ${TARGET_OUT}/obj)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

set(bootloader bootable/bootloader/legacy/include)
set(camera system/media/camera/include)
set(corecg external/skia/include/core)
set(frameworks-base frameworks/base/include)
set(frameworks-native frameworks/native/include)
set(graphics external/skia/include/core)
set(libc bionic/libc/include)
set(libhardware hardware/libhardware/include)
set(libhardware_legacy hardware/libhardware_legacy/include)
set(libhost build/libs/host/include)
set(libm bionic/libm/include)
set(libnativehelper libnativehelper/include)
set(libpagemap system/extras/libpagemap/include)
set(libril hardware/ril/include)
set(libstdc++ bionic/libstdc++/include)
set(libthread_db bionic/libthread_db/include)
set(mkbootimg system/core/mkbootimg)
set(opengl-tests-includes frameworks/native/opengl/tests/include)
set(recovery bootable/recovery)
set(system-core system/core/include)
set(audio-effects system/media/audio_effects/include)
set(audio-utils system/media/audio_utils/include)
set(audio-route system/media/audio_route/include)
set(wilhelm frameworks/wilhelm/include)
set(wilhelm-ut frameworks/wilhelm/src/ut)
set(speex external/speex/include)

#interl include
include_directories(SYSTEM ${TOPDIR}/system/core/include)
include_directories(SYSTEM ${TOPDIR}/hardware/libhardware/include)
include_directories(SYSTEM ${TOPDIR}/hardware/libhardware_legacy/include)
include_directories(SYSTEM ${TOPDIR}/hardware/ril/include)
include_directories(SYSTEM ${TOPDIR}/libnativehelper/include)
include_directories(SYSTEM ${TOPDIR}/frameworks/native/include)
include_directories(SYSTEM ${TOPDIR}/frameworks/native/opengl/include)
include_directories(SYSTEM ${TOPDIR}/frameworks/av/include)
include_directories(SYSTEM ${TOPDIR}/frameworks/base/include)
include_directories(SYSTEM ${TOPDIR}/external/skia/include)
include_directories(SYSTEM ${TOPDIR}/${libc})
include_directories(SYSTEM ${TARGET_OUT}/obj/include)
include_directories(SYSTEM ${TOPDIR}/bionic/libc/arch-arm/include)
include_directories(SYSTEM ${TOPDIR}/bionic/libc/kernel/common)
include_directories(SYSTEM ${TOPDIR}/bionic/libc/kernel/arch-arm)
include_directories(SYSTEM ${TOPDIR}/${libm})
include_directories(SYSTEM ${TOPDIR}/${libthread_db})

#external include
include_directories(${TOPDIR}/${libstdc++})
include_directories(${TOPDIR}/${graphics})
include_directories(${TOPDIR}/bionic)
include_directories(${TOPDIR}/external/stlport/stlport)
include_directories(.)
#internal defination
add_definitions(-DFORTIFY_SOURCE=2 -DANDROID -DOMX_SW_CODEC -DNDEBUG -D__ANDROID__ -D_FILE_OFFSET_BITS=64)

#external defination
add_definitions(-DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES -DBUILD_FOR_ANDROID -DGLM_FORCE_CXX98)

set(CMAKE_CXX_FLAGS "-c  -fno-exceptions -Wno-multichar -msoft-float -fpic -fPIE -ffunction-sections -fdata-sections -funwind-tables -fstack-protector -Wa,--noexecstack -Werror=format-security -fno-short-enums -march=armv7-a -mfloat-abi=softfp -mfpu=neon -include /home/citos-android/data/myandroid/build/core/combo/include/arch/linux-arm/AndroidConfig.h -I ${TOPDIR}/build/core/combo/include/arch/linux-arm/ -Wno-unused-but-set-variable -fno-builtin-sin -fno-strict-volatile-bitfields -Wno-psabi -mthumb-interwork  -fmessage-length=0 -W -Wall -Wno-unused -Winit-self -Wpointer-arith -Werror=return-type -Werror=non-virtual-dtor -Werror=address -Werror=sequence-point -g -Wstrict-aliasing=2 -fgcse-after-reload -frerun-cse-after-loop -frename-registers -DNDEBUG -UDEBUG -fvisibility-inlines-hidden -fmessage-length=0 -W -Wall -Wno-unused -Winit-self -Wpointer-arith -Wsign-promo -Werror=return-type -Werror=non-virtual-dtor -Werror=address -Werror=sequence-point -UDEBUG -mthumb -Os -fomit-frame-pointer -fno-strict-aliasing  -fno-rtti -MD -MF")

aux_source_directory(. SOURCE_FILES)

add_executable(camera ${SOURCE_FILES})

target_link_libraries(camera SHARED -lcutils -llog -landroidfw -lutils -lstlport -lbinder -lui -lskia -lEGL -lGLESv2 -lgui)