{% import 'channel.cl' as ch with context %}

{% macro node(node, idx) -%}

CL_SINGLE_TASK {{ node.kernel_name(idx) }}({{ node.parameter_buffers(bufferKind.GLOBAL) }})
{
{% if node.i_degree > 1 %}
    uint r = {{ idx % node.i_degree }};
{%- endif %}
{% if node.is_dispatch_RR() %}
    uint w = {{ idx % node.o_degree }};
{% endif %}
    uint EOS = 0;
    bool done = false;

{% if node.get_buffers(bufferKind.PRIVATE) | count > 0 %}
{{ node.declare_buffers(bufferKind.PRIVATE) | indent(4, true) }}
{% endif %}
{% if node.get_buffers(bufferKind.LOCAL)|count > 0 %}
{{ node.declare_buffers(bufferKind.LOCAL) | indent(4, true) }}
{% endif %}

    while (!done) {
        {{ node.declare_i_tuple('t_in') }};
        {{ ch.gather_tuple(node, idx, 'r', 't_in', 't_out', process_tuple) | indent(8) }}
    }

    {{ch.write_broadcast_EOS(node, idx)|indent(4)}}
}

{%- endmacro %}


{% macro process_tuple(node, idx, t_in, t_out) -%}
{{ node.create_o_tuple(t_out, node.call_function(t_in + '.data')) }};
{{ ch.dispatch_tuple(node, idx, 'w', t_out, true) }}
{%- endmacro %}


{% macro declare_function(node) -%}
{% set args = [node.i_datatype + ' in']%}
{% set args = args + node.parameter_buffers_list(bufferKind.ALL) %}
inline {{ node.o_datatype }} {{ node.function_name() }}({{ args | join(', ') }})
{
    // apply your map function to 'in' and store result to 'out'
    {{ node.o_datatype }} out;
    return out;
}
{%- endmacro %}
