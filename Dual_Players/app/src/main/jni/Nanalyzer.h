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

class Nanalyzer{
public:
        SuperpoweredDecoder* NativeAnalyzerWrapper::newSongDecoderForFilePath(const char *path);
       void NativeAnalyzerWrapper::processSong(const char* path);
       void NativeAnalyzerWrapper::deleteAllAfterFinished();

};

#endif
