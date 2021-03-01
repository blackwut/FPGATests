{% macro enable_channels(channels) -%}
{% if channels|length > 0 -%}
#pragma OPENCL EXTENSION cl_intel_channels : enable
{%- endif %}
{%- endmacro %}

// Basic Definitions

{% macro channel(c, i, j) -%}
{{c.name}}{% if c.i > 1 %}[{{i}}]{% endif %}{% if c.j > 1 %}[{{j}}]{% endif %}
{%- endmacro %}


{% macro declare_channels(channels) -%}
{% for c in channels %}
channel {{c.tuple_type}} {{channel(c, c.i, c.j)}} __attribute__((depth({{c.depth}})));
{% endfor %}
{%- endmacro %}


{% macro incr_var(var, max) -%}
if ({{var}} < {{max}}) {
    {{var}}++;
} else {
    {{var}} = 0;
}
{%- endmacro %}


// Gathering Methods

{% macro switch_read_blocking(node, idx, switch_var, tuple_var_in, tuple_var_out, incr, process_tuple) -%}
switch ({{switch_var}}) {
{% for i in range(node.i_degree): %}
    case {{i}}: {{tuple_var_in}} = read_channel_intel({{channel(node.i_channel, i, idx)}}); break;
{% endfor %}
}

{% if incr %}
{{incr_var(switch_var, node.i_degree)}}
{% endif %}

{{process_tuple(node, idx, tuple_var_in, tuple_var_out)}}
{%- endmacro %}



{% macro switch_read_non_blocking(node, idx, switch_var, tuple_var_in, tuple_var_out, incr, process_tuple) -%}
bool valid = false;
switch ({{switch_var}}) {
{% for i in range(node.i_degree): %}
    case {{i}}: {{tuple_var_in}} = read_channel_nb_intel({{channel(node.i_channel, i, idx)}}, &valid); break;
{% endfor %}
}

{% if incr %}
{{incr_var(switch_var, node.i_degree)}}
{% endif %}

if (valid) {
    if ({{tuple_var_in}}.EOS) {
        done = (++EOS == {{node.i_degree}});
    } else {
        {{process_tuple(node, idx, tuple_var_in, tuple_var_out)|indent(8)}}
    }
}
{%- endmacro %}




// Dispatching Methods

{% macro switch_write_rr_blocking(node, idx, switch_var, tuple_var_out, incr) -%}

switch ({{switch_var}}) {
    {% for i in range(node.o_degree): %}
    case {{i}}: write_channel_intel({{channel(node.o_channel, idx, i)}}, {{tuple_var_out}}); break;
    {% endfor %}
}

{% if incr %}
{{incr_var(switch_var, node.o_degree)}}
{% endif %}
{%- endmacro %}



{% macro switch_write_rr_non_blocking(node, idx, switch_var, tuple_var_out, incr) -%}
bool success = false;
do {
    switch ({{switch_var}}) {
    {% for i in range(node.o_degree): %}
        case {{i}}: if (!success) success = write_channel_nb_intel({{channel(node.o_channel, idx, i)}}, {{tuple_var_out}}); break;
    {% endfor %}
    }

    {% if incr %}
    {{incr_var(switch_var, node.o_degree)|indent(4)}}
    {% endif %}

} while (!success);
{%- endmacro %}



{% macro switch_write_keyby(node, idx, switch_var, tuple_var_out) -%}
switch ({{switch_var}}) {
{% for i in range(node.o_degree): %}
    case {{i}}: write_channel_intel({{channel(node.o_channel, idx, i)}}, {{tuple_var_out}}); break;
{% endfor %}
}
{%- endmacro %}


{% macro write_broadcast(node, idx, tuple_var_out) -%}
{{'#pragma unroll'}}
for (uint i = 0; i < {{node.o_degree}}; ++i) {
    write_channel_intel({{channel(node.o_channel, idx, 'i')}}, {{tuple_var_out}});
}
{%- endmacro %}


{% macro write_broadcast_EOS(node, idx) -%}
const {{node.o_channel.tuple_type}} tuple_eos = create_{{node.o_channel.tuple_type}}_EOS();
{{'#pragma unroll'}}
for (uint i = 0; i < {{node.o_degree}}; ++i) {
    write_channel_intel({{channel(node.o_channel, idx, 'i')}}, tuple_eos);
}
{%- endmacro %}
