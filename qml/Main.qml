import QtQuick 2.15
import QtQuick 2.4
import QtQuick.Window 2.15
import QtQuick 2.0
import QtQuick.Controls 2.5
import "board_utils.js" as BoardUtils
import QtQuick.Layouts 1

ApplicationWindow {
    id: id_appWindow
    width: 500
    // height: Screen.height
    height: 1000
    visible: true
    title: qsTr("Chess PGN Analyzer")

    Item {
        id: id_mainScreen

        InitialScreen {
            id: id_InitialScreen
            visible: state = "st_initial" // Use state to control visibility
            onSgnBtnAnalysisClicked: {
                id_mainScreen.state = "st_analysisScreen"
                console.log("Analysis Screen")
            }
        }

        AnalysisScreen {
            id: id_AnalysisScreen
            onSgnBtnSettingsClicked: {
                console.log("Initial Screen")
                id_mainScreen.state = "id_InitialScreen"
            }
        }

        states: [
            State {
                name: "st_initialScreen"
                PropertyChanges { target: id_InitialScreen; visible: true }
                PropertyChanges { target: id_AnalysisScreen; visible: false }
            },
            State {
                name: "st_analysisScreen"
                PropertyChanges { target: id_InitialScreen; visible: false }
                PropertyChanges { target: id_AnalysisScreen; visible: true }
            }
        ]

        state: "st_initialScreen"
    }
}
