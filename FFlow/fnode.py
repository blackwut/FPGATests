from enum import Enum


class FDispatchingMode(Enum):
    NONE = 1
    RR_BLOCKING = 2
    RR_NON_BLOCKING = 3
    KEYBY = 4
    BROADCAST = 5


class FGatheringMode(Enum):
    NONE = 1
    BLOCKING = 2
    NON_BLOCKING = 3


# class FFunctionType(Enum):
#     NONE = 1    # do nothing
#     TUPLE = 2   # process tuple only
#     GLOBAL = 3  # process global memory only
#     BOTH = 4    # process tuple and global memory


class FChannel:
    def __init__(self,
                 data_type: str,
                 name: str,
                 depth: int = 1,
                 o_degree: int = 1,
                 i_degree: int = 1):
        self.tuple_type = 'tuple_' + name
        self.data_type = data_type
        self.name = name
        self.depth = depth
        self.o_degree = o_degree
        self.i_degree = i_degree

        self.i = self.o_degree
        self.j = self.i_degree



class FNodeType(Enum):
    NONE = 1
    SOURCE = 2
    MAP = 3
    FILTER = 4
    SINK = 5


class FNode:
    i_degree = 0
    o_degree = 0

    def __init__(self,
                 name: str = '',
                 parallelism: int = 1,
                 node_type: FNodeType = FNodeType.NONE,
                 gathering_mode: FGatheringMode = FGatheringMode.BLOCKING,
                 dispatching_mode: FDispatchingMode = FDispatchingMode.RR_BLOCKING):
        self.name = name
        self.parallelism = parallelism
        self.node_type = node_type
        self.gathering_mode = gathering_mode
        self.dispatching_mode = dispatching_mode

    def set_i_channel(self, channel: FChannel):
        self.i_channel = channel

    def set_o_channel(self, channel: FChannel):
        self.o_channel = channel
