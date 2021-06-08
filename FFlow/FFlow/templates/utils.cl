{% import 'channel.cl' as ch with context %}
{% import 'tuple.cl' as tuple with context %}
{% import 'source.cl' as source with context %}
{% import 'filter.cl' as filter with context %}
{% import 'map.cl' as map with context %}
{% import 'flat_map.cl' as flat_map with context %}
{% import 'sink.cl' as sink with context %}

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

{% include f with context %}

{% endfor %}
{%- endmacro %}

{% macro declare_flatmap_functions(nodes) -%}
{% for node in nodes %}
{% if node.is_flat_map() %}
{% include node.flat_map with context %}
{% endif %}
{% endfor %}
{%- endmacro %}


{% macro declare_nodes(nodes) -%}
{% for node in nodes %}
{% for idx in range(node.par) %}
{% if node.is_source() %}
{{ source.node(node, idx) }}
{% elif node.is_filter() %}
{{ filter.node(node, idx) }}
{% elif node.is_map() %}
{{ map.node(node, idx) }}
{% elif node.is_flat_map() %}
{{ flat_map.node(node, idx) }}
{% elif node.is_sink() %}
{{ sink.node(node, idx) }}
{% else %}
{% endif %}

{% endfor %}
{% endfor %}
{%- endmacro %}
