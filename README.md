# pacmanAI

This is a pacman game, based on AI search algorithms (basically astar and bfs), which allow pacman and monsters to move by their own decisions.

Here how it works:

The project contains 3 monsters: Red, Turquoise and Green, one yellow pacman and about 300 coins distributed over the game board.

GAME BOARD:
The game board is 600x600 px, each wall's thickness is 5 px, except the letters 'H' and 'S' that are thicker. Every group of pawns (pacman and monsters) has its own maze matrix to track visited cells. 

COINS:
The coins distribution made by a BFS algorithm, which checks for every coin that pacman can eat it, by checking the closeness of each coin to the wall. (coin wouldn't be place in a distance less then 10 pixels from a wall). There are 301 coins on the board.

MONSTERS:
Three monsters start their way in the center 'pool', as in the original game.
All the monsters search their path within ASTAR algorithm, if one of them found its target, then it's ready to move, any way every monster will start moving after 300 iteration of its astar algo. After achieving its target point the monster will search again for a path to the updated target's location (whether it's pacman or a coin).
The turquoise monster (in the middle) starts its movement first of the three. After it's ready to move (by the terms explained above), it will perform move every 2 CPU clocks (counter in the idle method). 
The green monster will move every 4 CPU clocks, and the red will move every 6 CPU clocks.
Red and turquoise monsters' target is the pacman itself, and the green's target is the coin that pacman is moving towards.

 

PACMAN:
It starts with searching the nearest coin to its location when considering the distance of the coin from the nearest monster to pacman (the distance between the nearest monster and the coin should be over 200), that number isn't really calculated as the definition to compromise the nearest coin to pacman and the farther distance from monster isn't defined well, and can cause more problems in the pacman's ASTAR search.
After the nearest coin is set, pacman will start ASTAR search for path to that coin (the astar algorithm of pacman is different from monster's). pacman is allowed to move only straight (up, down, left, right) it's not allowed to move diagonal, while monsters can, so the astar algo is processing the possible path accordingly (not as we did in class). If it found a path to the coin, then it's ready to move, and it will move every 2 CPU clocks (as the turquoise monster). If it didn’t find path to coin, it will be ready to move after 600 astar iterations and the will search for a new coin to reach.




 
