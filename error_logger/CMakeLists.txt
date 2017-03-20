cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(error_logger)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

include_directories(
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

if (NOT ${RELEASE})
  add_definitions(-DENABLE_LOGGING)
endif()

add_executable(error_logger
  error_logger.c
  glog.c
  mtable.c
  qsort.c
  stacktrace.c
)

target_link_libraries(error_logger
  gcc
  taihen_stub
  SceLibKernel_stub
  SceIofilemgr_stub
  SceKernelModulemgr_stub
  SceKernelThreadMgr_stub
  SceSysmemForDriver_stub
)

set_target_properties(error_logger
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(error_logger.suprx error_logger
  UNSAFE
  CONFIG ${CMAKE_SOURCE_DIR}/error_logger.yml
)

install(DIRECTORY ${CMAKE_BINARY_DIR}/error_logger-stubs/
  DESTINATION lib
  FILES_MATCHING PATTERN "*.a"
)