cmake_minimum_required(VERSION 3.16)

project(QtAudioRecorder VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 🔹 Qt6 veya Qt5 bul, Widgets + Multimedia modüllerini iste
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia Network)

# 🔹 Projenin kaynak dosyaları
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

# 🔹 Qt6 ise qt_add_executable, Qt5 ise normal add_executable
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(QtAudioRecorder
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(QtAudioRecorder
        ${PROJECT_SOURCES}
    )
endif()

# 🔹 Qt kütüphanelerini linkle (include path vs. buradan geliyor)
target_link_libraries(QtAudioRecorder
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Multimedia
        Qt${QT_VERSION_MAJOR}::Network
)

set_target_properties(QtAudioRecorder PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QtAudioRecorder)
endif()
