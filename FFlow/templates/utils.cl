{% import 'channels.cl' as ch with context %}
{% import 'tuple.cl' as tuple with context %}
{% import 'source.cl' as source with context %}
{% import 'sink.cl' as sink with context %}
{% import 'basic_node.cl' as basic_node with context %}

{% macro decleare_defines() -%}
#define CL_AUTORUN                      \
__attribute__((max_global_work_dim(0))) \
__attribute__((autorun))                \
__kernel void

#define CL_SINGLE_TASK                      \
__attribute__((uses_global_work_offset(0))) \
__attribute__((max_global_work_dim(0)))     \
__kernel void
{%- endmacro %}

{% macro declare_includes(includes) -%}
{% for i in includes %}

{% include i ignore missing %}

{% endfor %}
{%- endmacro %}


{% macro declare_tuples(channels) -%}
{% for c in channels %}
{{ tuple.declare_tuple(c) }}
{% endfor %}
{%- endmacro %}


{% macro declare_functions(node_functions) -%}
{% for f in node_functions %}

{% include f %}

{% endfor %}
{%- endmacro %}


{% macro declare_nodes(nodes) -%}
{% for node in nodes %}
{% for idx in range(node.parallelism): %}

{% if node.node_type.value == nodeType.SOURCE.value %}
{{ source.source(node, idx) }}
{% elif node.node_type.value == nodeType.SINK.value %}
{{ sink.sink(node, idx) }}
{% else %}
{{ basic_node.basic_node(node, idx) }}
{% endif %}

{% endfor %}
{% endfor %}
{%- endmacro %}
