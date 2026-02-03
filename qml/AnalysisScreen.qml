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

    // Property to track the current move index for highlighting
    property int currentMoveIndex: 0

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

        // 2. RowLayout for the custom Vertical Progress Bar and ChessBoard
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true // Allows this RowLayout to take available vertical space

            // Custom Vertical Progress Bar (using Rectangles to avoid 'orientation' property)
            Rectangle {
                id: id_whiteEvaluationBar
                Layout.preferredWidth: 10 // Fixed small width for the bar
                Layout.preferredHeight: id_analysisChessBoard.width  // Same height as the chessboard
                color: "#60A060" // Greenish color for the filled portion
                border.width: 1

                // Expose a 'value' property for easy control (0.0 to 1.0)
                property real whiteAdvantage: 0.5 // Example value

                // The filling part of the progress bar
                Rectangle {
                    id: id_blackEvaluationBar
                    width: parent.width
                    height: parent.height * id_whiteEvaluationBar.whiteAdvantage // Control fill height based on 'value'
                    anchors.bottom: parent.bottom // The fill starts from the bottom
                    color: "#E0E0E0" // Light grey background for the bar f0d9b5
                    border.color: "#A0A0A0"
                }
            }

            // Chess Board (Item)
            ChessBoard {
                id: id_analysisChessBoard
                Layout.fillWidth: true
                Layout.preferredHeight: width
            }
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
                                    id: id_txtWhiteMove
                                    Layout.preferredWidth: parent.width * 0.35
                                    text: model.whiteMove
                                    verticalAlignment: Text.AlignVCenter
                                    color: (id_AnalysisScreen.currentMoveIndex === index * 2) ? "blue" : "black"
                                    font.bold: (id_AnalysisScreen.currentMoveIndex === index * 2)
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
                                    id: id_txtBlackMove
                                    Layout.preferredWidth: parent.width * 0.35
                                    text: model.blackMove
                                    verticalAlignment: Text.AlignVCenter
                                    color: (id_AnalysisScreen.currentMoveIndex === index * 2 + 1) ? "blue" : "black"
                                    font.bold: (id_AnalysisScreen.currentMoveIndex === index * 2 + 1)
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
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true // Fill available height in the column

                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                    TextArea {
                        id: id_TextArea_explanation
                        width: parent.width
                        height: implicitHeight
                        readOnly: false
                        font.pixelSize: 18
                        wrapMode: Text.WordWrap
                        background: Rectangle {
                            color: "grey"
                            border.width: 2
                            radius: 5
                        }
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
                        enabled: false
                        text: "<"
                        Layout.fillWidth: true
                        Layout.fillHeight: true // Will fill the parent's preferredHeight
                        onClicked: {
                            
                            console.log("Previous move");
                            id_boardHandler.prevMove();
                            currentMoveIndex = id_boardHandler.getCurrentMoveIndex();

                            //Update the explanation box
                            id_TextArea_explanation.text = id_aiHandler.gameExplanations[id_boardHandler.getCurrentMoveIndex()]?.explanation || "No explanation available.";
                            
                            //for testing purposes only
                            // id_aiHandler.requestMoveExplanation("startpos", "", 0);
                        }
                    }
                    Button {
                        id: id_btn_Next
                        enabled: false
                        text: ">"
                        Layout.fillWidth: true
                        Layout.fillHeight: true // Will fill the parent's preferredHeight
                        onClicked: {
                            console.log("Next move");
                            id_boardHandler.nextMove();
                            currentMoveIndex = id_boardHandler.getCurrentMoveIndex();

                            //Update the explanation box
                            id_TextArea_explanation.text = id_aiHandler.gameExplanations[id_boardHandler.getCurrentMoveIndex()]?.explanation || "No explanation available.";
                        }
                    }
                }
            }
        }
    }
    Connections {
        target: id_boardHandler
        onSgn_evalPositionsChanged: {
            // console.log("Evaluation is:  " + newEval )
            var whiteHeightRatio = (newEval + 100) / 200; // Normalize -100 to 100 to 0 to 1
            id_whiteEvaluationBar.whiteAdvantage =  whiteHeightRatio
        }
        onLastMoveChanged: {
            id_analysisChessBoard.highlightFrom = id_boardHandler.lastMoveFrom
            id_analysisChessBoard.highlightTo = id_boardHandler.lastMoveTo
        }
    }
    // Property to track AI loading status
    property bool explanationLoading: false
    // Connections for AI explanation
    Connections {
        target: id_aiHandler
        onMoveExplanationReady: {
            id_TextArea_explanation.text = explanation;
            loadingText.visible = false;
        }
        onExplanationRequestStatus: {
            explanationLoading = isLoading;
            if (isLoading) {
                id_TextArea_explanation.text = ""; // Clear previous explanation
            }
        }
        onGameExplanationReady: {
            // console.log("Explanations received:", moveExplanations.length)
            for (let i = 0; i < moveExplanations.length; i++) {
                console.log("Move", moveExplanations[i].moveIndex, moveExplanations[i].explanation)
            }
            id_btn_Next.enabled = true
            id_btn_Previous.enabled = true
            currentMoveIndex = 0
        }
    }
}
