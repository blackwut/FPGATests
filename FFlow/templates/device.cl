{% import 'channels.cl' as ch with context %}
{% import 'utils.cl' as ut with context %}
{{ ch.enable_channels(channels) }}


{{ ut.decleare_defines() }}

{{ ut.declare_includes(includes )}}

{{ ut.declare_tuples(channels) }}

{{ ut.declare_functions(node_functions) }}

{{ ch.declare_channels(channels)}}
{{ ut.declare_nodes(nodes) }}