#ifndef FIELD_H
#define FIELD_H

#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>

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
    explicit Edge(Point a, Point b) : a(a), b(b) {}
};

class Field {
public:
    int ball;
    int size,width,height;

    Field(Size type,bool halfLine);
    void addEdge(int a,int b);
    void removeEdge(int a,int b);
    bool passNext(int index);
    bool passNextDone(int index);
    bool isAlmostBlocked(int index);
    bool isBlocked(int index);
    bool onlyOneEmpty();
    void fillFreeNeibghours(vector<int> &ns, int index);
    bool isNextMoveGameover(Player player);
    bool isCutOffFromOpponentGoal(Player player);
    string shortWinningMoveForPlayer(Player player);
    vector<Edge> getEdges();
    Player goal(int index);
    Point getPosition(int index);
    char getDistanceChar(int a, int b);
    int getNeibghour(int dx, int dy);

private:
    vector<int> matrix;
    vector<int> matrixNodes;
    vector<vector<int> > matrixNeibghours;
    vector<Player> goalArray;
    vector<Player> almostGoalArray;
    vector<Player> cutOffGoalArray;

    vector<int> stos;
    vector<short> visited;

    void makeNeibghours(int index);
    int distance(Point p1, Point p2);
    vector<int> getNeibghours(int index);
    void removeAdjacency(int a, int b);
};

#endif // FIELD_H
