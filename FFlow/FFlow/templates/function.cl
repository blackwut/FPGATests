{% import 'buffer.cl' as buff with context %}
{% macro declare_function(node, tuple_type_in, tuple_type_out) -%}
{% set args = [] %}
{% set args = [ tuple_type_in + ' in'] %}
{% set args = args + (['uint idx'] if node.node_type.value == nodeType.FLAT_MAP.value else []) %}
{% set args = args + (['uint * w'] if node.node_type.value == nodeType.FLAT_MAP.value and node.dispatching_mode.value in [dispatchingMode.RR_BLOCKING.value, dispatchingMode.RR_NON_BLOCKING.value] else []) %}
inline {{ 'void' if node.node_type.value == nodeType.FLAT_MAP.value else tuple_type_out }} {{ node.name }}_function({{args | join(', ')}} {{-", " if node.buffers|count > 0 else "" }}{{buff.declare_all(node)}})
{%- endmacro %}

{{- declare_function(node, tuple_type_in, tuple_type_out) }}
{
    {% if node.node_type.value == nodeType.FLAT_MAP.value %}
    uint n = 0;
    bool done = false;
    while (!done) {

        data_t out = in;
        send(out);

        if (n < 10) {
            n++;
        } else {
            done = true;
        }
    }
    {% else %}
    {{tuple_type_out}} out = in;
    return out;
    {% endif %}
}
