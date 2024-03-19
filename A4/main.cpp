//
//  main.cpp
//  inklings
//
//  Authors: Jean-Yves Hervé, Shaun Wallace, and Luis Hernandez
//

 /*-------------------------------------------------------------------------+
 |	A graphic front end for a grid+state simulation.						|
 |																			|
 |	This application simply creates a a colored grid and displays           |
 |  some state information in the terminal using ASCII art.			        |
 |	Only mess with this after everything else works and making a backup		|
 |	copy of your project.                                                   |
 |																			|
 |	Current Keyboard Events                                     			|
 |		- 'ESC' --> exit the application									|
 |		- 'r' --> add red ink												|
 |		- 'g' --> add green ink												|
 |		- 'b' --> add blue ink												|
 +-------------------------------------------------------------------------*/

#include <random>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <mutex>
#include <algorithm>
#include "ascii_art.h"
#include <fstream>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>

//==================================================================================
//	Function prototypes
//==================================================================================
void displayGridPane(void);
void displayStatePane(void);
void initializeApplication(void);
std::string directionToString(TravelDirection dir);
void threadFunction(InklingInfo* inkling);
void getNewDirection(InklingInfo* inkling);
bool checkIfInCorner(InklingInfo* inkling);
void redColorThreadFunc();
void greenColorThreadFunc();
void blueColorThreadFunc();
bool checkEnoughInk(InklingInfo* inkling, int moveAmount);

//==================================================================================
//	Application-level global variables
//==================================================================================

//	The state grid and its dimensions
int** grid;
int NUM_ROWS, NUM_COLS;

//	the number of live threads (that haven't terminated yet)
int MAX_NUM_TRAVELER_THREADS;
int numLiveThreads = 0;

//  vector to store each struct
std::vector<InklingInfo> info;
bool DRAW_COLORED_TRAVELER_HEADS = true;

//	the ink levels
int MAX_LEVEL = 50;
int MAX_ADD_INK = 10;
int REFILL_INK = 10;
int redLevel = 20, greenLevel = 10, blueLevel = 40;

// create locks for color levels
std::mutex redLock;
std::mutex blueLock;
std::mutex greenLock;
std::mutex blueCellLock;
std::mutex redCellLock;
std::mutex greenCellLock;
std::mutex ink_mutex;

// ink producer sleep time (in microseconds)
// [min sleep time is arbitrary]
const int MIN_SLEEP_TIME = 30000; // 30000
int producerSleepTime = 100000; // 100000

// inkling sleep time (in microseconds)
int inklingSleepTime = 1000000; // 1000000


//==================================================================================
//	These are the functions that tie the simulation with the rendering.
//	Some parts are "don't touch."  Other parts need your help to ensure
//	that access to critical data and the ASCII art are properly synchronized
//==================================================================================

void displayGridPane(void) {
	//---------------------------------------------------------
	//	This is the call that writes ASCII art to render the grid.
	//
	//	Should we synchronize this call?
	//---------------------------------------------------------
    drawGridAndInklingsASCII(grid, NUM_ROWS, NUM_COLS, info);
}

void displayStatePane(void) {
	//---------------------------------------------------------
	//	This is the call that updates state information
	//
	//	Should we synchronize this call?
	//---------------------------------------------------------
	drawState(numLiveThreads, redLevel, greenLevel, blueLevel);
}

//------------------------------------------------------------------------
//	These are the functions that would be called by a inkling thread in
//	order to acquire red/green/blue ink to trace its trail.
//	You *must* synchronize access to the ink levels (C++ lock and unlock)
//------------------------------------------------------------------------
// You probably want to edit these...
bool acquireRedInk(int theRed) {
    std::lock_guard<std::mutex> lock(redLock);
    if (redLevel >= theRed) {
        redLevel -= theRed;
        return true;
    }
    return false;
}

bool acquireGreenInk(int theGreen) {
    std::lock_guard<std::mutex> lock(greenLock);
    if (greenLevel >= theGreen) {
        greenLevel -= theGreen;
        return true;
    }
    return false;
}

bool acquireBlueInk(int theBlue) {
    std::lock_guard<std::mutex> lock(blueLock);
    if (blueLevel >= theBlue) {
        blueLevel -= theBlue;
        return true;
    }
    return false;
}


//------------------------------------------------------------------------
//	These are the functions that would be called by a producer thread in
//	order to refill the red/green/blue ink tanks.
//	You *must* synchronize access to the ink levels (C++ lock and unlock)
//------------------------------------------------------------------------
// You probably want to edit these...
bool refillRedInk(int theRed) {
    std::lock_guard<std::mutex> lock(ink_mutex);

    if (redLevel + theRed <= MAX_LEVEL) {
        redLevel += theRed;
        return true;
    } else {
        redLevel = MAX_LEVEL;  // Cap the ink level at MAX_LEVEL
        return false;
    }
}
bool refillGreenInk(int theGreen) {
    std::lock_guard<std::mutex> lock(ink_mutex);

    if (greenLevel + theGreen <= MAX_LEVEL) {
        greenLevel += theGreen;
        return true;
    } else {
        greenLevel = MAX_LEVEL;  // Cap the ink level at MAX_LEVEL
        return false;
    }
}

bool refillBlueInk(int theBlue) {
    std::lock_guard<std::mutex> lock(ink_mutex);

    if (blueLevel + theBlue <= MAX_LEVEL) {
        blueLevel += theBlue;
        return true;
    } else {
        blueLevel = MAX_LEVEL;  // Cap the ink level at MAX_LEVEL
        return false;
    }
}
//------------------------------------------------------------------------
//	You shouldn't have to touch this one.  Definitely if you do not
//	add the "producer" threads, and probably not even if you do.
//------------------------------------------------------------------------
void speedupProducers(void) {
	// decrease sleep time by 20%, but don't get too small
	int newSleepTime = (8 * producerSleepTime) / 10;
	
	if (newSleepTime > MIN_SLEEP_TIME) {
		producerSleepTime = newSleepTime;
	}
}

void slowdownProducers(void) {
	// increase sleep time by 20%
	producerSleepTime = (12 * producerSleepTime) / 10;
}

//-------------------------------------------------------------------------------------
//	You need to change the TODOS in the main function to pass the the autograder tests
//-------------------------------------------------------------------------------------
int main(int argc, char** argv) {
    // a try/catch block for debugging to catch weird errors in your code
    try {
        // check that arguments are valid, must be a 20x20 or greater and at least 8 threads/inklings
        if (argc == 4) {
            if (std::stoi(argv[1]) >= 20 && std::stoi(argv[2]) >= 20 && std::stoi(argv[3]) >= 8) {
                NUM_ROWS = std::stoi(argv[1]);
                NUM_COLS = std::stoi(argv[2]);
                MAX_NUM_TRAVELER_THREADS = std::stoi(argv[3]);
                numLiveThreads = std::stoi(argv[3]);
            }
        } else {
            std::cout << "No arguments provided, running with 8x8 grid and 4 threads.\n\tThis message will dissapear in 2 seconds... \n";
            sleep(2); // so the user can read the message in std::cout one line up
            clearTerminal();
            // some small defaults, will these run?
            NUM_ROWS = 8;
            NUM_COLS = 8;
            MAX_NUM_TRAVELER_THREADS = 4;
            numLiveThreads = 4;
        }
        
        initializeFrontEnd(argc, argv, displayGridPane, displayStatePane);
        
        initializeApplication();

        // TODO: create producer threads that check the levels of each ink
        std::thread red_producer(redColorThreadFunc);
        std::thread green_producer(greenColorThreadFunc);
        std::thread blue_producer(blueColorThreadFunc);
        
        // TODO: create threads for the inklings
        std::vector<std::thread> inklingThreads;
        for (auto& inkling : info) {
            inklingThreads.emplace_back(threadFunction, &inkling);
        }
       
        // now we enter the main event loop of the program
        myEventLoop(0);

        // Join inkling threads:
        for (auto& thread : inklingThreads) {
          thread.join();
        }

        // Join producer threads:
        red_producer.join();
        green_producer.join();
        blue_producer.join();

        // ensure main does not return immediately, killing detached threads
        std::this_thread::sleep_for(std::chrono::seconds(30));
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR :: Oh snap! unhandled exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "ERROR :: Red handed! unknown exception caught" << std::endl;
    }

	return 0;
}


//==================================================================================
//
//	TODO this is a part that you have to edit and add to.
//
//==================================================================================

void cleanupAndQuit(const std::string& msg) {
  std::cout << "Somebody called quits, goodbye sweet digital world, this was their message: \n" << msg;


  // Deallocate grid memory
  for (int i = 0; i < NUM_ROWS; i++) {
    delete[] grid[i];
  }
  delete[] grid;

  // Clear any additional data structures here (if applicable)

  exit(0);
}



std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time_t), "%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << now_ms.count();

    return ss.str();
}
std::string colorToString(InklingType type) {
    switch (type) {
        case RED_TRAV: return "red";
        case GREEN_TRAV: return "green";
        case BLUE_TRAV: return "blue";
        default: return "unknown";
    }
}
void initializeApplication(void) {
    grid = new int*[NUM_ROWS];
    for (int i = 0; i < NUM_ROWS; i++) {
        grid[i] = new int[NUM_COLS]{0};  // Initialize grid cells to 0
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distRow(1, NUM_ROWS - 2);
    std::uniform_int_distribution<> distCol(1, NUM_COLS - 2);
    std::uniform_int_distribution<> colorDist(0, 2);

    std::filesystem::path logFolderPath = "./logFolder";
    std::filesystem::create_directory(logFolderPath);
    chmod(logFolderPath.c_str(), 0755);

    for (int i = 0; i < MAX_NUM_TRAVELER_THREADS; ++i) {
        int row, col;
        do {
            row = distRow(gen);
            col = distCol(gen);
        } while (std::any_of(info.begin(), info.end(), [row, col](const InklingInfo& inkling) {
            return inkling.row == row && inkling.col == col;
        }));

        InklingInfo newInkling = {
            i + 1,  // Assign ID starting from 1
            static_cast<InklingType>(colorDist(gen)),
            row, col,
            static_cast<TravelDirection>(gen() % 4),
            true
        };

        info.push_back(newInkling);

        std::string filename = logFolderPath / ("inkling" + std::to_string(newInkling.id) + ".txt");
        std::ofstream logFile(filename);
        logFile << getCurrentTimestamp() << ",inkling" << newInkling.id << "," 
                << colorToString(newInkling.type) << ",row" << newInkling.row 
                << ",col" << newInkling.col << std::endl;
        logFile.close();
        chmod(filename.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    }
}




// Function to check if the inkling is in a corner
bool checkIfInCorner(InklingInfo* inkling) {
  return (inkling->row == 0 || inkling->row == NUM_ROWS - 1) &&
         (inkling->col == 0 || inkling->col == NUM_COLS - 1);
}

// Function to check if the inkling has enough ink to move
bool checkEnoughInk(InklingInfo* inkling) {
  switch (inkling->type) {
    case RED_TRAV:
      return redLevel >= 1;
    case GREEN_TRAV:
      return greenLevel >= 1;
    case BLUE_TRAV:
      return blueLevel >= 1;
  }
  return false;
}

std::string directionToString(TravelDirection dir) {
    switch (dir) {
        case NORTH: return "north";
        case SOUTH: return "south";
        case EAST: return "east";
        case WEST: return "west";
        default: return "unknown";
    }
}
int getMaxValidSteps(InklingInfo* inkling, int proposedSteps) {
    int maxSteps = proposedSteps;
    while (maxSteps > 0) {
        switch(inkling->dir) {
            case NORTH:
                if (inkling->row - maxSteps < 0) maxSteps--;
                break;
            case SOUTH:
                if (inkling->row + maxSteps >= NUM_ROWS) maxSteps--;
                break;
            case WEST:
                if (inkling->col - maxSteps < 0) maxSteps--;
                break;
            case EAST:
                if (inkling->col + maxSteps >= NUM_COLS) maxSteps--;
                break;
        }
        if (maxSteps <= 0) {
            // Change direction if no valid steps
            inkling->dir = static_cast<TravelDirection>((inkling->dir + 2) % 4);
            maxSteps = proposedSteps;
        } else {
            break;
        }
    }
    return maxSteps;
}
void logMovement(std::ofstream& logFile, InklingInfo* inkling) {
    logFile << getCurrentTimestamp() << ",inkling" << inkling->id << "," 
            << directionToString(inkling->dir) << ",row" << inkling->row 
            << ",col" << inkling->col << std::endl;
}

void threadFunction(InklingInfo* inkling) {
    std::string logFilePath = "./logFolder/inkling" + std::to_string(inkling->id) + ".txt";
    std::ofstream logFile(logFilePath, std::ios_base::app);  // Open in append mode

    while (inkling->isLive) {
        std::random_device rd;
        std::mt19937 rn_gen(rd());
        std::uniform_int_distribution<> step_dist(1, 5);

        int proposedSteps = step_dist(rn_gen);
        int validSteps = getMaxValidSteps(inkling, proposedSteps);

        for (int i = 0; i < validSteps && checkEnoughInk(inkling) && inkling->isLive; i++) {
            logMovement(logFile, inkling);
            // Inline movement logic here
            switch (inkling->dir) {
                case NORTH: inkling->row--; break;
                case SOUTH: inkling->row++; break;
                case WEST:  inkling->col--; break;
                case EAST:  inkling->col++; break;
            }

            if (checkIfInCorner(inkling)) {
                inkling->isLive = false;
                numLiveThreads--;
            }

            // Ink consumption logic
            switch (inkling->type) {
                case RED_TRAV:   redLevel--; break;
                case GREEN_TRAV: greenLevel--; break;
                case BLUE_TRAV:  blueLevel--; break;
            }
        }

        getNewDirection(inkling);
        std::this_thread::sleep_for(std::chrono::microseconds(inklingSleepTime));
    }

    logFile << getCurrentTimestamp() << ",inkling" << inkling->id << ",terminated\n";
    logFile.close();
}

// Function to get a new random direction (implementation left for you)
void getNewDirection(InklingInfo* inkling) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dirDist(0, 3); // For four directions

    TravelDirection newDir = inkling->dir;
    do {
        newDir = static_cast<TravelDirection>(dirDist(gen));
    } while (newDir == inkling->dir || (newDir + 2) % 4 == inkling->dir);

    inkling->dir = newDir;
}

void redColorThreadFunc() {
  while (true) {
    // Simulate production time
    std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Adjust sleep time as needed

    // Lock access to red ink level
    std::lock_guard<std::mutex> lock(redLock);

    // Check if red ink level is below maximum
    if (redLevel < MAX_LEVEL) {
      // Refill red ink
      if (MAX_LEVEL - redLevel <= 10) {
        redLevel += MAX_LEVEL - redLevel;
      } else {
        redLevel += REFILL_INK;
      }
      printf("Red ink refilled to %d\n", redLevel); // Optional for visual output
    }
  }
}

void greenColorThreadFunc() {
  while (true) {
    // Simulate production time
    std::this_thread::sleep_for(std::chrono::milliseconds(1500)); // Adjust sleep time as needed

    // Lock access to green ink level
    std::lock_guard<std::mutex> lock(greenLock);

    if (greenLevel < MAX_LEVEL) {
      if (MAX_LEVEL - greenLevel <= 10) {
        greenLevel += MAX_LEVEL - greenLevel;
      } else {
        greenLevel += REFILL_INK;
      }
      printf("Green ink refilled to %d\n", greenLevel); // Optional for visual output
    }
  }
}

void blueColorThreadFunc() {
  while (true) {
    // Simulate production time
    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // Adjust sleep time as needed

    // Lock access to blue ink level
    std::lock_guard<std::mutex> lock(blueLock);

    if (blueLevel < MAX_LEVEL) {
      if (MAX_LEVEL - blueLevel <= 10) {
        blueLevel += MAX_LEVEL - blueLevel;
      } else {
        blueLevel += REFILL_INK;
      }
      printf("Blue ink refilled to %d\n", blueLevel); // Optional for visual output
    }
  }
}