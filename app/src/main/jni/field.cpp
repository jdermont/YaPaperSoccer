#include "field.h"

Field::Field(Size fieldSize, bool halfLine)  {
    switch (fieldSize) {
    case Size::SMALL:
    width = 6;
    height = 8;
    break;
    case Size::BIG:
    width = 10;
    height = 12;
    break;
    default:
    width = 8;
    height = 10;
    }

    int w = width+1;
    int h = height+1;

    int wh = w*h;
    size = wh+6;
    matrix.assign(size*size,0);
    matrixNodes.assign(size,0);

    // assign 2D points into matrix
    for (int i=0;i<wh;i++) {
        int x = i/h;
        int y = i%h;
        matrix[i*size+i] = (x<<8)+y;
    }
    // goals
    matrix[wh*size+wh] = ((width/2-1)<<8) + 0xFF;
    matrix[(wh+1)*size+wh+1] = ((width/2)<<8) + 0xFF;
    matrix[(wh+2)*size+wh+2] = ((width/2+1)<<8) + 0xFF;
    matrix[(wh+3)*size+wh+3] = ((width/2-1)<<8) + h;
    matrix[(wh+4)*size+wh+4] = ((width/2)<<8) + h;
    matrix[(wh+5)*size+wh+5] = ((width/2+1)<<8) + h;

    // make Neibghours
    for (int i=0;i<size;i++) {
        makeNeibghours(i);
    }
    // remove some 'adjacency' from goals
    removeAdjacency(wh,h*(width/2-2));
    removeAdjacency(wh+2,h*(width/2+2));
    removeAdjacency(wh+3,h*(width/2-1)-1);
    removeAdjacency(wh+5,h*(width/2+3)-1);

    // add edges except for goal
    for (int i=0;i<wh-1;i++) {
        for (int j=i+1;j<wh;j++) {
            Point p = getPosition(i);
            Point p2 = getPosition(j);
            if ((p.x == 0 && p2.x == 0) && distance(p,p2)<=1) addEdge(i,j);
            else if ((p.x == width && p2.x == width) && distance(p,p2)<=1) addEdge(i,j);
            else if ((p.y == 0 && p2.y == 0) && distance(p,p2)<=1) {
                if (p.x < width/2-1 || p.x >= width/2+1) addEdge(i,j);
           } else if ((p.y == height && p2.y == height) && distance(p,p2)<=1) {
                if (p.x < width/2-1 || p.x >= width/2+1) addEdge(i,j);
           } else if (halfLine && (p.y == height/2 && p2.y == height/2) && distance(p,p2)<=1) addEdge(i,j);
        }
    }
    // and goals
    addEdge(wh,wh+1);
    addEdge(wh+1,wh+2);
    addEdge(wh,h*(width/2-1));
    addEdge(wh+2,h*(width/2+1));
    addEdge(wh+3,wh+4);
    addEdge(wh+4,wh+5);
    addEdge(wh+3,h*(width/2)-1);
    addEdge(wh+5,h*(width/2+2)-1);

    // create adjacency list
    matrixNeibghours.reserve(size);
    for (int i=0;i<size;i++) {
        matrixNeibghours.push_back(getNeibghours(i));
        matrixNodes[i] += matrixNeibghours[i].size()<<4;
    }

    // create auxiliary arrays for goals
    goalArray.assign(size,Player::NONE);
    almostGoalArray.assign(size,Player::NONE);
    cutOffGoalArray.assign(size,Player::NONE);
    for (int i=wh;i<size;i++) {
        goalArray[i] = (i-wh)/3==0?Player::ONE:Player::TWO;
        almostGoalArray[i] = (i-wh)/3==0?Player::ONE:Player::TWO;
        cutOffGoalArray[i] = (i-wh)/3==0?Player::ONE:Player::TWO;
    }
    almostGoalArray[h*(width/2-1)] = Player::ONE;
    almostGoalArray[h*(width/2+1)] = Player::ONE;
    almostGoalArray[h*(width/2)-1] = Player::TWO;
    almostGoalArray[h*(width/2+2)-1] = Player::TWO;
    cutOffGoalArray[h*(width/2-1)] = Player::ONE;
    cutOffGoalArray[h*(width/2)] = Player::ONE;
    cutOffGoalArray[h*(width/2+1)] = Player::ONE;
    cutOffGoalArray[h*(width/2)-1] = Player::TWO;
    cutOffGoalArray[h*(width/2+1)-1] = Player::TWO;
    cutOffGoalArray[h*(width/2+2)-1] = Player::TWO;

    // others
    visited.assign(size,0);

    // ball in center
    ball = width/2 * h + h/2;
}

void Field::removeAdjacency(int a, int b) {
    matrix[a*size+b] = 0;
    matrix[b*size+a] = 0;
    matrixNodes[a]--;
    matrixNodes[b]--;
}

void Field::makeNeibghours(int index) {
    Point point = getPosition(index);

    for (int i=0;i<size;i++) {
        if (i == index) continue;
        Point p = getPosition(i);
        if (distance(point,p) <= 1) {
            matrixNodes[i]++;
            matrix[index*size+i] = 1;
            matrix[i*size+index] = 1;
        }
    }
}

int Field::distance(Point p1, Point p2) {
    return (int)sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y));
}

Point Field::getPosition(int index) {
    int x = (matrix[index*size+index]>>8)&0xFF;
    int y = matrix[index*size+index]&0xFF;
    if (y == 0xFF) y = -1;
    return Point(x,y);
}

vector<int> Field::getNeibghours(int index) {
    vector<int> neibghours;
    neibghours.reserve(8);
    for (int i=0;i<size;i++) {
        if (i == index) continue;
        if ((matrix[index*size+i]&1) == 1) neibghours.push_back(i);
    }

    return neibghours;
}

void Field::addEdge(int a, int b) {
    matrix[a*size+b] |= 2;
    matrix[b*size+a] |= 2;
    matrixNodes[a]--;
    matrixNodes[b]--;
}

void Field::removeEdge(int a, int b) {
    matrix[a*size+b] = 1;
    matrix[b*size+a] = 1;
    matrixNodes[a]++;
    matrixNodes[b]++;
}

bool Field::passNext(int index) {
    return (matrixNodes[index]&0x0F) < matrixNodes[index]>>4;
}

bool Field::passNextDone(int index) {
    return (matrixNodes[index]&0x0F) < (matrixNodes[index]>>4)-1;
}

bool Field::isAlmostBlocked(int index) {
    return (matrixNodes[index]&0x0F) <= 1;
}

bool Field::isBlocked(int index) {
    return (matrixNodes[index]&0x0F) == 0;
}

void Field::fillFreeNeibghours(vector<int> &ns,int index) {
    int n = matrixNodes[index]&0x0F;
    ns.resize(n);

    for (int i=0,j=0;j<n;i++) {
        if (matrix[index*size+matrixNeibghours[index][i]] == 1) {
            ns[j] = matrixNeibghours[index][i];
            j++;
        }
    }
}

bool Field::isNextMoveGameover(Player player) {
    visited.assign(size,false);
    //memset(&visited[0], 0, sizeof(visited[0]) * size);
    stos.push_back(ball);
    visited[ball] = true;

    while (!stos.empty()) {
        int q = stos.back();
        stos.pop_back();
        int t = q*size;
        for (auto &v : matrixNeibghours[q]) {
            if (visited[v] || matrix[t+v] > 1) continue;
            if (almostGoalArray[v] == player) {
                stos.resize(0);
                return true;
            } else {
                int n = matrixNodes[v]&0x0F;
                if (goalArray[v] == Player::NONE && n > 1 && n < matrixNodes[v]>>4) {
                    stos.push_back(v);
                }
            }
            visited[v] = true;
        }
    }
    return false;
}

bool Field::isCutOffFromOpponentGoal(Player player) {
    player = player==Player::ONE?Player::TWO:Player::ONE;
    visited.assign(size,false);
    //memset(&visited[0], 0, sizeof(visited[0]) * size);
    stos.push_back(ball);
    visited[ball] = true;

    while (!stos.empty()) {
        int q = stos.back();
        stos.pop_back();
        int t = q*size;
        for (auto &v : matrixNeibghours[q]) {
            if (visited[v] || matrix[t+v] > 1) continue;
            if (cutOffGoalArray[v] == player) {
                stos.resize(0);
                return false;
            } else if (goalArray[v] == Player::NONE && (matrixNodes[v]&0x0F) > 1) {
                stos.push_back(v);
            }
            visited[v] = true;
        }
    }

    return true;
}

string Field::shortWinningMoveForPlayer(Player player) {
    player = player==Player::ONE?Player::TWO:Player::ONE;
    //visited.assign(size,false);
    //memset(&visited[0], 0, sizeof(visited[0]) * size);
    queue<int> kolejka;
    vector<int> parents(size,-1);
    vector<int> distances(size,99);
    kolejka.push(ball);
    distances[ball] = 0;
    int e = -1;

    while (!kolejka.empty()) {
        int q = kolejka.front();
        kolejka.pop();
        for (auto &v : matrixNeibghours[q]) {
            if (matrix[q*size+v] > 1) continue;
            Player g = goalArray[v];
            if (g == Player::NONE && passNext(v) && distances[q]+1 < distances[v]) {
                parents[v] = q;
                distances[v] = distances[q]+1;
                kolejka.push(v);
            } else if (g == player && (e == -1 || distances[q]+1 < distances[e])) {
                e = v;
                parents[e] = q;
                distances[e] = distances[q]+1;
            }
        }
    }

    string move = "";
    while (parents[e] != -1) {
        move += getDistanceChar(parents[e],e);
        e = parents[e];
    }
    reverse(move.begin(),move.end());

    return move;
}

bool Field::onlyOneEmpty() {
    visited.assign(size,false);
    //memset(&visited[0], 0, sizeof(visited[0]) * size);
    stos.push_back(ball);
    visited[ball] = true;

    while (!stos.empty()) {
        int q = stos.back();
        stos.pop_back();
        int t = q*size;
        for (auto &v : matrixNeibghours[q]) {
            if (visited[v] || matrix[t+v] > 1) continue;
            Player g = goalArray[v];
            bool almostBlocked = isAlmostBlocked(v);
            if (g == Player::NONE && passNext(v) && !almostBlocked) {
                stos.push_back(v);
            } else if (g == Player::NONE && !almostBlocked) {
                stos.resize(0);
                return false;
            }
            visited[v] = true;
        }
    }

    return true;
}

Player Field::goal(int index) {
    return goalArray[index];
}

vector<Edge> Field::getEdges() {
    vector<Edge> edges;
    for (int i=0;i<size;i++) {
        for (int j=0;j<i;j++) {
            if ((matrix[i*size+j]&2) > 0) {
                Point a = getPosition(i);
                Point b = getPosition(j);
                Edge edge(a,b);
                edges.push_back(edge);
            }
        }
    }
    return edges;
}

char Field::getDistanceChar(int a, int b) {
    Point p1 = getPosition(a);
    Point p2 = getPosition(b);
    int dx = p2.x-p1.x;
    int dy = p2.y-p1.y;
    if (dx == 0) {
        if (dy == -1) return '0';
        return '4';
    } else if (dx == 1) {
        if (dy == -1) return '1';
        if (dy == 0) return '2';
        return '3';
    } else {
        if (dy == -1) return '7';
        if (dy == 0) return '6';
        if (dy == 1) return '5';
    }
    return -1;
}

int Field::getNeibghour(int dx, int dy) {
    Point point = getPosition(ball);
    for (auto &n : matrixNeibghours[ball]) {
        if (matrix[ball*size+n] > 1) continue;
        Point p = getPosition(n);
        if (point.x+dx == p.x && point.y+dy == p.y) return n;
    }
    return -1;
}
