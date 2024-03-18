# Design Document for Inklings Program


Prototyping First Approach
--------------------------

### Parts Implemented First

*   Created the Logging helper functions first and created logs.cpp
   
*   Implemented the creation of producer threads for each ink color (red, green, blue).
   
*   Implemented the creation of inkling threads and their associated data structures.
   

### Issues Encountered

*   Synchronization between inkling threads and producer threads proved to be more challenging than expected. Ensuring thread-safe access to shared resources required careful design and implementation of mutexes.
   
*   Logging the movements and events of inklings in a thread-safe manner while maintaining performance was a complex task.
   

### Areas to Research

1.  Thread synchronization techniques and best practices for multi-threaded programming in C++.
   
2.  Efficient logging mechanisms and strategies for concurrent access to log files.
   

### Plan to Solve Problems

1.  Conduct thorough research on thread synchronization techniques, focusing on the use of mutexes and lock-free programming.
   
2.  Implement synchronization mechanisms, such as mutexes, to protect shared resources like ink levels and grid cells.
   
3.  Design and implement a logging system that allows concurrent access to log files without compromising performance or data integrity.
   
4.  Continuously test and debug the program to identify and fix any synchronization issues or race conditions.
   

### Rough Outline

*   Implement synchronization mechanisms for ink levels and grid cells using mutexes.
   
*   Develop functions for inkling threads to acquire and consume ink safely.
   
*   Implement functions for producer threads to refill ink levels in a thread-safe manner.
   
*   Design and implement a logging system that supports concurrent access to log files.
   
*   Create a separate program (`logs.cpp`) to combine individual inkling log files into a single `actions.txt` file.
   

Planning First Approach
-----------------------

### Parts to Attempt in Order

1.  Design and implement the basic structure of the program, including the main function and necessary header files.
   
2.  Implement the creation of producer threads for each ink color (red, green, blue).
   
3.  Implement the creation of inkling threads and their associated data structures.
   
4.  Design and implement synchronization mechanisms for ink levels and grid cells using mutexes.
   
5.  Develop functions for inkling threads to acquire and consume ink safely.
   
6.  Implement functions for producer threads to refill ink levels in a thread-safe manner.
   
7.  Design and implement a logging system that supports concurrent access to log files.
   
8.  Create a separate program (`logs.cpp`) to combine individual inkling log files into a single `actions.txt` file.
   


## Overview
The Inklings program is a multi-threaded simulation that involves inklings moving on a grid and consuming ink as they move. The program utilizes ASCII art to render the grid and state information in the terminal. The main objective of the program is to ensure proper synchronization and coordination between the inkling threads and the ink producer threads.

## Design Decisions and Implementations

### 1. Creating Producer Threads
I plan to implement the creation of three producer threads, one for each ink color (red, green, and blue). These threads will be responsible for refilling the ink levels when they fall below the maximum level. The producer threads will be created using std::thread and will be assigned the respective color thread functions (redColorThreadFunc, greenColorThreadFunc, blueColorThreadFunc).

```cpp
std::thread red_producer(redColorThreadFunc);
std::thread green_producer(greenColorThreadFunc);
std::thread blue_producer(blueColorThreadFunc);

```

# 2. Creating Inkling Threads
I will create a thread for each inkling using std::thread and assign the threadFunction to each thread. The info vector will store the InklingInfo structs, which will contain information about each inkling, such as its ID, type, position, direction, and status. The inkling threads will be stored in the inklingThreads vector.
```cpp
std::vector<std::thread> inklingThreads;
for (auto& inkling : info) {
    inklingThreads.emplace_back(threadFunction, &inkling);
}
```

# 3. Joining Threads
After the main event loop (myEventLoop) finishes, I will join all the inkling threads and producer threads to ensure they have completed their execution before the program terminates. This will be done using the join function on each thread.

``` cpp
for (auto& thread : inklingThreads) {
    thread.join();
}

red_producer.join();
green_producer.join();
blue_producer.join();
```

# 4. Synchronization
To ensure proper synchronization and avoid race conditions, I plan to use mutexes to protect shared resources. Mutexes will be used to lock and unlock access to the ink levels (redLevel, greenLevel, blueLevel) and the grid cells.

The redLock, greenLock, and blueLock mutexes will be used to synchronize access to the respective ink levels.
The ink_mutex will be used to synchronize access to the ink levels in the producer threads.
I will implement the acquireRedInk, acquireGreenInk, and acquireBlueInk functions, which will be called by the inkling threads to acquire ink for movement. These functions will use the respective locks to ensure exclusive access to the ink levels.

```cpp
bool acquireRedInk(int theRed) {
    redLock.lock();
    bool ok = false;
    if (redLevel >= theRed) {
        redLevel -= theRed;
        ok = true;
    }
    redLock.unlock();
    return ok;
}
```
Similarly, I will implement the refillRedInk, refillGreenInk, and refillBlueInk functions, which will be called by the producer threads to refill the ink levels. These functions will use the ink_mutex to ensure exclusive access to the ink levels.

```cpp
bool refillRedInk(int theRed) {
    bool ok = false;
    ink_mutex.lock();
    if (redLevel + theRed <= MAX_LEVEL) {
        redLevel += theRed;
        ok = true;
    }
    ink_mutex.unlock();
    return ok;
}
```

# 5. Logging and Timestamping
I plan to include logging functionality to record the movements and events of each inkling. A separate log file will be created for each inkling in the "logFolder" directory. The log files will be named "inkling<id>.txt" and will contain timestamps, inkling ID, color, and position information.

I will implement the getCurrentTimestamp function to generate a formatted timestamp string for logging purposes. The logMovement function will be called within the threadFunction to log the movement of each inkling at each step.
```cpp
void logMovement(std::ofstream& logFile, InklingInfo* inkling) {
    logFile << getCurrentTimestamp() << ",inkling" << inkling->id << "," 
            << directionToString(inkling->dir) << ",row" << inkling->row 
            << ",col" << inkling->col << std::endl;
}
```

# logs.cpp

In addition to individual inkling log files, I will implement a separate program (logs.cpp) to combine all the log files into a single actions.txt file. This program will read all the log files in the "logFolder" directory, sort the log entries by timestamp, and write the combined logs to the actions.txt file.

The main function in logs.cpp will perform the following steps:

1. Define the path to the "logFolder" directory.
2. Create the "logFolder" directory if it does not exist.
3. Call the readAndCombineLogs function to read and combine all log files.
4. Call the writeCombinedLogs function to write the combined logs to actions.txt.

```c
int main() {
    fs::path logFolderPath = "./logFolder";
    
    if (!fs::exists(logFolderPath)) {
        fs::create_directory(logFolderPath);
        chmod(logFolderPath.c_str(), 0755);
    }

    std::vector<std::string> combinedLogs = readAndCombineLogs(logFolderPath);
    writeCombinedLogs(logFolderPath, combinedLogs);

    return 0;
}
```
The readAndCombineLogs function will iterate over all files in the "logFolder" directory, read their contents, and store them in a vector of strings. It will then sort the log entries by timestamp before returning the combined logs.

```c
std::vector<std::string> readAndCombineLogs(const fs::path& logFolderPath) {
    std::vector<std::string> combinedLogs;
    
    for (const auto& entry : fs::directory_iterator(logFolderPath)) {
        std::ifstream file(entry.path());
        if (file) {
            std::string line;
            while (std::getline(file, line)) {
                combinedLogs.push_back(line);
            }
        }
    }
    
    std::sort(combinedLogs.begin(), combinedLogs.end());
    
    return combinedLogs;
}
```
The writeCombinedLogs function will write the combined logs to the actions.txt file in the "logFolder" directory.

```c
void writeCombinedLogs(const fs::path& logFolderPath, const std::vector<std::string>& combinedLogs) {
    std::ofstream outFile(logFolderPath / "actions.txt");
    for (const auto& log : combinedLogs) {
        outFile << log << std::endl;
    }
}
```
The actions.txt file will provide a chronological view of all the inkling movements and events, making it easier to analyze and debug the program's behavior.



# Conclusion
The design and implementation of the Inklings program will focus on ensuring proper synchronization between the inkling threads and the ink producer threads. Mutexes will be used to protect shared resources and avoid race conditions. The program will also include logging functionality to record the movements and events of each inkling for analysis and debugging purposes.

The addition of the logs.cpp program will facilitate the combination of individual inkling log files into a single actions.txt file, providing a comprehensive view of the program's execution. The readAndCombineLogs function will read and combine the log files, while the writeCombinedLogs function will write the combined logs to the actions.txt file.
