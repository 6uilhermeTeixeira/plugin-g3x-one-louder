cmake_minimum_required(VERSION 3.25)
project(G3XLouder VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.15
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(G3XLouder
    COMPANY_NAME "G3X Audio"
    BUNDLE_ID "com.g3xaudio.louder"
    PLUGIN_MANUFACTURER_CODE G3xA
    PLUGIN_CODE LoGx
    FORMATS VST3 Standalone
    PRODUCT_NAME "G3X Louder"
    DESCRIPTION "One-control low-level compressor and peak maximizer"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx Dynamics
)

target_sources(G3XLouder PRIVATE Source/PluginEditor.cpp Source/PluginProcessor.cpp)
target_include_directories(G3XLouder PRIVATE Source)
target_compile_definitions(G3XLouder PUBLIC
    JUCE_REPORT_APP_USAGE=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
)
target_link_libraries(G3XLouder PRIVATE G3XLouderDsp juce::juce_audio_utils PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

add_library(G3XLouderDsp STATIC Source/LouderEngine.cpp Source/dsp/LouderDynamics.cpp)
set_target_properties(G3XLouderDsp PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(G3XLouderDsp PUBLIC Source)
if(MSVC)
    target_compile_options(G3XLouderDsp PRIVATE /W4 /permissive-)
else()
    target_compile_options(G3XLouderDsp PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

enable_testing()
add_executable(G3XLouderDspTests tests/LouderTests.cpp)
target_link_libraries(G3XLouderDspTests PRIVATE G3XLouderDsp)
add_test(NAME G3XLouderDspTests COMMAND G3XLouderDspTests)

add_executable(G3XLouderProcessorTests tests/ProcessorTests.cpp)
target_include_directories(G3XLouderProcessorTests PRIVATE Source)
target_compile_features(G3XLouderProcessorTests PRIVATE cxx_std_20)
target_compile_definitions(G3XLouderProcessorTests PRIVATE JUCE_USE_CURL=0 JUCE_WEB_BROWSER=0)
target_link_libraries(G3XLouderProcessorTests PRIVATE G3XLouder juce::juce_audio_utils)
add_test(NAME G3XLouderProcessorTests COMMAND G3XLouderProcessorTests)
