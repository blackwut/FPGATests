{% import 'channels.cl' as ch with context %}

{% macro source(node, idx) -%}

CL_SINGLE_TASK {{node.name}}_{{idx}}(__global const {{node.o_channel.data_type}} * restrict data, const uint N)
{
    {% if node.dispatching_mode.value in [dispatchingMode.RR_BLOCKING.value, dispatchingMode.RR_NON_BLOCKING.value] %}
    uint w = {{idx}} % {{node.o_degree}};
    {% endif %}

    for (uint n = 0; n < N; ++n) {
        const {{node.o_channel.tuple_type}} tuple_out = create_{{node.o_channel.tuple_type}}(data[n]);

        {{ ch.dispatch_tuple(node, idx, 'w', 'tuple_out', true)|indent(8) }}
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