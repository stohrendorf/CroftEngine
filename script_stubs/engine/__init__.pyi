from enum import Enum, auto


class ObjectInfo:
    def __init__(self):
        ...

    ai_agent: bool
    radius: int
    hit_points: int
    pivot_length: int
    target_update_chance: int


class ActivationState(Enum):
    INACTIVE = auto()
    DEACTIVATED = auto()
    INVISIBLE = auto()


class TrackType(Enum):
    AMBIENT = auto()
    INTERCEPTION = auto()
    AMBIENT_EFFECT = auto()
    LARA_TALK = auto()


class TR1SoundId(Enum):
    ...


class TR1TrackId(Enum):
    ...


class TR1ItemId(Enum):
    ...


class TrackInfo:
    def __init__(self, soundid: int, tracktype: TrackType, /):
        ...
