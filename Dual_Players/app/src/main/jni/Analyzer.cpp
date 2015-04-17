#define LOG_TAG "Analyzer"
#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>
#include "SuperpoweredAnalyzer.h"
#include "SuperpoweredDecoder.h"
#include "SuperpoweredMixer.h"
#include "Analyzer.h"
#include "SuperpoweredAudioBuffers.h"



static Analyzer *fullProcess = NULL;

Analyzer::Analyzer(const char* pathOfFile){
    //filePath = pathOfFile;
    hasFileBeenAnalysed = false;
    decodeFile(pathOfFile);
}

Analyzer::~Analyzer(){
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "open...");
    free(averageWaveForm);
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Free waveform");
    free(peakWaveForm);
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Free peakwave");
    free(overViewWaveForm);
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Free view");
    free(keyIndex);
    free(waveFormSize);
    free(averageDecibel);
    free(loudPartsAverageDecibel);
    free(peakDecibel);
    free(bpm);
    free(beatGridStart);
    delete analyzer;
}

void Analyzer::decodeFile(const char* path){
    SuperpoweredDecoder *decoder = new SuperpoweredDecoder(false);
       const char *openError = decoder->open(path);
       if (openError != NULL) {
           __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Decoder error: %s", openError);
           delete decoder;

       }
       if (decoder == NULL) {
               __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Decoder was not created");

           }

           int sampleRate = decoder->samplerate;
		   mySampleRate = sampleRate;
               double durationSeconds = decoder->durationSeconds;
               durationOfTrack = durationSeconds;
               SuperpoweredAudiobufferPool *bufferPool = new SuperpoweredAudiobufferPool(4, 1024 * 1024);             // Allow 1 MB max. memory for the buffer pool.
               analyzer = new SuperpoweredOfflineAnalyzer(sampleRate, 0, durationSeconds);

               short int *intBuffer = (short int *)malloc(decoder->samplesPerFrame * 2 * sizeof(short int) + 16384);
__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Processing...");
    int samplesMultiplier = 1;
    while (true) {
        // Decode one frame. samplesDecoded will be overwritten with the actual decoded number of samples.
        unsigned int samplesDecoded = decoder->samplesPerFrame * samplesMultiplier;
        unsigned int lastDecoded = samplesDecoded;
        if (decoder->decode(intBuffer, &samplesDecoded) != SUPERPOWEREDDECODER_OK) break;

        // Create an input buffer for the analyzer.
        SuperpoweredAudiobufferlistElement inputBuffer;
        bufferPool->createSuperpoweredAudiobufferlistElement(&inputBuffer, decoder->samplePosition, samplesDecoded + 8);

        // Convert the decoded PCM samples from 16-bit integer to 32-bit floating point.
        SuperpoweredStereoMixer::shortIntToFloat(intBuffer, bufferPool->floatAudio(&inputBuffer), samplesDecoded);
        inputBuffer.endSample = samplesDecoded;             // <-- Important!
        analyzer->process(bufferPool->floatAudio(&inputBuffer), samplesDecoded);
    }


    delete decoder;
    delete bufferPool;
    free(intBuffer);

    averageWaveForm = (unsigned char **)malloc(150 * sizeof(unsigned char *));
    peakWaveForm = (unsigned char **)malloc(150 * sizeof(unsigned char *));
    overViewWaveForm = (char **)malloc(durationSeconds * sizeof(char *));

    keyIndex = (int *)malloc(sizeof(int));
    waveFormSize = (int *)malloc(sizeof(int));

    averageDecibel = (float *)malloc(sizeof(float));
    loudPartsAverageDecibel = (float *)malloc(sizeof(float));
    peakDecibel = (float *)malloc(sizeof(float));
    bpm = (float *)malloc(sizeof(float));
    nonPointerBPM = bpm[0];
    beatGridStart = (float *)malloc(sizeof(float));


	
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Analyzing...");

    analyzer->getresults(averageWaveForm, peakWaveForm, waveFormSize, overViewWaveForm, averageDecibel,
        loudPartsAverageDecibel, peakDecibel, bpm, beatGridStart, keyIndex);
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"DONE...%f",bpm[0]);
//    notifyFinished(path, *averageDecibel, *loudPartsAverageDecibel, *peakDecibel,
//        *bpm, *beatGridStart);


}




extern "C" {
	JNIEXPORT void Java_com_players_jason_dualplayers_AnalyzerJNICom_getPathOfFile(JNIEnv *javaEnvironment, jobject self, jstring pathOfFile);
	JNIEXPORT void Java_com_players_jason_dualplayers_AnalyzerJNICom_destroyAnalyzer(JNIEnv *javaEnvironment, jobject self);
	JNIEXPORT jboolean Java_com_players_jason_dualplayers_AnalyzerJNICom_getIsFileAnalysed();
	JNIEXPORT jint Java_com_players_jason_dualplayers_AnalyzerJNICom_getFileKey(JNIEnv *javaEnvironment, jobject self);
	JNIEXPORT jfloat Java_com_players_jason_dualplayers_AnalyzerJNICom_getBPMofFile(JNIEnv *javaEnvironment, jobject self);
    JNIEXPORT jfloat Java_com_players_jason_dualplayers_AnalyzerJNICom_returnFirstBeatMs(JNIEnv *javaEnvironment, jobject self);
    JNIEXPORT jdouble Java_com_players_jason_dualplayers_AnalyzerJNICom_getTimeLengthOfFile(JNIEnv *javaEnvironment, jobject self);
    JNIEXPORT jint
    Java_com_players_jason_dualplayers_AnalyzerJNICom_getSampleRate(JNIEnv *javaEnvironment, jobject self);
}


// this is used to get the path of the file to analyse
JNIEXPORT void Java_com_players_jason_dualplayers_AnalyzerJNICom_getPathOfFile(JNIEnv *javaEnvironment, jobject self, jstring pathOfFile){
    const char *path = javaEnvironment->GetStringUTFChars(pathOfFile, JNI_FALSE);
    fullProcess = new Analyzer(path);
}

JNIEXPORT void Java_com_players_jason_dualplayers_AnalyzerJNICom_destroyAnalyzer(JNIEnv *javaEnvironment, jobject self){
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Attempt to destruct");
    fullProcess->~Analyzer();
}
/*
JNIEXPORT jboolean Java_com_players_jason_dualplayers_AnalyzerJNICom_getIsFileAnalysed(){
    return fullProcess->hasFileBeenAnalysed;


}
*/

JNIEXPORT jint Java_com_players_jason_dualplayers_AnalyzerJNICom_getFileKey(JNIEnv *javaEnvironment, jobject self){
    int key = fullProcess->keyIndex[0];
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "returning key");
	return key;
}

JNIEXPORT jfloat Java_com_players_jason_dualplayers_AnalyzerJNICom_getBPMofFile(JNIEnv *javaEnvironment, jobject self){
    float intBPM = fullProcess->bpm[0];
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"DONE 2...%f",intBPM);
    return intBPM;
}

JNIEXPORT jfloat Java_com_players_jason_dualplayers_AnalyzerJNICom_returnFirstBeatMs(JNIEnv *javaEnvironment, jobject self){
	float beatgridStartMS = fullProcess->beatGridStart[0];
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "returning beatgrid");
    return beatgridStartMS;
}

JNIEXPORT jdouble Java_com_players_jason_dualplayers_AnalyzerJNICom_getTimeLengthOfFile(JNIEnv *javaEnvironment, jobject self){
    return fullProcess->durationOfTrack;
}

JNIEXPORT jint
Java_com_players_jason_dualplayers_AnalyzerJNICom_getSampleRate(JNIEnv *javaEnvironment, jobject self){
return fullProcess->mySampleRate;
}



