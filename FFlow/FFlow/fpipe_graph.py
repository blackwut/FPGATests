import sys
from os import path
from shutil import copyfile
from .futils import *
from .fnode import *
from .fchannel import *


def generate_new_code_flat_map(filename):

    file = open(filename, mode='r')
    content = file.read()
    file.close()

    new_filename = filename + '.tmp'

    with open(new_filename, 'w+') as f:
        new_code = generate_flat_map_code(content)
        f.write(new_code)

    return new_filename


class FPipeGraph:

    def __init__(self, source_code_dir: str):
        self.source_code_dir = source_code_dir
        self.source = None
        self.internal_nodes = []
        self.sink = None

        self.prepare_folders()

    def add_source(self, source: FNode):
        if source.node_type == FNodeType.SOURCE:
            self.source = source
        else:
            sys.exit("Supplied node is not of type SOURCE")

    def add_sink(self, sink: FNode):
        if sink.node_type == FNodeType.SINK:
            self.sink = sink
        else:
            sys.exit("Supplied node is not of type SINK")

    def add(self, node: FNode):
        if node.node_type not in [FNodeType.SOURCE, FNodeType.SINK]:
            self.internal_nodes.append(node)
        else:
            sys.exit("Supplied node is not of type SOURCE")

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

        folders = [self.base_dir, self.common_dir, self.includes_dir, self.host_dir, self.device_dir, self.nodes_dir]

        for folder in folders:
            if not path.isdir(folder):
                os.mkdir(folder)

    def generate_device(self, data_type: str = 'data_t', rewrite=False):
        nodes = [self.source]
        nodes.extend(self.internal_nodes)
        nodes.append(self.sink)

        # Updates input and output degree
        for prv, cur, nxt in previous_current_next(nodes):
            cur.i_degree = prv.parallelism if prv is not None else 0
            cur.o_degree = nxt.parallelism if nxt is not None else 0

        # Creates channels
        channels = []
        for prv, cur, nxt in previous_current_next(nodes):
            if prv:
                c = FChannel(data_type,
                             prv.name + '_' + cur.name,
                             16,
                             prv.parallelism,
                             cur.parallelism)
                prv.o_channel = c
                cur.i_channel = c
                channels.append(c)

        # Creates functions
        node_functions = []
        for n in nodes:
            filename = path.join(self.nodes_dir, n.name + '.c')
            if path.isfile(filename):
                if n.node_type == FNodeType.FLAT_MAP:
                    n.flat_map = generate_new_code_flat_map(filename)
                else:
                    node_functions.append(filename)

        # Includes
        includes = [path.join(self.includes_dir, 'tuples.h')]

        template = read_template_file('.', 'device.cl')

        result = template.render(nodeType=FNodeType,
                                 dispatchingMode=FDispatchingMode,
                                 gatheringMode=FGatheringMode,
                                 bufferType=FBufferType,
                                 channels=channels,
                                 nodes=nodes,
                                 includes=includes,
                                 node_functions=node_functions)
        for n in nodes:
            if n.node_type == FNodeType.FLAT_MAP:
                if path.isfile(n.flat_map):
                    os.remove(n.flat_map)

        filename = path.join(self.device_dir, 'device.cl')
        if not path.isfile(filename) or rewrite:
            file = open(filename, mode='w')
            file.write(result)
            file.close()

    def generate_tuples(self, tuples=[]):
        filename = path.join(self.includes_dir, 'tuples.h')
        if not path.isfile(filename):
            result = ''
            for t in tuples:
                result += ("typedef struct {\n"
                           "    uint key;\n"
                           "    float value;\n"
                           "} " + t + ";")
            file = open(filename, mode='x')
            file.write(result)
            file.close()

    def generate_functions(self):
        nodes = self.internal_nodes

        template = read_template_file(self.source_code_dir, 'function.cl')

        for n in nodes:
            filename = path.join(self.nodes_dir, n.name + '.c')
            if not path.isfile(filename):
                file = open(filename, mode='x')
                result = template.render(node=n,
                                         tuple_type_in='data_t',
                                         tuple_type_out='data_t',
                                         nodeType=FNodeType,
                                         dispatchingMode=FDispatchingMode,
                                         bufferType=FBufferType)
                file.write(result)
                file.close()

    def generate_host(self, rewrite=False):
        template = read_template_file(self.source_code_dir, 'host.cpp')
        filename = path.join(self.host_dir, 'host.cpp')
        if not path.isfile(filename) or rewrite:
            file = open(filename, mode='w')
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
