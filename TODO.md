Octo Guacamole - TODO List
This document outlines the current tasks and upcoming features for Project Octo Guacamole.
Not in specific order.

High Priority
[x] Create Simple GUI
[x] Read and parse PGN
[x] Display list of moves
[x] Add Pieces to the board at initial position
[X] Parse FEN
[ ] Add pieces to any FEN

User Interface (UI) / User Experience (UX):
[X] Add evaluation bar
[ ] Show last move indicator (e.g., colored squares).
[ ] Add drag-and-drop functionality for pieces.
[ ] Support different board themes/colors.
[ ] Implement piece animation (e.g., smooth movement).
[ ] Implement GUI to pull online PGNs
[ ] Pull PGN from Lichess and Chess.com
[ ] Format move description textbox

Engine Integration:
[X] build stockfish and add it to the PATH. 
	makefish.sh 
	(https://official-stockfish.github.io/docs/stockfish-wiki/Compiling-from-source.html#compilers) 
[X] Define engine communication protocol (UCI/XBoard parsing).
[X] Handle engine output parsing (best move, evaluation, PV).
[X] Implement next and back buttons using engine and list of moves.
[ ] Install Android Engine
[X] Consider multi-threading for engine thinking (to keep GUI responsive).
[ ] Add option to play against AI or AI vs. AI.

Platform Specifics:
[ ] Conditional compilation for Android (Android-Linux/Windows)
[X] Install Android Engine and simulator Windows
[X] Set up Android build environment correctly (SDK, NDK, etc.). QCreator Windows
[x] Set up Android build environment correctly (SDK, NDK, etc.). QCreator Linux

AI Integration:
[ ] AI interaction!! Choose an AI
[ ] Create AI