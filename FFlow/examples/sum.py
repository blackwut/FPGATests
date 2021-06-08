import os
import sys
sys.path.insert(0, os.path.pardir)

from FFlow import *


p_source = FNode('source', 1,
                 FNodeKind.SOURCE,
                 FGatherMode.NONE,
                 FDispatchMode.KEYBY,
                 'data_t')
p_map = FNode('map', 3,
              FNodeKind.MAP,
              FGatherMode.NON_BLOCKING,
              FDispatchMode.RR_NON_BLOCKING,
              'data_t')
p_filter = FNode('filter', 2,
                 FNodeKind.FILTER,
                 FGatherMode.NON_BLOCKING,
                 FDispatchMode.RR_NON_BLOCKING,
                 'data_t')
p_sink = FNode('sink', 1,
               FNodeKind.SINK,
               FGatherMode.NON_BLOCKING,
               FDispatchMode.NONE,
               'data_t')

graph = FPipeGraph('sum', 'data_t')
graph.add_source(p_source)
graph.add(p_map)
graph.add(p_filter)
graph.add_sink(p_sink)

graph.finalize()
graph.generate_device()
graph.generate_host()
