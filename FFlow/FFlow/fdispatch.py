from enum import Enum


class FDispatchingMode(Enum):
    NONE = 1
    RR_BLOCKING = 2
    RR_NON_BLOCKING = 3
    KEYBY = 4
    BROADCAST = 5
