local tracks = {
    [2] = { id = 2, type = TrackType.AMBIENT }, -- Main theme
    [3] = { id = 11, type = TrackType.INTERCEPTION }, -- Poseidon
    [4] = { id = 12, type = TrackType.AMBIENT }, -- Main theme, short
    [5] = { id = 3, type = TrackType.AMBIENT }, -- Caves ambience
    [6] = { id = 13, type = TrackType.INTERCEPTION }, -- Thor
    [7] = { id = 14, type = TrackType.INTERCEPTION }, -- St. Francis
    [8] = { id = 15, type = TrackType.INTERCEPTION }, -- Danger
    [9] = { id = 16, type = TrackType.INTERCEPTION }, -- Stairs
    [10] = { id = 17, type = TrackType.INTERCEPTION }, -- Midas
    [11] = { id = 18, type = TrackType.INTERCEPTION }, -- Lever
    [12] = { id = 19, type = TrackType.INTERCEPTION }, -- Hmm...
    [13] = { id = 173, type = TrackType.AMBIENT_EFFECT }, -- Secret theme
    [14] = { id = 20, type = TrackType.INTERCEPTION }, -- Big secret theme
    [15] = { id = 21, type = TrackType.INTERCEPTION }, -- Raiders
    [16] = { id = 22, type = TrackType.INTERCEPTION }, -- Wolf
    [17] = { id = 23, type = TrackType.INTERCEPTION }, -- Awe
    [18] = { id = 24, type = TrackType.INTERCEPTION }, -- Gods
    [19] = { id = 25, type = TrackType.INTERCEPTION }, -- Main theme, reprise
    [20] = { id = 26, type = TrackType.INTERCEPTION }, -- Mummy
    [21] = { id = 27, type = TrackType.INTERCEPTION }, -- Midas, reprise
    [22] = { id = 7, type = TrackType.AMBIENT }, -- Natla cutscene
    [23] = { id = 8, type = TrackType.AMBIENT }, -- Larson cutscene
    [24] = { id = 9, type = TrackType.AMBIENT }, -- Natla scion cutscene
    [25] = { id = 10, type = TrackType.AMBIENT }, -- Tihocan cutscene
    [57] = { id = 3, type = TrackType.AMBIENT }, -- Silence
    [58] = { id = 4, type = TrackType.AMBIENT }, -- PC ONLY: Water ambience
    [59] = { id = 5, type = TrackType.AMBIENT }, -- PC ONLY: Wind  ambience
    [60] = { id = 6, type = TrackType.AMBIENT }, -- PC ONLY: Pulse ambience
};

for i = 26, 56 do
    tracks[i] = {
        id = i + 148,
        type = TrackType.LARA_TALK
    }
end

return tracks
