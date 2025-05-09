import QtQuick 2.0
import QtQuick.Controls 2.5
import "board_utils.js" as BoardUtils
import QtQuick.Layouts 1

Item {
    id: id_InitialScreen
    width: 500
    height: Screen.height
    visible: false

    signal sgnBtnAnalysisClicked()

    Column {
        anchors.fill: parent
        spacing: 10
        // padding: 10

        TextArea {
            id: id_pgnInput
            width: parent.width - 20
            height: 200
            placeholderText: qsTr("Paste Chess PGN here...")
            color: "gray" // Color of the input text
            background: Rectangle {
                color: "white"
            }
        }

        Button {
            id: idBtn_PGNAnalysis
            text: qsTr("Start Analysis")
            onClicked: {
                console.log("Analyzing PGN:")
                sgnBtnAnalysisClicked()
                id_engineHandler.parsePgn(id_pgnInput.text)
            }
        }
    }
}
