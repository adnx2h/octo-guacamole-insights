function drawPiece(x, y, piece) {
    if (piece !== "") { // Only draw if the piece string is not empty
        Image {
            source: "images/" + piece + ".svg" // Construct the image source path.  Assumes images are in an "images" folder.
            x: x * squareSize + (squareSize * 0.1) //centering
            y: y * squareSize + (squareSize * 0.1)
            width: squareSize * 0.8
            height: squareSize * 0.8
        }
    }
}

function drawSquare(x, y, color) {
      Rectangle {
          x: x * squareSize
          y: y * squareSize
          width: squareSize
          height: squareSize
          color: color
      }
  }
