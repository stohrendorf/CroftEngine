from ce.engine import TR1TrackId, TrackInfo

_AMBIENT = (0, True)
_CUTSCENE = (0, False)
_INTERCEPTION = (1, False)
_EFFECT = (2, False)


def _track(track_id: TR1TrackId, *args):
    return {
        track_id: TrackInfo([
            f"music/Track{int(track_id):>02}.flac",
        ], *args),
    }


tracks = {
    **_track(TR1TrackId.MainTheme, *_INTERCEPTION),  # Main theme
    **_track(TR1TrackId.Ambience1, *_AMBIENT, 30),  # Silence
    **_track(TR1TrackId.Ambience2, *_AMBIENT, 30),  # PC ONLY: Water ambience
    **_track(TR1TrackId.Ambience3, *_AMBIENT, 30),  # PC ONLY: Wind  ambience
    **_track(TR1TrackId.Ambience4, *_AMBIENT, 30),  # PC ONLY: Pulse ambience
    **_track(TR1TrackId.Cutscene1, *_CUTSCENE),  # Natla cutscene
    **_track(TR1TrackId.Cutscene2, *_CUTSCENE),  # Larson cutscene
    **_track(TR1TrackId.Cutscene3, *_CUTSCENE),  # Natla scion cutscene
    **_track(TR1TrackId.Cutscene4, *_CUTSCENE),  # Tihocan cutscene
    **_track(TR1TrackId.SaintFrancis, *_INTERCEPTION),  # St. Francis
    **_track(TR1TrackId.Danger, *_INTERCEPTION),  # Danger
    **_track(TR1TrackId.Stairs, *_INTERCEPTION),  # Stairs
    **_track(TR1TrackId.Midas, *_INTERCEPTION),  # Midas
    **_track(TR1TrackId.Level, *_INTERCEPTION),  # Lever
    **_track(TR1TrackId.Hmm, *_INTERCEPTION),  # Hmm...
    **_track(TR1TrackId.BigSecret, *_INTERCEPTION),  # Big secret theme
    **_track(TR1TrackId.Raiders, *_INTERCEPTION),  # Raiders
    **_track(TR1TrackId.Wolf, *_INTERCEPTION),  # Wolf
    **_track(TR1TrackId.Awe, *_INTERCEPTION),  # Awe
    **_track(TR1TrackId.Gods, *_INTERCEPTION),  # Gods
    **_track(TR1TrackId.MainThemeReprise, *_INTERCEPTION),  # Main theme, reprise
    **_track(TR1TrackId.Mummy, *_INTERCEPTION),  # Mummy
    **_track(TR1TrackId.MidasReprise, *_INTERCEPTION),  # Midas, reprise
    **_track(TR1TrackId.Secret, *_EFFECT),  # Secret theme
    **_track(TR1TrackId.LaraTalk0, *_INTERCEPTION),
    **_track(TR1TrackId.LaraTalk1, *_INTERCEPTION),
    **_track(TR1TrackId.LaraTalk2, *_INTERCEPTION),
    **_track(TR1TrackId.LaraTalk30, *_INTERCEPTION),
}
