from enum import Enum, auto
from typing import Optional, Dict, Set, List, Tuple
from loader.file.level import Game
from ce.core import Vec
from ce.objects import Object


class ObjectInfo:
    def __init__(self):
        ...

    ai_agent: bool
    radius: int
    hit_points: int
    pivot_length: int
    target_update_chance: int

    drop_limit: int
    step_limit: int
    fly_limit: int
    cannot_visit_blocked: bool
    cannot_visit_blockable: bool

    def customize(self, entity: Object):
        ...


class ActivationState(Enum):
    INACTIVE = auto()
    DEACTIVATED = auto()
    INVISIBLE = auto()


class WeaponType(Enum):
    None_ = auto()
    Pistols = auto()
    Magnums = auto()
    Uzis = auto()
    Shotgun = auto()


class TR1SoundEffect(Enum):
    ...


class TR1TrackId(Enum):
    ...


class TR1ItemId(Enum):
    ...


class TrackInfo:
    def __init__(self, paths: List[str], slot: int, looping: bool, fade_duration_seconds: int):
        ...


class LevelSequenceItem:
    ...


class Level(LevelSequenceItem):
    def __init__(
            self, *,
            name: str,
            titles: Dict[str, str],
            ambient: Optional[TR1TrackId] = None,
            item_titles: Dict[str, Dict[TR1ItemId, str]] = {},
            use_alternative_lara: bool = False,
            allow_save: bool = True,
            default_weapon: WeaponType = WeaponType.Pistols,
            water_color: Tuple[float, float, float] = ...,
            water_density: float = ...,
            alternative_splashscreen: Optional[str] = None,
            game: Game = Game.Unknown,
    ):
        ...


class ModifyInventory(LevelSequenceItem):
    def __init__(
            self, *,
            add_inventory: Dict[TR1ItemId, int] = {},
            drop_inventory: Set[TR1ItemId] = set(),
    ):
        ...


class TitleMenu(Level):
    ...


class Video(LevelSequenceItem):
    def __init__(self, *, paths: List[str], optional: bool = False):
        ...


class Cutscene(LevelSequenceItem):
    def __init__(
            self, *,
            name: str,
            track: TR1TrackId,
            camera_rot: float,
            weapon_swap: bool = False,
            flip_rooms: bool = False,
            camera_pos_x: Optional[int] = None,
            camera_pos_z: Optional[int] = None,
            game: Game = Game.Unknown,
    ):
        ...


class SplashScreen(LevelSequenceItem):
    def __init__(
            self, *,
            path: str,
            duration_seconds: int,
            fade_in_duration_seconds: int,
            fade_out_duration_seconds: int,
    ):
        ...


class StopAudioSlot(LevelSequenceItem):
    def __init__(self, *, slot: int):
        ...


class PlayAudioSlot(LevelSequenceItem):
    def __init__(self, *, slot: int, track: TR1TrackId):
        ...


class ResetSoundEngine(LevelSequenceItem):
    def __init__(self):
        ...


class Gameflow:
    def __init__(
            self, *,
            object_infos: Dict[TR1ItemId, ObjectInfo],
            tracks: Dict[TR1ItemId, TrackInfo],
            level_sequence: List[LevelSequenceItem],
            title_menu: LevelSequenceItem,
            title_menu_backdrop: str,
            lara_home: List[LevelSequenceItem],
            early_boot: List[LevelSequenceItem],
            cheats: dict,
            asset_root: str,
    ):
        ...


class Location:
    @property
    def position(self) -> Vec:
        ...
