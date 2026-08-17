cmake_minimum_required(VERSION 3.21)
project(dsh-desktop VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 Network Test)

qt_add_executable(dsh-desktop
  src/main.cpp
)

set(_dsh_qml_singletons
  src/styles/tokens/InkTokens.qml
  src/styles/motion/MotionBudget.qml
)

set_source_files_properties(${_dsh_qml_singletons} PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(dsh-desktop
  URI dsh
  VERSION 1.0
  QML_FILES
    src/qml/Main.qml
    src/styles/tokens/InkTokens.qml
    src/styles/motion/MotionBudget.qml
    src/styles/themes/InkLight.qml
    src/components/ink/PaperBackground.qml
    src/components/ink/InkBloom.qml
    src/components/chrome/TitleBar.qml
    src/pages/shell/MainShell.qml
)

target_link_libraries(dsh-desktop PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::Network)

# --- rpc sources added by Task 5 agent ---
include(cmake/RpcSources.cmake)

enable_testing()
