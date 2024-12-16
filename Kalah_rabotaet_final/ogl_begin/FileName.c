#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include < string.h >
#include <windows.h>
#include <time.h>
#include <C:\Users\katle\OneDrive\Desktop\CS\CS_sem2\Калахи\New Kalah\Kalah_rabotaet_final\stb_easy_font.h>
//  check to see if it works
#pragma comment(lib, "opengl32.lib") // Link the OpenGL library
//will this comment show up as change in github?
int currentTurn=1;
int numCompTurn=0;

int initialkalahBoard[14]={ 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 0 };


#define hConsole GetStdHandle(STD_OUTPUT_HANDLE)
#define MAX_VALUE 10000
#define setCursor(x,y) printf  ("\033[%d;%dH", (y), (x))

 
//enum difficultyLevel { EASY=6, MEDIUM=6, HARD=6 };
enum difficultyLevel {LEVEL=8};
enum SIDE { A, B };

//enum difficultyLevel maxSearchDepth=EASY;
enum difficultyLevel maxSearchDepth=LEVEL;


int initializeGame(void);    // select game parameters
int performTurn(int*, int); // scatters kamni to cells
int determineBestTurn(int*); // initial function to start calculating best currentTurn
int miniMaxAB(int*, int, enum SIDE, int, int); // main algorythm to calculate the best currentTurn for computer
int kalahBoardEvaluation(int*); // kalahBoardEvaluation terminal position
void distributeRemainingkamni(int* kalahBoard);
FILE* file;

void displayText(float x, float y, char* text, float r, float g, float b) {
	// Static buffer to hold vertex data for rendering text. Large enough to handle a lot of characters.
	static char buffer[99999]; // ~500 chars

	// Number of quads that will be used to render the text.
	int numCoordinates;

	// Call stb_easy_font_print to generate vertex data for the given text.
	// This function fills the buffer with vertex data and returns the number of quads generated.
	numCoordinates=stb_easy_font_print(x, y, text, NULL, buffer, sizeof(buffer));

	// Set the text color using RGB values.
	glColor3f(r, g, b);

	// Enable the vertex array client state. This tells OpenGL that we will be using vertex arrays.
	glEnableClientState(GL_VERTEX_ARRAY);

	// Specify the vertex data format to OpenGL.
	// - 2: Number of coordinates per vertex (x and y).
	// - GL_FLOAT: Data type of the coordinates.
	// - 16: Stride between consecutive vertices in bytes (2 floats * 8 bytes + color=16 bytes).
	// - buffer: Pointer to the vertex data buffer.
	glVertexPointer(2, GL_FLOAT, 16, buffer);

	// Draw the text as quads. 
	// - GL_QUADS: Mode to draw quads (rectangles) using vertex data.
	// - 0: Starting index in the vertex array.
	// - numCoordinates * 4: Total number of vertices to draw (each quad has 4 vertices).
	glDrawArrays(GL_QUADS, 0, numCoordinates * 4);

	// Disable the vertex array client state as we are done using vertex arrays.
	glDisableClientState(GL_VERTEX_ARRAY);
}




int checkSimulatedMove(int* kalahBoard, int move) {
	int i, kamni, repeatTurn=0;

	// Convert move to zero-based index (0-13) and get the number of kamni in the chosen pit
	i=--move;  // Move is adjusted to zero-based index
	kamni=kalahBoard[i];  // Get the number of kamni from the chosen pit
	kalahBoard[i]=0;  // Empty the chosen pit

	// Distribute the kamni around the kalahBoard
	while (kamni--) {
		i++;  // Move to the next pit
		kalahBoard[i]++;  // Place one stone in the next pit

		// Special handling for the end of the kalahBoard (Kalah) - skip opponent's Kalah
		if ((move > 6 && i == 6) || (move < 6 && i == 13)) {
			kalahBoard[i]--;  // Remove the stone from the opponent's Kalah
			kamni++;  // Decrement kamni to place the stone in the next pit
		}

		// If the distribution wraps around the kalahBoard
		if (i == 13 && kamni > 0) i=-1;  // Reset index to start from the first pit
	}

	// Check Kalah rule - if the last stone lands in the opposite player's  Kalah
	if ((kalahBoard[i] == 1 && kalahBoard[12 - i]) &&
		((move < 6 && i < 6) || (move > 6 && i > 6 && i < 13))) {

		// If the last stone lands in the opposite player's Kalah and the opposite pit has kamni
		if (move > 6) {
			kalahBoard[13]=kalahBoard[13] + kalahBoard[i] + kalahBoard[12 - i];
			kalahBoard[i]=kalahBoard[12 - i]=0;  // Empty the last pit and its opposite pit

		}
		else {
			kalahBoard[6]=kalahBoard[6] + kalahBoard[i] + kalahBoard[12 - i];
			kalahBoard[i]=kalahBoard[12 - i]=0;  // Empty the last pit and its opposite pit

		}

		// Capture the kamni from the last pit and its opposite pit
	

	}

	// Check Kalah rule - if the last stone lands in the player's own Kalah, they get an extra turn
	if ((move < 6 && i == 6) || (move > 6 && i == 13))
		repeatTurn=1;  // Set repeatTurn to 1 to indicate an extra turn

	return repeatTurn;  // Return whether an extra turn is granted
}


int checkEmptySideSingleTurn(int* kalahBoard) {
	int i;
	int nonEmptySideA=0;  // Flag to indicate if there are non-empty pits on side A
	int nonEmptySideB=0;  // Flag to indicate if there are non-empty pits on side B

	// Check for non-empty pits on side A (pits 0 to 5)
	for (i=0; i < 6; i++) {
		if (kalahBoard[i]) {
			nonEmptySideA=1;  // Set flag if any pit on side A has kamni
			break;  // No need to check further once we find a non-empty pit
		}
	}

	// Check for non-empty pits on side B (pits 7 to 12)
	for (i=7; i < 13; i++) {
		if (kalahBoard[i]) {
			nonEmptySideB=1;  // Set flag if any pit on side B has kamni
			break;  // No need to check further once we find a non-empty pit
		}
	}

	// If one side is empty, set nonEmptySideA to 0 (indicating no complete move possible)
	if (!nonEmptySideA || !nonEmptySideB) {
		nonEmptySideA=0;//if one side is empty, nonemptysideA=0;
	}

	// If side A has non-empty pits, return the flag (indicating a valid move on side A)
	if (nonEmptySideA) {
		return nonEmptySideA;//if no sides are empty, return valid/true/1
	}
	else {
		// If side A is empty, distribute remaining kamni and return the result
		distributeRemainingkamni(kalahBoard);
		return !nonEmptySideA;
		// Assuming distributeRemainingkamni() handles distribution and does not return a value,
		// this function will return implicitly with no value.
	}
}


int performTurn(int* kalahBoard, int move) {
	currentTurn=0;  // Initialize current turn (assuming it is a global or external variable)
	int i, kamni, repeatTurn=0;  // Variables: index `i`, number of kamni, and flag for repeating the turn

	// Adjust the move index to match the kalahBoard's zero-based indexing
	i=--move;

	// Number of kamni to distribute is from the chosen pit
	kamni=kalahBoard[i];
	// Empty the chosen pit
	kalahBoard[i]=0;

	// Distribute kamni one by one
	while (kamni--) {
		i++;  // Move to the next pit
		kalahBoard[i]++;  // Place one stone in the current pit

		// Handle the case where we wrap around the kalahBoard
		if ((move > 6 && i == 6) || (move < 6 && i == 13)) {
			// If we land in the opponent's Kalah, go back one pit and add the stone back to distribute again
			kalahBoard[i]--;
			kamni++;
		}

		// Handle the wrap-around case (if we reach the end of the kalahBoard, start again from the beginning)
		if (i == 13 && kamni > 0) i=-1;
	}

	// Check if the last stone landed in the player's own Kalah or empty pit
	if ((kalahBoard[i] == 1 && kalahBoard[12 - i]) &&
		// Check if the last stone is in the player's own Kalah and the opposite pit is not empty
		((move < 6 && i < 6) || (move > 6 && i > 6 && i < 13))) {


		// Determine which Kalah to add the captured kamni to
		if (move > 6) {
			kamni=13;  // Player 2’s Kalah index
		}
		else {
			kamni=6;  // Player 1’s Kalah index
		}

		// Add the kamni from the captured pit and its opposite pit to the player's Kalah
		kalahBoard[kamni]=kalahBoard[kamni] + kalahBoard[i] + kalahBoard[12 - i];
		// Empty the captured pits
		kalahBoard[i]=kalahBoard[12 - i]=0;

	}

	// Check if the turn should be repeated
	if ((move < 6 && i == 6) || (move > 6 && i == 13)) {
		// If the last stone ended up in the player’s own Kalah, the player gets another turn
		repeatTurn=1;
	}

	// Return whether the turn should be repeated (1 for yes, 0 for no)
	return repeatTurn;
}

int checkEmptySide(int* initialkalahBoard) {
	int i;
	int nonEmptySideA=0;  // Flag to indicate if Player A's side has any non-empty pits
	int nonEmptySideB=0;  // Flag to indicate if Player B's side has any non-empty pits

	// Check Player A's side (pits 0 to 5)
	for (i=0; i < 6; i++) {
		if (initialkalahBoard[i]) {
			nonEmptySideA=1;  // Mark Player A's side as non-empty if any pit has kamni
		}
	}

	// Check Player B's side (pits 7 to 12)
	for (i=7; i < 13; i++) {
		if (initialkalahBoard[i]) {
			nonEmptySideB=1;  // Mark Player B's side as non-empty if any pit has kamni
		}
	}

	// Check if both sides have at least one non-empty pit
	if (nonEmptySideA && nonEmptySideB) {
		return 0;  // Both sides have non-empty pits, game continues
	}
	else {
		// At least one side is empty; distribute remaining kamni
		distributeRemainingkamni(initialkalahBoard);

		// Compare the number of kamni in both Kalahs
		if (initialkalahBoard[6] > initialkalahBoard[13]) {
			return 1;  // Player A's Kalah has more kamni, Player A wins
		}
		else if (initialkalahBoard[6] < initialkalahBoard[13]) {
			return 2;  // Player B's Kalah has more kamni, Player B wins
		}
		else {
			return 3;  // Both Kalahs have an equal number of kamni, it's a tie
		}
	}
}


void distributeRemainingkamni(int* kalahBoard) {
	int i=0;  // Index for Player B's pits
	int j=0;  // Index for Player A's pits

	// Start loop to process remaining kamni from Player B's pits (indices 7 to 12)
	// and Player A's pits (indices 0 to 5)
	for (i=7, j=0; i < 13 && j < 6; i++, j++) {
		// Move all kamni from the current Player A's pit (index j) to Player A's Kalah (index 6)
		kalahBoard[6] += kalahBoard[j];
		// After moving kamni, empty the current Player A's pit
		kalahBoard[j]=0;

		// Move all kamni from the current Player B's pit (index i) to Player B's Kalah (index 13)
		kalahBoard[13] += kalahBoard[i];
		// After moving kamni, empty the current Player B's pit
		kalahBoard[i]=0;
	}
}


int kalahBoardEvaluation(int* kalahBoard) {
	// Set the current turn (this seems to be a global variable)
	currentTurn=1;

	// Initialize variables to count the total kamni in each player's pits
	int kamniPlayerA=0;
	int kamniPlayerB=0;
	int i=0; // Loop index

	// Sum up the kamni in Player B's pits (indices 0 to 5)
	for (i=0; i < 6; i++) {
		kamniPlayerB += kalahBoard[i];
	}

	// Sum up the kamni in Player A's pits (indices 7 to 12)
	for (i=7; i < 13; i++) {
		kamniPlayerA += kalahBoard[i];
	}

	// Calculate and return the kalahBoard evaluation score:
	// - Difference between the number of kamni in Player B's Kalah (index 13) and Player A's Kalah (index 6), scaled by 100
	// - Plus the difference in the total number of kamni between Player A and Player B (Player A's total minus Player B's total)
	return ((kalahBoard[13] - kalahBoard[6]) * 100 + kamniPlayerA - kamniPlayerB);
	// Optionally, you could just return the difference in Kalahs, if that is the only factor considered:
	// return (kalahBoard[13] - kalahBoard[6]);
}
//will this comment save?

int determineBestTurn(int* kalahBoard) {
	// Set the current turn to 1 (this might be a global variable)
	currentTurn=1;

	int move=0; // Variable to iterate over possible moves
	int j=0; // Loop index for copying the kalahBoard
	int repeatIndicator=0; // Variable to hold the result of checkSimulatedMove
	int score; // Variable to hold the score for each move
	int best=-MAX_VALUE; // Initialize best score to a very low value
	int simulatedkalahBoard[14]; // Array to simulate the kalahBoard state
	int bestMove=0; // Variable to hold the best move found

	// Iterate over possible moves for Player B (indices 7 to 12)
	for (move=12; move > 6; move--) {
		// Copy the current kalahBoard state to the simulatedkalahBoard
		for (j=0; j < 14; j++) {
			simulatedkalahBoard[j]=kalahBoard[j];
		}
     
		// Check if the selected move has kamni to play
		if (simulatedkalahBoard[move] > 0) {
			// Simulate the move and get the result (k)
			repeatIndicator=checkSimulatedMove(simulatedkalahBoard, move + 1);

			// Check if the move will leave one side of the kalahBoard empty
			if (!checkEmptySideSingleTurn(simulatedkalahBoard)) {//if it returns zero, one side is empty
				// Distribute remaining kamni and evaluate the kalahBoard
				distributeRemainingkamni(simulatedkalahBoard);
				score=kalahBoardEvaluation(simulatedkalahBoard);
			}
			else {
				// Use alpha-beta pruning to evaluate the move
				if (repeatIndicator == 1) {
					// If a repeat turn is allowed, search deeper
					score=miniMaxAB(simulatedkalahBoard, maxSearchDepth, B, -MAX_VALUE, MAX_VALUE);
				}
				else {
					// Otherwise, search with a reduced depth
					score=miniMaxAB(simulatedkalahBoard, maxSearchDepth - 1, A, -MAX_VALUE, MAX_VALUE);
				}
			}

			// Update the best move if this move has a better score
			if (score > best) {
				best=score;
				bestMove=move + 1;
			}
		}
	}

	// Return the best move adjusted to match the move range (subtract 7)
	return (bestMove - 7);
}


int miniMaxAB(int* kalahBoard, int curDepth, enum SIDE side, int alpha, int beta) {
	// Set the current turn for debugging or display purposes (not used in this function)
	currentTurn=1;

	int i, j, move, repeatIndicator;
	int score;
	int simulatedkalahBoard[14];
	int best;

	// Base case: if the current depth is zero, evaluate the kalahBoard and return the score
	if (curDepth == 0) return kalahBoardEvaluation(kalahBoard);

	// Initialize the best score based on the side (Player A or Player B)
	if (side == B) {
		best=-MAX_VALUE; // Initialize best score for maximizing player B

		for (move=12; move > 6; move--) { // Iterate over Player B's moves (pits 7 to 12)
			for (i=0; i < 14; i++) simulatedkalahBoard[i]=kalahBoard[i]; // Copy the current kalahBoard state

			// Check if the move is valid
			if (simulatedkalahBoard[move] > 0) {
				repeatIndicator=checkSimulatedMove(simulatedkalahBoard, move + 1); // Simulate the move

				// If no empty side is left after the move, distribute remaining kamni
				if (!checkEmptySideSingleTurn(simulatedkalahBoard)) {
					distributeRemainingkamni(simulatedkalahBoard); // End the turn by distributing remaining kamni
					score=kalahBoardEvaluation(simulatedkalahBoard); // Evaluate the kalahBoard after distribution
				}
				else {
					// Recursively call alpha-beta pruning on the new kalahBoard state
					if (repeatIndicator )
						score=miniMaxAB(simulatedkalahBoard, curDepth, side, alpha, beta);
					else if (side == B) {
						score=miniMaxAB(simulatedkalahBoard, curDepth - 1, A, alpha, beta);
					}
					else if (side == A) {
						score=miniMaxAB(simulatedkalahBoard, curDepth - 1, B, alpha, beta);
					}
				}

				// Update the best score found and perform alpha-beta pruning
				if (score > best) {
					best=score;
					alpha=(alpha > best) ? alpha : best; // Update alpha to the maximum value found
					
					if (beta<alpha) return alpha;
					else
						continue;
				}
			}
		}
	}
	else if (side == A) {
		currentTurn=1;
		best=MAX_VALUE; // Initialize best score for minimizing player A
		for (move=5; move >= 0; move--) { // Iterate over Player A's moves (pits 0 to 5)
			for (i=0; i < 14; i++) simulatedkalahBoard[i]=kalahBoard[i]; // Copy the current kalahBoard state

			// Check if the move is valid
			if (simulatedkalahBoard[move] > 0) {
				repeatIndicator=checkSimulatedMove(simulatedkalahBoard, move + 1); // Simulate the move

				// If no empty side is left after the move, distribute remaining kamni
				if (!checkEmptySideSingleTurn(simulatedkalahBoard)) {
					distributeRemainingkamni(simulatedkalahBoard); // End the turn by distributing remaining kamni
					score=kalahBoardEvaluation(simulatedkalahBoard); // Evaluate the kalahBoard after distribution
				}
				else {
					// Recursively call alpha-beta pruning on the new kalahBoard state
					if (repeatIndicator )
						score=miniMaxAB(simulatedkalahBoard, curDepth, side, alpha, beta);
					else
						score=miniMaxAB(simulatedkalahBoard, curDepth - 1, B, alpha, beta);
				}

				// Update the best score found and perform alpha-beta pruning
				if (score < best) {
					best=score;
					beta=(beta < best) ? beta : best; // Update beta to the minimum value found
					if (alpha>beta) return beta;
					else
						continue;
				}
			}
		}
	}
	return best; // Return the best score found for the current kalahBoard state
}


// Function prototypes for window procedures and OpenGL setup
LRESULT CALLBACK windowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void enableOpenGL(HWND hwnd, HDC* hdc, HGLRC* hglrc);
void disableOpenGL(HWND hwnd, HDC hdc, HGLRC hglrc);

int windowWidth, windowHeight; // Variables to store the window's width and height

// Structure to define a button with its properties
typedef struct {
	char buttonName[20];      // Name of the button
	float vertices[8];     // Vertices for drawing the button as a quadrilateral
	BOOL isHover;        // Flag to indicate if the button is being isHovered over
} Button;

// Array of buttons with their properties
Button buttons[]={
	{"players", {0,0,  100,0,  100,30, 0,30}, FALSE}, // Button for "players"
	{"computer",{0,40, 100,40, 100,70, 0,70}, FALSE}, // Button for "computer"
	{"quit",    {0,80, 100,80, 100,110, 0,110}, FALSE} // Button for "quit"
};

// Function to display a button on the screen
void displayButton(Button button)
{
	// Enable the use of vertex arrays. This tells OpenGL to use the vertex array stored in memory for rendering.
	glEnableClientState(GL_VERTEX_ARRAY);

	// Set the current drawing color to a custom shade of green using RGB values. 
	// The color is applied to all subsequent drawing commands until the color is changed.
	glColor3f(0.2, 0.5, 0.5);

	// Specify the format and location of the vertex data to be used for rendering.
	// Parameters:
	// - 2: Each vertex has 2 components (x, y).
	// - GL_FLOAT: The type of each component is a float.
	// - 0: No additional spacing (stride) between consecutive vertices.
	// - button.vertices: Pointer to the array of vertex data (coordinates) that define the button's shape.
	glVertexPointer(2, GL_FLOAT, 0, button.vertices);

	// Draw the button as a filled quadrilateral using the specified vertices.
	// - GL_TRIANGLE_FAN: Draws a series of triangles, where the first vertex is shared by all triangles (creating a "fan" shape).
	// - 0: The starting index in the vertex array.
	// - 4: The number of vertices to be used (4 vertices to define the quadrilateral).
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY); // Disable the use of vertex arrays
}


// Function to check if a point (x, y) is within the bounds of a button
BOOL isPointInButton(int x, int y, Button button)
{
	// Check if the x-coordinate of the point is between the left and right edges of the button
	// button.vert[0] is the x-coordinate of the button's top-left corner
	// button.vert[4] is the x-coordinate of the button's bottom-right corner
	// Check if the y-coordinate of the point is between the top and bottom edges of the button
	// button.vert[1] is the y-coordinate of the button's top-left corner
	// button.vert[5] is the y-coordinate of the button's bottom-right corner
	return (x > button.vertices[0]) && (x < button.vertices[4]) &&
		(y > button.vertices[1]) && (y < button.vertices[5]);

	//buttons[i].isHover=isPointInButton(LOWORD(lParam), HIWORD(lParam), buttons[i]);
}

// Function to display the menu by rendering each button
void displayMenu()
{
	// Save the current matrix state to restore it later
	glPushMatrix();

	// Load the identity matrix to reset transformations
	glLoadIdentity();

	// Set up an orthographic projection matrix
	// Defines the coordinate system for the display
	// (0, windowWidth) specifies the horizontal range
	// (windowHeight, 0) specifies the vertical range
	// -1 and 1 are the near and far clipping planes
	glOrtho(0, windowWidth, windowHeight, 0, -1, 1);

	// Loop through each button in the `buttons` array and display it
	// There are 3 buttons in the array: "players", "computer", and "quit"
	for (int i=0; i < 3; i++) // btnCnt=3
		displayButton(buttons[i]);

	// Restore the previous matrix state
	glPopMatrix();
}

// Array of Button structures representing game buttons with their properties
Button gameButtons[]={
	// Button "11" with coordinates for the rectangle's vertices
	// Vertices define a rectangle from (0,0) to (100,30)
	// `FALSE` indicates that this button is not currently being isHovered over
	{"B1", {0, 0, 100, 0, 100, 30, 0, 30}, FALSE},

	// Button "12" with coordinates for the rectangle's vertices
	// Vertices define a rectangle from (0,40) to (100,70)
	// `FALSE` indicates that this button is not currently being isHovered over
	{"B2", {0, 40, 100, 40, 100, 70, 0, 70}, FALSE},

	// Button "13" with coordinates for the rectangle's vertices
	// Vertices define a rectangle from (0,80) to (100,110)
	// `FALSE` indicates that this button is not currently being isHovered over
	{"B3", {0, 80, 100, 80, 100, 110, 0, 110}, FALSE},

	// Button "14" with coordinates for the rectangle's vertices
	// Vertices define a rectangle from (0,120) to (100,150)
	// `FALSE` indicates that this button is not currently being isHovered over
	{"B4", {0, 120, 100, 120, 100, 150, 0, 150}, FALSE},

	// Button "15" with coordinates for the rectangle's vertices
	// Vertices define a rectangle from (0,160) to (100,190)
	// `FALSE` indicates that this button is not currently being isHovered over
	{"B5", {0, 160, 100, 160, 100, 190, 0, 190}, FALSE},

	// Button "16" with coordinates for the rectangle's vertices
	// Vertices define a rectangle from (0,200) to (100,230)
	// `FALSE` indicates that this button is not currently being isHovered over
	{"B6", {0, 200, 100, 200, 100, 230, 0, 230}, FALSE},

	// Button "21" with coordinates for the rectangle's vertices
	// Vertices define a rectangle from (1000,0) to (1100,30)
	// `FALSE` indicates that this button is not currently being isHovered over
	{"A1", {1000, 0, 1100, 0, 1100, 30, 1000, 30}, FALSE},

	// Button "22" with coordinates for the rectangle's vertices
	// Vertices define a rectangle from (1000,40) to (1100,70)
	// `FALSE` indicates that this button is not currently being isHovered over
	{"A2", {1000, 40, 1100, 40, 1100, 70, 1000, 70}, FALSE},

	// Button "23" with coordinates for the rectangle's vertices
	// Vertices define a rectangle from (1000,80) to (1100,110)
	// `FALSE` indicates that this button is not currently being isHovered over
	{"A3", {1000, 80, 1100, 80, 1100, 110, 1000, 110}, FALSE},

	// Button "24" with coordinates for the rectangle's vertices
	// Vertices define a rectangle from (1000,120) to (1100,150)
	// `FALSE` indicates that this button is not currently being isHovered over
	{"A4", {1000, 120, 1100, 120, 1100, 150, 1000, 150}, FALSE},

	// Button "25" with coordinates for the rectangle's vertices
	// Vertices define a rectangle from (1000,160) to (1100,190)
	// `FALSE` indicates that this button is not currently being isHovered over
	{"A5", {1000, 160, 1100, 160, 1100, 190, 1000, 190}, FALSE},

	// Button "26" with coordinates for the rectangle's vertices
	// Vertices define a rectangle from (1000,200) to (1100,230)
	// `FALSE` indicates that this button is not currently being isHovered over
	{"A6", {1000, 200, 1100, 200, 1100, 230, 1000, 230}, FALSE}
};



// Function to display game buttons on the screen
void displayGameButtons()
{
	// Save the current matrix state to restore it later
	glPushMatrix();

	// Load the identity matrix to reset transformations
	glLoadIdentity();

	// Set up an orthographic projection matrix
	// Defines the coordinate system for the display
	// (0, windowWidth) specifies the horizontal range
	// (windowHeight, 0) specifies the vertical range
	// -1 and 1 are the near and far clipping planes
	glOrtho(0, windowWidth, windowHeight, 0, -1, 1);

	// Loop through each button in the `gameButtons` array and display it
	// There are 12 buttons in the array
	for (int i=0; i < 12; i++) // btnCnt=12
		displayButton(gameButtons[i]);

	// Restore the previous matrix state
	glPopMatrix();
}

// Function to display a subset of buttons for a secondary menu
void displaySecondaryMenu()
{
	// Save the current matrix state to restore it later
	glPushMatrix();

	// Load the identity matrix to reset transformations
	glLoadIdentity();

	// Set up an orthographic projection matrix
	// Defines the coordinate system for the display
	// (0, windowWidth) specifies the horizontal range
	// (windowHeight, 0) specifies the vertical range
	// -1 and 1 are the near and far clipping planes
	glOrtho(0, windowWidth, windowHeight, 0, -1, 1);

	// Display the first button in the `gameButtons` array
	// Typically used for a specific purpose in the secondary menu
	displayButton(gameButtons[0]);

	// Loop through the buttons from index 6 to 11 and display them
	// These are the buttons for the secondary menu
	for (int i=6; i < 12; i++) // btnCnt=6 (buttons from index 6 to 11)
		displayButton(gameButtons[i]);

	// Restore the previous matrix state
	glPopMatrix();
}







int gameState=0;  // Flag to control the game state

enum SIDE side=A;  // Enumeration to indicate the side (player or opponent)
int currentPlayerResponse=0;  // Variable to store the current player's response
int checkEmpty=0;  // Variable to check if a slot is empty
int difficultyLevel=0;  // Variable to store the difficulty level of the game

char whoseTurn='A';//player A or player B turn


// Entry point for a Windows application
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	file=fopen("resultFile.txt", "w");
	WNDCLASSEX wcex;  // Structure to define window class attributes
	HWND hwnd;  // Handle to the main window
	HDC hDC;  // Handle to the device context
	//device context includes:

//Target: The surface or device where rendering occurs(e.g., a window, a bitmap, or a printer).
//Settings : Information about colors, brushes, pens, fonts, and clipping regions.
	
	
	
	
	HGLRC hRC;  // Handle to the OpenGL rendering context
	/*
	Manages the actual rendering pipeline and drawing operations.
Controls how rendering operations are executed, including shaders, buffers, and texture bindings.
	
	
	*/
	MSG msg;  // Structure to store messages
	BOOL bQuit=FALSE;  // Flag to control the main message loop
	float rotationAngle=0.0f;  // Variable for rotation angle (used for rendering)

	/* Register window class */
	wcex.cbSize=sizeof(WNDCLASSEX);  // Size of the WNDCLASSEX structure
	wcex.style=CS_OWNDC;  // Window class style for OpenGL
	wcex.lpfnWndProc=windowProcedure;  // Pointer to the window procedure function
	wcex.cbClsExtra=0;  // Number of extra bytes allocated for the class
	wcex.cbWndExtra=0;  // Number of extra bytes allocated for the window
	wcex.hInstance=hInstance;  // Handle to the application instance
	wcex.hIcon=LoadIcon(NULL, IDI_APPLICATION);  // Handle to the window icon
	wcex.hCursor=LoadCursor(NULL, IDC_ARROW);  // Handle to the cursor
	wcex.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);  // Background brush
	wcex.lpszMenuName=NULL;  // Pointer to the menu name
	wcex.lpszClassName="GAME";  // Class name
	wcex.hIconSm=LoadIcon(NULL, IDI_APPLICATION);  // Small icon

	if (!RegisterClassEx(&wcex))  // Register the window class
		return 0;  // Exit if registration failed

	/* Create main window */
	hwnd=CreateWindowEx(0,
		"GAME",  // Class name
		"GAME",  // Window title
		WS_OVERLAPPEDWINDOW,  // Window style
		CW_USEDEFAULT,  // x position
		CW_USEDEFAULT,  // y position
		1080,  // Width
		800,  // Height
		NULL,  // Parent window
		NULL,  // Menu
		hInstance,  // Application instance
		NULL);  // Additional application data

	ShowWindow(hwnd, nCmdShow);  // Show the main window

	enableOpenGL(hwnd, &hDC, &hRC);  // Set up OpenGL for rendering

	while (!bQuit)  // Main message loop
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))  // Check for messages
		{
			if (msg.message == WM_QUIT)  // If the message is WM_QUIT, exit the loop
			{
				bQuit=TRUE;
			}
			else
			{
				//TranslateMessage(&msg);  // Translate virtual-key messages
				DispatchMessage(&msg);  // Dispatch the message to the window procedure
			}
		}
		else
		{
			// Handle the game's logic and rendering here

			char kalahBoard[100]="";  // Array to store the kalahBoard state as a string
			int boardCounter;  // Index for looping
			int count=0;  // Counter for the kalahBoard string
			int len[14]={ 0 };  // Array to store lengths of kalahBoard segments
			for (boardCounter = 0; boardCounter < 14; boardCounter++) {
				if (boardCounter < 7 || (boardCounter>7&&boardCounter>12)) {
					kalahBoard[1+count] = '0'+ initialkalahBoard[boardCounter] % 10;
					kalahBoard[count] = '0'+ initialkalahBoard[boardCounter] / 10;
				}
				else {
					kalahBoard[1+count] ='0'+ initialkalahBoard[boardCounter] / 10;
					kalahBoard[count] = '0'+ initialkalahBoard[boardCounter] % 10;
				}
				count+=2;
			}

			

			// Set the newline character at the current position in the kalahBoard array
			kalahBoard[count]='\n';

			// Initialize counters and arrays for formatting the output
			boardCounter =0;
			int currentArrayPositionCounter=0;
			int currentkalahBoardPositionCounter=0;
			char textForRightPits[50]=""; // Array to store formatted text for the bottom part of the kalahBoard

			// Process the first part of the kalahBoard (textForRightPits)
			currentArrayPositionCounter=0;
			for (boardCounter; boardCounter < 6; boardCounter++) {
				// If length is 1, add a single character followed by a space
				
					textForRightPits[currentArrayPositionCounter]=kalahBoard[currentkalahBoardPositionCounter];
					textForRightPits[currentArrayPositionCounter + 1]=kalahBoard[currentkalahBoardPositionCounter + 1];
					textForRightPits[currentArrayPositionCounter + 2]='\n'; // Adding two newlines seems to be a mistake; it's corrected here to just one newline
					currentArrayPositionCounter += 3;
					currentkalahBoardPositionCounter += 2;
				
			}
			textForRightPits[currentArrayPositionCounter]='\n'; // Ensure the final part ends with a newline

			// Initialize and format the text for the right part of the kalahBoard (bottomKalah)
			char bottomKalah[50]="";
			currentArrayPositionCounter=0;
			for (boardCounter; boardCounter < 7; boardCounter++) {
				// Similar processing as above but including spaces between two-character entries
					bottomKalah[currentArrayPositionCounter]=kalahBoard[currentkalahBoardPositionCounter];
					bottomKalah[currentArrayPositionCounter + 1]=kalahBoard[currentkalahBoardPositionCounter + 1];
					bottomKalah[currentArrayPositionCounter + 2]=' ';
					currentArrayPositionCounter += 3;
					currentkalahBoardPositionCounter += 2;
			}
			bottomKalah[currentArrayPositionCounter]='\n'; // End with a newline

			// Initialize and format the text for the top part of the kalahBoard (textForLeftPits)
			currentArrayPositionCounter=0;
			char textForLeftPits[50]="";
			for (boardCounter; boardCounter < 13; boardCounter++) {
				textForLeftPits[currentArrayPositionCounter]=kalahBoard[currentkalahBoardPositionCounter];
				textForLeftPits[currentArrayPositionCounter + 1]=kalahBoard[currentkalahBoardPositionCounter + 1];
				textForLeftPits[currentArrayPositionCounter + 2]='\n'; // Properly add newline here
				currentArrayPositionCounter += 3;
				currentkalahBoardPositionCounter += 2;
			}
			textForLeftPits[currentArrayPositionCounter]='\n'; // Ensure the final part ends with a newline

			// Initialize and format the text for the left part of the kalahBoard (topKalah)
			char topKalah[50]="";
			currentArrayPositionCounter=0;
			for (boardCounter; boardCounter < 14; boardCounter++) {
				
					topKalah[currentArrayPositionCounter]=kalahBoard[currentkalahBoardPositionCounter];
					topKalah[currentArrayPositionCounter + 1]=kalahBoard[currentkalahBoardPositionCounter + 1];
					topKalah[currentArrayPositionCounter + 2]=' ';
					currentArrayPositionCounter += 3;
					currentkalahBoardPositionCounter += 2;
			}
			topKalah[currentArrayPositionCounter]='\n'; // Ensure the final part ends with a newline

			// Reverse the textForLeftPits string for display purposes
			_strrev(textForLeftPits);

			// OpenGL rendering setup
			glClearColor(0.2f, 0.2f, 0.2f, 0.0f); // Set the background color
			glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer

			glPushMatrix(); // Save the current matrix state

			// Set the scale for rendering text
			glScalef(0.015, -0.015, 1);

			// Render text labels on the left side of the kalahBoard
			displayText(-30, -40.5, "6", 0.7, 0.48, 0.55);
			displayText(-30, -30, "5", 0.7, 0.48, 0.55);
			displayText(-30, -19, "4", 0.7, 0.48, 0.55);
			displayText(-30, -8, "3", 0.7, 0.48, 0.55);
			displayText(-30, 2, "2", 0.7, 0.48, 0.55);
			displayText(-30, 13, "1", 0.7, 0.48, 0.55);


			// Render text labels on the right side of the kalahBoard
			displayText(13, -40.5, "1", 0.7, 0.48, 0.55);
			displayText(13, -30, "2", 0.7, 0.48, 0.55);
			displayText(13, -19, "3", 0.7, 0.48, 0.55);
			displayText(13, -8, "4", 0.7, 0.48, 0.55);
			displayText(13, 2, "5", 0.7, 0.48, 0.55);
			displayText(13, 13, "6", 0.7, 0.48, 0.55);



			// Render the formatted kalahBoard sections
			// Render the text for the left part of the kalahBoard (topKalah)
			// Position at (-11, -50) with color red (0.5, 0, 0)
			displayText(-11, -50, topKalah, 0.5, 0, 0);

			// Render the text for the right part of the kalahBoard (bottomKalah)
			// Position at (-11, 22) with color green (0, 0.5, 0)
			displayText(-11, 22, bottomKalah, 0, 0.5, 0);

			// Apply scaling to adjust the size and orientation of the text
			// Scale factors are: x=0.8 (reduce width), y=0.9 (reduce height), z=0.5 (reduce depth)
			glScalef(0.8, 0.9, 0.5);

			// Render the text for the top part of the kalahBoard (textForLeftPits)
			// Position at (-25, -69) with color black (0.5, 0, 0)
			// This text is rendered with the scaling applied above
			displayText(-25, -69, textForLeftPits, 0.5, 0, 0);

			// Render the text for the bottom part of the kalahBoard (textForRightPits)
			// Position at (0, -45) with color green (0, 0.5, 0)
			// This text is also rendered with the scaling applied above
			displayText(0, -45, textForRightPits, 0, 0.5, 0);

			



			if (currentPlayerResponse == 0) {
				displayMenu();
				glScalef(0.5, 0.5, 1);
				displayText(-130, -145.8, "Player", 1, 1, 1);
				displayText(-130, -130, "Computer", 1, 1, 1);
				displayText(-130, -114.2, "Exit", 1, 1, 1);
			}
			if (currentPlayerResponse == 1) {
				displayMenu();
				glScalef(0.5, 0.5, 1);
				displayText(-132, -145.8, "6", 1, 1, 1);
				displayText(-132, -130, "5", 1, 1, 1);
				displayText(-132, -114.2, "4", 1, 1, 1);
				displayText(-132, -98.4, "3", 1, 1, 1);
				displayText(-132, -82.6, "2", 1, 1, 1);
				displayText(-132, -66.8, "1", 1, 1, 1);
				displayText(140, -145.8, "1", 1, 1, 1);
				displayText(140, -130, "2", 1, 1, 1);
				displayText(140, -114.2, "3", 1, 1, 1);
				displayText(140, -98.4, "4", 1, 1, 1);
				displayText(140, -82.6, "5", 1, 1, 1);
				displayText(140, -66.8, "6", 1, 1, 1);
				
			}
			if (currentPlayerResponse == 2) {
				displayMenu();
				glScalef(0.5, 0.5, 1);
				displayText(-130, -145.8, "Hard", 1, 1, 1);
				displayText(-130, -130, "Medium", 1, 1, 1);
				displayText(-130, -114.2, "Easy", 1, 1, 1);
			}
			if (currentPlayerResponse == 3) {

				glScalef(0.5, 0.5, 1);
				displayText(-130, -145.8, "Computer move", 1, 1, 1);
				displayText(140, -145.8, "1", 1, 1, 1);
				displayText(140, -130, "2", 1, 1, 1);
				displayText(140, -114.2, "3", 1, 1, 1);
				displayText(140, -98.4, "4", 1, 1, 1);
				displayText(140, -82.6, "5", 1, 1, 1);
				displayText(140, -66.8, "6", 1, 1, 1);
				
			}
			if (currentPlayerResponse == 1) {
				displayGameButtons();
				if (checkEmpty == 1) {
					displayText(-40, 100, "Player A wins", 0, 0.5, 0);
				}
				if (checkEmpty == 2) {
					displayText(-40, 100, "Player B wins", 0.5, 0, 0);
				}
				if (checkEmpty == 3) {
					displayText(-40, 100, "Draw", 0, 1, 1);
				}
			}
			if (currentPlayerResponse == 3) {
				displaySecondaryMenu();

				if (checkEmpty == 1) {
					displayText(-40, 100, "Player A wins", 0, 0.5, 0);
				}
				if (checkEmpty == 2) {
					displayText(-40, 100, "Player B wins", 0.5, 0, 0);
				}
				if (checkEmpty == 3) {
					displayText(-40, 100, "Draw", 0, 1, 1);
				}
			}

			

			if (currentPlayerResponse == 3 && side == B) {
				displayText(-65, 80, "Computer's turn", 0.5, 0.5, 0.5);
			}
			if (currentPlayerResponse == 3 && side == A) {
				displayText(-47, 80, "Player's move", 0.5, 0.5, 0.5);
			}

			if (currentPlayerResponse == 1 && side == B) {
				displayText(-65, 80, "Player B's turn", 0.5, 0.5, 0.5);
			}
			if (currentPlayerResponse == 1 && side == A) {
				displayText(-47, 80, "Player A's move", 0.5, 0.5, 0.5);
			}

			glPopMatrix();
			SwapBuffers(hDC);

		}
	}

	disableOpenGL(hwnd, hDC, hRC);
	DestroyWindow(hwnd);
	return msg.wParam;
}

LRESULT CALLBACK windowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_LBUTTONDOWN://clicked
		if (currentPlayerResponse == 1) {
			// If the game is in player vs player mode
			for (int i=0; i < 12; i++) {
					if (strcmp(gameButtons[i].buttonName, "B1") == 0 || strcmp(gameButtons[i].buttonName, "B2") == 0 ||
						strcmp(gameButtons[i].buttonName, "B3") == 0 || (strcmp(gameButtons[i].buttonName, "B4") == 0 ||
							strcmp(gameButtons[i].buttonName, "B5") == 0 || strcmp(gameButtons[i].buttonName, "B6") == 0 &&
							(side == B && initialkalahBoard[13-atoi(&gameButtons[i].buttonName)]))) {
						int output = performTurn(initialkalahBoard,14- atoi(&gameButtons[i].buttonName));
						checkEmpty = checkEmptySide(initialkalahBoard);
						if (output == 0) {
							// Switch sides if turn is valid
							side = (side == A) ? B : A;

						}
					}
					if (strcmp(gameButtons[i].buttonName, "A1") == 0 || strcmp(gameButtons[i].buttonName, "A2") == 0 ||
						strcmp(gameButtons[i].buttonName, "A3") == 0 || (strcmp(gameButtons[i].buttonName, "A4") == 0 ||
							strcmp(gameButtons[i].buttonName, "A5") == 0 || strcmp(gameButtons[i].buttonName, "A6") == 0 &&
							(side == A && initialkalahBoard[atoi(&gameButtons[i].buttonName) - 1]))) {
						int output = performTurn(initialkalahBoard, atoi(&gameButtons[i].buttonName));
						checkEmpty = checkEmptySide(initialkalahBoard);
						if (output == 0) {
							// Switch sides if turn is valid
							side = (side == A) ? B : A;

						}
					}
			}
		}
		else if (currentPlayerResponse == 0 || currentPlayerResponse == 2) {
			// If the game is in the initial state or computer setup phase
			for (int i=0; i < 4; i++) {
				// Iterate through the first set of buttons
				if (isPointInButton(LOWORD(lParam), HIWORD(lParam), buttons[i])) {
					// Check if the mouse click is within one of the buttons
					if ((strcmp(buttons[i].buttonName, "quit") == 0) && (currentPlayerResponse == 0)) {
						// If the "quit" button is clicked and it's the initial state, exit the application
						PostQuitMessage(0);
					}
					else if ((strcmp(buttons[i].buttonName, "players") == 0) && (currentPlayerResponse == 0)) {
						// If the "players" button is clicked and it's the initial state, switch to player vs player mode
						currentPlayerResponse=1;
					}
					else if ((strcmp(buttons[i].buttonName, "computer") == 0) && (currentPlayerResponse == 0)) {
						// If the "computer" button is clicked and it's the initial state, switch to player vs computer mode
						currentPlayerResponse=2;
					}
					else if ((strcmp(buttons[i].buttonName, "quit") == 0) && (currentPlayerResponse == 2)) {
						// If the "quit" button is clicked during the computer setup phase, set the game difficulty to EASY
						currentPlayerResponse=3;
						//maxSearchDepth=EASY;
						maxSearchDepth=LEVEL;

					}
					else if ((strcmp(buttons[i].buttonName, "players") == 0) && (currentPlayerResponse == 2)) {
						// If the "players" button is clicked during the computer setup phase, set the game difficulty to MEDIUM
						currentPlayerResponse=3;
						//maxSearchDepth=MEDIUM;
						maxSearchDepth=LEVEL;

					}
					else if ((strcmp(buttons[i].buttonName, "computer") == 0) && (currentPlayerResponse == 2)) {
						// If the "computer" button is clicked during the computer setup phase, set the game difficulty to HARD
						currentPlayerResponse=3;
						//maxSearchDepth=HARD;
						maxSearchDepth=LEVEL;

					}
				}
			}
		}

		else if (currentPlayerResponse == 3) {
			// If the game is in the computer's move phase
			
			for (int i=0; i < 12; i++) {
				// Iterate through the set of game buttons
				if (isPointInButton(LOWORD(lParam), HIWORD(lParam), gameButtons[i])) {
					// Check if the mouse click is within one of the game buttons
					if (strcmp(gameButtons[i].buttonName, "A1") == 0 || strcmp(gameButtons[i].buttonName, "A2") == 0 ||
						strcmp(gameButtons[i].buttonName, "A3") == 0 || (strcmp(gameButtons[i].buttonName, "A4") == 0 ||
							strcmp(gameButtons[i].buttonName, "A5") == 0 || strcmp(gameButtons[i].buttonName, "A6") == 0 &&
							(side == A && initialkalahBoard[atoi(&gameButtons[i].buttonName) - 1]))) {
						int output = performTurn(initialkalahBoard, atoi(&gameButtons[i].buttonName));
						checkEmpty = checkEmptySide(initialkalahBoard);
						if (output == 0) {
							// Switch sides if turn is valid
							side = (side == A) ? B : A;

						}
					}
					if ((side == B) && (strcmp(gameButtons[i].buttonName, "B1") == 0)) {
						clock_t start=clock();
						fprintf(file, "\nComputer turn: %d\n", ++numCompTurn);
						//fprintf(file, "\tStarting time: %.8lf\n", (((double)start)/CLOCKS_PER_SEC)*1000);
						//comment
						// If button "11" is clicked for side B (computer's turn), determine the best move and perform it
						int computerTurn=determineBestTurn(initialkalahBoard);
						int output=checkSimulatedMove(initialkalahBoard, computerTurn + 7);
						checkEmpty=checkEmptySide(initialkalahBoard);
						if (output == 0) {
							// Switch sides if computer's move is valid
							side=(side == A) ? B : A;
						}
						clock_t end=clock();
						fprintf(file, "\tExecuting time: %d \n", ((int)end - (int)start) );

					}
				}
			}
		}



		break;

	case WM_MOUSEMOVE:
		// Handle mouse movement events
		for (int i=0; i < 14; i++) {
			// Iterate through all buttons to update their isHover state
			buttons[i].isHover=isPointInButton(LOWORD(lParam), HIWORD(lParam), buttons[i]);
			// Set the isHover state based on whether the mouse is over the button
		}
		break;

	case WM_SIZE:
		// Handle window resizing events
		windowWidth=LOWORD(lParam);  // Get the new width of the window
		windowHeight=HIWORD(lParam); // Get the new height of the window
		glViewport(0, 0, windowWidth, windowHeight); // Set the OpenGL viewport to match the new window size
		glLoadIdentity(); // Reset the current matrix to the identity matrix
		float k=windowWidth / (float)windowHeight; // Calculate the aspect ratio
		glOrtho(-k, k, -1, 1, -1, 1); // Set the orthographic projection matrix with the correct aspect ratio
		break;

	case WM_DESTROY:
		// Handle window destruction
		return 0; // End the message loop and close the application

	case WM_KEYDOWN:
	{
		// Handle keykalahBoard input
		switch (wParam)
		{
		case VK_ESCAPE:
			// If the Escape key is pressed, post a quit message to exit the application
			PostQuitMessage(0);
			break;
		}
	}
	break;

	default:
		// Handle any other messages by default
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
		// Call the default window procedure for any unhandled messages
	}

	return 0; // Return 0 to indicate message has been processed
}
void enableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
	PIXELFORMATDESCRIPTOR pfd;
	int iFormat;

	// Get the device context (DC) for the specified window
	*hDC=GetDC(hwnd);

	// Zero out the memory for the pixel format descriptor (PFD)
	ZeroMemory(&pfd, sizeof(pfd));

	// Set up the pixel format descriptor for OpenGL rendering
	pfd.nSize=sizeof(pfd);             // Size of the structure
	pfd.nVersion=1;                    // Version of the structure
	pfd.dwFlags=PFD_DRAW_TO_WINDOW |   // Specify that the format supports drawing to a window
		PFD_SUPPORT_OPENGL |  // Specify that the format supports OpenGL
		PFD_DOUBLEBUFFER;     // Specify that the format supports double buffering
	pfd.iPixelType=PFD_TYPE_RGBA;      // Specify RGBA pixel type
	pfd.cColorBits=24;                 // Number of color bits per pixel
	pfd.cDepthBits=16;                 // Number of depth bits per pixel
	pfd.iLayerType=PFD_MAIN_PLANE;     // Specifies that the format is for the main drawing plane

	// Choose the best pixel format that matches the given PFD
	iFormat=ChoosePixelFormat(*hDC, &pfd);

	// Set the pixel format of the DC to the chosen format
	SetPixelFormat(*hDC, iFormat, &pfd);

	// Create an OpenGL rendering context
	*hRC=wglCreateContext(*hDC);

	// Make the rendering context current for the given device context
	wglMakeCurrent(*hDC, *hRC);
}

void disableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
	// Release the current OpenGL rendering context
	wglMakeCurrent(NULL, NULL);

	// Delete the OpenGL rendering context
	wglDeleteContext(hRC);

	// Release the device context associated with the window
	ReleaseDC(hwnd, hDC);
}
