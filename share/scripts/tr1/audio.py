from engine import TR1TrackId, TrackInfo

_AMBIENT = (0, True)
_INTERCEPTION = (1, False)
_EFFECT = (2, False)

tracks = {
    TR1TrackId.MainTheme: TrackInfo("AUDIO/002.ogg", *_INTERCEPTION),  # Main theme
    TR1TrackId.Ambience0: TrackInfo("AUDIO/003.ogg", *_AMBIENT),  # Caves ambience
    TR1TrackId.Ambience1: TrackInfo("AUDIO/003.ogg", *_AMBIENT),  # Silence
    TR1TrackId.Ambience2: TrackInfo("AUDIO/004.ogg", *_AMBIENT),  # PC ONLY: Water ambience
    TR1TrackId.Ambience3: TrackInfo("AUDIO/005.ogg", *_AMBIENT),  # PC ONLY: Wind  ambience
    TR1TrackId.Ambience4: TrackInfo("AUDIO/006.ogg", *_AMBIENT),  # PC ONLY: Pulse ambience
    TR1TrackId.Cutscene1: TrackInfo("AUDIO/007.ogg", *_AMBIENT),  # Natla cutscene
    TR1TrackId.Cutscene2: TrackInfo("AUDIO/008.ogg", *_AMBIENT),  # Larson cutscene
    TR1TrackId.Cutscene3: TrackInfo("AUDIO/009.ogg", *_AMBIENT),  # Natla scion cutscene
    TR1TrackId.Cutscene4: TrackInfo("AUDIO/010.ogg", *_AMBIENT),  # Tihocan cutscene
    TR1TrackId.Poseidon: TrackInfo("AUDIO/011.ogg", *_INTERCEPTION),  # Poseidon
    TR1TrackId.MainThemeShort: TrackInfo("AUDIO/012.ogg", *_INTERCEPTION),  # Main theme, short
    TR1TrackId.Thor: TrackInfo("AUDIO/013.ogg", *_INTERCEPTION),  # Thor
    TR1TrackId.SaintFrancis: TrackInfo("AUDIO/014.ogg", *_INTERCEPTION),  # St. Francis
    TR1TrackId.Danger: TrackInfo("AUDIO/015.ogg", *_INTERCEPTION),  # Danger
    TR1TrackId.Stairs: TrackInfo("AUDIO/016.ogg", *_INTERCEPTION),  # Stairs
    TR1TrackId.Midas: TrackInfo("AUDIO/017.ogg", *_INTERCEPTION),  # Midas
    TR1TrackId.Level: TrackInfo("AUDIO/018.ogg", *_INTERCEPTION),  # Lever
    TR1TrackId.Hmm: TrackInfo("AUDIO/019.ogg", *_INTERCEPTION),  # Hmm...
    TR1TrackId.BigSecret: TrackInfo("AUDIO/020.ogg", *_INTERCEPTION),  # Big secret theme
    TR1TrackId.Raiders: TrackInfo("AUDIO/021.ogg", *_INTERCEPTION),  # Raiders
    TR1TrackId.Wolf: TrackInfo("AUDIO/022.ogg", *_INTERCEPTION),  # Wolf
    TR1TrackId.Awe: TrackInfo("AUDIO/023.ogg", *_INTERCEPTION),  # Awe
    TR1TrackId.Gods: TrackInfo("AUDIO/024.ogg", *_INTERCEPTION),  # Gods
    TR1TrackId.MainThemeReprise: TrackInfo("AUDIO/025.ogg", *_INTERCEPTION),  # Main theme, reprise
    TR1TrackId.Mummy: TrackInfo("AUDIO/026.ogg", *_INTERCEPTION),  # Mummy
    TR1TrackId.MidasReprise: TrackInfo("AUDIO/027.ogg", *_INTERCEPTION),  # Midas, reprise
    TR1TrackId.Secret: TrackInfo("AUDIO/060.ogg", *_EFFECT),  # Secret theme
    **{
        TR1TrackId(int(TR1TrackId.LaraTalk0) + i): TrackInfo(f"AUDIO/{29 + i:>03}.ogg", *_INTERCEPTION)
        for i in range(int(TR1TrackId.LaraTalk30) - int(TR1TrackId.LaraTalk0) + 1)
    },
}
