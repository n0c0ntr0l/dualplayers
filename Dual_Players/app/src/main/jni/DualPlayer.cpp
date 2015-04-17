#define LOG_TAG "DualPlayer"
#include "SuperPoweredFilter.h"
#include "SuperPoweredFlanger.h"
#include "SuperPoweredFilter.h"
#include "SuperPoweredFX.h"
#include "SuperPoweredMixer.h"
#include "SuperPoweredRoll.h"
#include "DualPlayer.h"


#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>



static const SLboolean requireds[2] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

static DualPlayer *dualplayer = NULL;
static unsigned int androidSampleRate;
static unsigned int androidBufferSize;
static bool stopOrStartA = false;
static bool stopOrStartB = false;



static void playerEventCallbackA(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
    	SuperpoweredAdvancedAudioPlayer *playerA = *((SuperpoweredAdvancedAudioPlayer **)clientData);

		}
}



static void playerEventCallbackB(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
    	SuperpoweredAdvancedAudioPlayer *playerB = *((SuperpoweredAdvancedAudioPlayer **)clientData);
    	
		}
}

static void openSLESCallback(SLAndroidSimpleBufferQueueItf caller, void *pContext) {
	//__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"before we start processing");
	((DualPlayer *)pContext)->process(caller);
}

DualPlayer::DualPlayer(unsigned int theSampleRate, unsigned int theSizeOfTheBuffer){
		isEngineInitialized = false;
    // This will keep our player volumes and playback states in sync.
		buffersize = theSizeOfTheBuffer;
		currentBuffer=0;
		crossValue =0.0f;
		volB =0.0f; 
		volA =(1.0f * headroom);
		
		pthread_mutex_init(&mutex, NULL); 
		for (int n = 0; n < NUM_BUFFERS; n++) outputBuffer[n] = (float *)memalign(16, (buffersize + 16) * sizeof(float) * 2);
		hasAbeenPlayedYet = false;
		hasBbeenPlayedYet = false;
		samplerate = theSampleRate;
		
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"THE SAMPLERATE INSIDE THE CLAS IS...%i",theSampleRate);
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"THE BUFFERSIZE INSIDE THE CLASS...%i",theSizeOfTheBuffer);
        playerA = new SuperpoweredAdvancedAudioPlayer(&playerA, playerEventCallbackA, theSampleRate, 0);
        playerB = new SuperpoweredAdvancedAudioPlayer(&playerB, playerEventCallbackB, theSampleRate, 0);
        

        threeBandEQ = new Superpowered3BandEQ(theSampleRate);

        mixer = new SuperpoweredStereoMixer();



}

void DualPlayer::initialiseAudioEngine(){
     // Create the OpenSL ES engine.

	slCreateEngine(&openSLEngine, 0, NULL, 0, NULL, NULL);
	(*openSLEngine)->Realize(openSLEngine, SL_BOOLEAN_FALSE);
	SLEngineItf openSLEngineInterface = NULL;
	(*openSLEngine)->GetInterface(openSLEngine, SL_IID_ENGINE, &openSLEngineInterface);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"SL ENGINE OPENED");



	// Create the output mix.
	(*openSLEngineInterface)->CreateOutputMix(openSLEngineInterface, &outputMix, 0, NULL, NULL);
	(*outputMix)->Realize(outputMix, SL_BOOLEAN_FALSE);
	SLDataLocator_OutputMix outputMixLocator = { SL_DATALOCATOR_OUTPUTMIX, outputMix };
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"output mix created");
	// Create the buffer queue player.
	SLDataLocator_AndroidSimpleBufferQueue bufferPlayerLocator = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, NUM_BUFFERS };
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"samplerate is %i",samplerate);
	
	SLDataFormat_PCM bufferPlayerFormat = { SL_DATAFORMAT_PCM, 2, samplerate * 1000, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN };
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"b");
	SLDataSource bufferPlayerSource = { &bufferPlayerLocator, &bufferPlayerFormat };
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"c");
    const SLInterfaceID bufferPlayerInterfaces[1] = { SL_IID_BUFFERQUEUE };
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"d");
    SLDataSink bufferPlayerOutput = { &outputMixLocator, NULL };
	
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"e");
    (*openSLEngineInterface)->CreateAudioPlayer(openSLEngineInterface, &bufferPlayer, &bufferPlayerSource, &bufferPlayerOutput, 1, bufferPlayerInterfaces, requireds);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"f");
    (*bufferPlayer)->Realize(bufferPlayer, SL_BOOLEAN_FALSE);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"g");
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"buffered queue done");
    // Initialize and start the buffer queue.
    (*bufferPlayer)->GetInterface(bufferPlayer, SL_IID_BUFFERQUEUE, &bufferQueue);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"about to do callback");
    (*bufferQueue)->RegisterCallback(bufferQueue, openSLESCallback, this);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"i don't expect to see this");
    memset(outputBuffer[0], 0, buffersize * 4);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"h");
    memset(outputBuffer[1], 0, buffersize * 4);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"i");
    (*bufferQueue)->Enqueue(bufferQueue, outputBuffer[0], buffersize * 4);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"j");
    (*bufferQueue)->Enqueue(bufferQueue, outputBuffer[1], buffersize * 4);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"k");
    SLPlayItf bufferPlayerPlayInterface;
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"l");
    (*bufferPlayer)->GetInterface(bufferPlayer, SL_IID_PLAY, &bufferPlayerPlayInterface);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"m");
    (*bufferPlayerPlayInterface)->SetPlayState(bufferPlayerPlayInterface, SL_PLAYSTATE_PLAYING);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"n");
}

void DualPlayer::addNewTrackDeckA(const char *pathToFileA, double bpm, double startingBeatMs){
	if(!isEngineInitialized){
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"the logic test");
	this->initialiseAudioEngine();
	
	isEngineInitialized = true;
	}
    playerA->open(pathToFileA);


    playerA->setFirstBeatMs(startingBeatMs);

    playerA->setBpm(bpm);
	playerA->setSamplerate(samplerate);
	playerA->syncMode = playerB->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_TempoAndBeat;


}

void DualPlayer::addNewTrackDeckB(const char *pathToFileB, double bpm, double startingBeatMs){
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"the startingBeatMs %lf",startingBeatMs);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"%s",pathToFileB);
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"before A");
	playerB->open(pathToFileB);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"A");
	
    playerB->setFirstBeatMs(startingBeatMs);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"B");
	playerB->setBpm(bpm);
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"we added a track to deck B");
	playerB->setPosition(startingBeatMs,false,true);
	

	playerB->setSamplerate(samplerate);
	
	hasAbeenPlayedYet = true;



}

void DualPlayer::onCrossfader(int value) {
    pthread_mutex_lock(&mutex);
    crossValue = float(value) * 0.01f;
    if (crossValue < 0.01f) {
        volA = 1.0f * headroom;
        volB = 0.0f;
    } else if (crossValue > 0.99f) {
        volA = 0.0f;
        volB = 1.0f * headroom;
    } else { // constant power curve
        volA = cosf(M_PI_2 * crossValue) * headroom;
        volB = cosf(M_PI_2 * (1.0f - crossValue)) * headroom;
    };
    pthread_mutex_unlock(&mutex);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"and not here");
}



void DualPlayer::pauseDeckA(){
     playerA->togglePlayback();
}

void DualPlayer::pauseDeckB(){
    playerB->togglePlayback();
}

void DualPlayer::process(SLAndroidSimpleBufferQueueItf caller) {
    pthread_mutex_lock(&mutex);
	//__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"into process");
	float *stereoBuffer = outputBuffer[currentBuffer];

	bool masterIsA = (crossValue <= 0.5f);
	float masterBpm = 125.0;
	bpmMaster = 125;
	double msElapsedSinceLastBeatA;
	if(!hasAbeenPlayedYet){
		masterBpm = playerA->currentBpm;
		//__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG," A has not been played : %f",masterBpm);
		//__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG," A has not been played bpmMaster: %d",bpmMaster);
		
	} else {
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG," b has not been played: %f",masterBpm);
		 masterBpm = masterIsA ? playerA->currentBpm : playerB->currentBpm;
		 msElapsedSinceLastBeatA = playerA->msElapsedSinceLastBeat;
	}
	//__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"34");
     // When playerB needs it, playerA has already stepped this value, so save it now.

	
	
	
	bool silence;
	if(!hasAbeenPlayedYet){
		 silence = !playerA->process(stereoBuffer, false, buffersize, volA,bpmMaster, -1.0);
		
	} else {
	 silence = !playerA->process(stereoBuffer, false, buffersize, volA,bpmMaster, playerB->msElapsedSinceLastBeat);
		
	}
	//__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"78");
	if(hasAbeenPlayedYet){
		if (playerB->process(stereoBuffer, !silence, buffersize, volB, bpmMaster, msElapsedSinceLastBeatA)) silence = false;
	}
	//__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"90");
	pthread_mutex_unlock(&mutex);
	//__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"before short to int");
if (silence) memset(stereoBuffer, 0, buffersize * 4); else SuperpoweredStereoMixer::floatToShortInt(stereoBuffer, (short int *)stereoBuffer, buffersize);
//__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"after short to int");
	(*caller)->Enqueue(caller, stereoBuffer, buffersize * 4);
	//__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"stereobuffe put into queue");
	if (currentBuffer < NUM_BUFFERS - 1) currentBuffer++; else currentBuffer = 0;
}

#define MINFREQ 20.0f
#define MAXFREQ 22000.0f

static inline float floatToFrequency(float value) {
    if (value > 0.97f) return MAXFREQ;
    if (value < 0.03f) return MINFREQ;
    value = powf(10.0f, (value + ((0.4f - fabsf(value - 0.4f)) * 0.3f)) * log10f(MAXFREQ - MINFREQ)) + MINFREQ;
    return value < MAXFREQ ? value : MAXFREQ;
}


extern "C" {
JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_initializeAll(JNIEnv *javaEnvironment, jobject self, jint theAndroidBufferSize);

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_setSampleRate(JNIEnv *javaEnvironment, jobject self, jint theAndroidSampleRate);

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_addNewTrackDeckA(JNIEnv *javaEnvironment, jobject self, jstring filePath, jdoubleArray trackParamsA);


JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_addNewTrackDeckB(JNIEnv *javaEnvironment, jobject self, jstring filePath, jdoubleArray trackParamsB);

JNIEXPORT jdouble Java_com_players_jason_dualplayers_PlayerJNICom_positionOfDeckAInMs(JNIEnv *javaEnvironment, jobject self);

JNIEXPORT jdouble Java_com_players_jason_dualplayers_PlayerJNICom_positionOfDeckBInMs(JNIEnv *javaEnvironment, jobject self);

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_toggleDeckA(JNIEnv *javaEnvironment, jobject self);

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_toggleDeckB(JNIEnv *javaEnvironment, jobject self);

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_crossFaderController(JNIEnv *javaEnvironment, jobject self, jint value);

}

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_initializeAll(JNIEnv *javaEnvironment, jobject self, jint theAndroidBufferSize){

    androidBufferSize = theAndroidBufferSize;
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"THE BUFFERSIZE IS...%i",androidBufferSize);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"THE SAMPLERATE IS...%i",androidSampleRate);
    dualplayer = new DualPlayer(androidSampleRate,theAndroidBufferSize);
}

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_setSampleRate(JNIEnv *javaEnvironment, jobject self, jint theAndroidSampleRate){
    androidSampleRate = theAndroidSampleRate;
}



JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_addNewTrackDeckA(JNIEnv *javaEnvironment, jobject self, jstring filePath, jdoubleArray trackParamsA){
    jdouble *doubleParams = javaEnvironment->GetDoubleArrayElements(trackParamsA, JNI_FALSE);
    double arr[2];
    for(int i = 0; i < 2; i++) arr[i] = doubleParams[i];

    const char *pathToFileA = javaEnvironment->GetStringUTFChars(filePath, JNI_FALSE);

    dualplayer->addNewTrackDeckA(pathToFileA,arr[0],arr[1]);
}

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_addNewTrackDeckB(JNIEnv *javaEnvironment, jobject self, jstring filePath, jdoubleArray trackParamsB){
    jdouble *doubleParams = javaEnvironment->GetDoubleArrayElements(trackParamsB, JNI_FALSE);
    double arr[2];
    for(int i = 0; i < 2; i++) arr[i] = doubleParams[i];

	double startMsTest = arr[1];
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"the starting beat ms is    %lf",startMsTest);
    const char *pathToFileB = javaEnvironment->GetStringUTFChars(filePath, JNI_FALSE);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"before add track is called in c++s");
    dualplayer->addNewTrackDeckB(pathToFileB,arr[0],startMsTest);
}

JNIEXPORT jdouble Java_com_players_jason_dualplayers_PlayerJNICom_positionOfDeckAInMs(JNIEnv *javaEnvironment, jobject self){
     return dualplayer->playerA->positionMs;

 }

JNIEXPORT jdouble Java_com_players_jason_dualplayers_PlayerJNICom_positionOfDeckBInMs(JNIEnv *javaEnvironment, jobject self){
     return dualplayer->playerB->positionMs;

 }

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_toggleDeckA(JNIEnv *javaEnvironment, jobject self){
	if(!stopOrStartA){
		dualplayer->playerA->play(true);
		dualplayer->pauseDeckA();
		stopOrStartA = true;
	} else {
		dualplayer->pauseDeckA();
		stopOrStartA = false;
	}
}
JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_toggleDeckB(JNIEnv *javaEnvironment, jobject self){
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"before we toggle B");
	if(!stopOrStartB){
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"before we play B");
		dualplayer->playerB->play(false);
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"after we play B");
		//dualplayer->pauseDeckB();
		stopOrStartB = true;
	} else {
		dualplayer->pauseDeckB();
		stopOrStartB = false;
	}
}

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_crossFaderController(JNIEnv *javaEnvironment, jobject self, jint value) {
__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"does the crossfader break?");
	dualplayer->onCrossfader(value);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"not here");
}
