from engine import TR1TrackId, TrackInfo

_AMBIENT = (0, True)
_CUTSCENE = (0, False)
_INTERCEPTION = (1, False)
_EFFECT = (2, False)


def _track(track_id: TR1TrackId, ogg: int, *args):
    return {
        track_id: TrackInfo([
            f"Music/Track{int(track_id):>02}.flac",
            f"AUDIO/{ogg:>03}.ogg",
        ], *args),
    }


tracks = {
    **_track(TR1TrackId.MainTheme, 2, *_INTERCEPTION),  # Main theme
    **_track(TR1TrackId.Ambience0, 3, *_AMBIENT, 30),  # Caves ambience
    **_track(TR1TrackId.Ambience1, 3, *_AMBIENT, 30),  # Silence
    **_track(TR1TrackId.Ambience2, 4, *_AMBIENT, 30),  # PC ONLY: Water ambience
    **_track(TR1TrackId.Ambience3, 5, *_AMBIENT, 30),  # PC ONLY: Wind  ambience
    **_track(TR1TrackId.Ambience4, 6, *_AMBIENT, 30),  # PC ONLY: Pulse ambience
    **_track(TR1TrackId.Cutscene1, 7, *_CUTSCENE),  # Natla cutscene
    **_track(TR1TrackId.Cutscene2, 8, *_CUTSCENE),  # Larson cutscene
    **_track(TR1TrackId.Cutscene3, 9, *_CUTSCENE),  # Natla scion cutscene
    **_track(TR1TrackId.Cutscene4, 10, *_CUTSCENE),  # Tihocan cutscene
    **_track(TR1TrackId.Poseidon, 11, *_INTERCEPTION),  # Poseidon
    **_track(TR1TrackId.MainThemeShort, 12, *_INTERCEPTION),  # Main theme, short
    **_track(TR1TrackId.Thor, 13, *_INTERCEPTION),  # Thor
    **_track(TR1TrackId.SaintFrancis, 14, *_INTERCEPTION),  # St. Francis
    **_track(TR1TrackId.Danger, 15, *_INTERCEPTION),  # Danger
    **_track(TR1TrackId.Stairs, 16, *_INTERCEPTION),  # Stairs
    **_track(TR1TrackId.Midas, 17, *_INTERCEPTION),  # Midas
    **_track(TR1TrackId.Level, 18, *_INTERCEPTION),  # Lever
    **_track(TR1TrackId.Hmm, 19, *_INTERCEPTION),  # Hmm...
    **_track(TR1TrackId.BigSecret, 20, *_INTERCEPTION),  # Big secret theme
    **_track(TR1TrackId.Raiders, 21, *_INTERCEPTION),  # Raiders
    **_track(TR1TrackId.Wolf, 22, *_INTERCEPTION),  # Wolf
    **_track(TR1TrackId.Awe, 23, *_INTERCEPTION),  # Awe
    **_track(TR1TrackId.Gods, 24, *_INTERCEPTION),  # Gods
    **_track(TR1TrackId.MainThemeReprise, 25, *_INTERCEPTION),  # Main theme, reprise
    **_track(TR1TrackId.Mummy, 26, *_INTERCEPTION),  # Mummy
    **_track(TR1TrackId.MidasReprise, 27, *_INTERCEPTION),  # Midas, reprise
    **_track(TR1TrackId.Secret, 60, *_EFFECT),  # Secret theme
}

for i in range(int(TR1TrackId.LaraTalk30) - int(TR1TrackId.LaraTalk0) + 1):
    for k, v in _track(TR1TrackId(int(TR1TrackId.LaraTalk0) + i), 29 + i, *_INTERCEPTION).items():
        tracks[k] = v
