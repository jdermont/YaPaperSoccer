#include "game.h"

Game::Game() : startingPlayer(Player::NONE), currentPlayer(Player::NONE), field() {

}

void Game::startGame() {
    field.reset(new Field(Size::NORMAL,false));
    startingPlayer = startingPlayer==Player::ONE?Player::TWO:Player::ONE;
    currentPlayer = startingPlayer;
    movesNum = 0;
    notation = currentPlayer==Player::ONE?"A":"B";
}

bool Game::isOver() {
    return field->isBlocked(field->ball) || field->goal(field->ball)!=Player::NONE;
}

void Game::doMove(string move) {
    for (auto &c : move) {
        int n = nextNode(c);
        field->addEdge(field->ball,n);
        field->ball = n;
    }
    notation += move;
    movesNum++;
    if (!isOver() && !field->passNextDone(field->ball)) {
        changePlayer();
    }
}

int Game::nextNode(char c) {
    switch(c) {
        case '5': return field->getNeibghour(-1,1);
        case '4': return field->getNeibghour(0,1);
        case '3': return field->getNeibghour(1,1);
        case '6': return field->getNeibghour(-1,0);
        case '2': return field->getNeibghour(1,0);
        case '7': return field->getNeibghour(-1,-1);
        case '0': return field->getNeibghour(0,-1);
        case '1': return field->getNeibghour(1,-1);
    }
    return -1;
}

void Game::changePlayer() {
    currentPlayer = currentPlayer==Player::ONE?Player::TWO:Player::ONE;
}

Player Game::getWinner() {
    if (field->goal(field->ball)!=Player::NONE) {
        if (field->goal(field->ball) == Player::ONE) return Player::TWO;
        return Player::ONE;
    }
    if (currentPlayer == Player::ONE) return Player::TWO;
    return Player::ONE;
}
