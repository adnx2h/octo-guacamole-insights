import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import PGN_movesModule 1.0

Item {
    id: id_AnalysisScreen
    width: id_appWindow.width
    height: id_appWindow.height
    visible: false
    signal sgnBtnSettingsClicked()

    // Outer ColumnLayout to hold the top bar, chess board, and bottom row
    ColumnLayout {
        id: mainColumnLayout
        anchors.fill: parent

        // 1. Top Bar (RowLayout) - Height is now less
        RowLayout {
            id: id_rowLayout_top_bar
            Layout.fillWidth: true
            Layout.preferredHeight:  id_analysisChessBoard.height / 30 // Height of one chessboard square

            Button {
                id: id_Btn_Settings
                text: "..."
                width: id_analysisChessBoard.width / 9 // Use a fixed width
                Layout.fillHeight: true
                onClicked: {
                    console.log("Settings Button Clicked");
                    sgnBtnSettingsClicked()
                }
            }
        }

        // 2. Chess Board (Item)
        ChessBoard {
            id: id_analysisChessBoard
            Layout.fillWidth: true
            Layout.preferredHeight: id_AnalysisScreen.width // Keep board square, use preferredHeight
        }

        // 3. Main bottom RowLayout containing the movements list and controls column
        RowLayout {
            id: id_movements_comments_buttons_rowL
            Layout.fillWidth: true
            Layout.fillHeight: true // Fill the remaining vertical space

            // a) Movements List (Rectangle containing ListView)
            Rectangle {
                id: id_movementsContainer
                Layout.preferredWidth: parent.width * 0.25
                Layout.fillHeight: true
                color: "red"

                MovesListModel {
                    id: pgn_movesModel
                }

                ListView {
                    id: id_listView_movements
                    anchors.fill: parent // Use anchors to fill the parent Rectangle
                    model: movesModel
                    visible: true

                    delegate: Item {
                        width: ListView.view.width // Use ListView.view to reference the parent ListView
                        height: 30

                        Rectangle { // The background rectangle for each line
                            anchors.fill: parent
                            color: "white"
                            border.width: 1
                            border.color: "lightgray"

                            RowLayout {
                                anchors.fill: parent
                                spacing: 5

                                Text {
                                    Layout.preferredWidth: parent.width * 0.2
                                    text: model.moveNumber + "."
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Text {
                                    Layout.preferredWidth: parent.width * 0.35
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
                                    Layout.preferredWidth: parent.width * 0.35
                                    text: model.blackMove
                                    verticalAlignment: Text.AlignVCenter
                                    MouseArea {
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

            // b) Column containing the Text Area and Buttons Row
            ColumnLayout {
                id: id_comments_buttons_colL
                Layout.fillWidth: true // Take the remaining width
                Layout.fillHeight: true

                // i) Text Area
                TextArea {
                    id: id_TextArea_description
                    Layout.fillWidth: true
                    Layout.fillHeight: true // Fill available height in the column
                    readOnly: false
                    text: "Move analysis here..."
                    font.pixelSize: 18
                    background: Rectangle {
                        color: "grey"
                        border.width: 2
                        radius: 5
                    }
                }

                // ii) Buttons Row - Same height as top bar
                RowLayout {
                    id: id_buttonsRowL
                    Layout.fillWidth: true
                    Layout.fillHeight: false // Prevents it from expanding
                    // Set preferredHeight to be the same as the top bar's preferredHeight
                    Layout.preferredHeight:  id_analysisChessBoard.height / 8 // Height of one chessboard square
                    spacing: 5
                    Button {
                        id: id_btn_Previous
                        text: "<"
                        Layout.fillWidth: true
                        Layout.fillHeight: true // Will fill the parent's preferredHeight
                        onClicked: {
                            console.log("Previous move");
                            id_boardHandler.previousMove();
                        }
                    }
                    Button {
                        id: id_btn_Next
                        text: ">"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        onClicked: {
                            console.log("Next move");
                            id_boardHandler.nextMove();
                        }
                    }
                }
            }
        }
    }
}
