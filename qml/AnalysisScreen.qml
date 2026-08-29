import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import PGN_movesModule 1.0

Item {
    id: id_AnalysisScreen
    anchors.fill: parent
    visible: false
    signal sgnBtnSettingsClicked()

    // Property to track the current move index for highlighting
    property int currentMoveIndex: 0

    // Use explicit anchors instead of Layout-driven sizing to avoid recursive polish loops.
    Item {
        id: mainColumn
        anchors.fill: parent
        anchors.margins: 8

        Rectangle {
            id: id_rowLayout_top_bar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 40
            color: "transparent"

            Button {
                id: id_Btn_Settings
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                width: 40
                height: parent.height - 8
                text: "..."
                onClicked: {
                    console.log("Settings Button Clicked");
                    sgnBtnSettingsClicked()
                }
            }
        }

        Row {
            id: boardRow
            anchors.top: id_rowLayout_top_bar.bottom
            anchors.topMargin: 8
            anchors.left: parent.left
            anchors.right: parent.right
            height: boardSize
            spacing: 4

            property real boardSize: Math.max(200, boardRow.width - id_whiteEvaluationBar.width - boardRow.spacing)

            Rectangle {
                id: id_whiteEvaluationBar
                width: 10
                height: boardRow.boardSize
                color: "#60A060"
                border.width: 1

                property real whiteAdvantage: 0.5

                Rectangle {
                    id: id_blackEvaluationBar
                    width: parent.width
                    height: parent.height * id_whiteEvaluationBar.whiteAdvantage
                    anchors.bottom: parent.bottom
                    color: "#E0E0E0"
                    border.color: "#A0A0A0"
                }
            }

            ChessBoard {
                id: id_analysisChessBoard
                width: boardRow.boardSize
                height: width
            }
        }

        Row {
            id: id_movements_comments_buttons_rowL
            anchors.top: boardRow.bottom
            anchors.topMargin: 8
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            spacing: 8

            Rectangle {
                id: id_movementsContainer
                width: 180
                height: parent.height
                color: "red"

                MovesListModel {
                    id: pgn_movesModel
                }

                ListView {
                    id: id_listView_movements
                    anchors.fill: parent
                    model: movesModel
                    visible: true

                    delegate: Item {
                        width: ListView.view.width
                        height: 30

                        Rectangle {
                            anchors.fill: parent
                            color: "white"
                            border.width: 1
                            border.color: "lightgray"

                            Row {
                                anchors.fill: parent
                                spacing: 5

                                Text {
                                    width: parent.width * 0.2
                                    text: model.moveNumber + "."
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Text {
                                    id: id_txtWhiteMove
                                    width: parent.width * 0.35
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
                                    width: parent.width * 0.35
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

            Column {
                id: id_comments_buttons_colL
                width: parent.width - id_movementsContainer.width - parent.spacing
                height: parent.height
                spacing: 8

                ScrollView {
                    width: parent.width
                    height: parent.height - 50

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

                Row {
                    id: id_buttonsRowL
                    width: parent.width
                    height: 42
                    spacing: 5

                    Button {
                        id: id_btn_Previous
                        enabled: false
                        width: (parent.width - parent.spacing) / 2
                        height: parent.height
                        text: "<"
                        onClicked: {
                            console.log("Previous move");
                            id_boardHandler.prevMove();
                            currentMoveIndex = id_boardHandler.getCurrentMoveIndex();
                            id_TextArea_explanation.text = id_aiHandler.gameExplanations[id_boardHandler.getCurrentMoveIndex()]?.explanation || "No explanation available.";
                        }
                    }

                    Button {
                        id: id_btn_Next
                        enabled: false
                        width: (parent.width - parent.spacing) / 2
                        height: parent.height
                        text: ">"
                        onClicked: {
                            console.log("Next move");
                            id_boardHandler.nextMove();
                            currentMoveIndex = id_boardHandler.getCurrentMoveIndex();
                            id_TextArea_explanation.text = id_aiHandler.gameExplanations[id_boardHandler.getCurrentMoveIndex()]?.explanation || "No explanation available.";
                        }
                    }
                }
            }
        }
    }
    Connections {
        target: id_boardHandler

        function onSgn_evalPositionsChanged(newEval) {
            // console.log("Evaluation is:  " + newEval )
            var whiteHeightRatio = (newEval + 100) / 200; // Normalize -100 to 100 to 0 to 1
            id_whiteEvaluationBar.whiteAdvantage = whiteHeightRatio
        }

        function onLastMoveChanged() {
            id_analysisChessBoard.highlightFrom = id_boardHandler.lastMoveFrom
            id_analysisChessBoard.highlightTo = id_boardHandler.lastMoveTo
        }
    }
    // Property to track AI loading status
    property bool explanationLoading: false
    // Connections for AI explanation
    Connections {
        target: id_aiHandler

        function onMoveExplanationReady(explanation) {
            id_TextArea_explanation.text = explanation;
            loadingText.visible = false;
        }

        function onExplanationRequestStatus(isLoading) {
            explanationLoading = isLoading;
            if (isLoading) {
                id_TextArea_explanation.text = ""; // Clear previous explanation
            }
        }

        function onGameExplanationReady(moveExplanations) {
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
