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
            // placeholderText: qsTr("Paste Chess PGN here...")
            text: `[White "adnx2h"]
            [Black "Gioelone"]
            [Result "1-0"]
            1. d4 Nf6 2. Bf4 d6 3. c4 g6 4. e3 Bg7 5. Bd3 O-O 6. Nf3 c5 7. Nc3 Re8 8. O-O e5 9. dxe5 dxe5 10. Bg5 h6 1-0`
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
