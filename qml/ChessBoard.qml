import QtQuick 2.0
import "board_utils.js" as BoardUtils

Item {
    // id: id_chessBoard
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
