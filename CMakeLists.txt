cmake_minimum_required(VERSION 3.16)

project(Chess VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appChess
    Main.cpp
    qml.qrc
)

qt_add_qml_module(appChess
    URI Chess
    VERSION 1.0
    QML_FILES qml/InitialScreen.qml
    QML_FILES qml/AnalysisScreen.qml
    QML_FILES qml/board_utils.js
    QML_FILES qml/Main.qml
    QML_FILES qml/ChessBoard.qml
    RESOURCES qml.qrc
    SOURCES Middleware/boardHandler/BoardHandler.h Middleware/boardHandler/BoardHandler.cpp
    SOURCES Middleware/boardHandler/BoardTypes.h
    SOURCES Middleware/boardHandler/MovesListModel.h Middleware/boardHandler/MovesListModel.cpp
    SOURCES Middleware/engineHandler/EngineHandler.h Middleware/engineHandler/EngineHandler.cpp
    SOURCES Middleware/aiHandler/AiHandler.h Middleware/aiHandler/AiHandler.cpp
    SOURCES Middleware/externalLibraries/chess.hpp
)

if(ANDROID)
    set(CHESS_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../chess-library/build-android/include")
    set(STOCKFISH_BINARY "${CMAKE_CURRENT_LIST_DIR}/../Stockfish/stockfish-android-armv8-dotprod/stockfish/src/stockfish")
    if(NOT ANDROID_ABI STREQUAL "arm64-v8a")
        message(FATAL_ERROR "The Android Stockfish binary requires the arm64-v8a ABI")
    endif()
    if(NOT EXISTS "${STOCKFISH_BINARY}")
        message(FATAL_ERROR "Android Stockfish binary not found: ${STOCKFISH_BINARY}")
    endif()
    set(STOCKFISH_DEPLOY_BINARY "${CMAKE_CURRENT_BINARY_DIR}/libstockfish.so")
    configure_file("${STOCKFISH_BINARY}" "${STOCKFISH_DEPLOY_BINARY}" COPYONLY)
    set_target_properties(appChess PROPERTIES
        QT_ANDROID_EXTRA_LIBS "${STOCKFISH_DEPLOY_BINARY}"
        QT_ANDROID_LEGACY_PACKAGING TRUE
    )
else()
    set(CHESS_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/Middleware/externalLibraries")
endif()

target_include_directories(appChess PRIVATE
    ${CHESS_INCLUDE_DIR}
    Middleware/boardHandler
    Middleware/externalLibraries
    Middleware/engineHandler
    Middleware/aiHandler
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appChess PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appChess
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appChess
    PRIVATE Qt6::Quick Qt6::QuickControls2
)

include(GNUInstallDirs)
install(TARGETS appChess
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
