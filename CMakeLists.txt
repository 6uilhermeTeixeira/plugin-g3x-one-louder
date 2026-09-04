cmake_minimum_required(VERSION 3.25)
project(G3XOneLouder VERSION 0.1.0 LANGUAGES C CXX)
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

juce_add_plugin(G3XOneLouder
    COMPANY_NAME "G3X Audio"
    BUNDLE_ID "com.g3xaudio.onelouder"
    PLUGIN_MANUFACTURER_CODE G3xA
    PLUGIN_CODE LoGx
    FORMATS VST3 Standalone
    PRODUCT_NAME "G3X One Louder"
    DESCRIPTION "One-control low-level compressor and peak maximizer"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx Dynamics
)

target_sources(G3XOneLouder PRIVATE Source/PluginEditor.cpp Source/PluginProcessor.cpp)
target_include_directories(G3XOneLouder PRIVATE Source)
target_compile_definitions(G3XOneLouder PUBLIC
    JUCE_REPORT_APP_USAGE=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
)
target_link_libraries(G3XOneLouder PRIVATE G3XOneLouderDsp juce::juce_audio_utils PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

add_library(G3XOneLouderDsp STATIC Source/LouderEngine.cpp Source/dsp/LouderDynamics.cpp)
set_target_properties(G3XOneLouderDsp PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(G3XOneLouderDsp PUBLIC Source)
if(MSVC)
    target_compile_options(G3XOneLouderDsp PRIVATE /W4 /permissive-)
else()
    target_compile_options(G3XOneLouderDsp PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

enable_testing()
add_executable(G3XOneLouderDspTests tests/LouderTests.cpp)
target_link_libraries(G3XOneLouderDspTests PRIVATE G3XOneLouderDsp)
add_test(NAME G3XOneLouderDspTests COMMAND G3XOneLouderDspTests)

add_executable(G3XOneLouderProcessorTests tests/ProcessorTests.cpp)
target_include_directories(G3XOneLouderProcessorTests PRIVATE Source)
target_compile_features(G3XOneLouderProcessorTests PRIVATE cxx_std_20)
target_compile_definitions(G3XOneLouderProcessorTests PRIVATE JUCE_USE_CURL=0 JUCE_WEB_BROWSER=0)
target_link_libraries(G3XOneLouderProcessorTests PRIVATE G3XOneLouder juce::juce_audio_utils)
add_test(NAME G3XOneLouderProcessorTests COMMAND G3XOneLouderProcessorTests)
