import os
import sys
sys.path.insert(0, os.path.pardir)

from FFlow import *


p_source = FNode('source', 1,
                 FNodeType.SOURCE,
                 FGatheringMode.NONE,
                 FDispatchingMode.RR_NON_BLOCKING)
p_flat_map = FNode('flatmap', 3,
                   FNodeType.FLAT_MAP,
                   FGatheringMode.BLOCKING,
                   FDispatchingMode.RR_NON_BLOCKING)
p_counter = FNode('counter', 2,
                  FNodeType.MAP,
                  FGatheringMode.NON_BLOCKING,
                  FDispatchingMode.RR_NON_BLOCKING)
p_sink = FNode('sink', 1,
               FNodeType.SINK,
               FGatheringMode.NON_BLOCKING,
               FDispatchingMode.NONE)

graph = FPipeGraph('./wc_source')
graph.add_source(p_source)
graph.add(p_flat_map)
graph.add(p_counter)
graph.add_sink(p_sink)

graph.generate_device()