#ifndef FIELD_H
#define FIELD_H

#include <vector>
#include <queue>
#include <cmath>
#include <string>
#include <algorithm>
#include <string.h>
#include <set>

using namespace std;

enum class Size {
    SMALL,NORMAL,BIG
};

enum class Player {
    NONE,ONE,TWO
};

class Point {
public:
    int x;
    int y;
    explicit Point(int x = 0, int y = 0) : x(x), y(y) {}
};

class Edge {
public:
    Point a;
    Point b;
    Player player;
    int x;
    int y;
    explicit Edge(Point a, Point b, Player player = Player::NONE) : a(a), b(b), player(player) {}
    bool operator==(const Edge &rhs) const {
        return a.x == rhs.a.x && a.y == rhs.a.y && b.x == rhs.b.x && b.y == rhs.b.y;
    }
    bool operator < (const Edge& other) const {
        int hash = x > y ? (x<<8) + y : (y<<8) + x;
        int hash2 = other.x > other.y ? (other.x<<8) + other.y : (other.y<<8) + other.x;
        return hash < hash2;
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
    bool operator==(const Path &rhs) const {
        return hashCode == rhs.hashCode;
    }
    bool operator < (const Path& other) const {
        return hashCode < other.hashCode;
    }
    Path(const Path& that) : a(that.a), b(that.b), hashCode(that.hashCode) {}
};

class Field {
public:
    int ball;
    int size,width,height;

    Size fieldSize;
    bool halfLine;

    vector<unsigned short> matrix;
    vector<unsigned short> matrixNodes;

    vector<vector<int> > matrixNeibghours;

    Field(Size type,bool halfLine);
    void addEdge(int a,int b);
    void removeEdge(int a,int b);
    bool passNext(int index);
    bool passNextDone(int index);
    bool passNextDoneCutOff(int index);
    bool isAlmostBlocked(int index);
    bool isBlocked(int index);
    bool onlyOneEmpty();
    bool onlyTwoEmpty();
    void fillFreeNeibghours(vector<int> &ns, int index);
    bool isNextMoveGameover(Player player);
    bool isCutOffFromOpponentGoal(Player player);
    string shortWinningMoveForPlayer(Player player);
    vector<Edge> getEdges();
    unsigned long getHash();
    unsigned long murmurHash3(unsigned long x);
    Player goal(int index);
    Point getPosition(int index);
    char getDistanceChar(int a, int b);
    int getNeibghour(int dx, int dy);
    vector<int> distances;
    void calculateDistances(Player player);
    vector<int> fillBlockedKonce();
    vector<int> fillBlockedCykle();

    void makeNeibghours(int index);
    int distance(Point p1, Point p2);
    vector<int> getNeibghours(int index);
    void removeAdjacency(int a, int b);

    Player getWinner(Player currentPlayer);
    set<int> dfs(int root);

private:
    vector<int> stos;
    vector<short> visited;

    vector<Player> goalArray;
    vector<Player> almostGoalArray;
    vector<Player> cutOffGoalArray;


};

#endif // FIELD_H
