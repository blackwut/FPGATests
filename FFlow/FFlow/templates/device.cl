{% import 'channels.cl' as ch with context %}
{% import 'utils.cl' as ut with context %}
{{ ch.enable_channels(channels) }}


{{ ut.decleare_defines() }}

{{ ut.declare_includes(includes) }}

{{ ut.declare_tuples(channels) }}

{{ ch.declare_channels(channels) }}

{{ ut.declare_functions(node_functions) }}
{{ ut.declare_flatmap_functions(nodes) }}

{{ ut.declare_nodes(nodes) }}