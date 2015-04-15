#ifndef Header_Analyzer
#define Header_Analyzer

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <math.h>
#include <pthread.h>


#include "SuperpoweredAnalyzer.h"
#include "SuperpoweredDecoder.h"

class Analyzer {
public:
    Analyzer(const char* pathOfFiles);
    ~Analyzer();
    void decodeFile(const char *path);
    short int* mallocForPCMSamples(unsigned int numberOfSamplesInFile);
	float* pcmSamplesIntoFloat(short int* pcmInputIntegerSamples, int sampleSize);

    void analyzeFile();
    float returnBpm();
    int returnSampleRate();
    void deleteIntSamples();
    float returnFirstBeatMs();
    float returnSamples();
    bool hasFileBeenAnalysed;
    float* bpm;
    float* beatgridStartMs;
    int* keyIndex;
	double durationOfTrack;

private:
    SuperpoweredDecoder *decoder;
    SuperpoweredOfflineAnalyzer *analyzer;
    short int* pcmOutput;
    float floatSamples;
	float* floatSamplesPointer;
    const char* filePath;
    unsigned int* mySampleRatePointer;
	unsigned int mySampleRate;
    unsigned int* numberOfSamplesInFilePointer;
	unsigned int numberOfSamplesInFile;
	float* averageDecibel;
	float* peakDecibel;
	float* loudpartsAverageDecibel;
	
    


    unsigned char** averageWaveform;
    unsigned char** peakWaveform;
    int* waveformSize;
    char** overviewWaveform;


};

#endif