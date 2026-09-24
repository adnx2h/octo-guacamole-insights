import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import PGN_movesModule 1.0

Item {
    id: id_AnalysisScreen
    anchors.fill: parent
    visible: false
    signal sgnBtnBackClicked

    // Property to track the current move index for highlighting
    property int currentMoveIndex: 0
    property bool explanationLoading: false

    // Analysis State Tracking
    property string analysisState: "ready" // "stockfish", "ai", or "ready"
    property string statusMessage: {
        if (analysisState === "stockfish") return "Stockfish Analyzing..."
        if (analysisState === "ai") return "AI Interpreting..."
        return "Ready"
    }

    Item {
        id: mainColumn
        anchors.fill: parent
        anchors.margins: 8

        // Top Bar Container
        Item {
            id: id_topBarContainer
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 44

            // 1. Back Button (Anchored Left)
            Button {
                id: id_Btn_Back
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                height: parent.height - 8

                contentItem: Text {
                    text: "← Back"
                    font.pixelSize: 14
                    font.bold: true
                    color: id_Btn_Back.down ? "#1565C0" : "#212121"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 12
                    rightPadding: 12
                }

                background: Rectangle {
                    implicitWidth: 80
                    color: id_Btn_Back.down ? "#E0E0E0" : (id_Btn_Back.hovered ? "#F5F5F5" : "#FFFFFF")
                    border.color: id_Btn_Back.hovered ? "#1976D2" : "#D1D5DB"
                    border.width: 1
                    radius: 8

                    Behavior on color {
                        ColorAnimation {
                            duration: 150
                        }
                    }
                }

                onClicked: {
                    console.log("Back Button Clicked");
                    sgnBtnBackClicked();
                }
            }

            // 2. Status Message Area (Anchored Center)
            Item {
                id: id_statusMessageArea
                anchors.left: id_Btn_Back.right
                anchors.right: id_Btn_flipBoard.left
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                height: parent.height - 8

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 6

                    BusyIndicator {
                        id: statusBusyIndicator
                        running: id_AnalysisScreen.analysisState !== "ready"
                        visible: id_AnalysisScreen.analysisState !== "ready"
                        Layout.preferredWidth: 20
                        Layout.preferredHeight: 20
                    }

                    Text {
                        id: id_txtStatus
                        text: id_AnalysisScreen.statusMessage
                        font.pixelSize: 13
                        font.bold: true
                        color: id_AnalysisScreen.analysisState !== "ready" ? "#1976D2" : "#616161"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                }
            }

            // 3. Flip Board Button (Anchored Right)
            Button {
                id: id_Btn_flipBoard
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                height: parent.height - 8

                contentItem: Text {
                    text: "↺ Flip Board"
                    font.pixelSize: 14
                    font.bold: true
                    color: id_Btn_flipBoard.down ? "#1565C0" : "#212121"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 12
                    rightPadding: 12
                }

                background: Rectangle {
                    implicitWidth: 110
                    color: id_Btn_flipBoard.down ? "#E0E0E0" : (id_Btn_flipBoard.hovered ? "#F5F5F5" : "#FFFFFF")
                    border.color: id_Btn_flipBoard.hovered ? "#1976D2" : "#D1D5DB"
                    border.width: 1
                    radius: 8

                    Behavior on color {
                        ColorAnimation {
                            duration: 150
                        }
                    }
                }

                onClicked: {
                    console.log("Flip Board Button Clicked");
                    id_analysisChessBoard.state = (id_analysisChessBoard.state === "rotated" ? "" : "rotated");
                }
            }
        }

        // Board Row
        Row {
            id: boardRow
            anchors.top: id_topBarContainer.bottom
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

                // Automatically animate whiteAdvantage whenever it changes
                Behavior on whiteAdvantage {
                    NumberAnimation {
                        duration: 500 // Duration in milliseconds
                        easing.type: Easing.OutCubic // Smooth deceleration
                    }
                }

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

                states: [
                    State {
                        name: "rotated"
                        PropertyChanges {
                            target: id_analysisChessBoard
                            rotation: 180
                        }
                    }
                ]
            }
        }

        // Movements and Comments Section
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
                width: 85
                height: parent.height

                MovesListModel {
                    id: pgn_movesModel
                }

                ListView {
                    id: id_listView_movements
                    anchors.fill: parent
                    model: movesModel
                    visible: true
                    clip: true
                    spacing: 2

                    delegate: Item {
                        width: ListView.view.width
                        height: 36

                        // Alternating row background colors for better scannability
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 2
                            radius: 4
                            color: (index % 2 === 0) ? "#ffffff" : "#f1f3f5"

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: 8
                                anchors.rightMargin: 8
                                spacing: 4

                                // Move Number Column
                                Text {
                                    width: parent.width * 0.10
                                    height: parent.height
                                    text: model.moveNumber + "."
                                    verticalAlignment: Text.AlignVCenter
                                    color: "#757575"
                                    font.pixelSize: 13
                                    font.bold: true
                                }

                                // White Move Pill
                                Rectangle {
                                    width: parent.width * 0.42
                                    height: parent.height - 4
                                    anchors.verticalCenter: parent.verticalCenter
                                    radius: 4
                                    color: (id_AnalysisScreen.currentMoveIndex === index * 2) ? "#1976D2" : "transparent"

                                    Text {
                                        id: id_txtWhiteMove
                                        anchors.fill: parent
                                        anchors.leftMargin: 6
                                        text: model.whiteMove || ""
                                        verticalAlignment: Text.AlignVCenter
                                        color: (id_AnalysisScreen.currentMoveIndex === index * 2) ? "#ffffff" : "#212121"
                                        font.pixelSize: 14
                                        font.bold: (id_AnalysisScreen.currentMoveIndex === index * 2)
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            if (model.moveItemObject) {
                                                console.log(model.moveItemObject.moveNumber, " White move: ", model.moveItemObject.whiteMove)
                                            }
                                        }
                                    }
                                }

                                // Black Move Pill
                                Rectangle {
                                    width: parent.width * 0.42
                                    height: parent.height - 4
                                    anchors.verticalCenter: parent.verticalCenter
                                    radius: 4
                                    color: (id_AnalysisScreen.currentMoveIndex === index * 2 + 1) ? "#1976D2" : "transparent"

                                    Text {
                                        id: id_txtBlackMove
                                        anchors.fill: parent
                                        anchors.leftMargin: 6
                                        text: model.blackMove || ""
                                        verticalAlignment: Text.AlignVCenter
                                        color: (id_AnalysisScreen.currentMoveIndex === index * 2 + 1) ? "#ffffff" : "#212121"
                                        font.pixelSize: 14
                                        font.bold: (id_AnalysisScreen.currentMoveIndex === index * 2 + 1)
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
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

                    // Previous Move Button
                    Button {
                        id: id_btn_Previous
                        enabled: false
                        width: (parent.width - parent.spacing) / 2
                        height: parent.height
                        text: "◄ Previous"

                        contentItem: Text {
                            text: id_btn_Previous.text
                            font.bold: true
                            font.pixelSize: 14
                            color: id_btn_Previous.enabled ? "white" : "#9e9e9e"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            color: !id_btn_Previous.enabled ? "#e0e0e0" : (id_btn_Previous.down ? "#1565C0" : "#1976D2")
                            radius: 6
                        }

                        onClicked: {
                            console.log("Previous move");
                            id_boardHandler.prevMove();
                            currentMoveIndex = id_boardHandler.getCurrentMoveIndex();
                            id_TextArea_explanation.text = id_aiHandler.gameExplanations[id_boardHandler.getCurrentMoveIndex()]?.explanation || "No explanation available.";
                        }
                    }

                    // Next Move Button
                    Button {
                        id: id_btn_Next
                        enabled: false
                        width: (parent.width - parent.spacing) / 2
                        height: parent.height
                        text: "Next ►"

                        contentItem: Text {
                            text: id_btn_Next.text
                            font.bold: true
                            font.pixelSize: 14
                            color: id_btn_Next.enabled ? "white" : "#9e9e9e"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            color: !id_btn_Next.enabled ? "#e0e0e0" : (id_btn_Next.down ? "#1565C0" : "#1976D2")
                            radius: 6
                        }

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
            // console.log("Evaluation is: " + newEval )
            var whiteHeightRatio = (newEval + 100) / 200; // Normalize -100 to 100 to 0 to 1
            id_whiteEvaluationBar.whiteAdvantage = whiteHeightRatio
        }

        function onLastMoveChanged() {
            id_analysisChessBoard.highlightFrom = id_boardHandler.lastMoveFrom
            id_analysisChessBoard.highlightTo = id_boardHandler.lastMoveTo
        }
        //Stockfish start analysis
        function onSgn_uciMovesReady(){
            id_AnalysisScreen.analysisState = "stockfish";
        }
    }
    // Connections for AI explanation
    Connections {
        target: id_aiHandler

        function onMoveExplanationReady(explanation) {
            id_TextArea_explanation.text = explanation;
            loadingText.visible = false;
        }

        function onSgn_explanationRequestStatus(isLoading) {
            explanationLoading = isLoading;
            if (isLoading) {
                id_TextArea_explanation.text = ""; // Clear previous explanation
                id_AnalysisScreen.analysisState = "ai";
            }else{
                id_AnalysisScreen.analysisState = "ready";
            }
        }

        function onsgn_gameExplanationReady(moveExplanations) {
            // console.log("Explanations received:", moveExplanations.length)
            for (let i = 0; i < moveExplanations.length; i++) {
                console.log("Move", moveExplanations[i].moveIndex, moveExplanations[i].explanation);
            }
            id_btn_Next.enabled = true;
            id_btn_Previous.enabled = true;
            currentMoveIndex = 0;
            id_AnalysisScreen.analysisState = "ready";
        }
    }
    Connections{
        target: id_engineHandler

        function onSgn_stockfishAnalysisComplete(){
        // Transition to AI interpretation or back to ready
            if (id_AnalysisScreen.explanationLoading) {
                id_AnalysisScreen.analysisState = "ai";
            } else {
                id_AnalysisScreen.analysisState = "ready";
            }
        }
    }
}
