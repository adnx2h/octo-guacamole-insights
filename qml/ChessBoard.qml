import QtQuick 2.0
import "board_utils.js" as BoardUtils

Item {
    id: chessBoardRoot
    // width: parent.width
    // height: width
    // anchors.left: parent.left
    // anchors.top: id_rowLayout_top_bar.bottom
    //     color: "lightgray" // Background of the whole board area
    property real squareSize: chessBoardRoot.width / 8 // This will use the width set by the parent
    property real pieceScale: 0.95 // piece scale (95%)

    //54 squares
    Repeater {
        model: 64
        delegate: squareDelegate
    }

    //pieces initial position
    Repeater {
        model: id_boardHandler.piecePositions
        delegate: pieceDelegate
    }

    // --- Delegates ---
    Component {
        id: pieceDelegate
        Image {
            width: squareSize * chessBoardRoot.pieceScale
            height: width
            source:  "qrc:/images/" + modelData.piece + ".png" // Use the piece name to load the image

            // Calculate the offset dynamically based on pieceScale
            // (1 - pieceScale) gives the remaining space, divide by 2 for centering
            x: BoardUtils.setSquareX(modelData.index, squareSize) + (squareSize * (1 - chessBoardRoot.pieceScale) / 2)
            y: BoardUtils.setSquareY(modelData.index, squareSize) + (squareSize * (1 - chessBoardRoot.pieceScale) / 2)
            z:1 //pieces are on top
            visible: true
            Component.onCompleted: {
                // console.log("Piece created:", modelData.piece,
                //             "at index:", modelData.index,
                //             "Dimensions:", width + "x" + height,
                //             "Position: (x:" + x + ", y:" + y + ")");
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.log("Clicked piece:", modelData.piece, "at index:", modelData.index);
                }
            }
        }
    }

    Component {
        id: squareDelegate
        Rectangle {
            width: parent.width /8
            height: width
            color: BoardUtils.setSquareColor(index); // Pass the square index
            x: BoardUtils.setSquareX(index,squareSize);
            y: BoardUtils.setSquareY(index, squareSize);
            z: 0 //Ensure squares are underneath pieces

            Text {
                anchors.centerIn: parent
                // text: index
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
}
