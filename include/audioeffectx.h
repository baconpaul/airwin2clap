/*
 * The AIRWIN code #includes a file called 'audioeffectx.h' which is often shipped with
 * another SDK and is not in the AIRWIN repo.
 *
 * We do not need that file nor its contents to port to CLAP, but we do need
 * something we can include which provides the base class interface the airwindows use.
 * And to avoid modifying the Airwindows code we want something with the same name so
 * #include actually works.
 *
 * So create this file, but then simply include a subsequent file with our base class and typedef
 * the audioeffectx classes to it.
 */
#ifndef CLAUDIO_EFFECT_X_AUDIOEFFECTX_H
#define CLAUDIO_EFFECT_X_AUDIOEFFECTX_H

#include "airwin-to-clap.h"

typedef AirwinToClapPortBaseClass AudioEffect;
typedef AirwinToClapPortBaseClass AudioEffectX;


#endif //CLAUDIO_EFFECT_X_AUDIOEFFECTX_H
