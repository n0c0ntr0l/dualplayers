#ifndef Header_Analyzer
#define Header_Analyzer

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <math.h>
#include <pthread.h>


#include "SuperpoweredAnalyzer.h"
#include "SuperpoweredDecoder.h"
#include "SuperpoweredMixer.h"
#include "SuperpoweredAudioBuffers.h"

class Analyzer {
public:
    Analyzer(const char* pathOfFiles);
    ~Analyzer();
    void decodeFile(const char *path);
    short int* mallocForPCMSamples(unsigned int numberOfSamplesInFile);
	float* pcmSamplesIntoFloat(short int* pcmInputIntegerSamples, int sampleSize);
    SuperpoweredDecoder *decoder;
    SuperpoweredOfflineAnalyzer *analyzer;
    void analyzeFile();
    float returnBpm();
    int returnSampleRate();
    unsigned int mySampleRate;
    void deleteIntSamples();
    float returnFirstBeatMs();
    float returnSamples();
    bool hasFileBeenAnalysed;
    float* bpm;
    float nonPointerBPM;
    float* beatGridStart;
    int* keyIndex;
	double durationOfTrack;

private:

    float floatSamples;
    const char* filePath;

    unsigned int* numberOfSamplesInFilePointer;
	unsigned int numberOfSamplesInFile;
	float* averageDecibel;
	float* peakDecibel;
	float* loudPartsAverageDecibel;
    unsigned char** averageWaveForm;
    unsigned char** peakWaveForm;
    int* waveFormSize;
    char** overViewWaveForm;


};

#endif