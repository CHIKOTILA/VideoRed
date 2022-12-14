cmake_minimum_required(VERSION 3.14)
project(VideoRed C CXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/glfw)
add_subdirectory(lib/FFmpeg)

add_definitions(-DGL_SILENCE_DEPRECATION)

list(APPEND SOURCES
        main.cpp
        load_frame.cpp

        video_read.cpp
        video_read.h
        )
add_executable(VideoRed MACOSX_BUNDLE WIN32 ${SOURCES} )
if(APPLE)


    list(APPEND EXTRA_LIBS
            "-framework OpenGL"
            )
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/MacOSXBundleInfo.plist.in
                   ${CMAKE_CURRENT_BINARY_DIR}/assets/MacOSXBundleInfo.plist
            )

    set_target_properties(VideoRed PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/assets/MacOSXBundleInfo.plist
            )
elseif(WIN32)

    list(APPEND EXTRA_LIBS
            "-lglu32 -lopendl32"
            )
    set(CMAKE_EXE_LINKER_FLAGS "-std=gnu99 -static-libgcc -static-libstdc++ -mwindows")
else()

    list(APPEND EXTRA_LIBS
            "-lGL -lGLU -lX11"
            )
endif()




target_link_libraries(VideoRed FFmpeg glfw ${EXTRA_LIBS})