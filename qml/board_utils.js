function drawPiece(x, y, piece) {
    if (piece !== "") { // Only draw if the piece string is not empty
        // Image {
        //     source: "images/" + piece + ".svg" // Construct the image source path.  Assumes images are in an "images" folder.
        //     x: x * squareSize + (squareSize * 0.1) //centering
        //     y: y * squareSize + (squareSize * 0.1)
        //     width: squareSize * 0.8
        //     height: squareSize * 0.8
        // }
    }
}

function setSquareColor(squareIndex){
    //Calculate row and column index based on square index
    const rowIndex = Math.floor(squareIndex / 8);
    const columnIndex = squareIndex % 8;
   // console.debug("index:", squareIndex, "row:", rowIndex, "column:", columnIndex);

    const isLightSquare = (rowIndex + columnIndex) % 2 === 0;
    if (isLightSquare) {
        return "#f0d9b5";
    } else {
        return "#69923e";
    }
}

function setSquareX(index, width){
    return (index % 8) * width; // Calculate x position (column)
}

function setSquareY(index, height){
    return Math.floor(index / 8) * height // Calculate y position (row)
}
