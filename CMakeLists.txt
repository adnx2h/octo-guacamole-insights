cmake_minimum_required(VERSION 3.16)

project(Chess VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appChess
    Main.cpp
)

qt_add_qml_module(appChess
    URI Chess
    VERSION 1.0
    QML_FILES qml/InitialScreen.qml
    QML_FILES qml/AnalysisScreen.qml
    QML_FILES qml/board_utils.js
    RESOURCES images/bB.png images/bK.png images/bN.png images/bP.png images/bQ.png images/bR.png images/empty.png images/wB.png images/wK.png images/wN.png images/wP.png images/wQ.png images/wR.png
    SOURCES Middleware/engineHandler/EngineHandler.h Middleware/engineHandler/EngineHandler.cpp
    SOURCES Middleware/engineHandler/EngineTypes.h
    QML_FILES qml/Main.qml
    RESOURCES qml.qrc
    SOURCES Middleware/engineHandler/MovesListModel.h
    SOURCES Middleware/engineHandler/MovesListModel.cpp
    QML_FILES qml/ChessBoard.qml


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
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appChess
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
