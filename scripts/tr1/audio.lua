local tracks = {
    [TR1TrackId.MainTheme] = TrackInfo.new(2, TrackType.INTERCEPTION), -- Main theme
    [TR1TrackId.Poseidon] = TrackInfo.new(11, TrackType.INTERCEPTION), -- Poseidon
    [TR1TrackId.MainThemeShort] = TrackInfo.new(12, TrackType.AMBIENT), -- Main theme, short
    [TR1TrackId.Ambience0] = TrackInfo.new(3, TrackType.AMBIENT), -- Caves ambience
    [TR1TrackId.Thor] = TrackInfo.new(13, TrackType.INTERCEPTION), -- Thor
    [TR1TrackId.SaintFrancis] = TrackInfo.new(14, TrackType.INTERCEPTION), -- St. Francis
    [TR1TrackId.Danger] = TrackInfo.new(15, TrackType.INTERCEPTION), -- Danger
    [TR1TrackId.Stairs] = TrackInfo.new(16, TrackType.INTERCEPTION), -- Stairs
    [TR1TrackId.Midas] = TrackInfo.new(17, TrackType.INTERCEPTION), -- Midas
    [TR1TrackId.Level] = TrackInfo.new(18, TrackType.INTERCEPTION), -- Lever
    [TR1TrackId.Hmm] = TrackInfo.new(19, TrackType.INTERCEPTION), -- Hmm...
    [TR1TrackId.Secret] = TrackInfo.new(TR1SoundId.SecretFound, TrackType.AMBIENT_EFFECT), -- Secret theme
    [TR1TrackId.BigSecret] = TrackInfo.new(20, TrackType.INTERCEPTION), -- Big secret theme
    [TR1TrackId.Raiders] = TrackInfo.new(21, TrackType.INTERCEPTION), -- Raiders
    [TR1TrackId.Wolf] = TrackInfo.new(22, TrackType.INTERCEPTION), -- Wolf
    [TR1TrackId.Awe] = TrackInfo.new(23, TrackType.INTERCEPTION), -- Awe
    [TR1TrackId.Gods] = TrackInfo.new(24, TrackType.INTERCEPTION), -- Gods
    [TR1TrackId.MainThemeReprise] = TrackInfo.new(25, TrackType.INTERCEPTION), -- Main theme, reprise
    [TR1TrackId.Mummy] = TrackInfo.new(26, TrackType.INTERCEPTION), -- Mummy
    [TR1TrackId.MidasReprise] = TrackInfo.new(27, TrackType.INTERCEPTION), -- Midas, reprise
    [TR1TrackId.Cutscene1] = TrackInfo.new(7, TrackType.AMBIENT), -- Natla cutscene
    [TR1TrackId.Cutscene2] = TrackInfo.new(8, TrackType.AMBIENT), -- Larson cutscene
    [TR1TrackId.Cutscene3] = TrackInfo.new(9, TrackType.AMBIENT), -- Natla scion cutscene
    [TR1TrackId.Cutscene4] = TrackInfo.new(10, TrackType.AMBIENT), -- Tihocan cutscene
    [TR1TrackId.Ambience1] = TrackInfo.new(3, TrackType.AMBIENT), -- Silence
    [TR1TrackId.Ambience2] = TrackInfo.new(4, TrackType.AMBIENT), -- PC ONLY: Water ambience
    [TR1TrackId.Ambience3] = TrackInfo.new(5, TrackType.AMBIENT), -- PC ONLY: Wind  ambience
    [TR1TrackId.Ambience4] = TrackInfo.new(6, TrackType.AMBIENT), -- PC ONLY: Pulse ambience
};

for i = TR1TrackId.LaraTalk0, TR1TrackId.LaraTalk30 do
    tracks[i] = TrackInfo.new(TR1TrackId.LaraTalk0 + TR1SoundId.LaraTalk0, TrackType.LARA_TALK)
end

return tracks
