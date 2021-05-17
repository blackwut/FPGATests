import os, sys
sys.path.insert(0, os.path.join(os.path.pardir, 'FFlow'))

from fpipe_graph import FPipeGraph
from fnode import FNode, FNodeType, FGatheringMode, FDispatchingMode, FMemoryType


p_source = FNode('source', 1,
                 FNodeType.SOURCE,
                 FGatheringMode.NONE,
                 FDispatchingMode.KEYBY)
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

# p_map.add_memory(FMemoryType.GLOBAL, 'data_t', 'table', 1024)
# p_map.add_memory(FMemoryType.PRIVATE, 'uint', 'counter', 1)
# p_map.add_memory(FMemoryType.PRIVATE, 'uint', 'counter', 2)
# p_map.add_memory(FMemoryType.LOCAL, 'uint', 'sum', 2)

graph = FPipeGraph('./sd_source')
graph.add_source(p_source)
graph.add(p_map)
graph.add(p_filter)
graph.add_sink(p_sink)

graph.generate_device()
