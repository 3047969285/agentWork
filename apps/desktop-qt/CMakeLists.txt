cmake_minimum_required(VERSION 3.21)
project(dsh-desktop VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Network Test)

qt_add_executable(dsh-desktop
  src/main.cpp
)

qt_add_qml_module(dsh-desktop
  URI dsh
  VERSION 1.0
  QML_FILES
    src/qml/Main.qml
)

target_link_libraries(dsh-desktop PRIVATE Qt6::Quick Qt6::Network)
