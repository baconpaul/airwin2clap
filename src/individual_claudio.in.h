//
// Created by Paul Walker on 6/19/22.
//

// clang-format off
#ifndef CLAUDIO_EFFECT_@AIRWIN_NAME@_X_INDIVIDUAL_CLAUDIO_IN_H
#define CLAUDIO_EFFECT_@AIRWIN_NAME@_X_INDIVIDUAL_CLAUDIO_IN_H

#include "audioeffectx.h"
#include <clap/clap.h>

extern AudioEffect *@CREATE_FN@ (audioMasterCallback audioMaster);
extern clap_plugin_descriptor claudio_@AIRWIN_NAME@_desc;

#endif // CLAUDIO_EFFECT_X_INDIVIDUAL_CLAUDIO_IN_H
