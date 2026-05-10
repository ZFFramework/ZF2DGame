cmake_minimum_required(VERSION 3.16)
project(ZF2DGame)
set(ZF_ROOT_PATH "${PROJECT_SOURCE_DIR}/../../../../../../../ZFFramework")
include("${ZF_ROOT_PATH}/tools/spec/cmake/util.cmake")

include("${PROJECT_SOURCE_DIR}/zfprojConfig.cmake" OPTIONAL)

zfprojSrcFiles(
    ZF_SRC_FILES
    "${CMAKE_BINARY_DIR}/zfgensrc/zfgensrc_ZF2DGame.cpp"
    "${ZF_ROOT_PATH}"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/../../../.."
    )
if(COMMAND zfprojConfigBefore_ZF2DGame)
    zfprojConfigBefore_ZF2DGame(ZF2DGame ZF_SRC_FILES)
endif()
add_library(ZF2DGame SHARED ${ZF_SRC_FILES})
target_compile_options(ZF2DGame PRIVATE "$<$<CONFIG:DEBUG>:-DDEBUG>")
zfprojStripFILE(ZF2DGame)
set_target_properties(ZF2DGame PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(ZF2DGame PUBLIC ZFLIB_ZF2DGame=ZF_ENV_EXPORT)
if(COMMAND zfprojConfigAfter_ZF2DGame)
    zfprojConfigAfter_ZF2DGame(ZF2DGame)
endif()

target_include_directories(ZF2DGame PRIVATE "${PROJECT_SOURCE_DIR}/../../../../zfsrc")

find_library(log-lib log)
find_library(android-lib android)
target_link_libraries(ZF2DGame PRIVATE ${android-lib} ${log-lib})

function(ZFAddLib ZF_LIB_NAME_)
    target_include_directories(ZF2DGame PRIVATE "${ZF_ROOT_PATH}/_release/Android/all/src/main/cpp/include")
    add_library(${ZF_LIB_NAME_} SHARED IMPORTED)
    set_target_properties(${ZF_LIB_NAME_} PROPERTIES IMPORTED_LOCATION "${ZF_ROOT_PATH}/_release/Android/all/src/main/jniLibs/${ANDROID_ABI}/lib${ZF_LIB_NAME_}.so")
    target_link_libraries(ZF2DGame PRIVATE ${ZF_LIB_NAME_})
endfunction()

ZFAddLib(ZFCore)
ZFAddLib(ZFUtility)
ZFAddLib(ZFAlgorithm)
ZFAddLib(ZFUIKit)
ZFAddLib(ZFUIWidget)

