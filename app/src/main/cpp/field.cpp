#include "field.h"

Field::Field(Size fieldSize, bool halfLine) : fieldSize(fieldSize), halfLine(halfLine) {
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

//    matrix[(wh)*size+wh] = ((width/2)<<8) + 0xFF;
//    matrix[(wh+1)*size+wh+1] = ((width/2+1)<<8) + h;

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

    matrixNodes[wh+1]--;
    matrixNodes[wh+1]--;
    matrixNodes[wh+4]--;
    matrixNodes[wh+4]--;

    // others
    visited.assign(size,0);

    // ball in center
    ball = width/2 * h + h/2;

    //distances = vector<int>(size,32767);
}

void Field::calculateDistances(Player player) {
    distances = vector<int>(size,99);

    deque<int> kolejka;
    int s = (player == Player::ONE) ? (size-5) : (size-2);
    kolejka.push_back(s);
    distances[s] = 1;

    int kroki = 0;

    while (!kolejka.empty()) {
        kroki++;
        int q = kolejka.front();
        kolejka.pop_front();
        for (auto &v : matrixNeibghours[q]) {
            if (matrix[q*size+v] > 1) continue;
            if ((passNext(q)) && distances[v] > distances[q]) {
                distances[v] = distances[q];
                if (goalArray[v] == Player::NONE) {
                    if (passNext(v)) kolejka.push_front(v);
                    else kolejka.push_back(v);
                }
            } else if (distances[v] > distances[q]+1) {
                distances[v] = distances[q]+1;
                if (goalArray[v] == Player::NONE) {
                    if (passNext(v)) kolejka.push_front(v);
                    else kolejka.push_back(v);
                }
            }
            if (v == ball) return;
        }
    }
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

bool Field::passNextDoneCutOff(int index) {
    return (matrixNodes[index]&0x0F) < (matrixNodes[index]>>4)-2;
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
    //visited.assign(size,false);
    memset(&visited[0], 0, sizeof(visited[0]) * size);
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
                if (n > 1 && n < matrixNodes[v]>>4) {
                    stos.push_back(v);
                }
            }
            visited[v] = true;
        }
        visited[q] = true;
    }
    return false;
}

using namespace std;
#include <iostream>
vector<int> Field::fillBlockedKonce() {
    stos.resize(0);

    vector<int> ns(8);
    vector<int> edges;

    memset(&visited[0], 0, sizeof(visited[0]) * size);
    stos.push_back(ball);
    vector<int> rodzice(size,-1);
    vector<int> konce;
    while (!stos.empty()) {
        int q = stos.back();
        stos.pop_back();
        fillFreeNeibghours(ns,q);
        for (auto & v : ns) {
            if (visited[v]) {
                continue;
            }
            if (goalArray[v] != Player::NONE) {
                //konce.push_back(v);
            } else {
                if (passNext(v) && !isAlmostBlocked(v)) {
                    rodzice[v] = q;
                    stos.push_back(v);
                } else {
                    if (isAlmostBlocked(v)) {
                        rodzice[v] = q;
                        konce.push_back(v);
                    }
                }
            }
            visited[v] = true;
        }
        visited[q] = true;
    }

    for (auto &k : konce) {
        addEdge(k,rodzice[k]);
        edges.push_back(k);
        edges.push_back(rodzice[k]);
        int t = rodzice[k];
        while ((matrixNodes[t]&0x0F) == 1 && rodzice[t] != -1) {
            addEdge(t,rodzice[t]);
            edges.push_back(t);
            edges.push_back(rodzice[t]);
            t = rodzice[t];
        }
    }

    return edges;
}

vector<int> Field::fillBlockedCykle() {
    vector<int> konce = fillBlockedKonce();
    stos.resize(0);

    visited[ball] = true;

    vector<int> ns(8);
    vector<int> edges;

    set<Path> paths;
    memset(&visited[0], 0, sizeof(visited[0]) * size);
    stos.push_back(ball);
    vector<int> rodzice(size,-1);
    while (!stos.empty()) {
        int q = stos.back();
        stos.pop_back();
        fillFreeNeibghours(ns,q);
        for (auto & v : ns) {
            if (visited[v]) {
                if (passNext(v) && goalArray[v] == Player::NONE && rodzice[q] != -1 && rodzice[q] != v && rodzice[v] == rodzice[q]) {
                    int n = 0;
                    if ((matrixNodes[v]&0x0F) == 2) n++;
                    if ((matrixNodes[q]&0x0F) == 2) n++;
                    if ((matrixNodes[rodzice[q]]&0x0F) != 2) n++;
                    if (n == 3) {
                        //cout << "usuwam cykl 3. stopnia" << endl;
                        paths.insert(Path(v,q));
                        paths.insert(Path(q,rodzice[q]));
                        paths.insert(Path(v,rodzice[v]));
                    }
                } else  if (passNext(v) && goalArray[v] == Player::NONE && rodzice[q] != -1 && rodzice[q] != v) {
                    if (rodzice[q] == rodzice[rodzice[v]] || rodzice[v] == rodzice[rodzice[q]]) {
                        int t, root;
                        if (rodzice[q] == rodzice[rodzice[v]]) {
                            t = rodzice[v];
                            root =  rodzice[q];
                        } else {
                            t = rodzice[q];
                            root =  rodzice[v];
                        }
                        int n = 0;
                        if ((matrixNodes[v]&0x0F) == 2) n++;
                        if ((matrixNodes[q]&0x0F) == 2) n++;
                        if ((matrixNodes[t]&0x0F) == 2) n++;
                        if ((matrixNodes[root]&0x0F) != 2) n++;
                        if (n == 4) {
                            //cout << "usuwam cykl 4. stopnia" << endl;
                            paths.insert(Path(v,q));
                            paths.insert(Path(q,rodzice[q]));
                            paths.insert(Path(v,rodzice[v]));
                            paths.insert(Path(t,root));
                        }
                    }
                }
                continue;
            }
            if (goalArray[v] == Player::NONE && passNext(v)) {
                rodzice[v] = q;
                stos.push_back(v);
            }
            visited[v] = true;
        }
        visited[q] = true;
    }

    for (auto & path : paths) {
        addEdge(path.a,path.b);
        edges.push_back(path.a);
        edges.push_back(path.b);
    }

    edges.insert(edges.end(),konce.begin(),konce.end());

    return edges;
}

//vector<int> Field::fillBlockedCykle() {
//    vector<int> konce = fillBlockedKonce();
//    stos.resize(0);

//    visited[ball] = true;

//    vector<int> ns(8);
//    vector<int> edges;

//    set<Path> paths;
//    memset(&visited[0], 0, sizeof(visited[0]) * size);
//    stos.push_back(ball);
//    vector<int> rodzice(size,-1);
//    while (!stos.empty()) {
//        int q = stos.back();
//        stos.pop_back();
//         fillFreeNeibghours(ns,q);
//        for (auto & v : ns) {
//            if (visited[v]) {
//                if (passNext(v) && goalArray[v] == Player::NONE && rodzice[q] != -1 && rodzice[q] != v) {
//                    vector<int> cykl;
//                    vector<int> A;
//                    vector<int> B;
//                    int r = rodzice[v];
//                    while (r != -1) {
//                        A.push_back(r);
//                        r = rodzice[r];
//                    }
//                    r = rodzice[q];
//                    while (r != -1) {
//                        B.push_back(r);
//                        r = rodzice[r];
//                    }
//                    int x = 0;
//                    for (int i=0;i<B.size();i++) {
//                        auto it = find(A.begin(),A.end(),B[i]);
//                        if (it != A.end()) {
//                            auto index = it - A.begin();
//                            x = index;
//                            break;
//                        } else {
//                            cykl.push_back(B[i]);
//                        }
//                    }
//                    for (int i=x;i>=0;i--) {
//                        cykl.push_back(A[i]);
//                    }
//                    cykl.push_back(v);
//                    cykl.push_back(q);
//                    for (int i=0;i<cykl.size()-1;i++) {
//                        if ((matrixNodes[cykl[i]]&0x0F) == 2 && (matrixNodes[cykl[i+1]]&0x0F) == 2) {
//                            paths.insert(Path(cykl[i],cykl[i+1]));
//                        }
//                    }
//                    if ((matrixNodes[cykl[0]]&0x0F) == 2 && (matrixNodes[cykl[cykl.size()-1]]&0x0F) == 2) {
//                        paths.insert(Path(cykl[0],cykl[cykl.size()-1]));
//                    }
//                }
//                continue;
//            }
//            if (goalArray[v] == Player::NONE && passNext(v)) {
//                rodzice[v] = q;
//                stos.push_back(v);
//            }
//            visited[v] = true;
//        }
//        visited[q] = true;
//    }

//    for (auto & path : paths) {
//        addEdge(path.a,path.b);
//        edges.push_back(path.a);
//        edges.push_back(path.b);
//    }

//    edges.insert(edges.end(),konce.begin(),konce.end());
//    konce = fillBlockedKonce();
//    edges.insert(edges.end(),konce.begin(),konce.end());

//    return edges;
//}

bool Field::isCutOffFromOpponentGoal(Player player) {
    player = player==Player::ONE?Player::TWO:Player::ONE;
    //visited.assign(size,false);
    memset(&visited[0], 0, sizeof(visited[0]) * size);
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
            } else if ((matrixNodes[v]&0x0F) > 1) {
                stos.push_back(v);
            }
            visited[v] = true;
        }
        visited[q] = true;
    }

    return true;
}

string Field::shortWinningMoveForPlayer(Player player) {
    player = player==Player::ONE?Player::TWO:Player::ONE;
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
    memset(&visited[0], 0, sizeof(visited[0]) * size);
    //visited.assign(size,false);
    stos.push_back(ball);
    visited[ball] = true;

    while (!stos.empty()) {
        int q = stos.back();
        stos.pop_back();
        int t = q*size;
        for (auto &v : matrixNeibghours[q]) {
            if (visited[v] || matrix[t+v] > 1) continue;
            if (!isAlmostBlocked(v)) {
                if (passNext(v)) {
                    stos.push_back(v);
                } else {
                    stos.resize(0);
                    return false;
                }
            } else if (goalArray[v] != Player::NONE) {
                stos.resize(0);
                return false;
            }
            visited[v] = true;
        }
    }

    return true;
}

bool Field::onlyTwoEmpty() {
    memset(&visited[0], 0, sizeof(visited[0]) * size);
    stos.push_back(ball);
    visited[ball] = true;

    int c = 0;

    while (!stos.empty()) {
        int q = stos.back();
        stos.pop_back();
        int t = q*size;
        for (auto &v : matrixNeibghours[q]) {
            if (visited[v] || matrix[t+v] > 1) continue;
            if (!isAlmostBlocked(v)) {
                if (passNext(v)) {
                    stos.push_back(v);
                } else {
                    c++;
                    if (c > 1) {
                        stos.resize(0);
                        return false;
                    } else {
                        stos.push_back(v);
                    }
                }
            } else if (goalArray[v] != Player::NONE) {
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
    for (int i=1;i<size;i++) {
        for (int j=0;j<i;j++) {
            if ((matrix[i*size+j]&2) > 0) {
                Point a = getPosition(i);
                Point b = getPosition(j);
                Edge edge(a,b);
                edge.x = i;
                edge.y = j;
                edges.push_back(edge);
            }
        }
    }
    return edges;
}

unsigned long Field::getHash() {
    unsigned long hash = 0L;
    vector<Path> paths;
    for (int i=1;i<size;i++) {
        for (int j=0;j<i;j++) {
            if ((matrix[i*size+j]&2) > 0) {
                unsigned int p = (i>j)?((j<<16)+i):((i<<16)+j);
                hash ^= murmurHash3(202289*p);
            }
        }
    }
    return hash;
}

unsigned long Field::murmurHash3(unsigned long x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53L;
    x ^= x >> 33;
    return x;
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

Player Field::getWinner(Player currentPlayer) {
    Player g = goalArray[ball];
    if (g != Player::NONE) {
        return g == Player::ONE ? Player::TWO : Player::ONE;
    } else if ((matrixNodes[ball]&0x0F) == 0) {
        return currentPlayer == Player::ONE ? Player::TWO : Player::ONE;
    }
    return Player::NONE;
}


set<int> Field::dfs(int root) {
    stos.resize(0);

    visited[root] = true;
    vector<bool> visited(size,false);

    vector<int> ns(8);
    set<int> konce;

    stos.push_back(root);
    vector<int> rodzice(size,-1);
    while (!stos.empty()) {
        int q = stos.back();
        stos.pop_back();
        fillFreeNeibghours(ns,q);
        for (auto & v : ns) {
            if (visited[v]) {
                continue;
            }
            if (goalArray[v] == Player::NONE && passNext(v)) {
                rodzice[v] = q;
                stos.push_back(v);
            } else konce.insert(v);
            visited[v] = true;
        }
        visited[q] = true;
    }

    return konce;
}
