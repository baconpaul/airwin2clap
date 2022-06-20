//
// Created by Paul Walker on 6/20/22.
//

#ifndef AIRWIN_TO_CLAP_AIRWIN_TO_CLAP_H
#define AIRWIN_TO_CLAP_AIRWIN_TO_CLAP_H

// This file has the smallest possible API required to allow each of the Airwin
// MacVST to compile without any external API.


#include <cstdint>
#include <cstring>
#include <string>

#include <cmath>

struct clap_host;
typedef const clap_host *audioMasterCallback;
typedef int VstPlugCategory;
typedef int32_t VstInt32;
#define vst_strncpy strncpy

static constexpr uint32_t kVstMaxProgNameLen = 32;
static constexpr uint32_t kVstMaxParamStrLen = 64;
static constexpr uint32_t kVstMaxProductStrLen = 64;
static constexpr uint32_t kVstMaxVendorStrLen = 64;

static constexpr uint32_t kPlugCategEffect = 1;


inline void float2string(float f, char *c, uint32_t n) {
    strncpy(c, std::to_string(f).c_str(), n);
}

inline void dB2string(float value, char *t, uint32_t num) {
    if (value <= 0.00001) // -100 dB, show -inf from that point onwards
        vst_strncpy (t, "-inf", num);
    else
        float2string ((float)(20.0 * log10 (value)), t, num);
}

inline void int2string(int f, char *c, uint32_t n) {
    strncpy(c, std::to_string(f).c_str(), n);
}

struct AirwinToClapPortBaseClass {
    uint32_t numParams{0};
    AirwinToClapPortBaseClass(audioMasterCallback audioMaster, uint32_t kNumPrograms, uint32_t kNumParameters) : numParams(kNumParameters){}

    virtual ~AirwinToClapPortBaseClass() = default;


    double sr{1};

    void setSampleRate(double d) { sr = d; }
    double getSampleRate() { return sr; }

    int nin{0}, nout{0};
    void setNumInputs(uint32_t kNumInputs) { nin = kNumInputs; }
    void setNumOutputs(uint32_t kNumOutputs) { nout = kNumOutputs; }
    void setUniqueID(uint32_t kUniqueId) {}

    bool canProcessReplacing() { return false; }
    bool canDoubleReplacing() { return false; }

    void programsAreChunks(bool b) {}
    virtual void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) = 0;
    virtual void processDoubleReplacing(double **inputs, double **outputs, VstInt32 sampleFrames) = 0;

    virtual VstInt32 getChunk(void **data, bool isPreset) { return 0; };

    virtual VstInt32 setChunk(void *data, VstInt32 byteSize, bool isPreset) { return 0; };

    virtual float getParameter(VstInt32 index) { return 0.f; };
    virtual void setParameter(VstInt32 index, float value) {};
    virtual void getParameterLabel(VstInt32 index, char *text) {};
    virtual void getParameterName(VstInt32 index, char *text) {};
    virtual void getParameterDisplay(VstInt32 index, char *text) {};
    virtual VstInt32 canDo(char *text) = 0;
};

#endif // AIRWIN_TO_CLAP_AIRWIN_TO_CLAP_H
