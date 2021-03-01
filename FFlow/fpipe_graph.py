import sys
from os import path
from futils import previous_current_next, read_template_file
from fnode import FChannel, FNode, FNodeType, FGatheringMode, FDispatchingMode


class FPipeGraph:

    source_code_dir = ''
    source = None
    internal_nodes = []
    sink = None

    def __init__(self, source_code_dir: str):
        self.source_code_dir = source_code_dir

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

    def generate_device(self, data_type: str = 'data_t'):

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
            filename = n.name + '.c'
            if path.isfile(path.join(self.source_code_dir, filename)):
                node_functions.append(filename)

        # Includes
        includes = ['tuples.c']

        template = read_template_file(self.source_code_dir, 'device.cl')

        result = template.render(nodeType=FNodeType,
                                 dispatchingMode=FDispatchingMode,
                                 gatheringMode=FGatheringMode,
                                 channels=channels,
                                 nodes=nodes,
                                 includes=includes,
                                 node_functions=node_functions)
        print(result)
