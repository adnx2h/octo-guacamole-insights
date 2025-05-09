import QtQuick 2.0
import QtQuick.Controls 2.5
import "board_utils.js" as BoardUtils
import QtQuick.Layouts 1

Item{
    id: id_AnalysisScreen
    width: 500 //find the right value
    height: Screen.height
    visible: false
    signal sgnBtnSettingsClicked()

    RowLayout{
        id: id_rowLayout_top_bar
        width: parent.width - id_movementsContainer.width
        height: id_chessBoard.height / 14
        anchors.top: parent.top
        anchors.left: parent.left
        Button {
            id: id_Btn_Settings
            text: "..."
            Layout.fillWidth: false
            width: id_chessBoard.width / 9
            Layout.fillHeight: true
            onClicked: {
                console.log("Settings Button Clicked");
                sgnBtnSettingsClicked()
            }
        }
    }

    Rectangle {
        id: id_chessBoard
        width: parent.width
        height: width
        anchors.left: parent.left
        anchors.top: id_rowLayout_top_bar.bottom
        //     color: "lightgray" // Background of the whole board area
        Component {
            id: squareDelegate
            Rectangle {
                width: parent.width /8
                height: width
                color: BoardUtils.setSquareColor(index); // Pass the square index
                x: BoardUtils.setSquareX(index,width);
                y: BoardUtils.setSquareY(index, height);

                Text {
                    anchors.centerIn: parent
                    text: index
                    font.pixelSize: 12
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        console.log("Clicked overall index:", index);
                    }
                }
            }
        }

        Repeater {
            model: 64
            delegate: squareDelegate
        }
    }

    TextArea {
        id: idTextArea_description
        width: parent.width
        anchors.bottom: id_movementsContainer.top
        anchors.top: id_chessBoard.bottom
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

        ListView {
            id: id_listView_movements
            width: parent.width
            height: parent.height
            model: ListModel {
                id: id_listModel_moves
                ListElement {
                    moveNumber: 1
                    whiteMove: "e4"
                    blackMove: "c5"
                }
                ListElement {
                    moveNumber: 2
                    whiteMove: "d4"
                    blackMove: "cxd4"
                }
                ListElement {
                    moveNumber: 3
                    whiteMove: "Nxd4"
                    blackMove: "Bxd4"
                }
                ListElement {
                    moveNumber: 4
                    whiteMove: "Nxd4"
                    blackMove: "Nf6"
                }
                ListElement {
                    moveNumber: 5
                    whiteMove: "Nc3"
                    blackMove: "a6"
                }
            }
            visible: true

            delegate: Item {
                width: parent.width
                height: 30
                Rectangle { // The background rectangle for each line
                    width: parent.width
                    height: parent.height
                    color: "white" // Or any color you want for the background of each line
                    border.width: 1
                    border.color: "lightgray"
                    RowLayout {
                        width: parent.width
                        height: parent.height
                        spacing: 5
                        Text {
                            width: parent.width * 0.2 // Adjust width as needed
                            text: moveNumber + "."
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            width: parent.width * 0.35 // Adjust width as needed
                            text: whiteMove
                            verticalAlignment: Text.AlignVCenter
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    console.log(moveNumber, " White moves: ", whiteMove)
                                }
                            }
                        }

                        Text {
                            width: parent.width * 0.35 // Adjust width as needed
                            text: blackMove
                            verticalAlignment: Text.AlignVCenter
                            MouseArea{
                                anchors.fill: parent
                                onClicked: {
                                    console.log(moveNumber, " Black moves: ", blackMove)
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
