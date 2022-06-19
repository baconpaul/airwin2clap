//
// Created by Paul Walker on 6/18/22.
//

#ifndef CLAUDIO_EFFECT_X_AUDIOEFFECTX_H
#define CLAUDIO_EFFECT_X_AUDIOEFFECTX_H

#include <cstdint>
#include <cstring>

struct clap_host;
typedef const clap_host * audioMasterCallback;
typedef int VstPlugCategory;
typedef int32_t VstInt32;
#define vst_strncpy strncpy

static constexpr uint32_t kVstMaxProgNameLen = 32;
static constexpr uint32_t kVstMaxParamStrLen = 64;
static constexpr uint32_t kVstMaxProductStrLen = 64;
static constexpr uint32_t kVstMaxVendorStrLen = 64;

static constexpr uint32_t kPlugCategEffect = 1;

inline void float2string(float, const char*, uint32_t) {}

struct AudioEffect
{
    virtual ~AudioEffect() = default;


   double getSampleRate() { return 48000.0; }

   void setNumInputs(uint32_t kNumInputs) {}
   void setNumOutputs(uint32_t kNumOutputs) {}
   void setUniqueID(uint32_t kUniqueId) {}

   bool canProcessReplacing() { return false; }
   bool canDoubleReplacing() { return false; }

   void programsAreChunks(bool b) {}

    virtual bool getEffectName(char* name) = 0;                       // The plug-in name
    virtual VstPlugCategory getPlugCategory() = 0;                    // The general category for the plug-in
    virtual bool getProductString(char* text) = 0;                    // This is a unique plug-in string provided by Steinberg
    virtual bool getVendorString(char* text) = 0 ;                    // Vendor info
    virtual VstInt32 getVendorVersion() = 0;                         // Version number
    virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames) = 0;
    virtual void processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames) = 0;
    virtual VstInt32 getChunk (void** data, bool isPreset) = 0;
    virtual VstInt32 setChunk (void* data, VstInt32 byteSize, bool isPreset) = 0;
    virtual float getParameter(VstInt32 index) = 0;                  // get the parameter value at the specified index
    virtual void setParameter(VstInt32 index, float value) = 0;       // set the parameter at index to value
    virtual void getParameterLabel(VstInt32 index, char *text) = 0;  // label for the parameter (eg dB)
    virtual void getParameterName(VstInt32 index, char *text) = 0;    // name of the parameter
    virtual void getParameterDisplay(VstInt32 index, char *text) = 0; // text description of the current value
    virtual VstInt32 canDo(char *text) = 0;
};

struct AudioEffectX : public AudioEffect
{
    AudioEffectX(audioMasterCallback audioMaster, uint32_t kNumPrograms, uint32_t kNumParameters) {}
};

#endif //CLAUDIO_EFFECT_X_AUDIOEFFECTX_H
