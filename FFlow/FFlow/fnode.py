from enum import Enum

from .fdispatch import FDispatchMode
from .fgather import FGatherMode
from .fbuffer import FBuffer, FBufferKind


class FNodeKind(Enum):
    NONE = 1
    SOURCE = 2
    FILTER = 3
    MAP = 4
    FLAT_MAP = 5
    SINK = 6


class FNode:
    def __init__(self,
                 name: str,
                 par: int,
                 kind: FNodeKind,
                 gather_mode: FGatherMode,
                 dispatch_mode: FDispatchMode,
                 o_datatype: str):
        assert name
        assert par > 0
        assert isinstance(kind, FNodeKind)
        assert isinstance(gather_mode, FGatherMode)
        assert isinstance(dispatch_mode, FDispatchMode)
        assert o_datatype

        self.id = -1
        self.name = name
        self.par = par
        self.kind = kind
        self.gather_mode = gather_mode
        self.dispatch_mode = dispatch_mode
        self.i_datatype = ''
        self.o_datatype = o_datatype
        self.i_channel = None
        self.o_channel = None
        self.buffers = []

    def add_buffer(self,
                   kind: FBufferKind,
                   datatype: str,
                   name: str,
                   size: int = 1,
                   ptr: bool = True):
        self.buffers.append(FBuffer(kind, datatype, name, size, ptr))

    def get_buffers(self,
                    kind: FBufferKind):
        if kind is FBufferKind.ALL:
            return self.buffers
        return [b for b in self.buffers if b.kind is kind]

#
# Jinja2 auxiliary functions
#

    def kernel_name(self, idx):
        return self.name + '_' + str(idx)

    def function_name(self):
        return self.name + '_function'

    def call_function(self, parameter):
        return self.function_name() + '(' + parameter + (', ' if len(self.buffers) > 0 else '') + self.use_buffers(FBufferKind.ALL) + ')'

    def is_gather_b(self):
        return self.gather_mode.is_b()

    def is_gather_nb(self):
        return self.gather_mode.is_nb()

    def is_dispatch_RR(self):
        return self.dispatch_mode.is_RR()

    def is_dispatch_KEYBY(self):
        return self.dispatch_mode.is_KEYBY()

    def is_dispatch_BROADCAST(self):
        return self.dispatch_mode.is_BROADCAST()

# FNodeKind
    def is_source(self):
        return self.kind == FNodeKind.SOURCE

    def is_filter(self):
        return self.kind == FNodeKind.FILTER

    def is_map(self):
        return self.kind == FNodeKind.MAP

    def is_flat_map(self):
        return self.kind == FNodeKind.FLAT_MAP

    def is_sink(self):
        return self.kind == FNodeKind.SINK

# Channels
    def read(self, i, j):
        return self.i_channel.read(i, j)

    def read_nb(self, i, j, valid):
        return self.i_channel.read_nb(i, j, valid)

    def write(self, i, j, value):
        return self.o_channel.write(i, j, value)

    def write_nb(self, i, j, value):
        return self.o_channel.write_nb(i, j, value)

# Buffers
    def declare_buffers(self, kind: FBufferKind):
        buffs = self.get_buffers(kind)
        return ';\n'.join([b.declare() for b in buffs]) + (';' if len(buffs) > 0 else '')

    def parameter_buffers(self, kind: FBufferKind):
        buffs = self.get_buffers(kind)
        return ', '.join([b.parameter() for b in buffs])

    def parameter_buffers_list(self, kind: FBufferKind):
        buffs = self.get_buffers(kind)
        return [b.parameter() for b in buffs]

    def use_buffers(self, kind: FBufferKind):
        buffs = self.get_buffers(kind)
        return ', '.join([b.use() for b in buffs])

# Tuples
    def i_tupletype(self):
        return self.i_channel.tupletype

    def o_tupletype(self):
        return self.o_channel.tupletype

    def declare_i_tuple(self, name):
        return self.i_tupletype() + ' ' + name

    def declare_o_tuple(self, name):
        return self.o_tupletype() + ' ' + name

    def create_i_tuple(self, name, parameter):
        tupletype = self.i_tupletype()
        return tupletype + ' ' + name + ' = create_' + tupletype + '(' + parameter + ')'

    def create_o_tuple(self, name, parameter):
        tupletype = self.o_tupletype()
        return 'const ' + tupletype + ' ' + name + ' = create_' + tupletype + '(' + parameter + ')'

# Host

    def declare_macro_par(self):
        return self.use_macro_par() + ' ' + str(self.par)

    def use_macro_par(self):
        return self.name.upper() + '_PAR'
