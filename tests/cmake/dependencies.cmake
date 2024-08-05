cmake_minimum_required(VERSION 3.10)

find_package(Threads REQUIRED)

include(ExternalProject)

set(source_dir "${CMAKE_BINARY_DIR}/libserial-src")
set(build_dir "${CMAKE_BINARY_DIR}/libserial-build")

EXTERNALPROJECT_ADD(
  libserial
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-serial.git
  GIT_TAG           main
  PATCH_COMMAND     ${patching_cmd}
  PREFIX            libserial-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
  BUILD_COMMAND     cd ${build_dir}/build && cmake -D BUILD_SHARED_LIBS=ON
                    ${source_dir} && make -j 4
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)

include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)

set(source_dir ${CMAKE_BINARY_DIR}/googletest-src)
set(build_dir ${CMAKE_BINARY_DIR}/googletest-build)
set(work_dir ${CMAKE_BINARY_DIR}/googletest-workspace)

EXTERNALPROJECT_ADD(
  googletest
  GIT_REPOSITORY    https://github.com/google/googletest.git
  GIT_TAG           main
  PATCH_COMMAND     ${patching_cmd}
  PREFIX            ${work_dir}
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CMAKE_ARGS        -D CMAKE_INSTALL_PREFIX=${work_dir} -D BUILD_SHARED_LIBS=ON
  UPDATE_COMMAND    ""
)

include_directories(${source_dir}/googletest/include)
include_directories(${source_dir}/googlemock/include)
link_directories(${build_dir}/lib)
