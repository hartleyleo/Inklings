//
//  ascii_art.h
//
//  Authors: Jean-Yves Hervé and Shaun Wallace
//

#ifndef ASCII_ART_H
#define ASCII_ART_H

#include <vector>
#include <string>

//-----------------------------------------------------------------------------
//  Data types
//-----------------------------------------------------------------------------

// Travel direction data type
// Note that if you define a variable
// TravelDirection dir = whatever;
// you get the opposite directions from dir as (NUM_TRAVEL_DIRECTIONS - dir)
// you get left turn from dir as (dir + 1) % NUM_TRAVEL_DIRECTIONS
enum TravelDirection {
 NORTH = 0,
 WEST,
 SOUTH,
 EAST,
 NUM_TRAVEL_DIRECTIONS
};


// bc enums with unicode characters some compilers do not like
const std::string iconDirections[] = {
    "\u2191", // north / up
    "\u2190", // west / left
    "\u2193", // south / down
    "\u2192", // east / right
 	" ",   // nothing
};

// ANSII text colors
enum class TextColor {
    DEFAULT = 39,
    BLACK = 30,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37
};

enum InklingType {
 RED_TRAV = 0,
 GREEN_TRAV,
 BLUE_TRAV,
 NUM_TRAV_TYPES
};

// Example of Inkling thread info data type
struct InklingInfo {
    int id;  // Unique identifier for each inkling
    InklingType type;  // Type of the inkling (Red, Green, Blue)
    int row;  // Current row position in the grid
    int col;  // Current column position in the grid
    TravelDirection dir;  // Current direction of movement
    bool isLive;  // Status to check if the inkling is still active
};


//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

void myEventLoop(int val);
void initializeFrontEnd(int argc, char** argv, void (*gridCB)(void), void (*stateCB)(void));
void drawGridAndInklingsASCII(int**grid, int numRows, int numCols, std::vector<InklingInfo>& inklingList);
void drawState(int numLiveThreads, int redLevel, int greenLevel, int blueLevel);
void clearTerminal();
void cleanupAndQuit(const std::string& msg);
void slowdownProducers(void);
void speedupProducers(void);

#endif // ASCII_ART_H
