{% import 'source.cl' as source with context %}
{% import 'filter.cl' as filter with context %}
{% import 'map.cl' as map with context %}
{% import 'flat_map.cl' as flat_map with context %}
{% import 'sink.cl' as sink with context %}

{%- if node.is_source() %}
{{ source.declare_function(node) }}
{% elif node.is_filter() %}
{{ filter.declare_function(node) }}
{% elif node.is_map() %}
{{ map.declare_function(node) }}
{% elif node.is_flat_map() %}
{{ flat_map.declare_function(node) }}
{% elif node.is_sink() %}
{{ sink.declare_function(node) }}
{% else %}
// Error in creating function
{% endif %}
