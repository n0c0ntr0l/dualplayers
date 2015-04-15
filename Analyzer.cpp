#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>
#include "SuperPoweredAnalyzer.h"
#include "SuperPoweredDecoder.h"
#include "Analyzer.h"




static Analyzer *analyzer = NULL;

Analyzer::Analyzer(const char* pathOfFile){
    //filePath = pathOfFile;
    hasFileBeenAnalysed = false;
    decodeFile(pathOfFile);
}

Analyzer::~Analyzer(){
    delete floatSamplesPointer;
    delete pcmOutput;
    delete decoder;
    delete analyzer;
}

void Analyzer::decodeFile(const char* path){
    decoder = new SuperpoweredDecoder(false);
    decoder->open(path,false,0,0);
    mySampleRate = decoder->samplerate;
    durationOfTrack = decoder->durationSeconds;
    numberOfSamplesInFile = decoder->durationSamples;
	numberOfSamplesInFilePointer = &numberOfSamplesInFile;
	pcmOutput = mallocForPCMSamples(numberOfSamplesInFile);
    decoder->decode(pcmOutput,numberOfSamplesInFilePointer);
    analyzeFile();
}

 short int* Analyzer::mallocForPCMSamples(unsigned int numberOfSamplesInFile ){
numberOfSamplesInFile += sizeof (short int);
void *ptr = malloc((numberOfSamplesInFile * 4)+ 16384);
//*(short int) ptr = numberOfSamplesInFile;
return ((short int*) ptr) + 1;
}


void Analyzer::analyzeFile(){
    analyzer = new SuperpoweredOfflineAnalyzer(mySampleRate,0,durationOfTrack);
    analyzer->process(Analyzer::pcmSamplesIntoFloat(pcmOutput,numberOfSamplesInFile),numberOfSamplesInFile);
    analyzer->getresults(averageWaveform,peakWaveform,waveformSize,overviewWaveform,averageDecibel,loudpartsAverageDecibel,peakDecibel,bpm,beatgridStartMs,keyIndex);
}

// use this to convert int to float
 float* Analyzer::pcmSamplesIntoFloat(short int* pcmInputIntegerSamples, int sampleSize){
const float ONEOVERSHORTMAX = 3.0517578125e-5f;
sampleSize += sizeof(float);

void *floatSamplesVoided = malloc(sampleSize);


float* floatSamplesInside = (float*)floatSamplesVoided;

for(int i = 0; i < sampleSize; ++i){
    floatSamplesInside[i] = pcmInputIntegerSamples[i] * ONEOVERSHORTMAX;
}
return floatSamplesInside;
}


extern "C" {
	JNIEXPORT void Java_com_players_jason_dualplayers_AnalyzerJNICom_getPathOfFile(JNIEnv *javaEnvironment, jobject self, jstring pathOfFile);
	JNIEXPORT void Java_com_players_jason_dualplayers_AnalyzerJNICom_destroyAnalyzer(JNIEnv *javaEnvironment, jobject self);
	JNIEXPORT jboolean Java_com_players_jason_dualplayers_AnalyzerJNICom_getIsFileAnalysed();
	JNIEXPORT jint Java_com_players_jason_dualplayers_AnalyzerJNICom_getFileKey(JNIEnv *javaEnvironment, jobject self);
	JNIEXPORT jfloat Java_com_players_jason_dualplayers_AnalyzerJNICom_getBPMofFile(JNIEnv *javaEnvironment, jobject self);
    JNIEXPORT jfloat Java_com_players_jason_dualplayers_AnalyzerJNICom_returnFirstBeatMs(JNIEnv *javaEnvironment, jobject self);
    JNIEXPORT jdouble Java_com_players_jason_dualplayers_AnalyzerJNICom_getTimeLengthOfFile(JNIEnv *javaEnvironment, jobject self);
}


// this is used to get the path of the file to analyse
JNIEXPORT void Java_com_players_jason_dualplayers_AnalyzerJNICom_getPathOfFile(JNIEnv *javaEnvironment, jobject self, jstring pathOfFile){
    const char *path = javaEnvironment->GetStringUTFChars(pathOfFile, JNI_FALSE);
    analyzer = new Analyzer(path);
}

JNIEXPORT void Java_com_players_jason_dualplayers_AnalyzerJNICom_destroyAnalyzer(JNIEnv *javaEnvironment, jobject self){
    analyzer->~Analyzer();
}

JNIEXPORT jboolean Java_com_players_jason_dualplayers_AnalyzerJNICom_getIsFileAnalysed(){
    return analyzer->hasFileBeenAnalysed;


}

JNIEXPORT jint Java_com_players_jason_dualplayers_AnalyzerJNICom_getFileKey(JNIEnv *javaEnvironment, jobject self){
    int key = analyzer->keyIndex[0];
	return key;
}

JNIEXPORT jfloat Java_com_players_jason_dualplayers_AnalyzerJNICom_getBPMofFile(JNIEnv *javaEnvironment, jobject self){
    return analyzer->bpm[0];
}

JNIEXPORT jfloat Java_com_players_jason_dualplayers_AnalyzerJNICom_returnFirstBeatMs(JNIEnv *javaEnvironment, jobject self){
	float beatgridStartMS = analyzer->beatgridStartMs[0];
    return beatgridStartMS;
}

JNIEXPORT jdouble Java_com_players_jason_dualplayers_AnalyzerJNICom_getTimeLengthOfFile(JNIEnv *javaEnvironment, jobject self){
    return analyzer->durationOfTrack;
}



