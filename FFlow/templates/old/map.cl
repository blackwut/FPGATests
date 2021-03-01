{% import 'channels.cl' as ch with context %}

{% macro map(node, idx) -%}

CL_SINGLE_TASK map_{{ idx }}()
{
    uint r = {{ idx }} % {{node.i_degree}};
    uint n = 0;
    uint EOS = 0;
    bool done = false;

    while (!done) {
        tuple_t t;
        bool valid = false;

        switch (r) {
        {% for i in range(node.i_degree): %}
            case {{ i }}: t = read_channel_nb_intel({{ ch.i_channel(node, i, idx) }}, &valid); break;
        {% endfor %}
        }

        if (valid) {
            if (!t.EOS) {
                process_map(&t.data);
                {{ ch.keyby(node, idx) }}
                n++;
            } else {
                done = (++EOS == {{ node.i_degree }});
            }
        }

        if (++r == {{ node.i_degree}} ) r = 0;
    }

    const tuple_t t = createTupleEOS();
    #pragma unroll
    for (uint i = 0; i < {{ node.o_degree }}; ++i) {
        write_channel_intel({{ ch.o_channel(node, idx, 'i') }}, t);
    }
}

{%- endmacro %}


{#
{% macro map(node, idx) -%}

CL_SINGLE_TASK map_{{ idx }}()
{
    uint r = {{ idx }} % {{node.i_degree}};
    uint n = 0;
    uint EOS = 0;
    bool done = false;

    while (!done) {
        tuple_t t;
        bool valid = false;

        switch (r) {
        {% for i in range(node.i_degree): %}
            case {{ i }}: t = read_channel_nb_intel({{ ch.i_channel(node, i, idx) }}, &valid); break;
        {% endfor %}
        }

        if (valid) {
            if (!t.EOS) {
                const uint w = n % {{ node.o_degree }};
                process_map(&t.data);

                switch (w) {
                {% for i in range(node.o_degree): %}
                    case {{ i }}: if (test) write_channel_intel({{ ch.o_channel(node, idx, i) }}, t); break;
                {% endfor %}
                }

                n++;
            } else {
                done = (++EOS == {{ node.i_degree }});
            }
        }

        if (++r == {{ node.i_degree}} ) r = 0;
    }

    const tuple_t t = createTupleEOS();
    #pragma unroll
    for (uint i = 0; i < {{ node.o_degree }}; ++i) {
        write_channel_intel({{ ch.o_channel(node, idx, 'i') }}, t);
    }
}

{%- endmacro %}
#}