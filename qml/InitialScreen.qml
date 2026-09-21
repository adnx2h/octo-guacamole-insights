import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "board_utils.js" as BoardUtils

Item {
    id: id_InitialScreen
    width: id_appWindow.width
    height: id_appWindow.height
    visible: false

    signal sgnBtnAnalysisClicked()

    // Helper property to store the PGN string to analyze
    property string selectedPgn: ""

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        // ==========================================
        // SECTION 1: TOP (Source Selector Radio Group)
        // ==========================================
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            RadioButton {
                id: rbChessCom
                text: qsTr("Chess.com Recent Games")
                checked: true
            }

            RadioButton {
                id: rbPgnPaste
                text: qsTr("Paste PGN")
            }
        }

        // ==========================================
        // SECTION 2: MIDDLE (Option Controls Container)
        // ==========================================
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: rbChessCom.checked ? 0 : 1

            // --- PAGE 0: CHESS.COM GAME SELECTOR ---
            ColumnLayout {
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    TextField {
                        id: tfUsername
                        Layout.fillWidth: true
                        placeholderText: qsTr("Enter Chess.com Username")
                        text: ""
                    }

                    Button {
                        text: qsTr("Fetch Games")
                        onClicked: {
                            // TODO: Trigger your C++ or JS network request here to populate listModelGames
                            id_chessComHandler.fetchRecentGames(tfUsername.text)
                            console.log("Fetching recent games for:", tfUsername.text)
                        }
                    }
                }

                ListView {
                    id: lvChessComGames
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 8

                    model: ListModel {
                        id: listModelGames
                        // Sample mock data for preview
                        ListElement {
                            opponent: "MagnusC"
                            opponentRating: "2850"
                            result: "WIN"
                            resultColor: "#2e7d32"
                            userColor: "White"
                            date: "2026-09-12"
                            timeControl: "Rapid 10+0"
                            ending: "by Resignation"
                            pgnData: "[Event \"Live Chess\"]\n[White \"User\"]\n[Black \"MagnusC\"]\n[Result \"1-0\"]\n\n1. e4 e5 2. Nf3 Nc6 1-0"
                        }
                        ListElement {
                            opponent: "HikaruN"
                            opponentRating: "2820"
                            result: "LOSS"
                            resultColor: "#c62828"
                            userColor: "Black"
                            date: "2026-09-10"
                            timeControl: "Blitz 3+0"
                            ending: "on Time"
                            pgnData: "[Event \"Live Chess\"]\n[White \"HikaruN\"]\n[Black \"User\"]\n[Result \"1-0\"]\n\n1. d4 Nf6 2. c4 e6 1-0"
                        }
                        ListElement {
                            opponent: "ChessMaster99"
                            opponentRating: "920"
                            result: "DRAW"
                            resultColor: "#757575"
                            userColor: "White"
                            date: "2026-09-08"
                            timeControl: "Rapid 15+10"
                            ending: "by Stalemate"
                            pgnData: "[Event \"Live Chess\"]\n[White \"User\"]\n[Black \"ChessMaster99\"]\n[Result \"1/2-1/2\"]\n\n1. e4 c5 1/2-1/2"
                        }
                    }

                    delegate: Rectangle {
                        width: lvChessComGames.width
                        height: 70
                        radius: 6
                        color: lvChessComGames.currentIndex === index ? "#e0e0e0" : "#f5f5f5"
                        border.color: lvChessComGames.currentIndex === index ? "#2196F3" : "#d0d0d0"
                        border.width: lvChessComGames.currentIndex === index ? 2 : 1

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                lvChessComGames.currentIndex = index
                                id_InitialScreen.selectedPgn = model.pgnData
                            }
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 12

                            // Result Badge (WIN / LOSS / DRAW)
                            Rectangle {
                                width: 55
                                height: 35
                                radius: 4
                                color: model.resultColor

                                Text {
                                    anchors.centerIn: parent
                                    text: model.result
                                    color: "white"
                                    font.bold: true
                                    font.pixelSize: 12
                                }
                            }

                            // Match Details
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    text: "vs " + model.opponent + " (" + model.opponentRating + ")"
                                    font.bold: true
                                    font.pixelSize: 14
                                }

                                Text {
                                    text: model.userColor + " • " + model.timeControl + " • " + model.ending
                                    color: "#666666"
                                    font.pixelSize: 11
                                }
                            }

                            // Date
                            Text {
                                text: model.date
                                color: "#888888"
                                font.pixelSize: 11
                            }
                        }
                    }
                }
            }

            // --- PAGE 1: PGN TEXT AREA ---
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                TextArea {
                    id: id_pgnInput
                    placeholderText: qsTr("Paste Chess PGN here...")
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
                    color: "black"
                    wrapMode: TextEdit.Wrap
                    background: Rectangle {
                        color: "white"
                        border.color: "#cccccc"
                        radius: 4
                    }
                }
            }
        }

        // ==========================================
        // SECTION 3: BOTTOM (Analyze Button)
        // ==========================================
        Button {
            id: idBtn_PGNAnalysis
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            text: qsTr("Analyze Game")

            background: Rectangle {
                color: idBtn_PGNAnalysis.down ? "#1565C0" : "#1976D2"
                radius: 6
            }

            contentItem: Text {
                text: idBtn_PGNAnalysis.text
                color: "white"
                font.bold: true
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                var pgnToAnalyze = ""

                if (rbChessCom.checked) {
                    // Pull selected game PGN or fallback to active property
                    if (lvChessComGames.currentIndex >= 0 && listModelGames.count > 0) {
                        pgnToAnalyze = listModelGames.get(lvChessComGames.currentIndex).pgnData
                    } else {
                        pgnToAnalyze = id_InitialScreen.selectedPgn
                    }
                } else {
                    pgnToAnalyze = id_pgnInput.text
                }

                if (pgnToAnalyze !== "") {
                    console.log("Analyzing Selected PGN:\n", pgnToAnalyze)
                    sgnBtnAnalysisClicked()
                    id_boardHandler.parsePgn(pgnToAnalyze)
                } else {
                    console.log("No valid PGN selected or pasted.")
                }
            }
        }
    }
    Connections {
        target: id_chessComHandler

        onSgn_gamesFetchedSuccess: function(gamesList) {
            listModelGames.clear()

            for (var i = 0; i < gamesList.length; i++) {
                var g = gamesList[i]

                // Determine result & color based on active user
                var isUserWhite = (g.whiteUser.toLowerCase() === tfUsername.text.toLowerCase())
                var opponent = isUserWhite ? g.blackUser : g.whiteUser
                var opponentRating = isUserWhite ? g.blackRating : g.whiteRating
                var result = isUserWhite ? g.whiteResult : g.blackResult

                var resInfo = getResultDetails(result);

                listModelGames.append({
                    "opponent": opponent,
                    "opponentRating": opponentRating.toString(),
                    "result": resInfo.text,
                    "resultColor": resInfo.color,
                    "userColor": isUserWhite ? "White" : "Black",
                    "date": g.date,
                    "timeControl": g.timeControl.toUpperCase(),
                    "ending": "",
                    "pgnData": g.pgn
                })
            }
        }

        onSgn_gamesFetchFailed: function(errorMsg) {
            console.warn("Chess.com Fetch Error:", errorMsg)
        }
    }
    // Helper function to derive display label and color from raw API result
    function getResultDetails(rawResult) {
        if (rawResult === "win") {
                return { text: "WIN", color: "#2e7d32" };
            }

            // Check for standard draw conditions
            var drawResults = ["repetition", "insufficient", "stalemate", "agreed", "50move", "timevsinsufficient"];
            if (drawResults.indexOf(rawResult) !== -1) {
                return { text: "DRAW", color: "#757575" };
            }

            // Default remaining statuses (checkmated, resigned, timeout, abandoned) to LOSS
            return { text: "LOSS", color: "#c62828" };
    }
}