#ifndef GAME_H
#define GAME_H

#include <string>
#include <memory>
#include "field.h"

class Game
{
public:
    Player startingPlayer;
    Player currentPlayer;
    shared_ptr<Field> field;
    string notation;
    int movesNum;

    Game();
    void startGame();
    bool isOver();

    void doMove(string move);
    int nextNode(char c);
    void changePlayer();
    Player getWinner();
private:

};

#endif // GAME_H