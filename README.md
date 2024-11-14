# IZP
Author: Klára Jánová

Program description: The program searches for a way out in a maze specified by a text file according to arbitrary coordinates.
		
The program prints out the coordinates of successive shelves of the maze that lead out of the maze.

Program arguments:

--help: prints help and exits the program

--test filename.txt: checks if the map is specified correctly

--rpath filename.txt: searches and prints the path from the given coordinate point according to the right-hand rule

--lpath filename.txt: searches and prints the path from the given coordinate point according to the left-hand rule

--shortest filename.txt: searches and prints the shortest possible path from the given coordinate point

Parameter --shortest searches for the shortest way out of the maze
The indexes row and col can be anywhere inside the maze.
Uses the breadth first search algorithm using a queue.
All neighbors of the current triangle that can be accessed through a leaky wall are loaded into the queue, then the current triangle
is marked as processed, 
We select its neighbours from the queue and add the neighbours of these neighbours to the queue again, etc. Once a neighbor is out of the maze, the algorithm ends and the path is printed using the array of predecessors or successors 
