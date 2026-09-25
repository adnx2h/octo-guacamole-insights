import QtQuick 2.15
import "board_utils.js" as BoardUtils

Item {
    id: chessBoardRoot
    property real squareSize: chessBoardRoot.width / 8 // This will use the width set by the parent
    property real pieceScale: 0.95 // piece scale (95%)
    property int highlightFrom: -1
    property int highlightTo: -1

    // Helper to find piece type for the animated overlay
    function getPieceTypeAt(index) {
        var list = id_boardHandler.piecePositions;
        for (var i = 0; i < list.length; i++) {
            if (list[i].index === index) {
                return list[i].piece;
            }
        }
        return "";
    }

    // 1. Grid of 64 Board Squares
    Repeater {
        id: squareRepeater
        model: 64
        delegate: squareDelegate
    }

    // 2. Stationary Board Pieces
    Repeater {
        model: id_boardHandler.piecePositions
        delegate: pieceDelegate
    }

    // --- Delegates ---

    Component {
        id: pieceDelegate
        Item {
            id: pieceContainer
            x: BoardUtils.setSquareX(modelData.index, squareSize)
            y: BoardUtils.setSquareY(modelData.index, squareSize)
            width: squareSize
            height: squareSize
            z: 1

            // Hide the piece at the target landing square until the slide finishes
            visible: !(animatedPiece.visible && modelData.index === animatedPiece.activeTargetSquare)

            Image {
                anchors.centerIn: parent
                width: squareSize * chessBoardRoot.pieceScale
                height: width
                source: "qrc:/images/" + modelData.piece + ".png"

                // Counter-rotate the image so pieces remain right-side up
                rotation: chessBoardRoot.rotation === 180 ? 180 : 0
            }
        }
    }

    Component {
        id: squareDelegate
        Rectangle {
            id: squareItem
            width: squareSize
            height: squareSize
            x: BoardUtils.setSquareX(index, squareSize)
            y: BoardUtils.setSquareY(index, squareSize)
            z: 0

            color: {
                if (index === chessBoardRoot.highlightFrom || index === chessBoardRoot.highlightTo) {
                    return "lightgray";
                }
                return BoardUtils.setSquareColor(index);
            }

            // Helper properties to track orientation
            readonly property bool isFlipped: chessBoardRoot.rotation === 180
            readonly property int colIndex: index % 8
            readonly property int rowIndex: Math.floor(index / 8)

            // --- Rank Numbers (Visual Top-Left of Left Column) ---
            Text {
                id: rankText
                visible: squareItem.isFlipped ? (squareItem.colIndex === 7) : (squareItem.colIndex === 0)
                text: BoardUtils.getRankNumber(index)

                font.pixelSize: Math.max(9, squareSize * 0.2)
                font.bold: true
                color: BoardUtils.isDarkSquare(index) ? "#F0D9B5" : "#B58863"
                rotation: squareItem.isFlipped ? 180 : 0
                z: 0

                states: [
                    State {
                        name: "normal"
                        when: !squareItem.isFlipped
                        AnchorChanges {
                            target: rankText
                            anchors.left: squareItem.left
                            anchors.top: squareItem.top
                            anchors.right: undefined
                            anchors.bottom: undefined
                        }
                        PropertyChanges {
                            target: rankText
                            anchors.margins: Math.max(2, squareSize * 0.05)
                        }
                    },
                    State {
                        name: "flipped"
                        when: squareItem.isFlipped
                        AnchorChanges {
                            target: rankText
                            anchors.right: squareItem.right
                            anchors.bottom: squareItem.bottom
                            anchors.left: undefined
                            anchors.top: undefined
                        }
                        PropertyChanges {
                            target: rankText
                            anchors.margins: Math.max(2, squareSize * 0.05)
                        }
                    }
                ]
            }

            // --- File Letters (Visual Bottom-Right of Bottom Row) ---
            Text {
                id: fileText
                visible: squareItem.isFlipped ? (squareItem.rowIndex === 0) : (squareItem.rowIndex === 7)
                text: BoardUtils.getFileLetter(index)

                font.pixelSize: Math.max(9, squareSize * 0.2)
                font.bold: true
                color: BoardUtils.isDarkSquare(index) ? "#F0D9B5" : "#B58863"
                rotation: squareItem.isFlipped ? 180 : 0
                z: 0

                states: [
                    State {
                        name: "normal"
                        when: !squareItem.isFlipped
                        AnchorChanges {
                            target: fileText
                            anchors.right: squareItem.right
                            anchors.bottom: squareItem.bottom
                            anchors.left: undefined
                            anchors.top: undefined
                        }
                        PropertyChanges {
                            target: fileText
                            anchors.margins: Math.max(2, squareSize * 0.05)
                        }
                    },
                    State {
                        name: "flipped"
                        when: squareItem.isFlipped
                        AnchorChanges {
                            target: fileText
                            anchors.left: squareItem.left
                            anchors.top: squareItem.top
                            anchors.right: undefined
                            anchors.bottom: undefined
                        }
                        PropertyChanges {
                            target: fileText
                            anchors.margins: Math.max(2, squareSize * 0.05)
                        }
                    }
                ]
            }

            MouseArea {
                anchors.fill: parent
                onClicked: console.log("Clicked square:", index)
            }
        }
    }

    // 3. Sliding Move Overlay dummy Piece
    Image {
        id: animatedPiece
        width: squareSize * chessBoardRoot.pieceScale
        height: width
        z: 10
        visible: false

        // Counter-rotate overlay piece as well
        rotation: chessBoardRoot.rotation === 180 ? 180 : 0

        property real offset: (squareSize * (1 - chessBoardRoot.pieceScale)) / 2
        property int previousMoveIndex: -1
        property int activeTargetSquare: -1

        property real startX: 0
        property real startY: 0
        property real targetX: 0
        property real targetY: 0

        Connections {
            target: id_boardHandler
            function onSgn_isLastMoveForward(isForward) {
                var fromIndex = id_boardHandler.lastMoveFrom;
                var toIndex = id_boardHandler.lastMoveTo;

                if (fromIndex < 0 || toIndex < 0)
                    return;

                var startSq = isForward ? fromIndex : toIndex;
                var targetSq = isForward ? toIndex : fromIndex;
                animatedPiece.activeTargetSquare = targetSq;

                var pieceType = chessBoardRoot.getPieceTypeAt(targetSq);
                if (pieceType === "")
                    return;

                animatedPiece.source = "qrc:/images/" + pieceType + ".png";

                animatedPiece.startX = BoardUtils.setSquareX(startSq, squareSize) + animatedPiece.offset;
                animatedPiece.startY = BoardUtils.setSquareY(startSq, squareSize) + animatedPiece.offset;
                animatedPiece.targetX = BoardUtils.setSquareX(targetSq, squareSize) + animatedPiece.offset;
                animatedPiece.targetY = BoardUtils.setSquareY(targetSq, squareSize) + animatedPiece.offset;

                animatedPiece.x = animatedPiece.startX;
                animatedPiece.y = animatedPiece.startY;
                animatedPiece.visible = true;

                slideAnim.restart();
            }
        }

        ParallelAnimation {
            id: slideAnim
            NumberAnimation {
                target: animatedPiece
                property: "x"
                to: animatedPiece.targetX
                duration: 300
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                target: animatedPiece
                property: "y"
                to: animatedPiece.targetY
                duration: 300
                easing.type: Easing.OutCubic
            }
            onFinished: {
                animatedPiece.visible = false;
                animatedPiece.activeTargetSquare = -1;
            }
        }
    }
}