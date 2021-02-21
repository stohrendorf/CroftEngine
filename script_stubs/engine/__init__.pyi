from enum import Enum, auto
from typing import Optional, Dict, Set


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


class TR1SoundEffect(Enum):
    ...


class TR1TrackId(Enum):
    ...


class TR1ItemId(Enum):
    ...


class I18n(Enum):
    ...


class TrackInfo:
    def __init__(self, soundid: int, tracktype: TrackType, /):
        ...

class LevelSequenceItem:
    ...

class Level(LevelSequenceItem):
    def __init__(
            self, *,
            name: str,
            secrets: int,
            titles: Dict[str, str],
            track: Optional[TR1TrackId] = None,
            item_titles: Dict[str, Dict[TR1ItemId, str]] = {},
            inventory: Dict[TR1ItemId, int] = {},
            drop_inventory: Set[TR1ItemId] = set(),
            use_alternative_lara: bool = False,
            allow_save: bool = True,
    ):
        ...

class TitleMenu(Level):
    ...

class Video(LevelSequenceItem):
    def __init__(self, name: str):
        ...

class Cutscene(LevelSequenceItem):
    def __init__(
            self, *,
            name: str,
            track: TR1TrackId,
            camera_rot: float,
            gun_swap: bool = False,
            flip_rooms: bool=False,
            camera_pos_x: Optional[int] = None,
            camera_pos_z: Optional[int] = None,
    ):
        ...
