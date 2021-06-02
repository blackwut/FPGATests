import os
import sys
sys.path.insert(0, os.path.pardir)

from FFlow import *


p_source = FNode('source', 1,
                 FNodeType.SOURCE,
                 FGatheringMode.NONE,
                 FDispatchingMode.RR_NON_BLOCKING)
p_map = FNode('map', 3,
              FNodeType.MAP,
              FGatheringMode.NON_BLOCKING,
              FDispatchingMode.KEYBY)
p_flatmap = FNode('flatmap', 3,
                  FNodeType.FLAT_MAP,
                  FGatheringMode.NON_BLOCKING,
                  FDispatchingMode.RR_BLOCKING)
p_filter = FNode('filter', 2,
                 FNodeType.FILTER,
                 FGatheringMode.NON_BLOCKING,
                 FDispatchingMode.RR_NON_BLOCKING)
p_sink = FNode('sink', 1,
               FNodeType.SINK,
               FGatheringMode.NON_BLOCKING,
               FDispatchingMode.NONE)


# p_map.add_buffer(FBufferType.PRIVATE, 'float', 'avg')
# p_flatmap.add_buffer(FBufferType.GLOBAL, 'data_t', 'table', 1024)

graph = FPipeGraph('memory_source')
graph.add_source(p_source)
graph.add(p_map)
graph.add(p_flatmap)
graph.add(p_filter)
graph.add_sink(p_sink)

graph.generate_tuples(['data_t'])
graph.generate_functions()
graph.generate_device()
graph.generate_host()
