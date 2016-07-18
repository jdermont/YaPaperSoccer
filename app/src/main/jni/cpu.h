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
//#include <android/log.h>

#include "field.h"

#define MAX 500
#define MAXB 480
#define MINOB -440
#define MIN -460
#define MING -480
#define MINB -500
#define NOTDEFINED 30101

#define MOVECHANGE 30

using namespace std;
using namespace std::chrono;

enum class Difficulty {
    VERYEASY,EASY,NORMAL,ADVANCED,HARD
};

class Move {
public:
    int score;
    string move;
    int distance;
    bool terminate;
    explicit Move(int score, string move, int distance = 0, bool terminate = false) : score(score), move(move), distance(distance), terminate(terminate) {}
    friend ostream& operator<< (ostream &out, Move &move) {
        out << move.score << ": " << move.move;
        return out;
    }
    bool operator < (const Move& move) const {
        if (score == move.score) {
            return move.distance < distance;
        }
        return move.score < score;
    }
    bool operator > (const Move& move) const {
        if (score == move.score) {
            return move.distance > distance;
        }
        return move.score > score;
    }
};

class Path {
public:
    int a;
    int b;
    int hashCode;
    explicit Path(int a, int b) : a(a), b(b), hashCode((a>b)?((b<<16)+a):((a<<16)+b)) {}
    ~Path() { }
    Path& operator=(const Path& that) {
        a = that.a;
        b = that.b;
        hashCode = that.hashCode;

        return *this;
    }
    Path(const Path& that) : a(that.a), b(that.b), hashCode(that.hashCode) {}
};

class Cpu
{
public:
    Player player;
    int movesNum;
    shared_ptr<Field> field;
    Cpu(Player player, Difficulty difficulty);
    void setField(Field *field);
    void setField(shared_ptr<Field> field);
    string getBestMove();
    void release();

private:
    Difficulty difficulty;
    int levels;
    unsigned int limit;
    mt19937 mt;
    vector<vector<int> > scoresArray;
    bool measureTime;
    high_resolution_clock::time_point start;
    bool alreadyBlocking;
    bool alreadyBlocked;
    bool isReleased;

    int randomInt(int max);
    void updateScores();
    static int hashPaths(vector<Path> &paths);
    Move chooseBestMove(vector<Move> & moves);
    vector<Move> getMovesEasy();
    vector<Move> getMovesSophisticated(int level);
    int getScore(Player player, int level, int alpha, int beta);
    vector<Path> makeMove(string &move);
    void undoMove(vector<Path> & paths);
    int nextNode(char c);
};

#endif // CPU_H
