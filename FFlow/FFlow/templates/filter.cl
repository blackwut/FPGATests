{% import 'channel.cl' as ch with context %}

{% macro node(node, idx) -%}

CL_SINGLE_TASK {{ node.kernel_name(idx) }}({{ node.parameter_global_buffers() }})
{
{% if node.i_degree > 1 %}
    uint r = {{ idx % node.i_degree }};
{%- endif %}
{% if node.is_dispatch_RR() %}
    uint w = {{ idx % node.o_degree }};
{% endif %}
    uint EOS = 0;
    bool done = false;

{% if node.get_private_buffers() | count > 0 %}
{{ node.declare_private_buffers() | indent(4, true) }}
{% endif %}
{% if node.get_local_buffers() | count > 0 %}
{{ node.declare_local_buffers() | indent(4, true) }}
{% endif %}

    while (!done) {
        {{ node.declare_i_tuple('t_in') }};
        {{ ch.gather_tuple(node, idx, 'r', 't_in', 't_out', process_tuple) | indent(8) }}
    }

    {{ch.write_broadcast_EOS(node, idx)|indent(4)}}
}

{%- endmacro %}


{% macro process_tuple(node, idx, t_in, t_out) -%}
if ({{ node.call_function(t_in + '.data') }}) {
    {{ node.create_o_tuple(t_out, t_in + '.data') }};
    {{ ch.dispatch_tuple(node, idx, 'w', t_out, true) }}
}
{%- endmacro %}


{% macro declare_function(node) -%}
{% set args = [node.i_datatype + ' in']%}
{% set args = args + node.parameter_buffers_list() %}
inline bool {{ node.function_name() }}({{ args | join(', ') }})
{
    return (in.value >= 0.0f);
}
{%- endmacro %}