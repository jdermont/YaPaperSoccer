#ifndef CPU_H
#define CPU_H

#include <random>
#include <vector>
#include <string>
#include <stack>
#include <deque>
#include <utility>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <functional>
#include <memory>
#include <chrono>
#include <stdint.h>
#include <thread>
#include <atomic>
//#include <android/log.h>
#include <mutex>

#include "field.h"

#define MAX_GOAL 900
#define MAX_ONE_EMPTY 850
#define MAX_CUTOFF 800
#define MIN_CUTOFF -700
#define MIN_GOAL_ONE_EMPTY -750
#define MIN_GOAL_NEXT_MOVE -800
#define MIN_GOAL -850
#define MIN_BLOCKED -900

#define NOTDEFINED 30101
#define MAXTIME 10000000L

#define VIRTUAL_LOSS 3

using namespace std;
using namespace std::chrono;

enum class Difficulty {
    VERYEASY,EASY,NORMAL,ADVANCED,HARD,EXPERIMENTAL
};

class SpinLock {
    std::atomic_flag locked = ATOMIC_FLAG_INIT ;
public:
    void lock() {
        while (locked.test_and_set(std::memory_order_acquire)) { ; }
    }
    void unlock() {
        locked.clear(std::memory_order_release);
    }
};

class Move {
public:
    short score;
    string move;
    bool terminate;
    int score2;
    int games;
    Player player;
    Move *parent;
    unsigned long hash;
    vector<Move*> moves;
    shared_ptr<mutex> lock;
    unsigned char virtualLoss;
    explicit Move(short score = 0, string move = "", bool terminate = false, int score2 = 0, int games = 0, Player player = Player::NONE, Move *parent = NULL, unsigned long hash = 0L) : score(score), move(move), terminate(terminate), score2(score2), games(games), player(player), parent(parent), hash(hash) {
        virtualLoss = 0;
        lock.reset(new mutex());
    }
    virtual ~Move() { for (auto & m : moves) { delete m; } }
    friend ostream& operator<< (ostream &out, Move &move) {
        out << move.score << ": " << move.move;
        return out;
    }
    bool operator < (const Move& move) const {
        return move.score < score;
    }
    bool operator > (const Move& move) const {
        return move.score > score;
    }
};

class ShortMove {
public:
    short score;
    string move;

    explicit ShortMove(short score = 0, string move = "") : score(score), move(move) {

    }

};

#include <sstream>
namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

class Cpu
{
public:
    Player player;
    Difficulty difficulty;
    int movesNum;
    shared_ptr<Field> field;
    int ruchy;
    Cpu(Player player, Difficulty difficulty, int threadsNum);
    void setField(Field *field);
    void setField(shared_ptr<Field> field);
    string getBestMove();
    string getBestMoveMCTS();
    int benchmarkMCTS();
    void release();

private:
    int games;
    int levels;
    unsigned int limit;
    int threadsNum;
    long maxTime;
    vector<vector<int> > scoresArray;
    bool measureTime;
    high_resolution_clock::time_point start;
    bool alreadyBlocking;
    bool alreadyBlocked;
    bool isReleased;
    uint64_t seed[2];
    void fisherYates(vector<int> & ints);
    uint64_t xorshift128plus();

    unsigned int randomInt(unsigned int max);
    void updateScores();
    static int hashPaths(vector<Path> &paths);
    Move* chooseBestMove(vector<Move*> & moves);
    vector<Move*> getMovesEasy();
    vector<Move*> getMovesSophisticated(int level);
    int getScore(Player player, int level, int alpha, int beta);
    vector<Path> makeMove(string &move);
    void undoMove(vector<Path> & paths);
    int nextNode(char c);

    void fillRuchy(vector<Move*> & moves, SpinLock & globalLock);
    void wybierz(vector<Move*> & moves, int games, int level);
    vector<Move*> getMoves(Move *parent, Player player, int limit, int level = 0, int poziomy = 0);
    int getMovesScore(Player player, int limit, int level = 0);
    int simulateOnePointers(Player currentPlayer, int limes = 30);
    vector<ShortMove*> getMovePointers(Player player, int limit, int level = 0);

    bool provenEnd = false;
    const double C = 0.5;
    mt19937 mt;
};

#endif // CPU_H
