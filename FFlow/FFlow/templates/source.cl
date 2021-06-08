{% import 'channel.cl' as ch with context %}

{% macro node(node, idx) -%}

CL_SINGLE_TASK {{ node.kernel_name(idx) }}(__global const {{ node.i_datatype }} * restrict data,
{% filter indent(node.kernel_name(idx)|length + 16, true) %}
const uint size,
const uint shutdown)
{% endfilter %}
{
    {% if node.is_dispatch_RR() %}
    uint w = {{ idx % node.o_degree }};
    {% endif %}

    for (uint n = 0; n < size; ++n) {
        {{ node.create_o_tuple('t_out', 'data[n]') | indent(8) }};

        {{ ch.dispatch_tuple(node, idx, 'w', 't_out', true) | indent(8) }}
    }

    if (shutdown == 1) {
        {{ ch.write_broadcast_EOS(node, idx) | indent(8) }}
    }
}

{%- endmacro %}
