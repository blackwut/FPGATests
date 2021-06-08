import os
import sys
sys.path.insert(0, os.path.pardir)

from FFlow import *


p_source = FNode('source', 1,
                 FNodeKind.SOURCE,
                 FGatherMode.NONE,
                 FDispatchMode.RR_NON_BLOCKING,
                 'datasource_t')
p_map1 = FNode('map1', 2,
               FNodeKind.MAP,
               FGatherMode.NON_BLOCKING,
               FDispatchMode.RR_BLOCKING,
               'datamap1_t')
p_map2 = FNode('map2', 2,
               FNodeKind.MAP,
               FGatherMode.NON_BLOCKING,
               FDispatchMode.RR_NON_BLOCKING,
               'data_t')
p_map3 = FNode('map3', 2,
               FNodeKind.MAP,
               FGatherMode.NON_BLOCKING,
               FDispatchMode.KEYBY,
               'data_t')
p_map4 = FNode('map4', 2,
               FNodeKind.MAP,
               FGatherMode.NON_BLOCKING,
               FDispatchMode.BROADCAST,
               'data_t')
p_sink = FNode('sink', 1,
               FNodeKind.SINK,
               FGatherMode.NON_BLOCKING,
               FDispatchMode.NONE,
               'data_t')

graph = FPipeGraph('dispatch_source', 'data_t')
graph.add_source(p_source)
graph.add(p_map1)
graph.add(p_map2)
graph.add(p_map3)
graph.add(p_map4)
graph.add_sink(p_sink)

graph.finalize()
graph.generate_device()
graph.generate_host()
