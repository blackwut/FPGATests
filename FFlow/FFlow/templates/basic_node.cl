{% import 'channels.cl' as ch with context %}
{% import 'memory.cl' as mem with context %}

{% macro process_tuple(node, idx, tuple_var_in, tuple_var_out) -%}
{% if node.node_type.value == nodeType.FLAT_MAP.value %}
{% if node.dispatching_mode.value in [dispatchingMode.RR_BLOCKING.value, dispatchingMode.RR_NON_BLOCKING.value] %}
{{node.name}}_function({{tuple_var_in}}.data, idx, &w);
{% else %}
{{node.name}}_function({{tuple_var_in}}.data, idx);
{%- endif %}
{% else %}
{{node.o_channel.tuple_type}} {{tuple_var_out}} = create_{{node.o_channel.tuple_type}}({{node.name}}_function({{tuple_var_in}}.data));

{{ ch.dispatch_tuple(node, idx, 'w', 'tuple_out', true) }}
{%- endif %}
{%- endmacro %}


{% macro basic_node(node, idx) -%}

CL_SINGLE_TASK {{node.name}}_{{ idx }}({{ mem.declare_global_memories(node) }})
{
    const uint idx = {{idx}};
{% if node.i_degree > 1 %}
    uint r = {{idx}} % {{node.i_degree}};
{% endif %}
{% if node.dispatching_mode.value in [dispatchingMode.RR_BLOCKING.value, dispatchingMode.RR_NON_BLOCKING.value] %}
    uint w = {{idx}} % {{node.o_degree}};
{% endif %}
    uint EOS = 0;
    bool done = false;

{{- mem.declare_local_memories(node)|indent(4) }}
{{- mem.declare_private_memories(node)|indent(4) }}

    while (!done) {
        {{node.i_channel.tuple_type}} tuple_in;
        {% if node.gathering_mode.value == gatheringMode.BLOCKING.value %}
        {% if node.i_degree > 1 %}
        {{ ch.switch_read_blocking(node, idx, 'r', 'tuple_in', 'tuple_out', true, process_tuple)|indent(8) }}
        {% else %}
        {{ ch.single_read_blocking(node, idx, 'tuple_in', 'tuple_out', process_tuple)|indent(8) }}
        {% endif %}
        {% else %}
        {% if node.i_degree > 1 %}
        {{ ch.switch_read_non_blocking(node, idx, 'r', 'tuple_in', 'tuple_out', true, process_tuple)|indent(8) }}
        {% else %}
        {{ ch.single_read_non_blocking(node, idx, 'tuple_in', 'tuple_out', process_tuple)|indent(8) }}
        {% endif %}
        {% endif %}
    }

    {{ch.write_broadcast_EOS(node, idx)|indent(4)}}
}

{%- endmacro %}

{#
// RR_BLOCKING
// header
w

// inside while
channel
tuple_var_out
incr


// RR_NON_BLOCKING
// header
w

// inside while
channel
tuple_var_out
incr


// KEYBY
// header

// inside while
w
channel
tuple_var_out

// BROADCAST
// header

// inside while
channel
tuple_var_out
#}