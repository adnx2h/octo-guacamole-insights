import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1
import PGN_movesModule 1.0

Item{
    id: id_AnalysisScreen
    width: id_appWindow.width
    height: id_appWindow.height
    visible: false
    signal sgnBtnSettingsClicked()

    RowLayout{
        id: id_rowLayout_top_bar
        width: parent.width - id_movementsContainer.width
        height: id_analysisChessBoard.height / 14
        anchors.top: parent.top
        anchors.left: parent.left
        Button {
            id: id_Btn_Settings
            text: "..."
            Layout.fillWidth: false
            width: id_analysisChessBoard.width / 9
            Layout.fillHeight: true
            onClicked: {
                console.log("Settings Button Clicked");
                sgnBtnSettingsClicked()
            }
        }
    }

    ChessBoard {
        id: id_analysisChessBoard
        x: 0 // Start from the left edge of AnalysisScreen
        y: id_rowLayout_top_bar.height // Position it right below the top bar
        width: id_AnalysisScreen.width // Make the chessboard take the full width of AnalysisScreen
        height: width // Keep the chessboard square

        // I can change these values later if you want a smaller board,
        // e.g., width: id_AnalysisScreen.width * 0.8, x: (id_AnalysisScreen.width - width) / 2
    }

    TextArea {
        id: id_TextArea_description
        width: parent.width
        anchors.bottom: id_movementsContainer.top
        anchors.top: id_analysisChessBoard.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        readOnly: false
        text: "Move analysis here..."
        font.pixelSize: 18
        background: Rectangle {
            color: "grey"
            border.width: 2
            radius: 5
        }
    }

    Rectangle {
        id: id_movementsContainer
        width: parent.width * 0.25
        height: id_AnalysisScreen.height * 0.30
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        color: "red"

        MovesListModel {
            id: pgn_movesModel
        }

        ListView {
            id: id_listView_movements
            width: parent.width
            height: parent.height

            model: movesModel
            visible: true

            delegate: Item {
                //Conditinoally set width based on parents available width, to prevent this log error:
                // qrc:/qt/qml/Chess/qml/AnalysisScreen.qml:81:17: TypeError: Cannot read property 'width' of null
                width: parent ? parent.width : 0
                height: 30
                Rectangle { // The background rectangle for each line
                    width: parent.width
                    height: parent.height
                    color: "white"
                    border.width: 1
                    border.color: "lightgray"
                    RowLayout {
                        width: parent.width
                        height: parent.height
                        spacing: 5
                        Text {
                            width: parent.width * 0.2
                            text: model.moveNumber + "."
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            width: parent.width * 0.35
                            text: model.whiteMove
                            verticalAlignment: Text.AlignVCenter
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (model.moveItemObject) {
                                        console.log(model.moveItemObject.moveNumber, " White move: ", model.moveItemObject.whiteMove)
                                    }
                                }
                            }
                        }

                        Text {
                            width: parent.width * 0.35
                            text: model.blackMove
                            verticalAlignment: Text.AlignVCenter
                            MouseArea{
                                anchors.fill: parent
                                onClicked: {
                                    if (model.moveItemObject) {
                                        console.log(model.moveItemObject.moveNumber, " Black moves: ", model.moveItemObject.blackMove)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    RowLayout{
        width: parent.width - id_movementsContainer.width
        height: 60
        spacing: 10
        anchors.bottom: id_movementsContainer.bottom
        anchors.left: id_movementsContainer.right
        Item {
            width: 5
        }
        Button {
            id: id_btn_Previous
            text: "<"
            Layout.fillWidth: true
            Layout.fillHeight: true
            onClicked: {
                console.log("Previous move");
            }
        }
        Item {
            width: 5
        }
        Button {
            id: id_btn_Next
            text: ">"
            Layout.fillWidth: true
            Layout.fillHeight: true
            onClicked: {
                console.log("Next move");
            }
        }
        Item {
            width: 5
        }
    }
}

// Component {
//     id: pieceDelegate
//     Image {
//         property int index
//         property int modelData
//         property string piece

//         source: piece !== "" ? "images/" + piece + ".svg" : ""
//         x: index * parent.squareSize + (parent.squareSize * 0.1)
//         y: modelData * parent.squareSize + (parent.squareSize * 0.1)
//         width: parent.squareSize * 0.8
//         height: parent.squareSize * 0.8
//     }
// }




// Column {
//     Repeater {
//         model: 8
//         Row {
//             Repeater {
//                 model: 8
//                 delegate: {
//                     var piece = "";
//                     if (modelData === 0) {
//                         if (index === 0 || index === 7)
//                             piece = "bR";
//                         if (index === 1 || index === 6)
//                            piece = "bN";
//                         if (index === 2 || index === 5)
//                             piece = "bB";
//                         if (index === 3)
//                             piece = "bQ";
//                         if (index === 4)
//                             piece = "bK";
//                         }
//                     if (modelData === 1)
//                         piece = "bP";
//                     if (modelData === 6)
//                         piece = "wP";
//                     if (modelData === 7) {
//                         if (index === 0 || index === 7)
//                             piece = "wR";
//                         if (index === 1 || index === 6)
//                             piece = "wN";
//                         if (index === 2 || index === 5)
//                             piece = "wB";
//                         if (index === 3)
//                             piece = "wQ";
//                         if (index === 4)
//                             piece = "wK";
//                     }
// BoardUtils.drawPiece(index, modelData, piece);
// if(piece !== ""){
// Image {
//     source: "images/" + piece + ".png"
//     x: x * squareSize + (squareSize * 0.1) //centering
//     y: y * squareSize + (squareSize * 0.1)
//     width: squareSize * 0.8
//     height: squareSize * 0.8
// }}
//                     }
//                 }
//             }
//         }
//     }
