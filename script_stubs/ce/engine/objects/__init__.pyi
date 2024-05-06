from ce.core import Length


class Object:
    ...


class Doppelganger(Object):
    # WARNING: ALWAYS implement the following, seemingly useless constructor,
    # otherwise the C++ instance won't get initialized properly
    def __init__(self):
        super().__init__()

    def set_center(self, sector_x: Length, sector_z: Length) -> None:
        ...
