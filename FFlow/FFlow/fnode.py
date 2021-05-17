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


class FMemoryType(Enum):
    PRIVATE = 1
    LOCAL = 2
    GLOBAL = 3


class FMemory:
    def __init__(self,
                 memory_type: FMemoryType,
                 data_type: str,
                 name: str,
                 size: int):
        self.memory_type = memory_type
        self.data_type = data_type
        self.name = name
        self.size = size


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
    FLAT_MAP = 4
    FILTER = 5
    SINK = 6


class FNode:
    i_degree = 0
    o_degree = 0

    memories = []

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

    def add_memory(self,
                   memory_type: FMemoryType,
                   data_type: str,
                   name: str,
                   size: int):
        self.memories.append(FMemory(memory_type, data_type, name, size))
