local tracks = {
    [TR1TrackId.MainTheme] = { id = 2, type = TrackType.AMBIENT }, -- Main theme
    [TR1TrackId.Poseidon] = { id = 11, type = TrackType.INTERCEPTION }, -- Poseidon
    [TR1TrackId.MainThemeShort] = { id = 12, type = TrackType.AMBIENT }, -- Main theme, short
    [TR1TrackId.Ambience0] = { id = 3, type = TrackType.AMBIENT }, -- Caves ambience
    [TR1TrackId.Thor] = { id = 13, type = TrackType.INTERCEPTION }, -- Thor
    [TR1TrackId.SaintFrancis] = { id = 14, type = TrackType.INTERCEPTION }, -- St. Francis
    [TR1TrackId.Danger] = { id = 15, type = TrackType.INTERCEPTION }, -- Danger
    [TR1TrackId.Stairs] = { id = 16, type = TrackType.INTERCEPTION }, -- Stairs
    [TR1TrackId.Midas] = { id = 17, type = TrackType.INTERCEPTION }, -- Midas
    [TR1TrackId.Level] = { id = 18, type = TrackType.INTERCEPTION }, -- Lever
    [TR1TrackId.Hmm] = { id = 19, type = TrackType.INTERCEPTION }, -- Hmm...
    [TR1TrackId.Secret] = { id = TR1SoundId.SecretFound, type = TrackType.AMBIENT_EFFECT }, -- Secret theme
    [TR1TrackId.BigSecret] = { id = 20, type = TrackType.INTERCEPTION }, -- Big secret theme
    [TR1TrackId.Raiders] = { id = 21, type = TrackType.INTERCEPTION }, -- Raiders
    [TR1TrackId.Wolf] = { id = 22, type = TrackType.INTERCEPTION }, -- Wolf
    [TR1TrackId.Awe] = { id = 23, type = TrackType.INTERCEPTION }, -- Awe
    [TR1TrackId.Gods] = { id = 24, type = TrackType.INTERCEPTION }, -- Gods
    [TR1TrackId.MainThemeReprise] = { id = 25, type = TrackType.INTERCEPTION }, -- Main theme, reprise
    [TR1TrackId.Mummy] = { id = 26, type = TrackType.INTERCEPTION }, -- Mummy
    [TR1TrackId.MidasReprise] = { id = 27, type = TrackType.INTERCEPTION }, -- Midas, reprise
    [TR1TrackId.Cutscene1] = { id = 7, type = TrackType.AMBIENT }, -- Natla cutscene
    [TR1TrackId.Cutscene2] = { id = 8, type = TrackType.AMBIENT }, -- Larson cutscene
    [TR1TrackId.Cutscene3] = { id = 9, type = TrackType.AMBIENT }, -- Natla scion cutscene
    [TR1TrackId.Cutscene4] = { id = 10, type = TrackType.AMBIENT }, -- Tihocan cutscene
    [TR1TrackId.Ambience1] = { id = 3, type = TrackType.AMBIENT }, -- Silence
    [TR1TrackId.Ambience2] = { id = 4, type = TrackType.AMBIENT }, -- PC ONLY: Water ambience
    [TR1TrackId.Ambience3] = { id = 5, type = TrackType.AMBIENT }, -- PC ONLY: Wind  ambience
    [TR1TrackId.Ambience4] = { id = 6, type = TrackType.AMBIENT }, -- PC ONLY: Pulse ambience
};

for i = TR1TrackId.LaraTalk0, TR1TrackId.LaraTalk30 do
    tracks[i] = {
        id = i - TR1TrackId.LaraTalk0 + TR1SoundId.LaraTalk0,
        type = TrackType.LARA_TALK
    }
end

return tracks
