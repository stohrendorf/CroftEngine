from engine import TR1TrackId, TrackInfo, TrackType, TR1SoundEffect

tracks = {
    TR1TrackId.MainTheme: TrackInfo(2, TrackType.INTERCEPTION),  # Main theme
    TR1TrackId.Ambience0: TrackInfo(3, TrackType.AMBIENT),  # Caves ambience
    TR1TrackId.Ambience1: TrackInfo(3, TrackType.AMBIENT),  # Silence
    TR1TrackId.Ambience2: TrackInfo(4, TrackType.AMBIENT),  # PC ONLY: Water ambience
    TR1TrackId.Ambience3: TrackInfo(5, TrackType.AMBIENT),  # PC ONLY: Wind  ambience
    TR1TrackId.Ambience4: TrackInfo(6, TrackType.AMBIENT),  # PC ONLY: Pulse ambience
    TR1TrackId.Cutscene1: TrackInfo(7, TrackType.AMBIENT),  # Natla cutscene
    TR1TrackId.Cutscene2: TrackInfo(8, TrackType.AMBIENT),  # Larson cutscene
    TR1TrackId.Cutscene3: TrackInfo(9, TrackType.AMBIENT),  # Natla scion cutscene
    TR1TrackId.Cutscene4: TrackInfo(10, TrackType.AMBIENT),  # Tihocan cutscene
    TR1TrackId.Poseidon: TrackInfo(11, TrackType.INTERCEPTION),  # Poseidon
    TR1TrackId.MainThemeShort: TrackInfo(12, TrackType.INTERCEPTION),  # Main theme, short
    TR1TrackId.Thor: TrackInfo(13, TrackType.INTERCEPTION),  # Thor
    TR1TrackId.SaintFrancis: TrackInfo(14, TrackType.INTERCEPTION),  # St. Francis
    TR1TrackId.Danger: TrackInfo(15, TrackType.INTERCEPTION),  # Danger
    TR1TrackId.Stairs: TrackInfo(16, TrackType.INTERCEPTION),  # Stairs
    TR1TrackId.Midas: TrackInfo(17, TrackType.INTERCEPTION),  # Midas
    TR1TrackId.Level: TrackInfo(18, TrackType.INTERCEPTION),  # Lever
    TR1TrackId.Hmm: TrackInfo(19, TrackType.INTERCEPTION),  # Hmm...
    TR1TrackId.BigSecret: TrackInfo(20, TrackType.INTERCEPTION),  # Big secret theme
    TR1TrackId.Raiders: TrackInfo(21, TrackType.INTERCEPTION),  # Raiders
    TR1TrackId.Wolf: TrackInfo(22, TrackType.INTERCEPTION),  # Wolf
    TR1TrackId.Awe: TrackInfo(23, TrackType.INTERCEPTION),  # Awe
    TR1TrackId.Gods: TrackInfo(24, TrackType.INTERCEPTION),  # Gods
    TR1TrackId.MainThemeReprise: TrackInfo(25, TrackType.INTERCEPTION),  # Main theme, reprise
    TR1TrackId.Mummy: TrackInfo(26, TrackType.INTERCEPTION),  # Mummy
    TR1TrackId.MidasReprise: TrackInfo(27, TrackType.INTERCEPTION),  # Midas, reprise
    TR1TrackId.Secret: TrackInfo(60, TrackType.INTERCEPTION),  # Secret theme
    **{
        TR1TrackId(int(TR1TrackId.LaraTalk0) + i): TrackInfo(29 + i, TrackType.INTERCEPTION)
        for i in range(int(TR1TrackId.LaraTalk30) - int(TR1TrackId.LaraTalk0) + 1)
    },
}
