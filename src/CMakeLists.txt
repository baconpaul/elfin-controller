
cmake_minimum_required(VERSION 3.27)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0063 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
# https://github.com/taocpp/PEGTL/issues/347 would let us go to 13
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14 CACHE STRING "Minimum macOS version")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(elfin-controller VERSION 0.1.0)

include ("../cmake/CPM.cmake")

add_subdirectory(libs/JUCE)
add_subdirectory(libs/clap-juce-extensions)
add_subdirectory(libs/sst/sst-jucegui)

list(APPEND EWCO_FORMATS VST3)
list(APPEND EWCO_FORMATS AU)
list(APPEND EWCO_FORMATS Standalone)
if (UNIX AND NOT APPLE)
  # I know it works on other platforms but demand is low there
  list(APPEND EWCO_FORMATS LV2)
endif()

set(EWCO_PRODUCT_NAME "Airwindows Consolidated")

juce_add_plugin(${PROJECT_NAME}
  PRODUCT_NAME ${EWCO_PRODUCT_NAME}
  COMPANY_NAME "Hideaway Studios"
  COMPANY_WEBSITE ""
  BUNDLE_ID ""
  PLUGIN_MANUFACTURER_CODE HdWy
  PLUGIN_CODE elCo

  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT TRUE
  IS_MIDI_EFFECT TRUE

  COPY_PLUGIN_AFTER_BUILD ${COPY_PLUGIN_AFTER_BUILD}

  # LV2_URI https://airwindows.com/lv2/airwindows-all
  # LV2_SHARED_LIBRARY_NAME AirwindowsALL

  FORMATS ${EWCO_FORMATS}
  )


target_compile_definitions(${PROJECT_NAME} PUBLIC
        JUCE_ALLOW_STATIC_NULL_VARIABLES=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1

        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CAMERA=disabled

        JUCE_DISPLAY_SPLASH_SCREEN=0
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
  )

target_link_libraries(${PROJECT_NAME} PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors

)

clap_juce_extensions_plugin(TARGET ${PROJECT_NAME}
    CLAP_ID "org.hideawaystudios.elfincontroller"
    CLAP_PROCESS_EVENTS_RESOLUTION_SAMPLES TRUE
    CLAP_FEATURES
      "note-effect"
      )
