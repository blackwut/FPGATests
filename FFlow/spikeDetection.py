from fpipe_graph import FPipeGraph
from fnode import FNode, FNodeType, FGatheringMode, FDispatchingMode

p_source = FNode('source', 1,
                 FNodeType.SOURCE,
                 FGatheringMode.NONE,
                 FDispatchingMode.RR_BLOCKING)
p_map = FNode('map', 3,
              FNodeType.MAP,
              FGatheringMode.NON_BLOCKING,
              FDispatchingMode.KEYBY)
p_filter = FNode('filter', 2,
                 FNodeType.FILTER,
                 FGatheringMode.NON_BLOCKING,
                 FDispatchingMode.RR_NON_BLOCKING)
p_sink = FNode('sink', 1,
               FNodeType.SINK,
               FGatheringMode.NON_BLOCKING,
               FDispatchingMode.NONE)

graph = FPipeGraph('./source')
graph.add_source(p_source)
graph.add(p_map)
graph.add(p_filter)
graph.add_sink(p_sink)

graph.generate_device()
