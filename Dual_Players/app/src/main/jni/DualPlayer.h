#ifndef Header_DualPlayer
#define Header_DualPlayer

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <math.h>
#include <pthread.h>


#include "SuperpoweredAdvancedAudioPlayer.h"
#include "SuperpoweredFilter.h"
#include "SuperpoweredRoll.h"
#include "SuperpoweredFlanger.h"
#include "SuperpoweredMixer.h"
#include "Superpowered3BandEQ.h"

#define NUM_BUFFERS 2
#define HEADROOM_DECIBEL 3.0f
static const float headroom = powf(10.0f, -HEADROOM_DECIBEL * 0.025);

class DualPlayer {
public:
        DualPlayer(unsigned int theSampleRate, unsigned int theSizeOfTheBuffer);
        ~DualPlayer();
		void initialiseAudioEngine();
		
        void process(SLAndroidSimpleBufferQueueItf caller);
        void onPlayPause(bool play);
        void addNewTrackDeckA(const char *pathToFileA, double bpm, double startingBeatMs);
        void addNewTrackDeckB(const char *pathToFileB, double bpm, double startingBeatMs);
        void pauseDeckA();
        void pauseDeckB();
        void onCrossfader(int value);
        void lowKillDeckOnDeckA(int value);
        void lowKilDeckOnDeckB(int value);
		SuperpoweredAdvancedAudioPlayer *playerA, *playerB;
		bool isEngineInitialized;
        
private:
	SLObjectItf openSLEngine, outputMix, bufferPlayer;
	SLAndroidSimpleBufferQueueItf bufferQueue;
    SuperpoweredStereoMixer *mixer;
    Superpowered3BandEQ *threeBandEQ;
	double bpmMaster;
	unsigned int samplerate;
    unsigned char activeFx;
    float crossValue, volA, volB, lowKillDeckA, lowKillDeckB;
    pthread_mutex_t mutex;
	SuperpoweredAdvancedAudioPlayerCallback  	callbackA;
	SuperpoweredAdvancedAudioPlayerCallback  	callbackB;
	bool hasAbeenPlayedYet;
	bool hasBbeenPlayedYet;
    float *outputBuffer[NUM_BUFFERS];
	int currentBuffer, buffersize;

};
#endif