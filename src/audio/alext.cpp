#include "alext.h"
#include "utils.h"

#ifndef AL_ALEXT_PROTOTYPES

#include <boost/log/trivial.hpp>

// Effect objects
LPALGENEFFECTS alGenEffects = nullptr;
LPALDELETEEFFECTS alDeleteEffects = nullptr;
LPALISEFFECT alIsEffect = nullptr;
LPALEFFECTI alEffecti = nullptr;
LPALEFFECTIV alEffectiv = nullptr;
LPALEFFECTF alEffectf = nullptr;
LPALEFFECTFV alEffectfv = nullptr;
LPALGETEFFECTI alGetEffecti = nullptr;
LPALGETEFFECTIV alGetEffectiv = nullptr;
LPALGETEFFECTF alGetEffectf = nullptr;
LPALGETEFFECTFV alGetEffectfv = nullptr;

//Filter objects
LPALGENFILTERS alGenFilters = nullptr;
LPALDELETEFILTERS alDeleteFilters = nullptr;
LPALISFILTER alIsFilter = nullptr;
LPALFILTERI alFilteri = nullptr;
LPALFILTERIV alFilteriv = nullptr;
LPALFILTERF alFilterf = nullptr;
LPALFILTERFV alFilterfv = nullptr;
LPALGETFILTERI alGetFilteri = nullptr;
LPALGETFILTERIV alGetFilteriv = nullptr;
LPALGETFILTERF alGetFilterf = nullptr;
LPALGETFILTERFV alGetFilterfv = nullptr;

// Auxiliary slot object
LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = nullptr;
LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = nullptr;
LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot = nullptr;
LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = nullptr;
LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv = nullptr;
LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf = nullptr;
LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv = nullptr;
LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti = nullptr;
LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv = nullptr;
LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf = nullptr;
LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv = nullptr;

void audio::loadALExtFunctions(const gsl::not_null<ALCdevice*>& device)
{
    static bool isLoaded = false;
    if(isLoaded)
        return;

    BOOST_LOG_TRIVIAL(info) << "OpenAL device extensions: " << alcGetString(device, ALC_EXTENSIONS);
    BOOST_ASSERT(alcIsExtensionPresent(device, ALC_EXT_EFX_NAME) == ALC_TRUE);

#define GETPROC(type, name) \
    name = reinterpret_cast<type>(alGetProcAddress(#name)); \
    DEBUG_CHECK_AL_ERROR(); \
    Expects(name != nullptr)

    GETPROC(LPALGENEFFECTS, alGenEffects);
    GETPROC(LPALDELETEEFFECTS, alDeleteEffects);
    GETPROC(LPALISEFFECT, alIsEffect);
    GETPROC(LPALEFFECTI, alEffecti);
    GETPROC(LPALEFFECTIV, alEffectiv);
    GETPROC(LPALEFFECTF, alEffectf);
    GETPROC(LPALEFFECTFV, alEffectfv);
    GETPROC(LPALGETEFFECTI, alGetEffecti);
    GETPROC(LPALGETEFFECTIV, alGetEffectiv);
    GETPROC(LPALGETEFFECTF, alGetEffectf);
    GETPROC(LPALGETEFFECTFV, alGetEffectfv);
    GETPROC(LPALGENFILTERS, alGenFilters);
    GETPROC(LPALDELETEFILTERS, alDeleteFilters);
    GETPROC(LPALISFILTER, alIsFilter);
    GETPROC(LPALFILTERI, alFilteri);
    GETPROC(LPALFILTERIV, alFilteriv);
    GETPROC(LPALFILTERF, alFilterf);
    GETPROC(LPALFILTERFV, alFilterfv);
    GETPROC(LPALGETFILTERI, alGetFilteri);
    GETPROC(LPALGETFILTERIV, alGetFilteriv);
    GETPROC(LPALGETFILTERF, alGetFilterf);
    GETPROC(LPALGETFILTERFV, alGetFilterfv);
    GETPROC(LPALGENAUXILIARYEFFECTSLOTS, alGenAuxiliaryEffectSlots);
    GETPROC(LPALDELETEAUXILIARYEFFECTSLOTS, alDeleteAuxiliaryEffectSlots);
    GETPROC(LPALISAUXILIARYEFFECTSLOT, alIsAuxiliaryEffectSlot);
    GETPROC(LPALAUXILIARYEFFECTSLOTI, alAuxiliaryEffectSloti);
    GETPROC(LPALAUXILIARYEFFECTSLOTIV, alAuxiliaryEffectSlotiv);
    GETPROC(LPALAUXILIARYEFFECTSLOTF, alAuxiliaryEffectSlotf);
    GETPROC(LPALAUXILIARYEFFECTSLOTFV, alAuxiliaryEffectSlotfv);
    GETPROC(LPALGETAUXILIARYEFFECTSLOTI, alGetAuxiliaryEffectSloti);
    GETPROC(LPALGETAUXILIARYEFFECTSLOTIV, alGetAuxiliaryEffectSlotiv);
    GETPROC(LPALGETAUXILIARYEFFECTSLOTF, alGetAuxiliaryEffectSlotf);
    GETPROC(LPALGETAUXILIARYEFFECTSLOTFV, alGetAuxiliaryEffectSlotfv);

#undef GETPROC

    isLoaded = true;
}

#endif // ifndef AL_ALEXT_PROTOTYPES