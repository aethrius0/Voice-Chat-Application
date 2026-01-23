cmake_minimum_required(VERSION 3.16)

project(VoiceChat VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 veya Qt5 bul
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia Network)

# Boost.Asio
include_directories("C:/local/boost_1_89_0_b1")

# Kaynak dosyaları - TEK EXE (Client + Embedded Server)
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    server/voiceserver.cpp
    server/voiceserver.h
)

# Tek executable - hem client hem server içeriyor
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(VoiceChat
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(VoiceChat
        ${PROJECT_SOURCES}
    )
endif()

# Qt kütüphanelerini linkle
target_link_libraries(VoiceChat
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Multimedia
        Qt${QT_VERSION_MAJOR}::Network
        ws2_32
)

set_target_properties(VoiceChat PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(VoiceChat)
endif()
