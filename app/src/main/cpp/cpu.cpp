#include "cpu.h"

Cpu::Cpu(Player player, Difficulty difficulty) : isReleased(false), player(player), difficulty(difficulty), mt(random_device()()) {
    switch (difficulty) {
    case Difficulty::VERYEASY: limit = 20; break;
    case Difficulty::EASY: limit = 30; break;
    case Difficulty::NORMAL: limit = 60; levels = 1; break;
    case Difficulty::ADVANCED: limit = 150; levels = 2; break;
    case Difficulty::HARD: limit = 200; levels = 4; break;
    }
    measureTime = difficulty==Difficulty::HARD?true:false;
}

void Cpu::setField(Field *field) {
    this->field.reset(field);
    updateScores();
}

void Cpu::setField(shared_ptr<Field> field) {
    this->field = field;
    updateScores();
}

void Cpu::updateScores() {
    scoresArray.resize(field->size,vector<int>(2));
    for (int i=0;i<field->size;i++) {
        int h = field->getPosition(i).y;
        scoresArray[i][0] = abs(-1-h);
        scoresArray[i][1] = abs((field->height+1)-h);
    }
}

int Cpu::randomInt(int max) {
    uniform_int_distribution<int> dist(0, max-1);
    return dist(mt);
}

string Cpu::getBestMove() {
    movesNum = 0;
    if (field->isNextMoveGameover(player==Player::ONE?Player::TWO:Player::ONE)) {
        switch (difficulty) {
        case Difficulty::VERYEASY:
            if (randomInt(100) < 20) return field->shortWinningMoveForPlayer(player);
                break;
        case Difficulty::EASY:
            if (randomInt(100) < 60) return field->shortWinningMoveForPlayer(player);
                break;
        default:
            return field->shortWinningMoveForPlayer(player);
        }
    }

    vector<Move> moves;
    switch (difficulty) {
    case Difficulty::VERYEASY:
    case Difficulty::EASY:
        moves = getMovesEasy();
        break;
    case Difficulty::HARD: {
        start = high_resolution_clock::now();
        alreadyBlocking = field->isCutOffFromOpponentGoal(player==Player::ONE?Player::TWO:Player::ONE);
        alreadyBlocked = field->isCutOffFromOpponentGoal(player);
        moves = getMovesSophisticated(0);
        Move bestMoveSoFar = chooseBestMove(moves);
        for (int i=1;i<levels;i++) {
            int alpha = MINB-(levels-1);
            if (i%2 == 1) reverse(moves.begin(),moves.end());
            for (auto &move : moves) {
                if (move.terminate) {
                    if (move.score > alpha) alpha = move.score;
                    continue;
                }
                vector<Path> paths = makeMove(move.move);
                move.score = getScore(player==Player::ONE?Player::TWO:Player::ONE,i-1,alpha,MAX+(levels-1));
                undoMove(paths);
                movesNum++;
                if (move.score == NOTDEFINED) return bestMoveSoFar.move;
                if (move.score > alpha) alpha = move.score;
            }
            bestMoveSoFar = chooseBestMove(moves);
        }
        return bestMoveSoFar.move;
    }
    default:
        alreadyBlocking = field->isCutOffFromOpponentGoal(player==Player::ONE?Player::TWO:Player::ONE);
        alreadyBlocked = field->isCutOffFromOpponentGoal(player);
        moves = getMovesSophisticated(levels-1);
    }
    string move = chooseBestMove(moves).move;
    return move;
}
/*#include <sstream>
namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}*/
Move Cpu::chooseBestMove(vector<Move> &moves) {
    sort(moves.begin(),moves.end());
    //string log_output;
    //for (auto &move : moves) {
    //    log_output += patch::to_string(move.score)+": "+move.move+", ";
    //}
    //__android_log_print(ANDROID_LOG_DEBUG, "CPU", log_output.c_str());
    int n = 0;
    int last = NOTDEFINED;
    for (auto &move : moves) {
        if (move.score < 0) break;
        if (last == NOTDEFINED) last = move.score;
        else if (last > move.score) break;
        n++;
    }
    if (n < 2) return moves[0];
    return moves[randomInt(n)];
}

vector<Move> Cpu::getMovesEasy() {
    vector<Move> moves; moves.reserve(limit);
    deque<pair<int,vector<Path> > > talia;
    vector<int> vertices; vertices.reserve(20);
    vector<int> pathCycles; pathCycles.reserve(20);
    vector<int> blockedMoves; blockedMoves.reserve(20);
    string str;
    vertices.push_back(field->ball);
    talia.push_front(make_pair(field->ball,vector<Path>()));
    vector<int> ns(8);
    while (!talia.empty() && moves.size() < limit) {
        pair<int,vector<Path> > v_paths;
        v_paths = talia.back();
        talia.pop_back();
        int t = v_paths.first;
        vector<Path> & paths = v_paths.second;
        for (auto &p : paths) {
            str += field->getDistanceChar(p.a,p.b);
            field->addEdge(p.a,p.b);
            vertices.push_back(p.b);
        }
        field->ball = t;
        field->fillFreeNeibghours(ns,t);
        shuffle(ns.begin(),ns.end(),mt);
        for (auto &n : ns) {
            Player goal = field->goal(n);
            if (field->passNext(n) && !field->isAlmostBlocked(n) && goal == Player::NONE) {
                vector<Path> newPaths(paths);
                newPaths.push_back(Path(t,n));
                if (find(vertices.begin(), vertices.end(), n) != vertices.end()) {
                    int newPathsHash = hashPaths(newPaths);
                    if (find(pathCycles.begin(), pathCycles.end(), newPathsHash) == pathCycles.end()) {
                        pathCycles.push_back(newPathsHash);
                        talia.push_front(make_pair(n,newPaths));
                    }
                } else talia.push_front(make_pair(n,newPaths));
            } else {
                field->addEdge(t,n);
                field->ball = n;
                int score = scoresArray[n][player==Player::ONE?0:1];
                int distance = scoresArray[n][player==Player::ONE?0:1];
                if (goal != Player::NONE) {
                    if (goal == player) score = MING;
                    else score = MAX;
                } else if (field->isBlocked(n)) {
                    vector<Path> newPaths(paths);
                    newPaths.push_back(Path(t,n));
                    int newPathsHash = hashPaths(newPaths);
                    if (find(blockedMoves.begin(), blockedMoves.end(), newPathsHash) != blockedMoves.end()) {
                        field->ball = t;
                        field->removeEdge(t,n);
                        continue;
                    }
                    blockedMoves.push_back(newPathsHash);
                    score = MINB;
                } else {
                    int chance = difficulty==Difficulty::VERYEASY?85:100;
                    if (randomInt(100) < chance && field->isNextMoveGameover(player)) {
                        score = MIN;
                    }
                }
                field->ball = t;
                field->removeEdge(t,n);
                str += field->getDistanceChar(t,n);
                movesNum++;
                moves.push_back(Move(score,str,distance));
                str.pop_back();
                if (moves.size() >= limit) break;
            }
        }
        vertices.erase(vertices.begin()+1,vertices.end());
        str.clear();
        for (auto &p : paths) field->removeEdge(p.a,p.b);
        field->ball = vertices[0];
    }
    return moves;
}

vector<Move> Cpu::getMovesSophisticated(int level) {
    Player opponent = player==Player::ONE?Player::TWO:Player::ONE;
    int alpha = MINB-(levels-1);
    int beta = MAX+(levels-1);
    vector<Move> moves; moves.reserve(limit);
    deque<pair<int,vector<Path> > > talia;
    vector<int> vertices; vertices.reserve(25);
    vector<int> pathCycles; pathCycles.reserve(25);
    vector<int> blockedMoves; blockedMoves.reserve(25);
    string str = "";
    vertices.push_back(field->ball);
    talia.push_front(make_pair(field->ball,vector<Path>()));
    vector<int> ns(8);
    while (!talia.empty() && moves.size() < limit) {
        pair<int,vector<Path> > v_paths;
        if ((moves.size()+1)%MOVECHANGE == 0) {
            v_paths = talia.back();
            talia.pop_back();
        } else {
            v_paths = talia.front();
            talia.pop_front();
        }
        int t = v_paths.first;
        vector<Path> & paths = v_paths.second;
        for (auto &p : paths) {
            str += field->getDistanceChar(p.a,p.b);
            field->addEdge(p.a,p.b);
            vertices.push_back(p.b);
        }
        field->ball = t;
        field->fillFreeNeibghours(ns,t);
        shuffle(ns.begin(),ns.end(),mt);
        for (auto &n : ns) {
            Player goal = field->goal(n);
            if (goal == Player::NONE && !field->isAlmostBlocked(n) && field->passNext(n)) {
                //if (moves.size() >= limit) continue;
                vector<Path> newPaths(paths);
                newPaths.push_back(Path(t,n));
                if (find(vertices.begin(), vertices.end(), n) != vertices.end()) {
                    int newPathsHash = hashPaths(newPaths);
                    if (find(pathCycles.begin(), pathCycles.end(), newPathsHash) == pathCycles.end()) {
                        pathCycles.push_back(newPathsHash);
                        talia.push_front(make_pair(n,newPaths));
                    }
                } else talia.push_front(make_pair(n,newPaths));
            } else {
                bool terminate = false;
                field->addEdge(t,n);
                field->ball = n;
                int p = difficulty==Difficulty::HARD?(this->player==Player::ONE?1:0):(this->player==Player::ONE?0:1);
                int score = scoresArray[n][p];
                int distance = scoresArray[n][this->player==Player::ONE?0:1];
                if (goal != Player::NONE) {
                    terminate = true;
                    if (goal == this->player) score = MING-(levels-1);
                    else score = MAX+(levels-1);
                } else {
                    if (field->isBlocked(n)) {
                    vector<Path> newPaths(paths);
                    newPaths.push_back(Path(t,n));
                    int newPathsHash = hashPaths(newPaths);
                    if (find(blockedMoves.begin(), blockedMoves.end(), newPathsHash) != blockedMoves.end()) {
                        field->ball = t;
                        field->removeEdge(t,n);
                        continue;
                    }
                    blockedMoves.push_back(newPathsHash);
                    score = MINB-(levels-1);
                    terminate = true;
                    } else if (field->isNextMoveGameover(player)) {
                        terminate = true;
                        score = MIN-(levels-1);
                    } else if (field->onlyOneEmpty()) {
                        terminate = true;
                        score = MAX+(levels-1)-2;
                    } else if (!alreadyBlocked && field->isCutOffFromOpponentGoal(this->player)) {
                        terminate = true;
                        score = MINOB-(levels-1);
                    } else if (!alreadyBlocking && field->isCutOffFromOpponentGoal(this->player==Player::ONE?Player::TWO:Player::ONE)) {
                        terminate = true;
                        score = MAXB+(levels-1);
                    } else {
                        if (level > 0) {
                            score = getScore(opponent,level-1,alpha,beta);
                            if (score == NOTDEFINED) {
                                field->removeEdge(t,n);
                                for (auto &p : paths) field->removeEdge(p.a,p.b);
                                field->ball = vertices[0];
                                str += field->getDistanceChar(t,n);
                                movesNum++;
                                moves.push_back(Move(score,str,distance,terminate));
                                return moves;
                            }
                       } else if (level == 0 && field->isNextMoveGameover(opponent)) {
                            score = getScore(opponent,level-1,alpha,beta);
                            if (score == NOTDEFINED) {
                                field->removeEdge(t,n);
                                for (auto &p : paths) field->removeEdge(p.a,p.b);
                                field->ball = vertices[0];
                                str += field->getDistanceChar(t,n);
                                movesNum++;
                                moves.push_back(Move(score,str,distance,terminate));
                                return moves;
                            }
                        }
                    }
                }
                field->ball = t;
                field->removeEdge(t,n);
                str += field->getDistanceChar(t,n);
                movesNum++;
                if (score > alpha) {
                    alpha = score;
                    if (alpha >= beta) {
                        if (alpha == beta) score++;
                        for (auto &p : paths) field->removeEdge(p.a,p.b);
                        field->ball = vertices[0];
                        moves.push_back(Move(score,str,distance,terminate));
                        return moves;
                    }
                }
                moves.push_back(Move(score,str,distance,terminate));
                str.pop_back();
                if (moves.size() >= limit) break;
            }
        }
        vertices.erase(vertices.begin()+1,vertices.end());
        str.clear();
        for (auto &p : paths) field->removeEdge(p.a,p.b);
        field->ball = vertices[0];
    }

    return moves;
}

int Cpu::getScore(Player player, int level, int alpha, int beta) {
    if (isReleased) return NOTDEFINED;
    if (measureTime) {
        high_resolution_clock::time_point stop = high_resolution_clock::now();
        if (duration_cast<microseconds>( stop - start ).count() >= 10000000L) {
            return NOTDEFINED;
        }
    }
    Player opponent = player==Player::ONE?Player::TWO:Player::ONE;
    int output = NOTDEFINED;
    unsigned int size = 0;
    deque<pair<int,vector<Path> > > talia;
    vector<int> vertices; vertices.reserve(25);
    vector<int> pathCycles; pathCycles.reserve(25);
    vector<int> blockedMoves; blockedMoves.reserve(25);
    vertices.push_back(field->ball);
    talia.push_front(make_pair(field->ball,vector<Path>()));
    bool maximizer = this->player == player;
    vector<int> ns(8);
    while (!talia.empty() && size < limit) {
        pair<int,vector<Path> > v_paths;
        if ((size+1)%MOVECHANGE == 0) {
            v_paths = talia.back();
            talia.pop_back();
        } else {
            v_paths = talia.front();
            talia.pop_front();
        }
        int t = v_paths.first;
        vector<Path> & paths = v_paths.second;
        for (auto &p : paths) {
            field->addEdge(p.a,p.b);
            vertices.push_back(p.b);
        }
        field->ball = t;
        field->fillFreeNeibghours(ns,t);
        shuffle(ns.begin(),ns.end(),mt);
        for (auto &n : ns) {
            Player goal = field->goal(n);
            if (goal == Player::NONE && !field->isAlmostBlocked(n) && field->passNext(n)) {
                //if (size >= limit) continue;
                vector<Path> newPaths(paths);
                newPaths.push_back(Path(t,n));
                if (find(vertices.begin(), vertices.end(), n) != vertices.end()) {
                    int newPathsHash = hashPaths(newPaths);
                    if (find(pathCycles.begin(), pathCycles.end(), newPathsHash) == pathCycles.end()) {
                        pathCycles.push_back(newPathsHash);
                        talia.push_front(make_pair(n,newPaths));
                    }
                } else talia.push_front(make_pair(n,newPaths));
            } else {
                field->addEdge(t,n);
                field->ball = n;
                int p = difficulty==Difficulty::HARD?(this->player==Player::ONE?1:0):(this->player==Player::ONE?0:1);
                int score = scoresArray[n][p];
                if (goal != Player::NONE) {
                    if (goal == this->player) score = MING-level;
                    else score = MAX+level;
                } else {
                    if (field->isBlocked(n)) {
                        vector<Path> newPaths(paths);
                        newPaths.push_back(Path(t,n));
                        int newPathsHash = hashPaths(newPaths);
                        if (find(blockedMoves.begin(), blockedMoves.end(), newPathsHash) != blockedMoves.end()) {
                            field->ball = t;
                            field->removeEdge(t,n);
                            continue;
                        }
                        blockedMoves.push_back(newPathsHash);
                        score = maximizer?MINB-level:MAX+level;
                    } else if (field->isNextMoveGameover(player)) {
                        score = maximizer?MIN-level:MAX+level;
                    } else if (field->onlyOneEmpty()) {
                        score = maximizer?MAX+level-2:MIN-level+2;
                    } else if (!alreadyBlocked && field->isCutOffFromOpponentGoal(this->player)) {
                        score = MINOB-level;
                    } else if (!alreadyBlocking && field->isCutOffFromOpponentGoal(this->player==Player::ONE?Player::TWO:Player::ONE)) {
                        score = MAXB+level;
                    } else {
                        if (level > 0) {
                            score = getScore(opponent,level-1,alpha,beta);
                            if (score == NOTDEFINED) {
                                field->removeEdge(t,n);
                                for (auto &p : paths) field->removeEdge(p.a,p.b);
                                field->ball = vertices[0];
                                return NOTDEFINED;
                            }
                        } else if (level == 0 && field->isNextMoveGameover(opponent)) {
                            score = getScore(opponent,level-1,alpha,beta);
                            if (score == NOTDEFINED) {
                                field->removeEdge(t,n);
                                for (auto &p : paths) field->removeEdge(p.a,p.b);
                                field->ball = vertices[0];
                                return NOTDEFINED;
                            }
                        }
                    }
                }
                field->ball = t;
                field->removeEdge(t,n);
                movesNum++;
                size++;
                if (output == NOTDEFINED) output = score;
                if (maximizer) {
                    if (score > output) output = score;
                    if (output > alpha) {
                        alpha = output;
                        if (alpha >= beta) {
                            if (alpha == beta) output++;
                            for (auto &p : paths) field->removeEdge(p.a,p.b);
                            field->ball = vertices[0];
                            return output;
                        }
                    }
                } else {
                    if (score < output) output = score;
                    if (output < beta) {
                        beta = output;
                        if (alpha >= beta) {
                            if (alpha == beta) output--;
                            for (auto &p : paths) field->removeEdge(p.a,p.b);
                            field->ball = vertices[0];
                            return output;
                        }
                    }
                }
                if (size >= limit) break;
            }
        }
        vertices.erase(vertices.begin()+1,vertices.end());
        for (auto &p : paths) field->removeEdge(p.a,p.b);
        field->ball = vertices[0];
    }
    return output;
}

vector<Path> Cpu::makeMove(string &move) {
    vector<Path> paths; paths.reserve(move.size());
    for (auto &c : move) {
        int n = nextNode(c);
        paths.push_back(Path(field->ball,n));
        field->addEdge(field->ball,n);
        field->ball = n;
    }
    return paths;
}

void Cpu::undoMove(vector<Path> &paths) {
    field->ball = paths[0].a;
    for (auto &path : paths) {
        field->removeEdge(path.a,path.b);
    }
}

int Cpu::nextNode(char c) {
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

int Cpu::hashPaths(vector<Path> & paths) {
    int output = 0;
    for (auto &path : paths) { // taken from xorshift32
        int h = 78901*path.hashCode;
        h ^= h << 13;
        h ^= h >> 17;
        //h ^= h << 5;
        output += h;
    }
    return output;
}

void Cpu::release() {
    isReleased = true;
}
