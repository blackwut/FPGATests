from enum import Enum

from .fdispatch import FDispatchingMode
from .fgather import FGatheringMode
from .fbuffer import FBuffer, FBufferType


class FNodeType(Enum):
    NONE = 1
    SOURCE = 2
    MAP = 3
    FLAT_MAP = 4
    FILTER = 5
    SINK = 6


# name
# parallelism
# node_type
# gather_mode
# dispatch_mode
# output_data_type


class FNode:
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
        self.i_degree = 0
        self.o_degree = 0
        self.i_channel = None
        self.o_channel = None
        self.buffers = []
        self.id = -1

    def add_buffer(self,
                   buffer_type: FBufferType,
                   data_type: str,
                   name: str,
                   size: int = 1):
        assert size > 0
        self.buffers.append(FBuffer(buffer_type, data_type, name, size))

    def get_buffers(self,
                    buffer_type: FBufferType):
        return [b for b in self.buffers if b.buffer_type is buffer_type]

    def declare_macro_par(self):
        return self.name.upper() + '_PAR ' + str(self.parallelism)

    def get_macro_par_name(self):
        return self.name.upper() + '_PAR'
