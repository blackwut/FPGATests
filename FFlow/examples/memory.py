import os
import sys
sys.path.insert(0, os.path.pardir)

from FFlow import *


p_source = FNode('source', 1,
                 FNodeKind.SOURCE,
                 FGatherMode.NONE,
                 FDispatchMode.RR_NON_BLOCKING,
                 'data_t')
p_map = FNode('map', 2,
              FNodeKind.MAP,
              FGatherMode.NON_BLOCKING,
              FDispatchMode.KEYBY,
              'data_t')
p_flatmap = FNode('flatmap', 2,
                  FNodeKind.FLAT_MAP,
                  FGatherMode.NON_BLOCKING,
                  FDispatchMode.RR_BLOCKING,
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


p_map.add_private_buffer('float', 'avg', ptr=True)
p_map.add_private_buffer('float', 'avg2', ptr=False)
p_map.add_global_buffer('float', 'test', 512, access=FBufferAccess.RW)
p_flatmap.add_global_buffer('data_t', 'table', 1024)
p_filter.add_local_buffer('int', 'test', 128)

graph = FPipeGraph('memory_source', 'data_t')
graph.add_source(p_source)
graph.add(p_map)
graph.add(p_flatmap)
graph.add(p_filter)
graph.add_sink(p_sink)

graph.finalize()
graph.generate_device(rewrite=True)
graph.generate_host(rewrite=True)
