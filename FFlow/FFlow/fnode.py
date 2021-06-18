from enum import Enum

from .fdispatch import FDispatchMode
from .fgather import FGatherMode
from .fbuffer import FBufferPrivate, FBufferLocal, FBufferGlobal, FBufferAccess


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

    def add_private_buffer(self,
                           datatype: str,
                           name: str,
                           size: int = 1,
                           value=None,
                           ptr: bool = False):
        self.buffers.append(FBufferPrivate(datatype, name, size, value, ptr))

    def add_local_buffer(self,
                         datatype: str,
                         name: str,
                         size: int = 1,
                         value=None):
        self.buffers.append(FBufferLocal(datatype, name, size, value))

    def add_global_buffer(self,
                          datatype: str,
                          name: str,
                          size: int = 1,
                          access: FBufferAccess = FBufferAccess.READ_ALL,
                          ptr: bool = True):
        self.buffers.append(FBufferGlobal(datatype, name, size, access, ptr))

    def get_buffers(self):
        return self.buffers

    def get_private_buffers(self):
        return [b for b in self.buffers if type(b) is FBufferPrivate]

    def get_local_buffers(self):
        return [b for b in self.buffers if type(b) is FBufferLocal]

    def get_global_value_buffers(self):
        return [b for b in self.buffers if type(b) is FBufferGlobal and b.has_value()]

    def get_global_no_value_buffers(self):
        return [b for b in self.buffers if type(b) is FBufferGlobal and not b.has_value()]

    def get_global_buffers(self, incl_has_values=True):
        buffs = self.get_global_no_value_buffers()
        if incl_has_values:
            buffs.extend(self.get_global_value_buffers())
        return buffs

#
# Jinja2 auxiliary functions
#

    def kernel_name(self, idx):
        return self.name + '_' + str(idx)

    def function_name(self):
        return self.name + '_function'

    def call_function(self, parameter):
        return self.function_name() + '(' + parameter + (', ' if len(self.get_buffers()) > 0 else '') + self.use_buffers() + ')'

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
    def declare_buffers(self):
        return ''.join([b.declare() + ';\n' for b in self.get_buffers()])

    def declare_private_buffers(self):
        return ''.join([b.declare() + ';\n' for b in self.get_private_buffers()])

    def declare_local_buffers(self):
        return ''.join([b.declare() + ';\n' for b in self.get_local_buffers()])

    def declare_global_buffers(self):
        return ''.join([b.declare() + ';\n' for b in self.get_global_buffers()])

    def parameter_buffers(self):
        return ', '.join(b.parameter() for b in self.get_buffers())

    def parameter_private_buffers(self):
        return ', '.join([b.parameter() for b in self.get_private_buffers()])

    def parameter_local_buffers(self):
        return ', '.join([b.parameter() for b in self.get_local_buffers()])

    def parameter_global_buffers(self):
        return ', '.join([b.parameter() for b in self.get_global_buffers()])

    def parameter_buffers_list(self):
        return [b.parameter() for b in self.get_buffers()]

    def parameter_private_buffers_list(self):
        return [b.parameter() for b in self.get_private_buffers()]

    def parameter_local_buffers_list(self):
        return [b.parameter() for b in self.get_local_buffers()]

    def parameter_global_buffers_list(self):
        return [b.parameter() for b in self.get_global_buffers()]

    def use_buffers(self):
        return ', '.join([b.use() for b in self.get_buffers()])

    def use_private_buffers(self):
        return ', '.join([b.use() for b in self.get_private_buffers()])

    def use_local_buffers(self):
        return ', '.join([b.use() for b in self.get_local_buffers()])

    def use_global_buffers(self):
        return ', '.join([b.use() for b in self.get_global_buffers()])

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
