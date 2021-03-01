{% macro declare_tuple(channel)-%}
{% with %}
{% set tuple_t = channel.tuple_type %}
{% set data_t  = channel.data_type %}
// tuple_t of channel {{channel.name}}
typedef struct {
    {{data_t}} data;
    bool EOS;
} {{tuple_t}};

inline {{tuple_t}} create_{{tuple_t}}({{data_t}} data)
{
    return ({{tuple_t}}){
                .data = data,
                .EOS = false
            };
}

inline {{tuple_t}} create_{{tuple_t}}_EOS()
{
    return ({{tuple_t}}){
                .data = {},
                .EOS = true
            };
}

{% endwith %}
{% endmacro -%}