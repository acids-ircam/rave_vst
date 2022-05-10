set(torch_dir ${CMAKE_CURRENT_BINARY_DIR}/torch)

set(torch_lib_name torch)

set(torch_dir ${CMAKE_CURRENT_BINARY_DIR}/torch)
set(torch_lib_name torch)
find_library(torch_lib
  NAMES ${torch_lib_name}
  PATHS ${torch_dir}/libtorch/lib
)

if (DEFINED torch_version)
  message("setting torch version : ${torch_version}")
else()
  set(torch_version "1.11.0")
  message("torch version : ${torch_version}")
endif()


if (NOT torch_lib)
  message(STATUS "Downloading torch C API pre-built")

  if (UNIX AND APPLE AND ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "arm64")
    set(torch_url "https://anaconda.org/pytorch/pytorch/1.11.0.arm64/download/osx-arm64/pytorch-${torch_version}.arm64-py3.9_0.tar.bz2")
    file(DOWNLOAD
      ${torch_url}
      ${torch_dir}/torch_cc.tar.bz2
      SHOW_PROGRESS
    )
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf torch_cc.tar.bz2 WORKING_DIRECTORY ${torch_dir})
    file(REMOVE ${torch_dir}/torch_cc.tar.bz2)
  else()  # Not M1
    if (UNIX AND NOT APPLE)  # Linux
      set(torch_url "https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-${torch_version}%2Bcpu.zip")
    elseif (UNIX AND APPLE)  # OSX
      set(torch_url "https://download.pytorch.org/libtorch/cpu/libtorch-macos-${torch_version}.zip")
    else()                   # Windows
      set(torch_url "https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-${torch_version}%2Bcpu.zip")
    endif()
    file(DOWNLOAD
      ${torch_url}
      ${torch_dir}/torch_cc.zip
      SHOW_PROGRESS
    )
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf torch_cc.zip WORKING_DIRECTORY ${torch_dir})
    file(REMOVE ${torch_dir}/torch_cc.zip)
  endif()
endif()

# Find the libraries again
find_library(torch_lib
  NAMES ${torch_lib_name}
  PATHS ${torch_dir}/libtorch/lib
)
if (NOT torch_lib)
  message(FATAL_ERROR "torch could not be included")
endif()
