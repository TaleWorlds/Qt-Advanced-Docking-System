cmake_minimum_required(VERSION 3.5)
project(ads_example_framelessfloatingwidgets VERSION ${VERSION_SHORT}) 
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} 5.5 COMPONENTS Core Gui Widgets REQUIRED)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_executable(FramelessFloatingWidgetsExample WIN32 
    main.cpp
    mainwindow.cpp
    mainwindow.ui
)
target_include_directories(FramelessFloatingWidgetsExample PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../src")
target_link_libraries(FramelessFloatingWidgetsExample PRIVATE qt${QT_VERSION_MAJOR}advanceddocking)
target_link_libraries(FramelessFloatingWidgetsExample PUBLIC Qt${QT_VERSION_MAJOR}::Core 
                                                  Qt${QT_VERSION_MAJOR}::Gui 
                                                  Qt${QT_VERSION_MAJOR}::Widgets)
set_target_properties(FramelessFloatingWidgetsExample PROPERTIES 
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    VERSION ${VERSION_SHORT}
    EXPORT_NAME "Qt Advanced Docking System Frameless Floating Widgets Example"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/bin"
)
