# Paper Soccer

Paper soccer (or paper hockey) is an abstract strategy game played on a square grid representing a football or hockey field. Players take turns to extend a line representing the position of a ball, until it reaches one of the grid's two goal spaces. More on [wiki][wiki].

### Ya Paper Soccer

The game is available on [Google Play][yapapersoccer]. The limited source code (single player, AI, benchmark) is available in this repository. It wasn't intended to be published public at first, so parts of the code might seem stupid at least.

# Brief theory

Ya Paper Soccer uses [min-max][minmax] with [alpha-beta][alphabeta] pruning algorithm. There is also a bit of [iterative deepening][itedeep]. (Update: August 2018) Experimental level uses Monte Carlo Tree Search.

The move is whatever player can do during his turn. The move may contain one bounce but as well several bounces. This will be convenient because each turn can be represented as one ply. The order of move within a turn doesn't matter, so sometimes "different" moves will have the same result. Let's look at example:

![Paper Soccer 1](images/papersoccer01.png)

Moves ABCDA2 and ADCBA2 are the same (they have the same edges). Quite a time took me to write an algorithm that would detect early the repetitions and discard them. In above picture there are 57 all possible moves, but when ordering doesn't count, there are 52 unique moves.

# Some numbers

I could make huge mistakes within algorithm or the measures so don't take these numbers for granted.

I made program which played thousands games AI vs AI on normal (8x10) pitch. So the average number of possible moves per ply is about 62, the standard deviation is about 202.5. Sometimes there are few hundred possible moves and rarely thousands or even tens of thousands possible moves. Once I encountered over 400000 possible moves for one player! This is it:

![Paper Soccer 1](images/papersoccer02.png)

To get there, you need simply do 13064113570225017036575014630745741422243523574227742164435274617703502106741745431672745757134661453130361 (notation: 0 - go up, 1 - go up right, 2 - go right etc.). And the all possible moves are written in this [file](papersoccer02_possible_moves).

# Strategy

The simplest strategy is to move towards opponent goal. But to my surprise going backwards is more efficient. In AI vs AI battles, when the depth and limits are the same, the AI which go backwards wins from 2 to 5 times more than the 'normal'! Provided I didn't make stupid mistake like letting get to my goal in 1 turn, when using this strategy, I could easier win with the computer. Computer of course wouldn't make easily those mistakes, since checking if the turn will result in goal is quite trivial.

In earliest versions of Ya Paper Soccer the hardest cpu and the second hardest cpu differed only that first was going backwards and seconds forwards. Well, with newest versions the hardest cpu uses more depth and iterative deepening.

# Code

The pitch is implemented as undirected graph. This data structure seems the most natural. The lines intersections are vertices, the lines themselves are edges. Each vertice has up to 8 neighbours. There are (width+1) * (height+1) + 6 vertices, so the normal (8x10) pitch contains 105 vertices.

The graph itself is represented as adjacency matrix and adjacency list. It may require a little bit more memory but thanks to that many operations will have O(1) time complexity. Generally the faster you can manipulate graph, the more moves you can calculate. On my phone (LG L9 II / LG D605) the speed is about 100000 moves per second.

Movement calculation uses depth-first search and a bit of breadth-first search with memory about cycles. It traverses the adjacent vertices randomly. It is trivial to check if there is a path to our or opponent's goal. It is also quite trivial to see if we are cut off from opponent goal or we cut off opponent from our goal. Because there can be many possible moves per ply, I limit them to few hundred.

Algorithm used in Ya Paper Soccer:
- very easy - depth 1, limit 20, additional limits
- easy - depth 1, limit 30, additional limits
- normal - depth 1+, limit 100, checks for cut offs
- advanced - depth 2+, limit 200, checks for cut offs
- hard - goes backwards, depth do 3+ (iterative deepening, max 10 seconds for turn), limit 200, checks for cut offs

Limit is max moves calculated per ply. Depth is game tree depth. 1 means our moves. 2 means our moves and opponent's responses etc. The + means that in some situation it will check additional depth. A mini solution for [horizon effect][horizon].

The AI itself is written in C++. On phones it is several times faster than pure java and on PC about 2-3x faster than java version.

   [minmax]: <https://en.wikipedia.org/wiki/Minimax#Minimax_algorithm_with_alternate_moves>
   [alphabeta]: <https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning>
   [yapapersoccer]: <https://play.google.com/store/apps/details?id=pl.derjack.papersoccer>
   [wiki]: <https://en.wikipedia.org/wiki/Paper_soccer>
   [horizon]: <https://en.wikipedia.org/wiki/Horizon_effect>
   [itedeep]: <https://en.wikipedia.org/wiki/Iterative_deepening_depth-first_search>
   
