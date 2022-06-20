//
// Created by Paul Walker on 6/19/22.
//

// clang-format off
#include "audioeffectx.h"
#include "aw2c_@AIRWIN_NAME@.h"

const char *@AIRWIN_NAME@_features[] = {CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, nullptr};
clap_plugin_descriptor aw2c_@AIRWIN_NAME@_desc = {
    CLAP_VERSION,
    "unofficial.com.airwindows.@AIRWIN_NAME@", // change this change the cmake generatortoo
    "AW @AIRWIN_NAME@ (Unofficial)",
    "Unsupported",
    "https://airwindows.com",
    "",
    "",
    "1.0.0",
    "The airwindows effect @AIRWIN_NAME@.",
    @AIRWIN_NAME@_features};