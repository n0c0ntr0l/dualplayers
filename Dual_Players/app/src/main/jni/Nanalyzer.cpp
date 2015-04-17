#define LOG_TAG "NativeAnalyzerWrapper"
#ifndef Header_Nanalyzer
#define Header_Nanalyzer

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <math.h>
#include <pthread.h>


#include "SuperpoweredAnalyzer.h"
#include "SuperpoweredDecoder.h"
#include "SuperpoweredMixer.h"
#include "SuperpoweredAudioBuffers.h"
#include "Nanalyzer.h"


SuperpoweredDecoder* NativeAnalyzerWrapper::newSongDecoderForFilePath(const char *path) {
    SuperpoweredDecoder *decoder = new SuperpoweredDecoder(false);
    const char *openError = decoder->open(path);
    if (openError != NULL) {
        __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Decoder error: %s", openError);
        delete decoder;
        return NULL;
    }
    return decoder;
}

void NativeAnalyzerWrapper::processSong(const char* path) {
    SuperpoweredDecoder *decoder = newSongDecoderForFilePath(path);
    if (decoder == NULL) {
        __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Decoder was not created");
        return;
    }

    int sampleRate = decoder->samplerate;
    double durationSeconds = decoder->durationSeconds;

    SuperpoweredAudiobufferPool *bufferPool = new SuperpoweredAudiobufferPool(4, 1024 * 1024);             // Allow 1 MB max. memory for the buffer pool.
    SuperpoweredOfflineAnalyzer *analyzer = new SuperpoweredOfflineAnalyzer(sampleRate, 0, durationSeconds);

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

    unsigned char **averageWaveForm = (unsigned char **)malloc(150 * sizeof(unsigned char *));
    unsigned char **peakWaveForm = (unsigned char **)malloc(150 * sizeof(unsigned char *));
    char **overViewWaveForm = (char **)malloc(durationSeconds * sizeof(char *));

    int *keyIndex = (int *)malloc(sizeof(int));
    int *waveFormSize = (int *)malloc(sizeof(int));

    float *averageDecibel = (float *)malloc(sizeof(float));
    float *loudPartsAverageDecibel = (float *)malloc(sizeof(float));
    float *peakDecibel = (float *)malloc(sizeof(float));
    float *bpm = (float *)malloc(sizeof(float));
    float *beatGridStart = (float *)malloc(sizeof(float));

    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Analyzing...");

    analyzer->getresults(averageWaveForm, peakWaveForm, waveFormSize, overViewWaveForm, averageDecibel,
        loudPartsAverageDecibel, peakDecibel, bpm, beatGridStart, keyIndex);

    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Done!");
//    notifyFinished(path, *averageDecibel, *loudPartsAverageDecibel, *peakDecibel,
//        *bpm, *beatGridStart);

    
}

void deleteAllAfterFinished(){
delete analyzer;
    free(averageWaveForm);
    free(peakWaveForm);
    free(overViewWaveForm);
    free(keyIndex);
    free(waveFormSize);
    free(averageDecibel);`
    free(loudPartsAverageDecibel);
    free(peakDecibel);
    free(bpm);
    free(beatGridStart);
}


// this is used to get the path of the file to analyse
JNIEXPORT void Java_com_players_jason_dualplayers_AnalyzerJNICom_getPathOfFile(JNIEnv *javaEnvironment, jobject self, jstring pathOfFile){
    const char *path = javaEnvironment->GetStringUTFChars(pathOfFile, JNI_FALSE);
    nativeAnalyzer = new NativeAnalyzerWrapper(path);
}

JNIEXPORT void Java_com_players_jason_dualplayers_AnalyzerJNICom_destroyAnalyzer(JNIEnv *javaEnvironment, jobject self){
    nativeAnalyzer->~Analyzer();
}

JNIEXPORT jboolean Java_com_players_jason_dualplayers_AnalyzerJNICom_getIsFileAnalysed(){
    return nativeAnalyzer->hasFileBeenAnalysed;


}

JNIEXPORT jint Java_com_players_jason_dualplayers_AnalyzerJNICom_getFileKey(JNIEnv *javaEnvironment, jobject self){
    int key = nativeAnalyzer->keyIndex;
	return key;
}

JNIEXPORT jfloat Java_com_players_jason_dualplayers_AnalyzerJNICom_getBPMofFile(JNIEnv *javaEnvironment, jobject self){
    return nativeAnalyzer->bpm;
}

JNIEXPORT jfloat Java_com_players_jason_dualplayers_AnalyzerJNICom_returnFirstBeatMs(JNIEnv *javaEnvironment, jobject self){
	float beatgridStartMS = nativeAnalyzer->beatgridStartMs;
    return beatgridStartMS;
}

JNIEXPORT jdouble Java_com_players_jason_dualplayers_AnalyzerJNICom_getTimeLengthOfFile(JNIEnv *javaEnvironment, jobject self){
    return nativeAnalyzer->durationOfTrack;
}