/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class pl_derjack_papersoccer_objects_Cpu */

#ifndef _Included_pl_derjack_papersoccer_objects_Cpu
#define _Included_pl_derjack_papersoccer_objects_Cpu
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_pl_derjack_papersoccer_objects_Cpu_initializeCpu(JNIEnv *, jobject, jint, jint);
JNIEXPORT void JNICALL Java_pl_derjack_papersoccer_objects_Cpu_startGame(JNIEnv *, jobject, jint, jboolean);
JNIEXPORT void JNICALL Java_pl_derjack_papersoccer_objects_Cpu_updateGame(JNIEnv *, jobject, jstring);
JNIEXPORT jstring JNICALL Java_pl_derjack_papersoccer_objects_Cpu_getBestMove(JNIEnv *, jobject, jint);
JNIEXPORT void JNICALL Java_pl_derjack_papersoccer_objects_Cpu_releaseCpu(JNIEnv *, jobject);
JNIEXPORT jlongArray JNICALL Java_pl_derjack_papersoccer_objects_Cpu_benchmarkOneGame(JNIEnv *, jobject);
JNIEXPORT jint JNICALL Java_pl_derjack_papersoccer_objects_Cpu_benchmarkMCTS(JNIEnv *env, jclass type, jint);

#ifdef __cplusplus
}
#endif
#endif