cmake_minimum_required(VERSION 3.15)
project(rave-vst VERSION 0.0.1)
set (target-name ${PROJECT_NAME})

if(APPLE)
  set (formats AU VST3 Standalone)
else()
  set (formats VST3 Standalone)
endif()

##### Compilation options
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

include("cmake/compiler_settings.cmake")
setup_compilers()
set(CMAKE_CXX_FLAGS "-fPIC")
# I need to define CMAKE_CXX_STANDARD here and not in the setup_compilers function
# as it just does not work? Spent 40mins figuring out
# why the build wasn't working this way and it still makes no sense
set(CMAKE_CXX_STANDARD 20)

##### Helper function to add all images from a given folder into a JUCE binary data file
function(add_images_from_directory target imagesSubFolder)
    set (ImagesDir ${CMAKE_CURRENT_LIST_DIR}/${imagesSubFolder})
    file(GLOB_RECURSE images
            "${ImagesDir}/*.jpg"
            "${ImagesDir}/*.png"
            "${ImagesDir}/*.jpeg"
            # Adding fonts as well
            "${ImagesDir}/*.ttf"
            "${ImagesDir}/*.otf"
          )
    if(NOT images STREQUAL "")
        set (ImagesTarget "${target}-Images")
        juce_add_binary_data(${ImagesTarget} SOURCES ${images})
        target_link_libraries(${target} PRIVATE ${ImagesTarget})
    endif()
endfunction()

# set (FontsTarget "${target}-Fonts")
# juce_add_binary_data(${FontsTarget} SOURCES "${ImagesDir}/*.ttf")
# target_link_libraries(${target} PRIVATE ${FontsTarget})


##### Torch related
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(add_torch)
list(APPEND CMAKE_PREFIX_PATH "${torch_dir}/libtorch")
find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")


##### Juce related
option(JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)
option(JUCE_BUILD_EXAMPLES "Build JUCE Examples" OFF)
# As we're getting JUCE via git submodules and not using a possible system JUCE
add_subdirectory(juce)


##### `juce_add_console_app` adds an executable target with the name passed as the first argument
# (ConsoleAppExample here). This target is a normal CMake target, but has a lot of extra properties
# set up by default. This function accepts many optional arguments. Check the readme at
# `docs/CMake API.md` in the JUCE repo for the full list.
juce_add_plugin(${target-name}
    ICON_BIG assets/icon.png                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    ICON_SMALL assets/icon.png
    COMPANY_NAME acids                          # Specify the name of the plugin's author
    VST2_CATEGORY kPlugCategEffect
    VST3_CATEGORIES Fx Generator Synth
    PLUGIN_MANUFACTURER_CODE Acid              # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Rave                            # A unique four-character plugin id with exactly one upper-case character
    FORMATS ${formats}                     # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME "RAVE")        # The name of the final executable, which can differ from the target name

##### `juce_generate_juce_header` will create a JuceHeader.h for a given target, which will be generated
# into the build tree. This header should be included with `#include <JuceHeader.h>`. The include
# path for this header will be automatically added to the target. The main function of the
# JuceHeader is to include all the JUCE module headers for a particular target; if you're happy to
# include module headers directly, you probably don't need to call this.
juce_generate_juce_header(rave-vst)

##### `target_sources` adds source files to a target. We pass the target that needs the sources as the
# first argument, then a visibility parameter for the sources which should normally be PRIVATE.
# Finally, we supply a list of source files that will be built into the target. This is a standard
# CMake command.
add_subdirectory(source)
add_images_from_directory(${target-name} assets)

##### `target_compile_definitions` adds some preprocessor definitions to our target. In a Projucer
# project, these might be passed in the 'Preprocessor Definitions' field. JUCE modules also make use
# of compile definitions to switch certain features on/off, so if there's a particular feature you
# need that's not on by default, check the module header for the correct flag to set here. These
# definitions will be visible both to your code, and also the JUCE module code, so for new
# definitions, pick unique names that are unlikely to collide! This is a standard CMake command.
target_compile_definitions(${target-name}
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_console_app` call
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)    # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_console_app` call

# If the target needs extra binary assets, they can be added here. The first argument is the name of
# a new static library target that will include all the binary resources. There is an optional
# `NAMESPACE` argument that can specify the namespace of the generated binary data class. Finally,
# the SOURCES argument should be followed by a list of source files that should be built into the
# static library. These source files can be of any kind (wav data, images, fonts, icons etc.).
# Conversion to binary-data will happen when the target is built.

# juce_add_binary_data(ConsoleAppData SOURCES ...)

##### Automatically bundle libtorch into the targets
get_target_property(active_formats ${target-name} JUCE_FORMATS)
if (APPLE)
  set(libtorch_files
    "${TORCH_INSTALL_PREFIX}/lib/libtorch.dylib"
    "${TORCH_INSTALL_PREFIX}/lib/libtorch_cpu.dylib"
    "${TORCH_INSTALL_PREFIX}/lib/libiomp5.dylib"
    "${TORCH_INSTALL_PREFIX}/lib/libc10.dylib"
  )
endif (APPLE)
##### Unix
if (UNIX AND NOT APPLE)
  if (NOT EXISTS "${TORCH_INSTALL_PREFIX}/lib/libiomp5.so")
    message(STATUS "UNIX plateform detected: Downloading MKL shared libs")

    set(mkl_dir ${TORCH_INSTALL_PREFIX}/lib/)
    set(mkl_url "https://github.com/01org/mkl-dnn/releases/download/v0.9/mklml_lnx_2018.0.20170425.tgz")

    file(DOWNLOAD
      ${mkl_url}
      ${mkl_dir}/mkl_cc.tgz
      SHOW_PROGRESS
    )
    message(STATUS "extracting")

    execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf mkl_cc.tgz
                    WORKING_DIRECTORY ${mkl_dir})

    file(REMOVE ${mkl_dir}/mkl_cc.tgz)

    configure_file(
      "${TORCH_INSTALL_PREFIX}/lib/mklml_lnx_2018.0.20170425/lib/libiomp5.so"
      "${TORCH_INSTALL_PREFIX}/lib/libiomp5.so"
    )
  endif()

  set(libtorch_files
    "${TORCH_INSTALL_PREFIX}/lib/libtorch.so"
    "${TORCH_INSTALL_PREFIX}/lib/libtorch_cpu.so"
    "${TORCH_INSTALL_PREFIX}/lib/libc10.so"
    "${TORCH_INSTALL_PREFIX}/lib/libiomp5.so"
  )
endif (UNIX AND NOT APPLE)
##### Windows
if (MSVC)
  file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
  add_custom_command(TARGET target-name POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     ${TORCH_DLLS}
                     ${base_folder}/)
endif()


#####
foreach(kind IN LISTS active_formats)
    foreach(resource_file IN LISTS libtorch_files)
        target_sources(${target-name}_${kind} PRIVATE ${resource_file})
        set_source_files_properties(${resource_file} PROPERTIES
          MACOSX_PACKAGE_LOCATION "torch/libtorch"
        )
    endforeach()
endforeach()

# `target_link_libraries` links libraries and JUCE modules to other libraries or executables. Here,
# we're linking our executable target to the `juce::juce_core` module. Inter-module dependencies are
# resolved automatically. If you'd generated a binary data target above, you would need to link to
# it here too. This is a standard CMake command.
target_link_libraries(${target-name}
    PRIVATE
        # ConsoleAppData            # If you'd created a binary data target, you'd link to it here
        juce::juce_core
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_audio_utils
        juce::juce_audio_basics
        juce::juce_dsp
        torch
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

set(base_folder "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
