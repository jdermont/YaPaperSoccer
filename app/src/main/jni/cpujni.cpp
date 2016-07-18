#include "pl_derjack_papersoccer_objects_Cpu.h"
//#include <android/log.h>
#include <memory>
#include "cpu.h"
#include "game.h"

using namespace std;

shared_ptr<Cpu> cpu;

int nextNode(char c) {
    switch(c) {
        case '5': return cpu->field->getNeibghour(-1,1);
        case '4': return cpu->field->getNeibghour(0,1);
        case '3': return cpu->field->getNeibghour(1,1);
        case '6': return cpu->field->getNeibghour(-1,0);
        case '2': return cpu->field->getNeibghour(1,0);
        case '7': return cpu->field->getNeibghour(-1,-1);
        case '0': return cpu->field->getNeibghour(0,-1);
        case '1': return cpu->field->getNeibghour(1,-1);
    }
    return -1;
}

void doMove(string move) {
    for (auto &c : move) {
        int n = nextNode(c);
        cpu->field->addEdge(cpu->field->ball,n);
        cpu->field->ball = n;
    }
}

JNIEXPORT void JNICALL Java_pl_derjack_papersoccer_objects_Cpu_initializeCpu(JNIEnv *env, jobject instance, jint difficulty) {
    Difficulty diff;
    switch (difficulty) {
        case 0: diff = Difficulty::VERYEASY; break;
        case 1: diff = Difficulty::EASY; break;
        case 3: diff = Difficulty::ADVANCED; break;
        case 4: diff = Difficulty::HARD; break;
        default: diff = Difficulty::NORMAL;
    }
    cpu.reset(new Cpu(Player::ONE,diff));
}

JNIEXPORT void JNICALL Java_pl_derjack_papersoccer_objects_Cpu_startGame(JNIEnv *env, jobject instance, jint fieldSize, jboolean halfLine) {
    Size fSize;
    switch (fieldSize) {
        case 0: fSize = Size::SMALL; break;
        case 2: fSize = Size::BIG; break;
        default: fSize = Size::NORMAL;
    }
    //__android_log_print(ANDROID_LOG_DEBUG, "CPU", "startGame %d %d", fieldSize, halfLine);
    cpu->setField(new Field(fSize,(bool)halfLine));
}

JNIEXPORT void JNICALL Java_pl_derjack_papersoccer_objects_Cpu_updateGame(JNIEnv *env, jobject instance, jstring move) {
    env->MonitorEnter(instance);
    if (!cpu) {
        env->MonitorExit(instance);
        return;
    }
    const char *nativeString = env->GetStringUTFChars(move, JNI_FALSE);
    string moveString(nativeString);
    //__android_log_print(ANDROID_LOG_DEBUG, "CPU", "updateGame %s", moveString.c_str());
    doMove(moveString);
    env->MonitorExit(instance);
    env->ReleaseStringUTFChars(move, nativeString);
}

JNIEXPORT jstring JNICALL Java_pl_derjack_papersoccer_objects_Cpu_getBestMove(JNIEnv *env, jobject instance) {
    env->MonitorEnter(instance);
    if (!cpu) {
        env->MonitorExit(instance);
        return env->NewStringUTF("");
    }
    string move = cpu->getBestMove();
    env->MonitorExit(instance);
    //__android_log_print(ANDROID_LOG_DEBUG, "CPU", "getBestMove %s", move.c_str());
    return env->NewStringUTF(move.c_str());
}

JNIEXPORT void JNICALL Java_pl_derjack_papersoccer_objects_Cpu_releaseCpu(JNIEnv *env, jobject instance) {
    cpu->release();
    env->MonitorEnter(instance);
    cpu.reset();
    env->MonitorExit(instance);
    //__android_log_print(ANDROID_LOG_DEBUG, "CPU", "releaseCpu");
}

JNIEXPORT jlongArray JNICALL Java_pl_derjack_papersoccer_objects_Cpu_benchmarkOneGame(JNIEnv *env, jobject instance) {
    Game game;
    game.startGame();
    Cpu cpu1(Player::ONE,Difficulty::ADVANCED);
    Cpu cpu2(Player::TWO,Difficulty::ADVANCED);
    cpu1.setField(game.field);
    cpu2.setField(game.field);
    int moves = 0;
    long czas = 0;
    //__android_log_print(ANDROID_LOG_DEBUG, "CPU", "kupa 1");
    while (true) {
        if (game.currentPlayer == Player::ONE) {
            high_resolution_clock::time_point t1 = high_resolution_clock::now();
            string move = cpu1.getBestMove();
            high_resolution_clock::time_point t2 = high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
            moves += cpu1.movesNum;
            czas += duration;
            game.doMove(move);
        } else {
            high_resolution_clock::time_point t1 = high_resolution_clock::now();
            string move = cpu2.getBestMove();
            high_resolution_clock::time_point t2 = high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
            moves += cpu2.movesNum;
            czas += duration;
            game.doMove(move);
        }
        if (game.isOver()) break;
    }
    //__android_log_print(ANDROID_LOG_DEBUG, "CPU", "kupa 2");
    jlongArray result = env->NewLongArray(2);
    jlong resultTmp[] = {moves, czas};
    env->SetLongArrayRegion(result, 0, 2, resultTmp);
    return result;
}
