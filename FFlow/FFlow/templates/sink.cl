{% import 'channels.cl' as ch with context %}


{% macro process_tuple(node, idx, tuple_var_in, tuple_var_out) -%}
{%- endmacro %}



{% macro sink(node, idx) -%}

CL_SINGLE_TASK {{node.name}}_{{idx}}()
{
    uint r = {{idx}} % {{node.i_degree}};
    uint EOS = 0;
    bool done = false;

    while (!done) {
        {{node.i_channel.tuple_type}} tuple_in;
        {% if node.gathering_mode.value == gatheringMode.BLOCKING.value %}
        {{ch.switch_read_blocking(node, idx, 'r', 'tuple_in', '', true, process_tuple)|indent(8)}}
        {% else %}
        {{ch.switch_read_non_blocking(node, idx, 'r', 'tuple_in', '', true, process_tuple)|indent(8)}}
        {% endif %}
    }
}

{%- endmacro %}

{# // Sink
// channel_data_type
// sink data_type (in case of store to global)

// gathering:
// - blocking
// - non_blocking

// sink_function
// - none
// - count
// - store to global memory
// - wait up to N tuples, apply function and then store to global memory the result
#}