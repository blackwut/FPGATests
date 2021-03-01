{% import 'channels.cl' as ch with context %}

{% macro source(node, idx) -%}

CL_SINGLE_TASK {{node.name}}_{{idx}}(__global const {{node.o_channel.data_type}} * restrict data, const uint N)
{
    {% if node.dispatching_mode.value == dispatchingMode.RR_BLOCKING.value %}
    uint w = {{idx}} % {{node.o_degree}};
    {% endif %}

    for (uint n = 0; n < N; ++n) {
        const {{node.o_channel.tuple_type}} tuple_out = create_{{node.o_channel.tuple_type}}(data[n]);

        {% if node.dispatching_mode.value == dispatchingMode.RR_BLOCKING.value %}
        {{ch.switch_write_rr_blocking(node, idx, 'w', 'tuple_out', false)|indent(8)}}
        {% endif %}
        {% if node.dispatching_mode.value == dispatchingMode.RR_NON_BLOCKING.value %}
        uint w = n % {{node.o_degree}};
        {{ch.switch_write_rr_non_blocking(node, idx, 'w', 'tuple_out', false)|indent(8)}}
        {% endif %}
        {% if node.dispatching_mode.value == dispatchingMode.KEYBY.value %}
        const uint w = t.data.key % {{node.o_degree}};
        {{ch.switch_write_keyby(node, idx, 'w', 'tuple_out')|indent(8)}}
        {% endif %}
        {% if node.dispatching_mode.value == dispatchingMode.BROADCAST.value %}
        {{ch.write_broadcast(node, idx, 'tuple_out')|indent(8)}}
        {% endif %}
    }

    {{ch.write_broadcast_EOS(node, idx)|indent(4)}}
}

{%- endmacro %}


{# // Source

// source data_type (in case of reading from global)
// channel_data_type

// generation:
// - from function
// - from global memory (N-Buffering, e.g. double/triple buffering but generalized)

// dispatching mode:
// - RR_BLOCKING
// - RR_NON_BLOCKING
// - KEYBY
// - BROADCAST

#}