#ifndef Header_DualPlayer
#define Header_DualPlayer

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <math.h>
#include <pthread.h>

#include "DualPlayer.h"
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
        DualPlayer(unsigned int buffer, unsigned int sample);
        ~DualPlayer();
		//void initialiseAudioEngine();
        void process(SLAndroidSimpleBufferQueueItf caller);
        //void onPlayPause(bool play);
        void setPathForDeckA(const char* path, double bpm, double setPosition, unsigned int sampleRate);
        void setPathForDeckB(const char* path, double bpm, double setPosition, unsigned int sampleRate);
        void onPlayPauseDeckA(bool play);
        void onPlayPauseDeckB(bool play);
		void onFxSelect(int value);
		void onFxOff();
		void onFxValue(int value);
        void onCrossfader(int value);
		double getPositionOfDeckAMs();
		double getPositionOfDeckBMs();
		void initializeAll();
        //void lowKillDeckOnDeckA(int value);
        //void lowKilDeckOnDeckB(int value);
		SuperpoweredAdvancedAudioPlayer *playerA, *playerB;
		//bool isEngineInitialized;
        
private:
	SLObjectItf openSLEngine, outputMix, bufferPlayer;
	SLAndroidSimpleBufferQueueItf bufferQueue;
	bool firstPlay;
	bool deckAIsPlaying;
	bool deckBIsPlaying;
    SuperpoweredRoll *roll;
    SuperpoweredFilter *filter;
    SuperpoweredFlanger *flanger;
    SuperpoweredStereoMixer *mixer;
    unsigned char activeFx;
    unsigned int samplerate;
    float crossValue, volA, volB;
    pthread_mutex_t mutex;
    float *outputBuffer[NUM_BUFFERS];
	int currentBuffer, buffersize;

};
#endif