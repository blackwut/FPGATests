{% macro declare_global_memories(node) -%}
{% for m in node.memories if m.memory_type.value == memoryType.GLOBAL.value%}
__global {{ m.data_type }} * {{ m.name }}{{ ", " if not loop.last else "" }}
{%- endfor %}
{%- endmacro %}

{% macro declare_local_memories(node) -%}
{% for m in node.memories if m.memory_type.value == memoryType.LOCAL.value%}
__local {{ m.data_type }} {{ m.name }}{{ "[" + m.size|string + "]" if m.size > 1 else "" }};
{% endfor %}
{%- endmacro %}

{% macro declare_private_memories(node) -%}
{% for m in node.memories if m.memory_type.value == memoryType.PRIVATE.value%}
{{ m.data_type }} {{ m.name }}{{ "[" + m.size|string + "]" if m.size > 1 else "" }};
{% endfor %}
{%- endmacro %}