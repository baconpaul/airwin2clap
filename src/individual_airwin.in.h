//
// Created by Paul Walker on 6/19/22.
//

// clang-format off
#ifndef AW2C_@AIRWIN_NAME@_X_INDIVIDUAL_AW2C_IN_H
#define AW2C_@AIRWIN_NAME@_X_INDIVIDUAL_AW2C_IN_H

#include "audioeffectx.h"
#include <clap/clap.h>

extern AudioEffect *@CREATE_FN@ (audioMasterCallback audioMaster);
extern clap_plugin_descriptor aw2c_@AIRWIN_NAME@_desc;

#endif
