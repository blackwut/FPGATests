{% import 'channels.cl' as ch with context %}

{% macro process_tuple(node, idx, tuple_var) -%}
{{node.name}}_function(&({{tuple_var}}.data));

{% if node.dispatching_mode.value == dispatchingMode.RR_BLOCKING.value %}
{{ch.switch_write_rr_blocking(node, idx, 'w', 't', true)}}
{% endif %}
{% if node.dispatching_mode.value == dispatchingMode.RR_NON_BLOCKING.value %}
uint w = {{idx}} % {{node.o_degree}};
{{ch.switch_write_rr_non_blocking(node, idx, 'w', 't', true)}}
{% endif %}
{% if node.dispatching_mode.value == dispatchingMode.KEYBY.value %}
const uint w = t.data.key % {{node.o_degree}};
{{ch.switch_write_keyby(node, idx, 'w', 't')}}
{% endif %}
{% if node.dispatching_mode.value == dispatchingMode.BROADCAST.value %}
{{ch.write_broadcast(node, idx, 't')}}
{% endif %}
{%- endmacro %}



{% macro filter(node, idx) -%}

CL_SINGLE_TASK {{node.name}}_{{ idx }}()
{
    uint r = {{idx}} % {{node.i_degree}};
    {% if node.dispatching_mode.value == dispatchingMode.RR_BLOCKING.value %}
    uint w = {{idx}} % {{node.o_degree}};
    {% endif %}
    uint EOS = 0;
    bool done = false;

    while (!done) {
        {{node.i_channel.tuple_type}} t;
        {% if node.gathering_mode.value == gatheringMode.BLOCKING.value %}
        {{ch.switch_read_blocking(node, idx, 'r', 't', true, process_tuple)|indent(8)}}
        {% else %}
        {{ch.switch_read_non_blocking(node, idx, 'r', 't', true, process_tuple)|indent(8)}}
        {% endif %}
    }

    {{ch.write_broadcast_EOS(node, idx)|indent(4)}}
}

{%- endmacro %}
