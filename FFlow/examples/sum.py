import os
import sys
sys.path.insert(0, os.path.pardir)

from FFlow import *


p_source = FNode('source', 1,
                 FNodeType.SOURCE,
                 FGatheringMode.NONE,
                 FDispatchingMode.KEYBY)
p_map = FNode('map', 3,
              FNodeType.MAP,
              FGatheringMode.NON_BLOCKING,
              FDispatchingMode.RR_NON_BLOCKING)
# p_filter = FNode('filter', 2,
#                  FNodeType.FILTER,
#                  FGatheringMode.NON_BLOCKING,
#                  FDispatchingMode.RR_NON_BLOCKING)
p_sink = FNode('sink', 1,
               FNodeType.SINK,
               FGatheringMode.NON_BLOCKING,
               FDispatchingMode.NONE)

graph = FPipeGraph('sum')
graph.add_source(p_source)
graph.add(p_map)
# graph.add(p_filter)
graph.add_sink(p_sink)

graph.generate_tuples(['data_t'])
graph.generate_functions()
graph.generate_device(rewrite=True)
graph.generate_host(rewrite=True)
