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

function setSquareColor(squareIndex) {
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

function setSquareX(index, squareSize) {
    return (index % 8) * squareSize; // Calculate x position (column)
}

function setSquareY(index, squareSize) {
    return Math.floor(index / 8) * squareSize // Calculate y position (row)
}

function getFileLetter(index) {
    var col = index % 8;
    return String.fromCharCode(97 + col); // 0 -> 'a', 1 -> 'b', ..., 7 -> 'h'
}

function getRankNumber(index) {
    var row = Math.floor(index / 8);
    return (8 - row).toString(); // 0 -> '8', 7 -> '1' (assuming 0 top-left standard indexing)
}

function isDarkSquare(index) {
    var row = Math.floor(index / 8);
    var col = index % 8;
    return (row + col) % 2 !== 0;
}

function jumpToMove(targetIndex) {
    if (targetIndex < -1) return;

    // Call C++ handler to advance/rewind the board
    id_boardHandler.goToMove(targetIndex);

    // Update current move index property
    currentMoveIndex = id_boardHandler.getCurrentMoveIndex();

    // Ensure target move is centered in the ListView
    var rowIndex = Math.floor(currentMoveIndex / 2);
    id_listView_movements.positionViewAtIndex(rowIndex, ListView.Center);

    // Update explanation text
    id_TextArea_explanation.text = id_aiHandler.gameExplanations[currentMoveIndex]?.explanation
        || "No explanation available.";
}