//
//  ascii_art.cpp
//
//  Authors: Jean-Yves Hervé, Shaun Wallace, and Luis Hernandez
//

#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <string>
#include <map>
#include <chrono>
#include <thread>
#include <functional>
#include <termios.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ascii_art.h"
#include <mutex>
#include <atomic>

std::mutex mtx; // mutex for thread-safe control over recursiveDepth

//---------------------------------------------------------------------------
//	ink access functions.
//---------------------------------------------------------------------------

bool acquireRedInk(int theRed);
bool acquireGreenInk(int theGreen);
bool acquireBlueInk(int theBlue);
bool refillRedInk(int theRed);
bool refillGreenInk(int theGreen);
bool refillBlueInk(int theBlue);

//---------------------------------------------------------------------------
//  Private functions' prototypes
//---------------------------------------------------------------------------

void fillTank(int y, int LEVEL_WIDTH);
void myKeyboard(unsigned char c);
void myEventLoop(int val);

//---------------------------------------------------------------------------
//  Interface constants
//---------------------------------------------------------------------------

extern bool DRAW_COLORED_TRAVELER_HEADS;
extern int inklingSleepTime;
//extern int numRedProducers;
//extern int numBlueProducers;
//extern int numGreenProducers;

//---------------------------------------------------------------------------
//  File-level global variables
//---------------------------------------------------------------------------

void (*gridDisplayFunc)(void);
void (*stateDisplayFunc)(void);

extern int MAX_LEVEL;
extern int MAX_ADD_INK;
extern int MAX_NUM_TRAVELER_THREADS;

// producer sleep times
// extern const int MIN_SLEEP_TIME;
extern int producerSleepTime;

// path to the pipe
std::string pipePath = "/tmp/my_pipe";
static unsigned int numPipesOpen = 0;
static int pipe_fd;

//---------------------------------------------------------------------------
//	Util Terminal Print ASCII functions
//---------------------------------------------------------------------------

// set text color
void setTextColor(TextColor color) {
    std::cout << "\033[" << static_cast<int>(color) << "m";
}

// reset text color to default
void resetTextColor() {
    setTextColor(TextColor::DEFAULT);
}

// base function to be reused
void print() {
    resetTextColor();  // reset color at the end of printing
    std::cout << std::endl;
}

// variadic template function for the magic sauce
template <typename T, typename... Args>
void print(const T& first, const Args&... rest) {
    if constexpr (std::is_same_v<T, TextColor>) {
        setTextColor(first);
    } else {
        std::cout << first;
    }
    print(rest...);
}

// base function to be reused
void printCell() {
    resetTextColor();  // reset color at the end of printing
}

// variadic template function for the magic sauce
template <typename T, typename... Args>
void printCell(const T& first, const Args&... rest) {
    if constexpr (std::is_same_v<T, TextColor>) {
        setTextColor(first);
    } else {
        std::cout << first;
    }
    printCell(rest...);
}

void clearTerminal() {
	/* system specific
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
	*/

	// this is easier
	std::cout << "\033[H\033[J"; // clear the terminal screen ;)
}

//---------------------------------------------------------------------------
//	Drawing functions
//---------------------------------------------------------------------------

void drawGridAndInklingsASCII(int**grid, int numRows, int numCols, std::vector<InklingInfo>& inklingList) {
	// redraw grid
	for (int row = 0; row < numRows; row++) {
        for (int col = 0; col < numCols; col++) {
			bool inklingFound = false;
			// is there a inkling in this grid spot?
			for (auto inkling : inklingList) {
				if (inkling.isLive) {
					if (row == inkling.row && col == inkling.col) {
						// BUG high grid count: extra vertical lines, but no extra horizontal
						switch ((int)(inkling.type)) {
							case RED_TRAV:
								printCell(TextColor::RED, "[", iconDirections[inkling.dir], "]");
								inklingFound = true;
								break;
							case GREEN_TRAV:
								printCell(TextColor::GREEN, "[", iconDirections[inkling.dir], "]");
								inklingFound = true;
								break;
							case BLUE_TRAV:
								printCell(TextColor::BLUE, "[", iconDirections[inkling.dir], "]");
								inklingFound = true;
								break;
						}
					}  
				}
			}
			if(!inklingFound) {
				printCell(TextColor::BLACK, "[ ]");
			}
        }
        std::cout << std::endl; // create new row
    }

	//exit(1); // uncomment this to test a single grid
}

void fillTank(int y, int LEVEL_WIDTH) {
	// implement?
    print("fillTank", y, LEVEL_WIDTH);
}

void drawState(int numLiveThreads, int redLevel, int greenLevel, int blueLevel) {
	// build, then display text info for the red, green, and blue tanks
	print(TextColor::BLACK, "Ink Tank Levels, the MAX is: ", MAX_LEVEL);
	print(TextColor::RED, "Red: ", redLevel, TextColor::GREEN, " Green: ", greenLevel, TextColor::BLUE, " Blue: ", blueLevel);
	
	// display info about number of live threads
	print("Live Threads: ", numLiveThreads);
    print();

	if(numLiveThreads == 0) {
		cleanupAndQuit("drawState callint it quits, no threads left...");
	}
}

void updateTerminal(void) {
    try {
        clearTerminal();
        gridDisplayFunc();  
        stateDisplayFunc(); 
    } catch (const std::exception& e) {
        std::cerr << "ERROR :/ updateTerminal :: caught exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "ERROR :/ updateTerminal :: caught an unknown exception" << std::endl;
    }
}

//---------------------------------------------------------------------------
//	Keyboard functions
//---------------------------------------------------------------------------

void enableRawMode() {
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO); // disable echo and canonical mode
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void disableRawMode() {
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ICANON | ECHO; // re-enable echo and canonical mode
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// this function will run in its own thread
void keyListener() {
    // enable raw mode - this will work in linux and MacOS
    enableRawMode();

    char c;
    c = std::cin.get();
    myKeyboard(c);

    // clean up (e.g., disable raw mode)
    disableRawMode();
}

//	this callback function is called when a keyboard event occurs
void myKeyboard(unsigned char c) {
	bool ok = false;
	switch (c) {
		// 'esc' to quit
		case 27:           // 27 is the ASCII value for ESC
			cleanupAndQuit("I see you pressed ESC, game over...");
			break;
            
        // slow down the production of ink for the producer threads
        case '<':
            slowdownProducers();
            break;
            
        // speed up the production of ink for the producer threads
        case '>':
            speedupProducers();
            break;
            
		// test red ink up/down
		case 'r':
			ok = refillRedInk(MAX_ADD_INK);
			break;

        // make inklings move faster
        case '+':
            if (inklingSleepTime - 100000 > 0) {
                inklingSleepTime -= 100000;
            }
            break;
        
        // make inklings move slower
        case '-':
            if (inklingSleepTime + 100000 < 2147483647) {
                inklingSleepTime += 100000;
            }
            break;
            
		// test green ink up/down
		case 'g':
			ok = refillGreenInk(MAX_ADD_INK);
			break;

		// test blue ink up/down
		case 'b':
			ok = refillBlueInk(MAX_ADD_INK);
			break;

		default:
			ok = true;
			break;
	}

	if (!ok) {
		// print a or msg maybe?
	}
	
	// should we regenerate the interface here?
	// updateTerminal();
}

//---------------------------------------------------------------------------
//	Timer functions
//---------------------------------------------------------------------------

// custom timer function
void customTimerFunc(int milliseconds, std::function<void(int)> func, int val) {
    std::thread([=]() {
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
            func(val);
        } catch (const std::exception& e) {
            std::cerr << "ERROR darn :: customTimerFunc :: exception in thread: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "ERROR ah fudge :: customTimerFunc :: unknown exception in thread" << std::endl;
        }
    }).detach();
}

void myEventLoop(int val) {
    // the event loop is the main thread
    updateTerminal();

    // start the keyboard event listener thread
    // std::thread listenerThread(keyListener); // Hmmm, what does this do?

    // to track recursion depth
    static std::atomic<int> recursiveDepth = 0;
    
    // guard to ensure thread-safe access to recursiveDepth
    {
        std::lock_guard<std::mutex> lock(mtx);

        if (recursiveDepth >= 5) {
            return;
        }

        ++recursiveDepth;
    }
    
    customTimerFunc(1000, myEventLoop, val);
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        --recursiveDepth;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //check if the pipe exists
    if (access(pipePath.c_str(), F_OK) == 0) {
        //non blocking opening of the pipe that returns 0 if there is nothing to read in the pipe
        //only open the pipe once, eventually get a "too many files open error" without conditional
        if(numPipesOpen == 0){
            pipe_fd = open(pipePath.c_str(), O_RDONLY | O_NONBLOCK);
            numPipesOpen++;
        }
        
        //if pipe contains something, read from the pipe
        if (pipe_fd > 0) {
            char command;
            //read from the pipe
            int bytes_read = read(pipe_fd, &command, sizeof(command));
            if (bytes_read > 0) {
                //send the command to the myKeyboard function
                myKeyboard(command);
            }
        }
    }
}

void initializeFrontEnd(int argc, char** argv, void (*gridDisplayCB)(void), void (*stateDisplayCB)(void)) {
	// pass callbacks for drawing the grid and state of levels
	gridDisplayFunc = gridDisplayCB;
	stateDisplayFunc = stateDisplayCB;
    updateTerminal();
}