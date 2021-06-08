import os
import sys
sys.path.insert(0, os.path.pardir)

from FFlow import *


p_source = FNode('source', 1,
                 FNodeKind.SOURCE,
                 FGatherMode.NONE,
                 FDispatchMode.RR_NON_BLOCKING,
                 'data_t')
p_flat_map = FNode('flatmap', 3,
                   FNodeKind.FLAT_MAP,
                   FGatherMode.BLOCKING,
                   FDispatchMode.RR_NON_BLOCKING,
                   'data_t')
p_counter = FNode('counter', 2,
                  FNodeKind.MAP,
                  FGatherMode.NON_BLOCKING,
                  FDispatchMode.RR_NON_BLOCKING,
                  'data_t')
p_sink = FNode('sink', 1,
               FNodeKind.SINK,
               FGatherMode.NON_BLOCKING,
               FDispatchMode.NONE,
               'data_t')

graph = FPipeGraph('wc_source', 'data_t')
graph.add_source(p_source)
graph.add(p_flat_map)
graph.add(p_counter)
graph.add_sink(p_sink)

graph.finalize()
graph.generate_device()
graph.generate_host()
