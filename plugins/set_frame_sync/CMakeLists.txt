cmake_minimum_required(VERSION 3.10)

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
    link_libraries(pthread)
endif()

add_library(set_frame_sync SHARED
        ${PROJECT_SOURCE_DIR}/plugins/set_frame_sync/SetFrameSync.cpp
)

if(UNIX)
    set_target_properties(set_frame_sync PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

if(WIN32)
    target_link_libraries(set_frame_sync PRIVATE
        "-static -static-libgcc -static-libstdc++ -lpthread"
        "C:/ControlApi/Release/Dll/ControlLib.lib"
        "C:/ControlApi/Release/Dll/ControlLib32.lib"
        "C:/ControlApi/Release/Dll/IntelControlLib.lib"
        "C:/ControlApi/Release/Dll/IntelControlLib32.lib"
    )
else()
    find_package(Threads REQUIRED)
    target_link_libraries(set_frame_sync PRIVATE Threads::Threads)
endif()

target_compile_definitions(set_frame_sync PRIVATE SET_FRAME_SYNC_EXPORTS)
target_include_directories(set_frame_sync PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src/interface "C:/ControlApi/Include" "C:/ControlApi/Samples/inc")
