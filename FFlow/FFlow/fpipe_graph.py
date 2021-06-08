import sys
from os import path
from shutil import copyfile
from .futils import *
from .fnode import *
from .fchannel import *


class FPipeGraph:

    def __init__(self,
                 source_code_dir: str,
                 datatype: str,
                 channel_depth: int = 16):
        assert source_code_dir
        assert datatype
        assert channel_depth > 0

        self.source_code_dir = source_code_dir
        self.datatype = datatype
        self.channel_depth = channel_depth

        self.source = None
        self.internal_nodes = []
        self.sink = None

    def prepare_folders(self):
        """
        app
        ├── common
        ├── device
        │   └── nodes
        ├── host
        ├── includes
        └── sources
        """
        self.base_dir = self.source_code_dir
        self.common_dir = path.join(self.source_code_dir, 'common')
        self.includes_dir = path.join(self.source_code_dir, 'includes')
        self.host_dir = path.join(self.source_code_dir, 'host')
        self.device_dir = path.join(self.source_code_dir, 'device')
        self.nodes_dir = path.join(self.device_dir, 'nodes')

        for folder in (self.base_dir, self.common_dir, self.includes_dir, self.host_dir, self.device_dir, self.nodes_dir):
            if not path.isdir(folder):
                os.mkdir(folder)

    def get_nodes(self):
        nodes = [self.source]
        nodes.extend(self.internal_nodes)
        nodes.append(self.sink)
        return nodes

#
# User's functions
#
    def add_source(self,
                   source: FNode):
        assert source
        assert not self.source

        if source.kind == FNodeKind.SOURCE:
            self.source = source
        else:
            sys.exit("Supplied node is not of type SOURCE")

    def add_sink(self,
                 sink: FNode):
        assert sink
        assert not self.sink

        if sink.kind == FNodeKind.SINK:
            self.sink = sink
        else:
            sys.exit("Supplied node is not of type SINK")

    def add(self,
            node: FNode):
        assert node

        if node.kind not in (FNodeKind.SOURCE, FNodeKind.SINK):
            self.internal_nodes.append(node)
        else:
            sys.exit("Supplied node is not of type MAP, FILTER or FLAT_MAP")

    def finalize(self):
        nodes = self.get_nodes()

        # Checks duplicate names
        names = set()
        for n in nodes:
            if n.name in names:
                sys.exit("Node's name '" + n.name + " already taken!")
            else:
                names.add(n.name)

        # Updates input and output degree
        for prv, cur, nxt in previous_current_next(nodes):
            cur.i_degree = prv.par if prv is not None else 0
            cur.o_degree = nxt.par if nxt is not None else 0

        # Updates input datatype
        for prv, cur, nxt in previous_current_next(nodes):
            cur.i_datatype = prv.o_datatype if prv is not None else self.datatype

        # Creates channels
        self.channels = []
        for prv, cur, nxt in previous_current_next(nodes):
            if prv:
                c = FChannel(prv, cur, self.channel_depth)
                prv.o_channel = c
                cur.i_channel = c
                self.channels.append(c)

        # Creates folders
        self.prepare_folders()

    def generate_tuples(self,
                        rewrite=False):
        filename = path.join(self.includes_dir, 'tuples.h')
        if path.isfile(filename) and not rewrite:
            return

        nodes = [self.source]
        nodes.extend(self.internal_nodes)

        # Gathers all unique datatype
        tuples = set([self.datatype])
        for n in nodes:
            tuples.add(n.o_datatype)

        # Generates all unique datatype
        result = ''
        for t in tuples:
            result += ("typedef struct {\n"
                       "    uint key;\n"
                       "    float value;\n"
                       "} " + t + ";\n"
                       "\n"
                       "uint " + t + "_getKey(" + t + " data) {\n"
                       "    return data.key;\n"
                       "}\n"
                       "\n")
        file = open(filename, mode='w+')
        file.write(result)
        file.close()

    def generate_functions(self,
                           rewrite=False):
        template = read_template_file(self.source_code_dir, 'function.cl')
        for n in self.internal_nodes:
            filename = path.join(self.nodes_dir, n.name + '.c')
            if not path.isfile(filename) or rewrite:
                file = open(filename, mode='w+')
                result = template.render(node=n,
                                         nodeKind=FNodeKind,
                                         dispatchMode=FDispatchMode,
                                         bufferKind=FBufferKind)
                file.write(result)
                file.close()

    def generate_device(self,
                        rewrite=False):
        self.generate_tuples(rewrite)
        self.generate_functions(rewrite)

        nodes = self.get_nodes()
        includes = [path.join(self.includes_dir, 'tuples.h')]

        # Creates functions
        node_functions = []
        for n in nodes:
            filename = path.join(self.nodes_dir, n.name + '.c')
            if path.isfile(filename):
                if n.is_flat_map():
                    n.flat_map = generate_flat_map_code(n.name, filename)
                else:
                    node_functions.append(filename)

        template = read_template_file('.', 'device.cl')
        result = template.render(nodeKind=FNodeKind,
                                 gatherKind=FGatherMode,
                                 dispatchKind=FDispatchMode,
                                 bufferKind=FBufferKind,
                                 nodes=nodes,
                                 channels=self.channels,
                                 includes=includes,
                                 node_functions=node_functions)

        filename = path.join(self.device_dir, 'device.cl')
        if not path.isfile(filename) or rewrite:
            file = open(filename, mode='w+')
            file.write(result)
            file.close()

        # Removes 'flat_map' temporary files
        for n in nodes:
            if n.kind == FNodeKind.FLAT_MAP:
                if path.isfile(n.flat_map):
                    os.remove(n.flat_map)

    def generate_host(self, rewrite=False):
        template = read_template_file(self.source_code_dir, 'host.cpp')
        filename = path.join(self.host_dir, 'host.cpp')
        if not path.isfile(filename) or rewrite:
            file = open(filename, mode='w+')
            result = template.render(nodes=self.internal_nodes,
                                     source=self.source,
                                     sink=self.sink)
            file.write(result)
            file.close()

        common_dir = os.path.join(os.path.dirname(__file__), "src", 'common')
        files = ['buffers.hpp', 'common.hpp', 'ocl.hpp', 'opencl.hpp', 'utils.hpp']

        for f in files:
            src_path = path.join(common_dir, f)
            dest_path = path.join(self.common_dir, f)
            if not path.isfile(dest_path):
                copyfile(src_path, dest_path)

        # Makefile
        make_src_dir = os.path.join(os.path.dirname(__file__), "src", 'Makefile')
        make_dst_dir = path.join(self.base_dir, 'Makefile')

        if not path.isfile(make_dst_dir):
            copyfile(make_src_dir, make_dst_dir)
