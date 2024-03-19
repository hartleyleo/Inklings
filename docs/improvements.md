## Change 1: Inklings paint their path (VISUAL ONLY)


### Proposed Change Summary
Since the `int** grid` variable can technically be gotten rid of and have no effect on the program, we can either remove it or use it for something. We want to use it to have inklings actually paint their path for the grid.

### The Problem
Visually, this program is great but doesn't do much besides show where the inkling is and where it is going. 

### The Solution
Since the whole idea of the program is to have the inklings paint the grid they are on, we can go ahead and use the existing grid variable to store which inklings have most recently visited which cell. This will provide a better visual for the user to understand what is happening in the program.

### The Implemented Solution
Utilizing the previously mentioned `int** grid` 2D array, we can store a number representing a color at each cell. These values will be populated into the array as an inkling travels over it and has access to an ink resource. For instance, if a RED inkling passes over `[2][2]`, and has enough ink resource, it will mark that cell as `1`, a BLUE inkling will mark it `2`, and Green a `3`. Unmarked cells will remain a `0`.

Utilizing this grid of values, we can have it used in the `drawGridAndInklingsASCII` function. This will take in the grid, and as the loops go through the grid, it will print out the cell as the respective color. We use this to check if the cell has an inkling, if not, then it will go into a switch statement that will see what the current value of the cell is. It will then paint it `RED` `BLUE` `GREEN` or `BLACK` respectively based on the cell values of `1` `2` `3` `0`.

### Describe how you Tested your Implemented Solution
To test the solution that we implemented, we had to go through two iterations. The first iteration was the create a struct that had an int to store color, and an inkling set to null to store an inkling that visited it. It was at this point of coding that we found that the grid actually didn't store inklings, and actually had little to no purpose at all. So we could technically have sped the program up by deleting it, but rather we kept it so that it would serve the purpose of storing color data. This switch might add some weight to the program, but in reality it helps the user visually see what is happening.

We tested it further by trying many different input sizes, and adding in multiple inklings. With heavy print statements we could see the integer values of each cell in the grid being changed as the program ran, especially seeing that when two opposing inklings ran over the same cell, it took the last inkling to run over it as the latest value.




## Change 2: Capping ink levels at the MAX_LEVEL

### Proposed Change Summary


### Describe the Problem
The program will add ink to the ink tanks at a random incriment. Since the program at that point in time just refilled the ink via the `INK_REFILL` variable, so it always refills that amount. The program doesnt check the ink levels beforehand, so sometimes it'll fill the tank over the max fill level. 

### Describe the Solution
Create a way to check if the ink that will be added to the tank is going to make it overflow, if so, then it will only add the amount of ink that will completely fill it, and not overfill it. Therefore, it will only ever cap at the `MAX_INK_AMOUNT`. If the amount of ink to be put in the tank will not make it overflow, then it will just fill as normal.


### Describe the Implemented Solution
The solution implementation is quite simple. The code originally just checked if the ink levels were below max, and then refilled them. For our solution, we do the same, but rather than straight refilling them, we:
1. Check if the max level minus the current ink level is <= the refill amount
2. If so, add the max level minus the current level amount of ink.
3. If not, just add the refull amount.

This implementation makes it so that it will actually only ever be filled to the tanks capacity, and not somehow overfill to a state above the max capacity.


### Describe how you Tested your Implemented Solution
We tested the implemented solution by printing to the console the total amounts of ink that was being added and when. It helped us understand the totals and how they were being checked by the program. In addition to this, we also had the visual update on the page to see the current ink levels for the user to view. We also updated it to include a live thread count counter that actually updated the total live threads.