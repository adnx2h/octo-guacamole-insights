import QtQuick 2.0
import QtQuick.Controls 2.5
import "board_utils.js" as BoardUtils
import QtQuick.Layouts 1

Item {
    id: id_InitialScreen
    width: id_appWindow.width
    height: id_appWindow.height
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
            // placeholderText: qsTr("Paste Chess PGN here...")
            text: `[Event "Live Chess"]
            [Site "Chess.com"]
            [Date "2025.07.19"]
            [Round "?"]
            [White "ananb"]
            [Black "adnx2h"]
            [Result "1-0"]
            [TimeControl "300+5"]
            [WhiteElo "827"]
            [BlackElo "773"]
            [Termination "ananb won by checkmate"]
            [ECO "B20"]
            [EndTime "23:32:03 GMT+0000"]
            [Link "https://www.chess.com/game/live/140888135440?move=0"]

            1. e4 c5 2. Bc4 Nc6 3. d3 e6 4. c3 a6 5. Be3 b5 6. Bb3 e5 7. Qh5 Nf6 8. Qxf7#
            1-0`
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
                id_boardHandler.parsePgn(id_pgnInput.text)
            }
        }
    }
}
