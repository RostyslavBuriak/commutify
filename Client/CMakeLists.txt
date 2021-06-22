cmake_minimum_required(VERSION 3.5)

project(commutify LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 sqlite3 COMPONENTS Widgets Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network REQUIRED)

set(PROJECT_SOURCES
        src/main.cpp
        src/main_window.cpp
        src/main_window.h
        src/input_field.cpp
        src/input_field.h
        src/message_box.cpp
        src/message_box.h
        src/dialog_box.cpp
        src/dialog_box.h
        src/scrollarea_widget.cpp
        src/scrollarea_widget.h
        src/database.cpp
        src/database.h
        src/login.cpp
        src/login.h
        src/connection.cpp
        src/connection.h
        src/package.cpp
        src/package.h
        src/networktypes.h
)

qt_add_resources(PROJECT_SOURCES resources.qrc)


qt_add_executable(commutify ${PROJECT_SOURCES})


target_link_libraries(commutify PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network PUBLIC sqlite3)
