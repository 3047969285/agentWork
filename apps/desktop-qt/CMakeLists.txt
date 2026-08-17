cmake_minimum_required(VERSION 3.21)
project(dsh-desktop VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 Network Test)
qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_add_executable(dsh-desktop
  src/main.cpp
  src/app/Application.cpp
  src/app/WindowController.cpp
  src/hooks/ConnectionHook.cpp
  src/hooks/StudyHook.cpp
  src/utils/StudyJson.cpp
  src/services/host/HostProcess.cpp
)

target_include_directories(dsh-desktop PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

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
    src/components/chrome/Sidebar.qml
    src/components/chat/ConversationPane.qml
    src/components/chat/Composer.qml
    src/pages/shell/MainShell.qml
)

target_link_libraries(dsh-desktop PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::Network)

if (WIN32)
  set(_dsh_windeployqt "${Qt6_DIR}/../../../bin/windeployqt.exe")
  cmake_path(NORMAL_PATH _dsh_windeployqt)
  add_custom_command(TARGET dsh-desktop POST_BUILD
    COMMAND "${_dsh_windeployqt}"
            --$<IF:$<CONFIG:Debug>,debug,release>
            --no-compiler-runtime
            --no-translations
            --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/src"
            "$<TARGET_FILE:dsh-desktop>"
    COMMENT "Deploy Qt runtime beside dsh-desktop (no vcredist dialog)"
  )
endif()

# --- rpc sources added by Task 5 agent ---
include(cmake/RpcSources.cmake)

# --- host test added by Task 4 ---
qt_add_executable(tst_host_args
  tests/tst_host_args.cpp
  src/services/host/HostProcess.cpp
)
target_include_directories(tst_host_args PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(tst_host_args PRIVATE Qt6::Core Qt6::Network Qt6::Test)
add_test(NAME tst_host_args COMMAND tst_host_args)

qt_add_executable(tst_study_json
  tests/tst_study_json.cpp
  src/utils/StudyJson.cpp
)
target_include_directories(tst_study_json PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(tst_study_json PRIVATE Qt6::Core Qt6::Test)
add_test(NAME tst_study_json COMMAND tst_study_json)

enable_testing()
