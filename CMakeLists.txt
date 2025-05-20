# vi:set sw=2 et:
cmake_minimum_required(VERSION 3.27)

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0063 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14 CACHE STRING "Minimum macOS version")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(elfin-controller VERSION 0.2.0)

option(ELFIN_COPY_AFTER_BUILD "Copy after Build" FALSE)

include (cmake/compile-options.cmake)

include(libs/sst/sst-plugininfra/cmake/git-version-functions.cmake)
version_from_versionfile_or_git()
message(STATUS "DISPLAY_VERSION=${GIT_IMPLIED_DISPLAY_VERSION}; COMMIT_HASH=${GIT_COMMIT_HASH}; BRANCH=${GIT_BRANCH}")

add_subdirectory(libs/JUCE)
add_subdirectory(libs/clap-juce-extensions)
add_subdirectory(libs/sst/sst-jucegui)
add_subdirectory(libs/sst/sst-plugininfra)


list(APPEND ELFCO_FORMATS VST3)
list(APPEND ELFCO_FORMATS AU)
list(APPEND ELFCO_FORMATS Standalone)

set(ELFCO_PRODUCT_NAME "Elfin Controller")
set(ELFCO_PLUGIN_NAME "The Elfin Controller")


include(cmake/CmakeRC.cmake)
file(GLOB_RECURSE CONTENT_GLOB  "resources/content/**")
cmrc_add_resource_library(${PROJECT_NAME}-content NAMESPACE elfin_content ${CONTENT_GLOB})


juce_add_plugin(${PROJECT_NAME}
  PRODUCT_NAME ${ELFCO_PRODUCT_NAME}
  PLUGIN_NAME ${ELFCO_PLUGIN_NAME}
  COMPANY_NAME "Hideaway Studio"
  COMPANY_WEBSITE "fill.in"
  BUNDLE_ID "fill.in"
  PLUGIN_MANUFACTURER_CODE HwSt
  PLUGIN_CODE elCo

  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT TRUE
  IS_MIDI_EFFECT TRUE

  COPY_PLUGIN_AFTER_BUILD ${ELFIN_COPY_AFTER_BUILD}

  VST3_AUTO_MANIFEST FALSE

  AU_SANDBOX_SAFE TRUE

  FORMATS ${ELFCO_FORMATS}
  )


target_compile_definitions(${PROJECT_NAME} PUBLIC
        JUCE_ALLOW_STATIC_NULL_VARIABLES=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1

        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CAMERA=disabled

        JUCE_REPORT_APP_USAGE=0

        JUCE_MODAL_LOOPS_PERMITTED=0

        JUCE_COREGRAPHICS_DRAW_ASYNC=1

        JUCE_ALSA=1
        JUCE_JACK=0

        JUCE_WASAPI=1
        JUCE_DIRECTSOUND=1

        JUCE_CATCH_UNHANDLED_EXCEPTIONS=0

        BUILD_HASH="${BUILD_HASH}"

        _USE_MATH_DEFINES=1
)
target_sources(${PROJECT_NAME} PRIVATE
  src/ElfinEditor.cpp
  src/ElfinProcessor.cpp
  src/ElfinMainPanel.cpp
  src/ElfinAbout.cpp
  src/ElfinKnob.cpp
  src/PresetManager.cpp
  src/configuration.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    sst-jucegui
    sst-plugininfra
    sst-plugininfra::version_information
    sst-plugininfra::filesystem
    sst-plugininfra::strnatcmp
    ${PROJECT_NAME}-content
)

clap_juce_extensions_plugin(TARGET ${PROJECT_NAME}
    CLAP_ID "org.hideaway.elfincontroller"
    CLAP_PROCESS_EVENTS_RESOLUTION_SAMPLES TRUE
    CLAP_FEATURES
      "note-effect"
     )

include(cmake/basic_installer.cmake)
