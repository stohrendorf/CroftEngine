from engine import TR1TrackId, TrackInfo

_AMBIENT = (0, True)
_INTERCEPTION = (1, False)
_EFFECT = (2, False)


def _tracks(n: int):
    return [
        f"Music/Track{n:>02}.flac",
        f"AUDIO/{n:>03}.ogg",
    ]


tracks = {
    TR1TrackId.MainTheme: TrackInfo(_tracks(2), *_INTERCEPTION),  # Main theme
    TR1TrackId.Ambience0: TrackInfo(_tracks(3), *_AMBIENT, 30),  # Caves ambience
    TR1TrackId.Ambience1: TrackInfo(_tracks(3), *_AMBIENT, 30),  # Silence
    TR1TrackId.Ambience2: TrackInfo(_tracks(4), *_AMBIENT, 30),  # PC ONLY: Water ambience
    TR1TrackId.Ambience3: TrackInfo(_tracks(5), *_AMBIENT, 30),  # PC ONLY: Wind  ambience
    TR1TrackId.Ambience4: TrackInfo(_tracks(6), *_AMBIENT, 30),  # PC ONLY: Pulse ambience
    TR1TrackId.Cutscene1: TrackInfo(_tracks(7), *_AMBIENT),  # Natla cutscene
    TR1TrackId.Cutscene2: TrackInfo(_tracks(8), *_AMBIENT),  # Larson cutscene
    TR1TrackId.Cutscene3: TrackInfo(_tracks(9), *_AMBIENT),  # Natla scion cutscene
    TR1TrackId.Cutscene4: TrackInfo(_tracks(10), *_AMBIENT),  # Tihocan cutscene
    TR1TrackId.Poseidon: TrackInfo(_tracks(11), *_INTERCEPTION),  # Poseidon
    TR1TrackId.MainThemeShort: TrackInfo(_tracks(12), *_INTERCEPTION),  # Main theme, short
    TR1TrackId.Thor: TrackInfo(_tracks(13), *_INTERCEPTION),  # Thor
    TR1TrackId.SaintFrancis: TrackInfo(_tracks(14), *_INTERCEPTION),  # St. Francis
    TR1TrackId.Danger: TrackInfo(_tracks(15), *_INTERCEPTION),  # Danger
    TR1TrackId.Stairs: TrackInfo(_tracks(16), *_INTERCEPTION),  # Stairs
    TR1TrackId.Midas: TrackInfo(_tracks(17), *_INTERCEPTION),  # Midas
    TR1TrackId.Level: TrackInfo(_tracks(18), *_INTERCEPTION),  # Lever
    TR1TrackId.Hmm: TrackInfo(_tracks(19), *_INTERCEPTION),  # Hmm...
    TR1TrackId.BigSecret: TrackInfo(_tracks(20), *_INTERCEPTION),  # Big secret theme
    TR1TrackId.Raiders: TrackInfo(_tracks(21), *_INTERCEPTION),  # Raiders
    TR1TrackId.Wolf: TrackInfo(_tracks(22), *_INTERCEPTION),  # Wolf
    TR1TrackId.Awe: TrackInfo(_tracks(23), *_INTERCEPTION),  # Awe
    TR1TrackId.Gods: TrackInfo(_tracks(24), *_INTERCEPTION),  # Gods
    TR1TrackId.MainThemeReprise: TrackInfo(_tracks(25), *_INTERCEPTION),  # Main theme, reprise
    TR1TrackId.Mummy: TrackInfo(_tracks(26), *_INTERCEPTION),  # Mummy
    TR1TrackId.MidasReprise: TrackInfo(_tracks(27), *_INTERCEPTION),  # Midas, reprise
    TR1TrackId.Secret: TrackInfo(_tracks(60), *_EFFECT),  # Secret theme
    **{
        TR1TrackId(int(TR1TrackId.LaraTalk0) + i): TrackInfo(_tracks(29 + i), *_INTERCEPTION)
        for i in range(int(TR1TrackId.LaraTalk30) - int(TR1TrackId.LaraTalk0) + 1)
    },
}
