{% macro enable_channels(channels) -%}
{% if channels|length > 0 -%}
#pragma OPENCL EXTENSION cl_intel_channels : enable
{%- endif %}
{%- endmacro %}

// Basic Definitions
{% macro declare_channels(channels) -%}
{% for c in channels %}
{{ c.declare() }};
{% endfor %}
{%- endmacro %}


{% macro incr_var(var, max) -%}
if ({{var}} == {{max - 1}}) {
    {{var}} = 0;
} else {
    {{var}} += 1;
}
{%- endmacro %}


// -----------------------------------------------------------------------------
//
// Gather template functions
//
// -----------------------------------------------------------------------------

{% macro single_read_blocking(node, idx, t_in, t_out, process_tuple) -%}
{{ t_in }} = {{ node.read(i, idx) }};

if ({{ t_in }}.EOS) {
    done = true;
} else {
    {{ process_tuple(node, idx, t_in, t_out) | indent(4) }}
}
{%- endmacro %}



{% macro switch_read_blocking(node, idx, switch_var, t_in, t_out, incr, process_tuple) -%}
switch ({{ switch_var }}) {
{% for i in range(node.i_degree): %}
    case {{i}}: {{ t_in }} = {{ node.read(i, idx) }}; break;
{% endfor %}
}

{% if incr %}
{{ incr_var(switch_var, node.i_degree) }}
{% endif %}

if ({{ t_in }}.EOS) {
    done = true;
} else {
    {{ process_tuple(node, idx, t_in, t_out) | indent(4) }}
}
{%- endmacro %}


// UNUSED
{% macro single_read_non_blocking(node, idx, t_in, t_out, process_tuple) -%}
bool valid = false;

{{ t_in }} = {{ node.read_nb(i, idx, 'valid') }};

if (valid) {
    if ({{ t_in }}.EOS) {
        done = true;
    } else {
        {{ process_tuple(node, idx, t_in, t_out) | indent(8) }}
    }
}
{%- endmacro %}


{% macro switch_read_non_blocking(node, idx, switch_var, t_in, t_out, incr, process_tuple) -%}
bool valid = false;

switch ({{ switch_var }}) {
{% for i in range(node.i_degree): %}
    case {{i}}: {{ t_in }} = {{ node.read_nb(i, idx, 'valid') }}; break;
{% endfor %}
}

{% if incr %}
{{ incr_var(switch_var, node.i_degree) }}
{% endif %}

if (valid) {
    if ({{ t_in }}.EOS) {
        done = (++EOS == {{ node.i_degree }});
    } else {
        {{ process_tuple(node, idx, t_in, t_out) | indent(8) }}
    }
}
{%- endmacro %}


{% macro gather_tuple(node, idx, r, t_in, t_out, process_tuple) -%}
{% if node.gather_mode == gatherKind.BLOCKING %}
{% if node.i_degree > 1 %}
{{ switch_read_blocking(node, idx, r, t_in, t_out, true, process_tuple) }}
{% else %}
{{ single_read_blocking(node, idx, t_in, t_out, process_tuple) }}
{% endif %}
{% else %}
{% if node.i_degree > 1 %}
{{ switch_read_non_blocking(node, idx, r, t_in, t_out, true, process_tuple) }}
{% else %}
{{ single_read_blocking(node, idx, t_in, t_out, process_tuple) }}
{% endif %}
{% endif %}
{%- endmacro %}


// -----------------------------------------------------------------------------
//
// Dispatch template functions
//
// -----------------------------------------------------------------------------

{% macro single_write_rr_blocking(node, idx, t_out) -%}
{{ node.write(idx, None, t_out) }};
{%- endmacro %}



{% macro switch_write_rr_blocking(node, idx, switch_var, t_out, incr) -%}
switch ({{ switch_var }}) {
    {% for i in range(node.o_degree): %}
    case {{i}}: {{ node.write(idx, i, t_out) }}; break;
    {% endfor %}
}

{% if incr %}
{{ incr_var(switch_var, node.o_degree) }}
{% endif %}
{%- endmacro %}



{% macro single_write_rr_non_blocking(node, idx, t_out) -%}
{{ single_write_rr_blocking(node, idx, t_out) }}
{%- endmacro %}



{% macro switch_write_rr_non_blocking(node, idx, switch_var, t_out, incr) -%}
bool success = false;
do {
    switch ({{switch_var}}) {
    {% for i in range(node.o_degree): %}
        case {{i}}: if (!success) success = {{ node.write_nb(idx, i, t_out) }}; break;
    {% endfor %}
    }

    {% if incr %}
    {{ incr_var(switch_var, node.o_degree) | indent(4) }}
    {% endif %}

} while (!success);
{%- endmacro %}



{% macro single_write_keyby(node, idx, t_out) -%}
{{ single_write_rr_blocking(node, idx, t_out) }}
{%- endmacro %}



{% macro switch_write_keyby(node, idx, t_out) -%}
const uint w = {{ node.o_datatype }}_getKey({{ t_out }}.data) % {{ node.o_degree }};
switch (w) {
{% for i in range(node.o_degree): %}
    case {{i}}: {{ node.write(idx, i, t_out) }}; break;
{% endfor %}
}
{%- endmacro %}


{% macro write_broadcast(node, idx, t_out) -%}
#pragma unroll
for (uint i = 0; i < {{ node.o_degree }}; ++i) {
    {{ node.write(idx, 'i', t_out) }};
}
{%- endmacro %}


{% macro write_broadcast_EOS(node, idx) -%}
const {{ node.o_channel.tupletype }} tuple_eos = create_{{ node.o_channel.tupletype }}_EOS();
{{ write_broadcast(node, idx, 'tuple_eos') }}
{%- endmacro %}



{% macro dispatch_tuple(node, idx, switch_var, t_out, incr) -%}
{% if node.dispatch_mode == dispatchKind.RR_BLOCKING %}
{% if node.o_degree > 1 %}
{{ switch_write_rr_blocking(node, idx, switch_var, t_out, incr) }}
{% else %}
{{ single_write_rr_blocking(node, idx, t_out) }}
{% endif %}
{% endif %}
{% if node.dispatch_mode == dispatchKind.RR_NON_BLOCKING %}
{% if node.o_degree > 1 %}
{{ switch_write_rr_non_blocking(node, idx, switch_var, t_out, incr) }}
{% else %}
{{ single_write_rr_non_blocking(node, idx, t_out) }}
{% endif %}
{% endif %}
{% if node.dispatch_mode == dispatchKind.KEYBY %}
{% if node.o_degree > 1 %}
{{ switch_write_keyby(node, idx, t_out) }}
{% else %}
{{ single_write_keyby(node, idx, t_out) }}
{% endif %}
{% endif %}
{% if node.dispatch_mode == dispatchKind.BROADCAST %}
{{ write_broadcast(node, idx, t_out) }}
{% endif %}
{%- endmacro %}
