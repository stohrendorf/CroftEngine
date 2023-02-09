from typing import overload


class Length:
    def __init__(self, value: int):
        ...

    @property
    def value(self) -> int:
        ...


class Vec:
    x: Length
    y: Length
    z: Length

    def __init__(self):
        ...

    @overload
    def __init__(self, x: Length, y: Length, z: Length):
        pass

SectorSize: int
QuarterSectorSize: int
