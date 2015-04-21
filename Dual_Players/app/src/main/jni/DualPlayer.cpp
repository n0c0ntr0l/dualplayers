
#define LOG_TAG "DualPlayer"

#include "DualPlayer.h"
#include "SuperPoweredFilter.h"
#include "SuperPoweredFlanger.h"
#include "SuperPoweredFilter.h"
#include "SuperPoweredFX.h"
#include "SuperPoweredMixer.h"
#include "SuperPoweredRoll.h"

#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>

static unsigned int deckATrackSampleRate;
static unsigned int deckBTrackSampleRate;
static bool firstTime = true;
static unsigned int initialSampleR;


static void playerEventCallbackA(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
    	SuperpoweredAdvancedAudioPlayer *playerA = *((SuperpoweredAdvancedAudioPlayer **)clientData);
    };
}

static void playerEventCallbackB(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
    	SuperpoweredAdvancedAudioPlayer *playerB = *((SuperpoweredAdvancedAudioPlayer **)clientData);
    };
}

static void openSLESCallback(SLAndroidSimpleBufferQueueItf caller, void *pContext) {
	((DualPlayer *)pContext)->process(caller);
}

static const SLboolean requireds[2] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

DualPlayer::DualPlayer(unsigned int buffer, unsigned int sample){
    pthread_mutex_init(&mutex, NULL); // This will keep our player volumes and playback states in sync.
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"before player");
	currentBuffer = 0;
	buffersize = buffer;
	samplerate = sample;
	deckAIsPlaying = false;
	deckBIsPlaying = false;
	firstPlay = true;
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"buffer is %i",buffersize);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"samplerate is %i",samplerate);
	crossValue = 0.50f;
	volB = 1.0f * headroom;
	volA = 1.0f * headroom;
	
	

    for (int n = 0; n < NUM_BUFFERS; n++) outputBuffer[n] = (float *)memalign(16, (buffersize + 16) * sizeof(float) * 2);

    
	
    playerA = new SuperpoweredAdvancedAudioPlayer(&playerA ,playerEventCallbackA, samplerate, 0);
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "player A made");
	
    playerB = new SuperpoweredAdvancedAudioPlayer(&playerB, playerEventCallbackB, samplerate, 0);
     __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "player B made");
   

    playerA->syncMode = playerB->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_TempoAndBeat;
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "sync enabled");

    roll = new SuperpoweredRoll(samplerate);
    filter = new SuperpoweredFilter(SuperpoweredFilter_Resonant_Lowpass, samplerate);
    flanger = new SuperpoweredFlanger(samplerate);

    mixer = new SuperpoweredStereoMixer();

    
	

}

void DualPlayer::initializeAll(){

	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"before sl engine is created");
	slCreateEngine(&openSLEngine, 0, NULL, 0, NULL, NULL);
	(*openSLEngine)->Realize(openSLEngine, SL_BOOLEAN_FALSE);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"SL ENGINE created");
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

DualPlayer::~DualPlayer() {
	for (int n = 0; n < NUM_BUFFERS; n++) free(outputBuffer[n]);
    delete playerA;
    delete playerB;
    delete mixer;
    pthread_mutex_destroy(&mutex);
}

void DualPlayer::onPlayPauseDeckA(bool play) {
    //pthread_mutex_lock(&mutex);
    if (!play) {
				__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG," It got told to pause");
        playerA->pause(); 
    } else {
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG," before hitting play on deck A");
		if(firstPlay){
			__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,"Toggle Playback");
			playerA->togglePlayback();
			deckAIsPlaying = true;
			firstPlay = false;
		} else {
			playerA->play(true);
			deckAIsPlaying = true;
			//playerB->play(true);
			__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG," after hitting play on deck A");
		}
    }
    //pthread_mutex_unlock(&mutex);
}

void DualPlayer::onPlayPauseDeckB(bool play) {
	pthread_mutex_lock(&mutex);
    if (!play) {
        playerB->pause();
		deckBIsPlaying = false;		
    } else {
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG," play B c++");
        playerB->togglePlayback();
		deckBIsPlaying = true;
		//playerA->play(true);
    }
	pthread_mutex_unlock(&mutex);
}

void DualPlayer::setPathForDeckA(const char* path, double bpm, double setPosition, unsigned int sampleRate){
	playerA->open(path);
	playerA->setBpm(bpm);
	playerA->setFirstBeatMs(setPosition);
	playerA->setPosition(playerA->firstBeatMs, false, true);
}

void DualPlayer::setPathForDeckB(const char* path, double bpm, double setPosition, unsigned int sampleRate){
	playerB->open(path);
	playerB->setBpm(bpm);
	playerB->setFirstBeatMs(setPosition);
	playerB->setPosition(playerB->firstBeatMs, false, true);
}

double DualPlayer::getPositionOfDeckAMs(){
	double playerAPosition = playerA->positionMs;
	return playerAPosition;
}

double DualPlayer::getPositionOfDeckBMs(){
	double playerBPosition = playerB->positionMs;
	return playerBPosition;
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
}

void DualPlayer::onFxSelect(int value) {
	__android_log_print(ANDROID_LOG_VERBOSE, "SuperpoweredExample", "FXSEL %i", value);
	activeFx = value;
}

void DualPlayer::onFxOff() {
    filter->enable(false);
    roll->enable(false);
    flanger->enable(false);
}

#define MINFREQ 35.0f
#define MAXFREQ 20000.0f

static inline float floatToFrequency(float value) {
    if (value > 0.97f) return MAXFREQ;
    if (value < 0.03f) return MINFREQ;
    value = powf(10.0f, (value + ((0.4f - fabsf(value - 0.4f)) * 0.3f)) * log10f(MAXFREQ - MINFREQ)) + MINFREQ;
    return value < MAXFREQ ? value : MAXFREQ;
}

void DualPlayer::onFxValue(int ivalue) {
    float value = float(ivalue) * 0.01f;
    switch (activeFx) {
        case 1:
            filter->setResonantParameters(floatToFrequency(1.0f - value), 0.2f);
            filter->enable(true);
            flanger->enable(false);
            roll->enable(false);
            break;
        case 2:
            if (value > 0.8f) roll->beats = 0.0625f;
            else if (value > 0.6f) roll->beats = 0.125f;
            else if (value > 0.4f) roll->beats = 0.25f;
            else if (value > 0.2f) roll->beats = 0.5f;
            else roll->beats = 1.0f;
            roll->enable(true);
            filter->enable(false);
            flanger->enable(false);
            break;
        default:
            flanger->setWet(value);
            flanger->enable(true);
            filter->enable(false);
            roll->enable(false);
    };
}

void DualPlayer::process(SLAndroidSimpleBufferQueueItf caller) {
    pthread_mutex_lock(&mutex);
    float *stereoBuffer = outputBuffer[currentBuffer];
	
    bool masterIsA = (crossValue <= 0.5f);
    float masterBpm = masterIsA ? playerA->currentBpm : playerB->currentBpm;
    double msElapsedSinceLastBeatA = playerA->msElapsedSinceLastBeat; // When playerB needs it, playerA has already stepped this value, so save it now.
	
	if(deckAIsPlaying && deckBIsPlaying){
		playerA->process(stereoBuffer,false,buffersize,volA,masterBpm,playerB->msElapsedSinceLastBeat);
		playerB->process(stereoBuffer,true,buffersize,volB,masterBpm,playerA->msElapsedSinceLastBeat);	
	} else if(deckAIsPlaying && !deckBIsPlaying) {
		playerA->process(stereoBuffer,false,buffersize,volA,masterBpm,-1.0);
	} else if (!deckAIsPlaying && deckBIsPlaying){
		playerB->process(stereoBuffer,false,buffersize,volB,masterBpm,-1.0);
	}
	
	/*
    bool silence = !playerA->process(stereoBuffer, false, buffersize, volA, masterBpm, playerB->msElapsedSinceLastBeat);
    if (playerB->process(stereoBuffer, !silence, buffersize, volB, masterBpm, msElapsedSinceLastBeatA)) silence = false;
	*/
	/*
    roll->bpm = flanger->bpm = masterBpm; // Syncing fx is one line.

    if (roll->process(silence ? NULL : stereoBuffer, stereoBuffer, buffersize) && silence) silence = false;
    if (!silence) {
        filter->process(stereoBuffer, stereoBuffer, buffersize);
        flanger->process(stereoBuffer, stereoBuffer, buffersize);
    };
	*/
    pthread_mutex_unlock(&mutex);

    // The stereoBuffer is ready now, let's put the finished audio into the requested buffers.
    if (!deckAIsPlaying && !deckBIsPlaying) memset(stereoBuffer, 0, buffersize * 4); else SuperpoweredStereoMixer::floatToShortInt(stereoBuffer, (short int *)stereoBuffer, buffersize);

	(*caller)->Enqueue(caller, stereoBuffer, buffersize * 4);
	if (currentBuffer < NUM_BUFFERS - 1) currentBuffer++; else currentBuffer = 0;
}

extern "C" {

	JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_dualplayer(JNIEnv *javaEnvironment, jobject self, jint param);
	
	
	JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_addNewTrackDeckA(JNIEnv *javaEnvironment, jobject self, jstring pathToFile, jdoubleArray params);
	
	JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_addNewTrackDeckB(JNIEnv *javaEnvironment, jobject self, jstring pathToFile, jdoubleArray params);
	
	JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_pauseDeckA();
	
	JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_pauseDeckB();
	
	JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_playDeckA();
	
	JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_playDeckB();
	
	JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_onCrossfader(JNIEnv *javaEnvironment, jobject self, jint value);
	
	JNIEXPORT jdouble Java_com_players_jason_dualplayers_PlayerJNICom_positionOfDeckAInMs();
	
	JNIEXPORT jdouble Java_com_players_jason_dualplayers_PlayerJNICom_positionOfDeckBInMs();
	
	JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_setSampleRateDeckA(JNIEnv *javaEnvironment, jobject self, jint samplerateOfTrack);
	
	JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_setSampleRateDeckB(JNIEnv *javaEnvironment, jobject self, jint samplerateOfTrack);
	
	JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_setInitialSampleRate(JNIEnv *javaEnvironment, jobject self, jint intialSampleRate);
	
}

static DualPlayer *dualplayer = NULL;

// Android is not passing more than 2 custom parameters, so we had to pack file offsets and lengths into an array.
JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_dualplayer(JNIEnv *javaEnvironment, jobject self, jint param) {
	// Convert the input jlong array to a regular int array.
    unsigned int bufferSize = param;
    
    dualplayer = new DualPlayer(bufferSize,initialSampleR);
}

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_setInitialSampleRate(JNIEnv *javaEnvironment, jobject self, jint intialSampleRate){
	initialSampleR = intialSampleRate;
}

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_setSampleRateDeckA(JNIEnv *javaEnvironment, jobject self, jint samplerateOfTrack){
	deckATrackSampleRate = samplerateOfTrack;
}

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_setSampleRateDeckB(JNIEnv *javaEnvironment, jobject self, jint samplerateOfTrack){
	deckBTrackSampleRate = samplerateOfTrack;
}

JNIEXPORT void
Java_com_players_jason_dualplayers_PlayerJNICom_addNewTrackDeckA(JNIEnv *javaEnvironment, jobject self, jstring pathToFile, jdoubleArray params){
	jdouble *doubleParams = javaEnvironment->GetDoubleArrayElements(params, JNI_FALSE);
	double arr[2];
	for(int i = 0; i <2; i++) arr[i] = doubleParams[i];
	javaEnvironment->ReleaseDoubleArrayElements(params,doubleParams,JNI_ABORT);
	
	const char *path = javaEnvironment->GetStringUTFChars(pathToFile, JNI_FALSE);
	
	dualplayer->setPathForDeckA(path,arr[0], arr[1], deckATrackSampleRate);
	
	javaEnvironment->ReleaseStringUTFChars(pathToFile, path);
}

JNIEXPORT void	
Java_com_players_jason_dualplayers_PlayerJNICom_addNewTrackDeckB(JNIEnv *javaEnvironment, jobject self, jstring pathToFile, jdoubleArray params){
	jdouble *doubleParams = javaEnvironment->GetDoubleArrayElements(params, JNI_FALSE);
	double arr[2];
	for(int i = 0; i <2; i++) arr[i] = doubleParams[i];
	javaEnvironment->ReleaseDoubleArrayElements(params,doubleParams,JNI_ABORT);
	
	const char *path = javaEnvironment->GetStringUTFChars(pathToFile, JNI_FALSE);
	
	dualplayer->setPathForDeckA(path,arr[0], arr[1], deckBTrackSampleRate);
	
	javaEnvironment->ReleaseStringUTFChars(pathToFile, path);
}

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_playDeckA(){
	if(firstTime){
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "initialisationtime");
		dualplayer->initializeAll();
		firstTime = false;
		
		
	}
	dualplayer->onPlayPauseDeckA(true);
}

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_playDeckB(){
	
	dualplayer->onPlayPauseDeckB(true);
}

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_pauseDeckA(){
	dualplayer->onPlayPauseDeckA(false);
}

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_pauseDeckB(){
	dualplayer->onPlayPauseDeckB(false);
}

JNIEXPORT jdouble Java_com_players_jason_dualplayers_PlayerJNICom_positionOfDeckAInMs(){
	double pos = dualplayer->getPositionOfDeckAMs();
	return pos;
}

JNIEXPORT jdouble Java_com_players_jason_dualplayers_PlayerJNICom_positionOfDeckBInMs(){
	double pos = dualplayer->getPositionOfDeckBMs();
	return pos;
}

JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_onCrossfader(JNIEnv *javaEnvironment, jobject self, jint value) {
	dualplayer->onCrossfader(value);
}

/*
JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_onFxSelect(JNIEnv *javaEnvironment, jobject self, jint value) {
	example->onFxSelect(value);
}


JNIEXPORT void Java_com_players_jason_dualplayers_PlayerJNICom_onFxOff(JNIEnv *javaEnvironment, jobject self) {
	example->onFxOff();
}

JNIEXPORT void Java_com_superpowered_crossexample_MainActivity_onFxValue(JNIEnv *javaEnvironment, jobject self, jint value) {
	example->onFxValue(value);
}
*/