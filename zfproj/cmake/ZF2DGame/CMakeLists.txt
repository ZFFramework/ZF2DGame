cmake_minimum_required(VERSION 3.16)
project(ZF2DGame)
set(ZF_ROOT_PATH "${PROJECT_SOURCE_DIR}/../../../../../../ZFFramework")
include("${ZF_ROOT_PATH}/tools/spec/cmake/util.cmake")
add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG>")

include("${PROJECT_SOURCE_DIR}/zfprojConfig.cmake" OPTIONAL)

zfprojSrcFiles(
    ZF_SRC_FILES
    "${CMAKE_BINARY_DIR}/zfgensrc/zfgensrc_ZF2DGame.cpp"
    "${ZF_ROOT_PATH}"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/../../.."
    )
if(COMMAND zfprojConfigBefore_ZF2DGame)
    zfprojConfigBefore_ZF2DGame(ZF2DGame ZF_SRC_FILES)
endif()
add_library(ZF2DGame SHARED ${ZF_SRC_FILES})
zfprojStripFILE(ZF2DGame)
set_target_properties(ZF2DGame PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(ZF2DGame PUBLIC ZFLIB_ZF2DGame=ZF_ENV_EXPORT)
if(COMMAND zfprojConfigAfter_ZF2DGame)
    zfprojConfigAfter_ZF2DGame(ZF2DGame)
endif()

target_include_directories(ZF2DGame PRIVATE "${PROJECT_SOURCE_DIR}/../../../zfsrc")

function(ZFAddLib ZF_LIB_NAME_)
    target_include_directories(ZF2DGame PRIVATE "${ZF_ROOT_PATH}/_release/cmake/all/include")
    target_link_directories(ZF2DGame PUBLIC "${ZF_ROOT_PATH}/_release/cmake/module/${ZF_LIB_NAME_}/lib")
    target_link_libraries(ZF2DGame PRIVATE ${ZF_LIB_NAME_})
endfunction()

ZFAddLib(ZFCore)
ZFAddLib(ZFUtility)
ZFAddLib(ZFAlgorithm)
ZFAddLib(ZFUIKit)
ZFAddLib(ZFUIWidget)

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/../../../zfsrc/"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
    )
install(
    TARGETS ZF2DGame
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
    )
zfprojExec("${ZF_ROOT_PATH}/tools/util/copy_res" "${PROJECT_SOURCE_DIR}/../../../zfres/" "${CMAKE_INSTALL_PREFIX}/bin/zfres")

