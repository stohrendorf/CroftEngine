from engine import TR1TrackId, TrackInfo, TrackType, TR1SoundEffect

tracks = {
    TR1TrackId.MainTheme: TrackInfo("AUDIO/002.ogg", TrackType.INTERCEPTION),  # Main theme
    TR1TrackId.Ambience0: TrackInfo("AUDIO/003.ogg", TrackType.AMBIENT),  # Caves ambience
    TR1TrackId.Ambience1: TrackInfo("AUDIO/003.ogg", TrackType.AMBIENT),  # Silence
    TR1TrackId.Ambience2: TrackInfo("AUDIO/004.ogg", TrackType.AMBIENT),  # PC ONLY: Water ambience
    TR1TrackId.Ambience3: TrackInfo("AUDIO/005.ogg", TrackType.AMBIENT),  # PC ONLY: Wind  ambience
    TR1TrackId.Ambience4: TrackInfo("AUDIO/006.ogg", TrackType.AMBIENT),  # PC ONLY: Pulse ambience
    TR1TrackId.Cutscene1: TrackInfo("AUDIO/007.ogg", TrackType.AMBIENT),  # Natla cutscene
    TR1TrackId.Cutscene2: TrackInfo("AUDIO/008.ogg", TrackType.AMBIENT),  # Larson cutscene
    TR1TrackId.Cutscene3: TrackInfo("AUDIO/009.ogg", TrackType.AMBIENT),  # Natla scion cutscene
    TR1TrackId.Cutscene4: TrackInfo("AUDIO/010.ogg", TrackType.AMBIENT),  # Tihocan cutscene
    TR1TrackId.Poseidon: TrackInfo("AUDIO/011.ogg", TrackType.INTERCEPTION),  # Poseidon
    TR1TrackId.MainThemeShort: TrackInfo("AUDIO/012.ogg", TrackType.INTERCEPTION),  # Main theme, short
    TR1TrackId.Thor: TrackInfo("AUDIO/013.ogg", TrackType.INTERCEPTION),  # Thor
    TR1TrackId.SaintFrancis: TrackInfo("AUDIO/014.ogg", TrackType.INTERCEPTION),  # St. Francis
    TR1TrackId.Danger: TrackInfo("AUDIO/015.ogg", TrackType.INTERCEPTION),  # Danger
    TR1TrackId.Stairs: TrackInfo("AUDIO/016.ogg", TrackType.INTERCEPTION),  # Stairs
    TR1TrackId.Midas: TrackInfo("AUDIO/017.ogg", TrackType.INTERCEPTION),  # Midas
    TR1TrackId.Level: TrackInfo("AUDIO/018.ogg", TrackType.INTERCEPTION),  # Lever
    TR1TrackId.Hmm: TrackInfo("AUDIO/019.ogg", TrackType.INTERCEPTION),  # Hmm...
    TR1TrackId.BigSecret: TrackInfo("AUDIO/020.ogg", TrackType.INTERCEPTION),  # Big secret theme
    TR1TrackId.Raiders: TrackInfo("AUDIO/021.ogg", TrackType.INTERCEPTION),  # Raiders
    TR1TrackId.Wolf: TrackInfo("AUDIO/022.ogg", TrackType.INTERCEPTION),  # Wolf
    TR1TrackId.Awe: TrackInfo("AUDIO/023.ogg", TrackType.INTERCEPTION),  # Awe
    TR1TrackId.Gods: TrackInfo("AUDIO/024.ogg", TrackType.INTERCEPTION),  # Gods
    TR1TrackId.MainThemeReprise: TrackInfo("AUDIO/025.ogg", TrackType.INTERCEPTION),  # Main theme, reprise
    TR1TrackId.Mummy: TrackInfo("AUDIO/026.ogg", TrackType.INTERCEPTION),  # Mummy
    TR1TrackId.MidasReprise: TrackInfo("AUDIO/027.ogg", TrackType.INTERCEPTION),  # Midas, reprise
    TR1TrackId.Secret: TrackInfo("AUDIO/060.ogg", TrackType.INTERCEPTION),  # Secret theme
    **{
        TR1TrackId(int(TR1TrackId.LaraTalk0) + i): TrackInfo(f"AUDIO/{29 + i:>03}.ogg", TrackType.INTERCEPTION)
        for i in range(int(TR1TrackId.LaraTalk30) - int(TR1TrackId.LaraTalk0) + 1)
    },
}
