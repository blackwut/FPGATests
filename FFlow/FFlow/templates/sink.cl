{% import 'channel.cl' as ch with context %}


{% macro process_tuple(node, idx, tuple_var_in, tuple_var_out) -%}
    data[n] = {{ tuple_var_in }}.data;
    n++;

    if (n == N) {
        filled = true;
    }
{%- endmacro %}



{% macro sink(node, idx) -%}

CL_SINGLE_TASK {{node.name}}_{{idx}}(__global data_t * restrict data, __global uint * restrict received, __global uint * restrict shutdown, const uint last_EOS, const uint N)
{
    uint r = {{idx}} % {{node.i_degree}};
    uint EOS = last_EOS;
    bool done = false;
    uint n = 0;
    bool filled = false;


    while (!filled && !done) {
        {{node.i_channel.tuple_type}} tuple_in;
        {% if node.gathering_mode.value == gatheringMode.BLOCKING.value %}
        {{ch.switch_read_blocking(node, idx, 'r', 'tuple_in', '', true, process_tuple)|indent(8)}}
        {% else %}
        {{ch.switch_read_non_blocking(node, idx, 'r', 'tuple_in', '', true, process_tuple)|indent(8)}}
        {% endif %}
    }

    *received = n;
    *shutdown = EOS;
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