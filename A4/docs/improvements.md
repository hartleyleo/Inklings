## Change 1: Enhanced Thread Safety for Ink Acquisition and Refilling


### Proposed Change Summary
Improve the thread safety mechanism in the program to handle ink acquisition and refilling more efficiently, preventing race conditions and ensuring accurate ink level tracking.

### Describe the Problem
The current implementation has potential race conditions in ink acquisition (acquireRedInk, acquireGreenInk, acquireBlueInk) and refilling functions (refillRedInk, refillGreenInk, refillBlueInk). While mutexes are used, they are not consistently applied, and some functions (acquireGreenInk and acquireBlueInk) incorrectly return false regardless of the operation's success.



### Describe the Solution
Implement a more robust thread synchronization mechanism using std::lock_guard to manage mutexes automatically, ensuring that locks are always released, even if an exception occurs. Fix the return values in acquireGreenInk and acquireBlueInk to accurately reflect the operation's success. The first version using std::lock_guard is better because it automatically locks and unlocks the mutex, ensuring exception safety and preventing resource leaks. The second version with manual locking is error-prone and can lead to issues like deadlocks if the mutex is not properly unlocked.


### Describe the Implemented Solution
- Replaced manual mutex locking with std::lock_guard in ink acquisition and refilling functions
- Fixed return values in acquireGreenInk and acquireBlueInk to accurately reflect success
- Removed redundant mutex locking in ink refilling functions
- Updated code to use new functions consistently


### Describe how you Tested your Implemented Solution
- Unit testing for ink acquisition and refilling functions
- Integration testing with modified functions in the main program
- Stress testing with large number of inklings and high ink consumption rates





## Change 2: <short 1-line headline describing change>


### Proposed Change Summary


### Describe the Problem


### Describe the Solution


### Describe the Implemented Solution


### Describe how you Tested your Implemented Solution

