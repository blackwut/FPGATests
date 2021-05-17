import os, sys
sys.path.insert(0, os.path.join(os.path.pardir, 'FFlow'))

from fpipe_graph import FPipeGraph
from fnode import FNode, FNodeType, FGatheringMode, FDispatchingMode


p_source = FNode('source', 1,
                 FNodeType.SOURCE,
                 FGatheringMode.NONE,
                 FDispatchingMode.RR_NON_BLOCKING)
p_map1 = FNode('map1', 2,
               FNodeType.MAP,
               FGatheringMode.NON_BLOCKING,
               FDispatchingMode.RR_BLOCKING)
p_map2 = FNode('map2', 2,
               FNodeType.MAP,
               FGatheringMode.NON_BLOCKING,
               FDispatchingMode.RR_NON_BLOCKING)
p_map3 = FNode('map3', 2,
               FNodeType.MAP,
               FGatheringMode.NON_BLOCKING,
               FDispatchingMode.KEYBY)
p_map4 = FNode('map4', 2,
               FNodeType.MAP,
               FGatheringMode.NON_BLOCKING,
               FDispatchingMode.BROADCAST)
p_sink = FNode('sink', 1,
               FNodeType.SINK,
               FGatheringMode.NON_BLOCKING,
               FDispatchingMode.NONE)

graph = FPipeGraph('dispatch_source')
graph.add_source(p_source)
graph.add(p_map1)
graph.add(p_map2)
graph.add(p_map3)
graph.add(p_map4)
graph.add_sink(p_sink)

graph.generate_device()
