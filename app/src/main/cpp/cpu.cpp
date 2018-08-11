#include "cpu.h"

Cpu::Cpu(Player player, Difficulty difficulty, int threadsNum) : isReleased(false), player(player), difficulty(difficulty), mt(random_device()()), threadsNum(threadsNum) {
    std::random_device rd;
    seed[0] = rd();
    seed[1] = rd();

    if ((seed[0]|seed[1]) == 0) {
        seed[0] = 123456789L;
        seed[1] = 6549878L;
    }

    switch (difficulty) {
        case Difficulty::VERYEASY: limit = 20; break;
        case Difficulty::EASY: limit = 30; break;
        case Difficulty::NORMAL: limit = 60; levels = 1; break;
        case Difficulty::ADVANCED: limit = 150; levels = 2; break;
        case Difficulty::HARD: limit = 200; levels = 4; break;
        case Difficulty::EXPERIMENTAL: limit = 250; levels = 4; break;
    }
    measureTime = difficulty == Difficulty::HARD;
    maxTime = MAXTIME;
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

    if (difficulty == Difficulty::HARD) {
        for (int i=0;i<field->size;i++) {
            int r = randomInt(3)-1;
            int h = field->getPosition(i).y;
            scoresArray[i][0] = abs((field->height+1)-h) + r;
            scoresArray[i][1] = abs(-1-h) + r;
        }
    } else if (difficulty == Difficulty::EXPERIMENTAL) {
        for (int i=0;i<field->size;i++) {
            int r = randomInt(5)-1;
            int h = field->getPosition(i).y;
            scoresArray[i][0] = 2*abs((field->height+1)-h) + r;
            scoresArray[i][1] = 2*abs(-1-h) + r;
        }
    } else {
        for (int i=0;i<field->size;i++) {
            int h = field->getPosition(i).y;
            scoresArray[i][0] = abs(-1-h);
            scoresArray[i][1] = abs((field->height+1)-h);
        }
    }
}

uint64_t Cpu::xorshift128plus() {
    uint64_t x = seed[0];
    uint64_t const y = seed[1];
    seed[0] = y;
    x ^= x << 23; // a
    seed[1] = x ^ y ^ (x >> 17) ^ (y >> 26); // b, c
    return seed[1] + y;
}

unsigned int Cpu::randomInt(unsigned int max) {
    return xorshift128plus()%max;
}

void Cpu::fisherYates(vector<int> & ints) {
    int i, j, tmp;

    for (i = ints.size() - 1; i > 0; i--) {
        j = randomInt(i+1);
        tmp = ints[j];
        ints[j] = ints[i];
        ints[i] = tmp;
    }
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

    vector<Move*> moves;
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
            Move* bestMoveSoFar = chooseBestMove(moves);
            for (int i=1;i<levels;i++) {
                int alpha = MIN_BLOCKED-(levels-1);
                shuffle(moves.begin(),moves.end(),mt);
                for (auto &move : moves) {
                    if (move->terminate) {
                        if (move->score > alpha) alpha = move->score;
                        continue;
                    }
                    vector<Path> paths = makeMove(move->move);
                    move->score = getScore(player==Player::ONE?Player::TWO:Player::ONE,i-1,alpha,MAX_GOAL+(levels-1));
                    undoMove(paths);
                    movesNum++;
                    if (move->score == NOTDEFINED) {
                        for (auto & m : moves) { delete m; }
                        return bestMoveSoFar->move;
                    }
                    if (move->score > 200 || move->score < -200) {
                        move->terminate = true;
                    }
                    if (move->score > alpha) alpha = move->score;
                }
                bestMoveSoFar = chooseBestMove(moves);
            }
            string output = bestMoveSoFar->move;
            for (auto & m : moves) { delete m; }
            return output;
        }
        default:
            alreadyBlocking = field->isCutOffFromOpponentGoal(player==Player::ONE?Player::TWO:Player::ONE);
            alreadyBlocked = field->isCutOffFromOpponentGoal(player);
            moves = getMovesSophisticated(levels-1);
    }

    Move* move = chooseBestMove(moves);
    string output = move->move;
    for (auto & m : moves) { delete m; }
    return output;
}

Move* Cpu::chooseBestMove(vector<Move*> &moves) {
    sort(moves.begin(),moves.end(), [](Move *a, Move *b) -> bool {
        return a->score > b->score;
    });

    int n = 0;
    int last = NOTDEFINED;
    for (auto &move : moves) {
        if (move->score < -100) break;
        if (last == NOTDEFINED) last = move->score;
        else if (last > move->score) break;
        n++;
    }
    if (n < 2) return moves[0];
    return moves[randomInt(n)];
}

string Cpu::getBestMoveMCTS() {
//    __android_log_print(ANDROID_LOG_ERROR, "CPU", "ruchy %d", ruchy);

    if (field->isNextMoveGameover(player==Player::ONE?Player::TWO:Player::ONE)) {
        return field->shortWinningMoveForPlayer(this->player);
    }

    int r1, r2;
    switch (field->fieldSize) {
        case Size::SMALL:
            r1 = 4; r2 = 8;
            break;
        case Size::BIG:
            r1 = 8; r2 = 12;
            break;
        default:
            r1 = 6; r2 = 10;
    }
    if (ruchy < r1 || (ruchy < r2 && field->getPosition(field->ball).y > field->height/2)) {
        measureTime = true;
        maxTime = 1100000L;
        limit = 200;
        return getBestMove();
    }

    alreadyBlocking = field->isCutOffFromOpponentGoal(player==Player::ONE?Player::TWO:Player::ONE);
    alreadyBlocked = field->isCutOffFromOpponentGoal(player);
    if (alreadyBlocked) {
        measureTime = true;
        maxTime = 1100000L;
        limit = 200;
        return getBestMove();
    }

    limit = 500;
    measureTime = false;
    start = high_resolution_clock::now();
    vector<Move*> moves = getMovesSophisticated(0);
    sort(moves.begin(),moves.end(), [](const Move* a, const Move* b) -> bool
    {
        return a->score > b->score;
    });

    if (moves[0]->score >= MAX_CUTOFF-10 || moves[0]->score <= MIN_CUTOFF+10 || moves.size() == 1 || moves[1]->score <= MIN_CUTOFF+10) {
        int max = moves[0]->score;
//        __android_log_print(ANDROID_LOG_ERROR, "CPU", " max %d ", moves[0]->score);
        int t = 1;
        for (int i=1;i<moves.size();i++) {
            if (max == moves[i]->score) t++;
            else break;
        }
        string move = moves[randomInt(t)]->move;
        for (auto m : moves) { delete m; }
        return move;
    }

    maxTime = 1300000L + 100000L * (ruchy-r2);
    if (maxTime < 1300000L) maxTime = 1300000L;
    if (maxTime > 3250000L) maxTime = 3250000L;
    if (alreadyBlocking && maxTime > 1750000L) maxTime = 1750000L;

//    __android_log_print(ANDROID_LOG_ERROR, "CPU", "maxTime %ld", maxTime);

    threadsNum = 4;
    SpinLock globalLock;
    vector<thread> threads; threads.reserve(threadsNum);

    for (int i=0;i<threadsNum;i++) {
        threads.push_back(thread(&Cpu::fillRuchy,this,ref(moves),ref(globalLock)));
    }

    for (auto &t : threads) t.join();

    int sum = 0;
    for (auto &move : moves) {
        sum += move->games;
    }
//    __android_log_print(ANDROID_LOG_ERROR, "CPU", "done %d", sum);

    sort(moves.begin(),moves.end(), [](const Move* a, const Move* b) -> bool
    {
        if (a->games == b->games) {
            return a->score2 > b->score2;
        }
        return a->games > b->games;
    });

    int max = moves[0]->games;
    int t = 1;
    for (int i=1;i<moves.size();i++) {
        if (max == moves[i]->games) t++;
        else break;
    }

    Move *move = moves[randomInt(t)];
    string moveStr = move->move;
//    __android_log_print(ANDROID_LOG_ERROR, "CPU", "%d / %d", move->score2, move->games);
    for (auto m : moves) { delete m; }
    return moveStr;
}

int Cpu::benchmarkMCTS() {
    limit = 500;
    measureTime = false;
    vector<Move*> moves = getMovesSophisticated(0);
    sort(moves.begin(),moves.end(), [](const Move* a, const Move* b) -> bool
    {
        return a->score > b->score;
    });

    maxTime = 1000000L;
    threadsNum = 4;
    SpinLock globalLock;
    vector<thread> threads; threads.reserve(threadsNum);

    for (int i=0;i<threadsNum;i++) {
        threads.push_back(thread(&Cpu::fillRuchy,this,ref(moves),ref(globalLock)));
    }

    for (auto &t : threads) t.join();

    int sum = 0;
    for (auto &move : moves) {
        sum += move->games;
    }

    for (auto m : moves) { delete m; }
    return sum;
}

vector<Move*> Cpu::getMovesEasy() {
    vector<Move*> moves; moves.reserve(limit);
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
        fisherYates(ns);
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
                if (goal != Player::NONE) {
                    if (goal == player) score = MIN_GOAL;
                    else score = MAX_GOAL;
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
                    score = MIN_BLOCKED;
                } else {
                    int chance = difficulty==Difficulty::VERYEASY?60:90;
                    if (randomInt(100) < chance && field->isNextMoveGameover(player)) {
                        score = MIN_GOAL_NEXT_MOVE;
                    }
                }
                field->ball = t;
                field->removeEdge(t,n);
                str += field->getDistanceChar(t,n);
                movesNum++;
                moves.push_back(new Move(score,str));
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

vector<Move*> Cpu::getMovesSophisticated(int level) {
    Player opponent = player==Player::ONE?Player::TWO:Player::ONE;
    int alpha = MIN_BLOCKED-level;
    int beta = MAX_GOAL+level;
    vector<Move*> moves; moves.reserve(50);
    deque<pair<int,vector<Path> > > talia;
    vector<int> vertices; vertices.reserve(25);
    vector<int> pathCycles; pathCycles.reserve(25);
    vector<int> blockedMoves; blockedMoves.reserve(25);
    string str = "";
    vertices.push_back(field->ball);
    talia.push_back(make_pair(field->ball,vector<Path>()));
    vector<int> ns(8);
    vector<int> konceEdges;
    field->calculateDistances(player);
    bool check = field->distances[field->ball] < 3;
    if (difficulty == Difficulty::EXPERIMENTAL && !field->onlyOneEmpty()) {
        konceEdges = field->fillBlockedKonce();
    }

    int loop = 0;
    while (!talia.empty() && moves.size() < limit) {
        loop++;
        pair<int,vector<Path> > v_paths;
        if ((loop&15) == 0) {
            v_paths = talia.front();
            talia.pop_front();
        } else {
            v_paths = talia.back();
            talia.pop_back();
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
        fisherYates(ns);
        for (auto &n : ns) {
            if (!field->isAlmostBlocked(n) && field->passNext(n)) {
                vector<Path> newPaths(paths);
                newPaths.push_back(Path(t,n));
                if (find(vertices.begin(), vertices.end(), n) != vertices.end()) {
                    int newPathsHash = hashPaths(newPaths);
                    if (find(pathCycles.begin(), pathCycles.end(), newPathsHash) == pathCycles.end()) {
                        pathCycles.push_back(newPathsHash);
                        talia.push_back(make_pair(n,newPaths));
                    }
                } else talia.push_back(make_pair(n,newPaths));
            } else {
                Player goal = field->goal(n);
                bool terminate = false;
                field->addEdge(t,n);
                field->ball = n;
                int p = this->player==Player::ONE?0:1;
                int score = scoresArray[n][p];
                if (goal != Player::NONE) {
                    terminate = true;
                    if (goal == this->player) score = MIN_GOAL-(levels-1);
                    else score = MAX_GOAL+(levels-1);
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
                        score = MIN_BLOCKED-(levels-1);
                        terminate = true;
                    } else {
                        if (check && field->isNextMoveGameover(player)) {
                            terminate = true;
                            score = MIN_GOAL_NEXT_MOVE-(levels-1);
                        } else if (field->onlyOneEmpty()) {
                            terminate = true;
                            score = MAX_ONE_EMPTY+(levels-1);
                        } else if (field->onlyTwoEmpty()) {
                            terminate = true;
                            score = MIN_GOAL_ONE_EMPTY+(levels-1);
                        } else if (!alreadyBlocked && field->passNextDoneCutOff(t) && field->isCutOffFromOpponentGoal(this->player)) {
                            terminate = true;
                            score = MIN_CUTOFF-level;
                        } else if (difficulty != Difficulty::NORMAL && !alreadyBlocking && field->passNextDoneCutOff(t) && field->isCutOffFromOpponentGoal(this->player==Player::ONE?Player::TWO:Player::ONE)) {
                            terminate = true;
                            score = MAX_CUTOFF+(levels-1);
                            alreadyBlocking = true;
                            int t = limit;
                            limit = 30;
                            int s = getScore(opponent,-1,-500,MAX_GOAL+level);
                            limit = t;
                            alreadyBlocking = false;
                            terminate = true;
                            if (s > -100) score = MAX_CUTOFF+(levels-1);
                            else score = s;
                        } else {
                            if (level > 0) {
                                score = getScore(opponent,level-1,alpha,beta);
                                if (score == NOTDEFINED) {
                                    field->removeEdge(t,n);
                                    for (auto &p : paths) field->removeEdge(p.a,p.b);
                                    for (int i=0;i<konceEdges.size()/2;i++) {
                                        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
                                    }
                                    field->ball = vertices[0];
                                    str += field->getDistanceChar(t,n);
                                    movesNum++;
                                    moves.push_back(new Move(score,str,terminate, 0, 0, this->player, NULL));
                                    return moves;
                                }
                            } else if (level == 0 && field->isNextMoveGameover(opponent)) {
                                score = getScore(opponent,level-1,alpha,beta);
                                if (score == NOTDEFINED) {
                                    field->removeEdge(t,n);
                                    for (auto &p : paths) field->removeEdge(p.a,p.b);
                                    for (int i=0;i<konceEdges.size()/2;i++) {
                                        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
                                    }
                                    field->ball = vertices[0];
                                    str += field->getDistanceChar(t,n);
                                    movesNum++;
                                    moves.push_back(new Move(score,str,terminate, 0, 0, this->player, NULL));
                                    return moves;
                                }
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
                        for (int i=0;i<konceEdges.size()/2;i++) {
                            field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
                        }
                        for (auto &p : paths) field->removeEdge(p.a,p.b);
                        field->ball = vertices[0];
                        moves.push_back(new Move(score,str,terminate, 0, 0, this->player, NULL));
                        return moves;
                    }
                }
                moves.push_back(new Move(score,str,terminate, 0, 0, this->player, NULL));
                str.pop_back();
                if (moves.size() >= limit) break;
            }
        }
        vertices.erase(vertices.begin()+1,vertices.end());
        str.clear();
        for (auto &p : paths) field->removeEdge(p.a,p.b);
        field->ball = vertices[0];
    }

    for (int i=0;i<konceEdges.size()/2;i++) {
        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
    }

    return moves;
}

int Cpu::getScore(Player player, int level, int alpha, int beta) {
    if (isReleased) return NOTDEFINED;
    if (measureTime) {
        high_resolution_clock::time_point stop = high_resolution_clock::now();
        if (duration_cast<microseconds>( stop - start ).count() >= maxTime) {
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
    talia.push_back(make_pair(field->ball,vector<Path>()));
    bool maximizer = this->player == player;
    vector<int> ns(8);
    vector<int> nsBlocked(8);
    vector<int> nsBlocking(8);
    vector<int> konceEdges;
    field->calculateDistances(player);
    bool check = field->distances[field->ball] < 3;
    if (difficulty == Difficulty::EXPERIMENTAL && !field->onlyOneEmpty()) {
        konceEdges = field->fillBlockedKonce();
    }
    int loop = 0;
    while (!talia.empty() && size < limit) {
        loop++;
        pair<int,vector<Path> > v_paths;
        if ((loop&15) == 0) {
            v_paths = talia.front();
            talia.pop_front();
        } else {
            v_paths = talia.back();
            talia.pop_back();
        }
        int t = v_paths.first;
        vector<Path> & paths = v_paths.second;
        for (auto &p : paths) {
            field->addEdge(p.a,p.b);
            vertices.push_back(p.b);
        }
        field->ball = t;
        field->fillFreeNeibghours(ns,t);
        fisherYates(ns);
        for (auto &n : ns) {
            if (/*goal == Player::NONE &&*/ !field->isAlmostBlocked(n) && field->passNext(n)) {
                vector<Path> newPaths(paths);
                newPaths.push_back(Path(t,n));
                if (find(vertices.begin(), vertices.end(), n) != vertices.end()) {
                    int newPathsHash = hashPaths(newPaths);
                    if (find(pathCycles.begin(), pathCycles.end(), newPathsHash) == pathCycles.end()) {
                        pathCycles.push_back(newPathsHash);
                        talia.push_back(make_pair(n,newPaths));
                    }
                } else talia.push_back(make_pair(n,newPaths));
            } else {
                Player goal = field->goal(n);
                field->addEdge(t,n);
                field->ball = n;
                int p = this->player==Player::ONE?0:1;
                int score = scoresArray[n][p];
                if (goal != Player::NONE) {
                    if (goal == this->player) score = MIN_GOAL-level;
                    else score = MAX_GOAL+level;
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
                        score = maximizer?MIN_BLOCKED-level:MAX_GOAL+level;
                    } else {
                        if (check && field->isNextMoveGameover(player)) {
                            score = maximizer?MIN_GOAL_NEXT_MOVE-level:MAX_GOAL+level;
                        } else if (field->onlyOneEmpty()) {
                            score = maximizer?MAX_ONE_EMPTY+level:MIN_GOAL_ONE_EMPTY-level;
                        } else if (field->onlyTwoEmpty()) {
                            score = maximizer?MIN_GOAL_ONE_EMPTY-level:MAX_ONE_EMPTY+level;
                        } else if (!alreadyBlocked && field->passNextDoneCutOff(t) && field->isCutOffFromOpponentGoal(this->player)) {
                            score = MIN_CUTOFF-level;
                        } else if (difficulty != Difficulty::NORMAL && !alreadyBlocking && field->passNextDoneCutOff(t) && field->isCutOffFromOpponentGoal(this->player==Player::ONE?Player::TWO:Player::ONE)) {
                            score = MAX_CUTOFF+(levels-1);
                            if (level >= 0) {
                                alreadyBlocking = true;
                                int t = limit;
                                limit = 30;
                                int s = getScore(opponent,-1,-500,MAX_GOAL+level);
                                limit = t;
                                alreadyBlocking = false;
                                if (s > -100) score = MAX_CUTOFF+(levels-1);
                                else score = s;
                            }
                        } else {
                            if (level > 0) {
                                score = getScore(opponent,level-1,alpha,beta);
                                if (score == NOTDEFINED) {
                                    field->removeEdge(t,n);
                                    for (auto &p : paths) field->removeEdge(p.a,p.b);
                                    for (int i=0;i<konceEdges.size()/2;i++) {
                                        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
                                    }
                                    field->ball = vertices[0];
                                    return NOTDEFINED;
                                }
                            } else if (level == 0 && field->isNextMoveGameover(opponent)) {
                                score = getScore(opponent,level-1,alpha,beta);
                                if (score == NOTDEFINED) {
                                    field->removeEdge(t,n);
                                    for (auto &p : paths) field->removeEdge(p.a,p.b);
                                    for (int i=0;i<konceEdges.size()/2;i++) {
                                        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
                                    }
                                    field->ball = vertices[0];
                                    return NOTDEFINED;
                                }
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
                            for (int i=0;i<konceEdges.size()/2;i++) {
                                field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
                            }
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
                            for (int i=0;i<konceEdges.size()/2;i++) {
                                field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
                            }
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
    for (int i=0;i<konceEdges.size()/2;i++) {
        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
    }

    if (output == NOTDEFINED) output = maximizer?MIN_BLOCKED-level:MAX_GOAL+level;
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

void Cpu::fillRuchy(vector<Move*> & moves, SpinLock & globalLock) {
    Field *copy = new Field(field->fieldSize,field->halfLine);
    copy->ball = field->ball;
    copy->matrix = field->matrix;
    copy->matrixNodes = field->matrixNodes;
    copy->matrixNeibghours = field->matrixNeibghours;

    Cpu cpu(player,difficulty, 1);
    cpu.setField(copy);
    cpu.limit = 500;
    cpu.start = high_resolution_clock::now();
    cpu.games = 0;
    long duration = duration_cast<microseconds>( high_resolution_clock::now() - cpu.start).count();
    games = 0;
    while (duration < maxTime && games < 2000000 && !cpu.provenEnd && !isReleased) {
        cpu.wybierz(moves, games+1, 0);
        globalLock.lock();
        ++games;
        ++cpu.games;
        globalLock.unlock();
        duration = duration_cast<microseconds>( high_resolution_clock::now() - cpu.start).count();
    }

//    __android_log_print(ANDROID_LOG_DEBUG, "CPU", "odpalam fillRuchy %d",cpu.games);
}

void Cpu::wybierz(vector<Move*> & moves, int games, int level) {
    double max = -1000000.0;
    vector<int> indexes;
    double a = 0.0, b = 0.0, c = 0.0;
    for (int i=0;i<moves.size();i++) {
        Move *move = moves[i];
        a = 0.0; b = 0.0; c = 0.0;
        if (move->player == this->player) {
            if (move->score < -200) c = move->score/10.0;
            else if (move->score > 200) c = move->score/10.0;
        } else {
            if (move->score < -200) c = -move->score/10.0;
            else if (move->score > 200) c = -move->score/10.0;
        }
        if (c == 0.0) {
            if (move->games == 0) {
                a = 0.50;
                b = 0.50;
                a += -move->virtualLoss / 10.0;
                if (level > 0) a += (move->score-1.0) / 25.0;
            } else {
                a = (double)(move->score2-move->virtualLoss) / (move->games);
                b = C * sqrt( log(move->parent==NULL?games:move->parent->games)/(move->games) );
                c = c / move->games;
                if (level > 0) {
                    a += 2.0 * move->score / move->games;
                }
            }
        }

        if (a + b + c > max) {
            max = a + b + c ;
            indexes.resize(0);
            indexes.push_back(i);
        } else if ((a + b + c) == max) {
            indexes.push_back(i);
        }
    }

    Move *move = moves[indexes[randomInt(indexes.size())]];
    move->lock->lock();
    if (move->score2 < 0 || (this->player == move->player && move->score < -100) || (this->player != move->player && move->score > 100)) {
        bool allChildrenBad = true;
        for (auto & m : moves) {
            if (m->score2 < 0 || (this->player == m->player && m->score < -100) || (this->player != m->player && m->score > 100)) {

            } else {
                allChildrenBad = false;
                break;
            }
        }
        move->score2--;
        move->games++;
        move->lock->unlock();
        Move *parent = move->parent;
        if (allChildrenBad) {
            if (level == 0) {
                provenEnd = true;
                return;
            }
            if (parent != NULL) {
                parent->lock->lock();
                parent->virtualLoss -= VIRTUAL_LOSS;
                parent->games++;
                parent->score2 = 5 * parent->games;
                parent->lock->unlock();
                parent = parent->parent;

                if (parent != NULL) {
                    parent->lock->lock();
                    parent->virtualLoss -= VIRTUAL_LOSS;
                    parent->games++;
                    parent->score2 = -5 * parent->games;
                    parent->lock->unlock();
                    parent = parent->parent;
                }
            }
        }
        while (parent != NULL) {
            parent->lock->lock();
            parent->virtualLoss -= VIRTUAL_LOSS;
            parent->score2 += parent->player == move->player ? 0 : 1;
            parent->games++;
            parent->lock->unlock();
            parent = parent->parent;
        }
        return;
    }
    if (move->score2 > move->games || (this->player == move->player && move->score > 100) || (this->player != move->player && move->score < -100)) {
        move->games++;
        move->score2 = 10*move->games;
        move->lock->unlock();
        if (level == 0) {
            move->games += 100000;
            move->score2 = 10*move->games;
            provenEnd = true;
            return;
        }
        Move *parent = move->parent;
        if (parent != NULL) {
            parent->lock->lock();
            parent->virtualLoss -= VIRTUAL_LOSS;
            parent->games++;
            parent->score2 = -5 * parent->games;
            parent->lock->unlock();
            parent = parent->parent;
        }
        while (parent != NULL) {
            parent->lock->lock();
            parent->virtualLoss -= VIRTUAL_LOSS;
            parent->score2 += parent->player == move->player ? 1 : 0;
            parent->games++;
            parent->lock->unlock();
            parent = parent->parent;
        }
        return;
    }
    vector<Path> paths = makeMove(move->move);
    vector<int> konceEdges = field->fillBlockedKonce();
    alreadyBlocking = field->isCutOffFromOpponentGoal(player==Player::ONE?Player::TWO:Player::ONE);
    alreadyBlocked = field->isCutOffFromOpponentGoal(player);
    move->virtualLoss += VIRTUAL_LOSS;
    if (move->games > 0) {
        if (move->moves.size() > 0) {
            move->lock->unlock();
            wybierz(move->moves,games,level+1);
        } else if (!(field->isBlocked(field->ball) || field->goal(field->ball)!=Player::NONE)) {
            move->moves = getMoves(move,(move->player==Player::ONE)?Player::TWO:Player::ONE, limit, 1);

            sort(move->moves.begin(),move->moves.end(), [](const Move *a, const Move *b) -> bool
            {
                return a->score > b->score;
            });

            if (move->player == this->player) {
                reverse(move->moves.begin(),move->moves.end());
            }

            if (move->player == this->player) {
                if (move->moves[0]->score < -100) {
                    move->virtualLoss -= VIRTUAL_LOSS;
                    move->games++;
                    move->score2 = -10 * move->games;
                    move->lock->unlock();
                    Move *parent = move->parent;
                    while (parent != NULL) {
                        parent->lock->lock();
                        parent->virtualLoss -= VIRTUAL_LOSS;
                        parent->score2 += parent->player == move->player ? 0 : 1;
                        parent->games++;
                        parent->lock->unlock();
                        parent = parent->parent;
                    }
                    undoMove(paths);
                    for (int i=0;i<konceEdges.size()/2;i++) {
                        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
                    }
                    return;
                } else if (move->moves[0]->score > 100) {
                    move->virtualLoss -= VIRTUAL_LOSS;
                    move->games++;
                    move->score2 = 10*move->games;
                    move->lock->unlock();
                    Move *parent = move->parent;
                    if (parent != NULL) {
                        parent->lock->lock();
                        parent->virtualLoss -= VIRTUAL_LOSS;
                        parent->games++;
                        parent->score2 = -5 * parent->games;
                        parent->lock->unlock();
                        parent = parent->parent;
                    }
                    while (parent != NULL) {
                        parent->lock->lock();
                        parent->virtualLoss -= VIRTUAL_LOSS;
                        parent->score2 += parent->player == move->player ? 1 : 0;
                        parent->games++;
                        parent->lock->unlock();
                        parent = parent->parent;
                    }
                    undoMove(paths);
                    for (int i=0;i<konceEdges.size()/2;i++) {
                        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
                    }
                    return;
                }
            } else {
                if (move->moves[0]->score > 100) {
                    move->virtualLoss -= VIRTUAL_LOSS;
                    move->games++;
                    move->score2 = -10 * move->games;
                    move->lock->unlock();
                    Move *parent = move->parent;
                    while (parent != NULL) {
                        parent->lock->lock();
                        parent->virtualLoss -= VIRTUAL_LOSS;
                        parent->score2 += parent->player == move->player ? 0 : 1;
                        parent->games++;
                        parent->lock->unlock();
                        parent = parent->parent;
                    }
                    undoMove(paths);
                    for (int i=0;i<konceEdges.size()/2;i++) {
                        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
                    }
                    return;
                } else if (move->moves[0]->score < -100) {
                    move->virtualLoss -= VIRTUAL_LOSS;
                    move->games++;
                    move->score2 = 10*move->games;
                    move->lock->unlock();
                    Move *parent = move->parent;
                    if (parent != NULL) {
                        parent->lock->lock();
                        parent->virtualLoss -= VIRTUAL_LOSS;
                        parent->games++;
                        parent->score2 = -5 * parent->games;
                        parent->lock->unlock();
                        parent = parent->parent;
                    }
                    while (parent != NULL) {
                        parent->lock->lock();
                        parent->virtualLoss -= VIRTUAL_LOSS;
                        parent->score2 += parent->player == move->player ? 1 : 0;
                        parent->games++;
                        parent->lock->unlock();
                        parent = parent->parent;
                    }
                    undoMove(paths);
                    for (int i=0;i<konceEdges.size()/2;i++) {
                        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
                    }
                    return;
                }
            }
            move->lock->unlock();
            wybierz(move->moves,games,level+1);
        } else { // terminate
            int score2 = field->getWinner(move->player) == move->player ? 1 : 0;
            move->virtualLoss -= VIRTUAL_LOSS;
            move->games++;
            Move *parent = move->parent;
            if (score2 == 1) {
                move->score2 = 10 * move->games;
                if (parent != NULL) {
                    parent->lock->lock();
                    parent->virtualLoss -= VIRTUAL_LOSS;
                    parent->games++;
                    parent->score2 = -5 * parent->games;
                    parent->lock->unlock();
                    parent = parent->parent;
                }
            } else {
                move->score2 = -10 * move->games;
            }
            move->lock->unlock();
            while (parent != NULL) {
                parent->lock->lock();
                parent->virtualLoss -= VIRTUAL_LOSS;
                parent->score2 += parent->player == move->player ? score2 : 1-score2;
                parent->games++;
                parent->lock->unlock();
                parent = parent->parent;
            }
        }
    } else {
        if (!(field->isBlocked(field->ball) || field->goal(field->ball)!=Player::NONE)) {
            int score2 = simulateOnePointers((move->player==Player::ONE)?Player::TWO:Player::ONE,30);
            score2 = move->player == this->player ? score2 : 1 - score2;
            move->virtualLoss -= VIRTUAL_LOSS;
            move->score2 += score2;
            move->games++;
            move->lock->unlock();
            Move *parent = move->parent;
            while (parent != NULL) {
                parent->lock->lock();
                parent->virtualLoss -= VIRTUAL_LOSS;
                parent->score2 += parent->player == move->player ? score2 : 1-score2;
                parent->games++;
                parent->lock->unlock();
                parent = parent->parent;
            }
        } else { // terminate
            int score2 = field->getWinner(move->player) == move->player ? 1 : 0;
            move->virtualLoss -= VIRTUAL_LOSS;
            move->games++;
            Move *parent = move->parent;
            if (score2 == 1) {
                move->score2 = 10 * move->games;
                if (parent != NULL) {
                    parent->lock->lock();
                    parent->virtualLoss -= VIRTUAL_LOSS;
                    parent->games++;
                    parent->score2 = -5 * parent->games;
                    parent->lock->unlock();
                    parent = parent->parent;
                }
            } else {
                move->score2 = -10 * move->games;
            }
            move->lock->unlock();
            while (parent != NULL) {
                parent->lock->lock();
                parent->virtualLoss -= VIRTUAL_LOSS;
                parent->score2 += parent->player == move->player ? score2 : 1-score2;
                parent->games++;
                parent->lock->unlock();
                parent = parent->parent;
            }
        }
    }
    undoMove(paths);
    for (int i=0;i<konceEdges.size()/2;i++) {
        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
    }
}

vector<Move*> Cpu::getMoves(Move *parent, Player player, int limit, int level, int poziomy) {
    vector<Move*> moves; moves.reserve(50);
    deque<pair<int,vector<Path> > > talia;
    vector<int> vertices; vertices.reserve(25);
    vector<int> pathCycles; pathCycles.reserve(25);
    vector<int> blockedMoves; blockedMoves.reserve(25);
    string str = "";
    vertices.push_back(field->ball);
    talia.push_back(make_pair(field->ball,vector<Path>()));
    bool maximizer = this->player == player;
    vector<int> ns(8);
    vector<int> konceEdges;
    bool check = true;
    if (!field->onlyOneEmpty()) {
        konceEdges = field->fillBlockedKonce();
        field->calculateDistances(player);
        check = field->distances[field->ball] < 3;
    }
    int loop = 0;
    set<int> begin;
    while (!talia.empty() && moves.size() < limit) {
        loop++;
        pair<int,vector<Path> > v_paths;
        if ((loop&15) == 0) {
            v_paths = talia.front();
            talia.pop_front();
        } else {
            v_paths = talia.back();
            talia.pop_back();
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
        fisherYates(ns);
        for (auto &n : ns) {
            if (!field->isAlmostBlocked(n) && field->passNext(n)) {
                vector<Path> newPaths(paths);
                newPaths.push_back(Path(t,n));
                if (find(vertices.begin(), vertices.end(), n) != vertices.end()) {
                    int newPathsHash = hashPaths(newPaths);
                    if (find(pathCycles.begin(), pathCycles.end(), newPathsHash) == pathCycles.end()) {
                        pathCycles.push_back(newPathsHash);
                        talia.push_back(make_pair(n,newPaths));
                    }
                } else talia.push_back(make_pair(n,newPaths));
            } else {
                Player goal = field->goal(n);
                field->addEdge(t,n);
                field->ball = n;
                int score = 0;
                if (goal != Player::NONE) {
                    if (goal == this->player) score = MIN_GOAL;
                    else score = MAX_GOAL;
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
                        score = maximizer?MIN_BLOCKED:MAX_GOAL;
                    } else {
                        if (check && field->isNextMoveGameover(player)) {
                            score = maximizer?MIN_GOAL_NEXT_MOVE:MAX_GOAL;
                        } else if (field->onlyOneEmpty()) {
                            score = maximizer?MAX_ONE_EMPTY:MIN_GOAL_ONE_EMPTY;
                        } else if (field->onlyTwoEmpty()) {
                            score = maximizer?MIN_GOAL_ONE_EMPTY:MAX_ONE_EMPTY;
                        } else if (!alreadyBlocked && field->passNextDoneCutOff(t) && field->isCutOffFromOpponentGoal(this->player)) {
                            score = MIN_CUTOFF;
                        } else if (!alreadyBlocking && field->passNextDoneCutOff(t) && field->isCutOffFromOpponentGoal(this->player==Player::ONE?Player::TWO:Player::ONE)) {
                            score = MAX_CUTOFF;
                            alreadyBlocking = true;
                            int t = limit;
                            limit = 30;
                            int s = getScore(player==Player::ONE?Player::TWO:Player::ONE,-1,-500,MAX_GOAL+level);
                            limit = t;
                            alreadyBlocking = false;
                            if (s > -100) score = MAX_CUTOFF;
                            else {
                                score = s;
                                //cout << "taka sytuacja 3" << s << endl;
                            }
                        } else if (level > 0) {
                            int x = getMovesScore(player==Player::ONE?Player::TWO:Player::ONE,limit,level-1);
                            if (x < -200 || x > 200) score = x;
                            else {
                                field->calculateDistances(player);
                                score = field->distances[n];
                            }
                        }
                    }
                }
                field->ball = t;
                field->removeEdge(t,n);
                moves.push_back(new Move(score,str+field->getDistanceChar(t,n),false,0,0,player,parent));
                if (((maximizer&&score>200)||(!maximizer&&score<-200))) {
                    for (auto &p : paths) field->removeEdge(p.a,p.b);
                    field->ball = vertices[0];
                    for (int i=0;i<konceEdges.size()/2;i++) {
                        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
                    }
                    return moves;
                }
            }
        }
        vertices.erase(vertices.begin()+1,vertices.end());
        str.clear();
        for (auto &p : paths) field->removeEdge(p.a,p.b);
        field->ball = vertices[0];
    }

    for (int i=0;i<konceEdges.size()/2;i++) {
        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
    }

    return moves;
}


int Cpu::getMovesScore(Player player, int limit, int level) {
    deque<pair<int,vector<Path> > > talia;
    vector<int> vertices; vertices.reserve(25);
    vector<long> pathCycles; pathCycles.reserve(25);
    vector<long> blockedMoves; blockedMoves.reserve(25);
    vertices.push_back(field->ball);
    talia.push_front(make_pair(field->ball,vector<Path>()));
    bool maximizer = this->player == player;
    vector<int> ns(8);
    vector<int> konceEdges = field->fillBlockedKonce();
    int loop = 0;
    int size = 0;
    int output = this->player == player ? MIN_BLOCKED : MAX_GOAL;
    while (!talia.empty() && size < limit) {
        loop++;
        pair<int,vector<Path> > v_paths;
        if ((loop&15) == 0) {
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
        fisherYates(ns);
        for (auto &n : ns) {
            if (!field->isAlmostBlocked(n) && field->passNext(n)) {
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
                Player goal = field->goal(n);
                field->addEdge(t,n);
                field->ball = n;
                int score = 0;
                if (goal != Player::NONE) {
                    if (goal == this->player) score = MIN_GOAL;
                    else score = MAX_GOAL;
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
                        score = maximizer?MIN_BLOCKED:MAX_GOAL;
                    } else {
                        if (field->isNextMoveGameover(player)) {
                            score = maximizer?MIN_GOAL_NEXT_MOVE:MAX_GOAL;
                        } else if (field->onlyOneEmpty()) {
                            score = maximizer?MAX_ONE_EMPTY:MIN_GOAL_ONE_EMPTY;
                        } else if (field->onlyTwoEmpty()) {
                            score = maximizer?MIN_GOAL_ONE_EMPTY:MAX_ONE_EMPTY;
                        } else if (!alreadyBlocked && field->passNextDoneCutOff(t) && field->isCutOffFromOpponentGoal(this->player)) {
                            score = MIN_CUTOFF;
                        } else if (!alreadyBlocking && field->passNextDoneCutOff(t) && field->isCutOffFromOpponentGoal(this->player==Player::ONE?Player::TWO:Player::ONE)) {
                            score = MAX_CUTOFF;
                        } else if (level > 0) {
                            score = getMovesScore(player==Player::ONE?Player::TWO:Player::ONE,limit,level-1);
                        }
                    }
                }
                field->ball = t;
                field->removeEdge(t,n);
                size++;
                output = maximizer ? max(score,output) : min(score,output);
                if ((this->player == player && output >= 0) || (this->player != player && output <= 0)) {
                    for (auto &p : paths) field->removeEdge(p.a,p.b);
                    field->ball = vertices[0];
                    for (int i=0;i<konceEdges.size()/2;i++) {
                        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
                    }
                    return output;
                }
            }
        }
        vertices.erase(vertices.begin()+1,vertices.end());
        for (auto &p : paths) field->removeEdge(p.a,p.b);
        field->ball = vertices[0];
    }

    for (int i=0;i<konceEdges.size()/2;i++) {
        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
    }

    return output;
}

int Cpu::simulateOnePointers(Player currentPlayer, int limes) {
    vector< vector<Path>> paths;
    while (true) {
        vector<ShortMove*> moves = getMovePointers(currentPlayer,limes,1);
        sort(moves.begin(),moves.end(), [](ShortMove * a, ShortMove * b) -> bool
        {
            return a->score > b->score;
        });
        int r = 0;
        int t = 0;
        if (currentPlayer == this->player) {
            if (moves[0]->score > 200) t = 1;
            else {
                for (auto &move : moves) {
                    if (move->score < -200) break;
                    r++;
                }
            }
        } else {
            reverse(moves.begin(),moves.end());
            if (moves[0]->score < -200) t = 1;
            else {
                for (auto &move : moves) {
                    if (move->score > 200) break;
                    r++;
                }
            }
        }

        if (t > 0) {
            while (!paths.empty()) {
                vector<Path> p = paths.back();
                paths.pop_back();
                undoMove(p);
            }
            for (auto m : moves) { delete m; }
            return currentPlayer == this->player ? 1 : 0;
        } else if (r == 0) {
            while (!paths.empty()) {
                vector<Path> p = paths.back();
                paths.pop_back();
                undoMove(p);
            }
            for (auto m : moves) { delete m; }
            return currentPlayer != this->player ? 1 : 0;
        }

        ShortMove* m = moves[randomInt(r)];
        vector<Path> p = makeMove(m->move);
        paths.push_back(p);

        Player winner = field->getWinner(currentPlayer);
        if (winner != Player::NONE) {
            while (!paths.empty()) {
                vector<Path> p = paths.back();
                paths.pop_back();
                undoMove(p);
            }
            for (auto m : moves) { delete m; }
            return winner == this->player ? 1 : 0;
        }
        currentPlayer = (currentPlayer == Player::ONE ? Player::TWO : Player::ONE);
        if (field->isNextMoveGameover(currentPlayer==Player::ONE?Player::TWO:Player::ONE)) {
            while (!paths.empty()) {
                vector<Path> p = paths.back();
                paths.pop_back();
                undoMove(p);
            }
            for (auto m : moves) { delete m; }
            return currentPlayer == this->player ? 1 : 0;
        }

        for (auto m : moves) { delete m; }
    }

    return 0;
}

vector<ShortMove*> Cpu::getMovePointers(Player player, int limit, int level) {
    vector<ShortMove*> moves; moves.reserve(40);
    deque<pair<int,vector<Path> > > talia;
    vector<int> vertices; vertices.reserve(16);
    vector<int> pathCycles; pathCycles.reserve(16);
    vector<int> blockedMoves; blockedMoves.reserve(16);
    string str = "";
    vertices.push_back(field->ball);
    talia.push_back(make_pair(field->ball,vector<Path>()));
    bool maximizer = this->player == player;
    vector<int> ns(8);
    vector<int> konceEdges;
    bool check = true;
    if (!field->onlyOneEmpty()) {
        konceEdges = field->fillBlockedKonce();
        field->calculateDistances(player);
        check = field->distances[field->ball] < 3;
    }
    int loop = 0;
    while (!talia.empty() && moves.size() < limit) {
        loop++;
        pair<int,vector<Path> > v_paths;
        if ((loop&15) == 0) {
            v_paths = talia.front();
            talia.pop_front();
        } else {
            v_paths = talia.back();
            talia.pop_back();
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
        fisherYates(ns);
        for (auto &n : ns) {
            if (!field->isAlmostBlocked(n) && field->passNext(n)) {
                vector<Path> newPaths(paths);
                newPaths.push_back(Path(t,n));
                if (find(vertices.begin(), vertices.end(), n) != vertices.end()) {
                    int newPathsHash = hashPaths(newPaths);
                    if (find(pathCycles.begin(), pathCycles.end(), newPathsHash) == pathCycles.end()) {
                        pathCycles.push_back(newPathsHash);
                        talia.push_back(make_pair(n,newPaths));
                    }
                } else talia.push_back(make_pair(n,newPaths));
            } else {
                Player goal = field->goal(n);
                field->addEdge(t,n);
                field->ball = n;
                int score = 0;
                if (goal != Player::NONE) {
                    if (goal == this->player) score = MIN_GOAL;
                    else score = MAX_GOAL;
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
                        score = maximizer?MIN_BLOCKED:MAX_GOAL;
                    } else {
                        if (check && field->isNextMoveGameover(player)) {
                            score = maximizer?MIN_GOAL_NEXT_MOVE:MAX_GOAL;
                        } else if (field->onlyOneEmpty()) {
                            score = maximizer?MAX_ONE_EMPTY:MIN_GOAL_ONE_EMPTY;
                        } else if (field->onlyTwoEmpty()) {
                            score = maximizer?MIN_GOAL_ONE_EMPTY:MAX_ONE_EMPTY;
                        } else if (level > 0) {
                            score = getMovesScore(player==Player::ONE?Player::TWO:Player::ONE,limit, level-1);
                        }
                    }
                }
                field->ball = t;
                field->removeEdge(t,n);
                moves.push_back(new ShortMove(score,str+field->getDistanceChar(t,n)));
                if (((player==this->player&&score>0)||(player!=this->player&&score<0))) {
                    for (auto &p : paths) field->removeEdge(p.a,p.b);
                    field->ball = vertices[0];
                    for (int i=0;i<konceEdges.size()/2;i++) {
                        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
                    }
                    return moves;
                }
            }
        }
        vertices.erase(vertices.begin()+1,vertices.end());
        str.clear();
        for (auto &p : paths) field->removeEdge(p.a,p.b);
        field->ball = vertices[0];
    }

    for (int i=0;i<konceEdges.size()/2;i++) {
        field->removeEdge(konceEdges[2*i],konceEdges[2*i+1]);
    }

    return moves;
}