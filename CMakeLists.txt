cmake_minimum_required(VERSION 3.16)
project(CampusGuide VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
if(NOT Qt6_FOUND)
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
    set(QT_VERSION_MAJOR 5)
endif()

set(SOURCES
    main.cpp
    graph.cpp
    mainwindow.cpp
    graphwidget.cpp
)

set(HEADERS
    graph.h
    mainwindow.h
    graphwidget.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets)
else()
    target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets)
endif()

