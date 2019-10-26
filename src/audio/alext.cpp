#include "alext.h"

#include "utils.h"

#ifndef AL_ALEXT_PROTOTYPES

#  include <boost/log/trivial.hpp>

// ReSharper disable IdentifierTypo

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

#  define GETPROC(name)                                                             \
    name = reinterpret_cast<decltype(name)>(AL_ASSERT_FN(alGetProcAddress(#name))); \
    Expects(name != nullptr)

  GETPROC(alGenEffects);
  GETPROC(alDeleteEffects);
  GETPROC(alIsEffect);
  GETPROC(alEffecti);
  GETPROC(alEffectiv);
  GETPROC(alEffectf);
  GETPROC(alEffectfv);
  GETPROC(alGetEffecti);
  GETPROC(alGetEffectiv);
  GETPROC(alGetEffectf);
  GETPROC(alGetEffectfv);
  GETPROC(alGenFilters);
  GETPROC(alDeleteFilters);
  GETPROC(alIsFilter);
  GETPROC(alFilteri);
  GETPROC(alFilteriv);
  GETPROC(alFilterf);
  GETPROC(alFilterfv);
  GETPROC(alGetFilteri);
  GETPROC(alGetFilteriv);
  GETPROC(alGetFilterf);
  GETPROC(alGetFilterfv);
  GETPROC(alGenAuxiliaryEffectSlots);
  GETPROC(alDeleteAuxiliaryEffectSlots);
  GETPROC(alIsAuxiliaryEffectSlot);
  GETPROC(alAuxiliaryEffectSloti);
  GETPROC(alAuxiliaryEffectSlotiv);
  GETPROC(alAuxiliaryEffectSlotf);
  GETPROC(alAuxiliaryEffectSlotfv);
  GETPROC(alGetAuxiliaryEffectSloti);
  GETPROC(alGetAuxiliaryEffectSlotiv);
  GETPROC(alGetAuxiliaryEffectSlotf);
  GETPROC(alGetAuxiliaryEffectSlotfv);

  // ReSharper restore IdentifierTypo

#  undef GETPROC

  isLoaded = true;
}

#endif
