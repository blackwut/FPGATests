{% import 'channel.cl' as ch with context %}

{% macro node(node, idx) -%}

CL_SINGLE_TASK {{node.kernel_name(idx)}}(__global data_t * restrict data,
{% filter indent(node.kernel_name(idx)|length + 16, true) %}
const uint size,
const uint last_EOS,
__global uint * restrict received,
__global uint * restrict shutdown)
{% endfilter %}
{
    uint r = {{ idx % node.i_degree }};
    uint n = 0;
    uint EOS = last_EOS;
    bool done = false;
    bool filled = false;

    while (!filled && !done) {
        {{ node.declare_i_tuple('t_in') }};
        {{ ch.gather_tuple(node, idx, 'r', 't_in', 't_out', process_tuple) | indent(8) }}
    }

    *received = n;
    *shutdown = EOS;
}

{%- endmacro %}


{% macro process_tuple(node, idx, t_in, t_out) -%}
data[n] = {{ t_in }}.data;
n++;

if (n == size) {
    filled = true;
}
{%- endmacro %}