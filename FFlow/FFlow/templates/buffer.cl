{% macro declare_global(node) -%}
{% for b in node.get_buffers(bufferType.GLOBAL) %}
__global {{ b.data_type }} * {{ b.name }}{{ ', ' if not loop.last else '' }}
{%- endfor %}
{%- endmacro %}

{% macro declare_local(node) -%}
{% for b in node.get_buffers(bufferType.LOCAL) %}
__local {{ b.data_type }} {{ b.name }}{{ '[' + b.size|string + ']' if b.size > 1 else '' }};
{% endfor %}
{%- endmacro %}

{% macro declare_private(node) -%}
{% for b in node.get_buffers(bufferType.PRIVATE) %}
{{ b.data_type }} {{ b.name }}{{ '[' + b.size|string + ']' if b.size > 1 else '' }};
{% endfor %}
{%- endmacro %}


{% macro declare_all(node) -%}
{% for b in node.buffers %}
{{ b.get_declaration() }}{{ ', ' if not loop.last else '' }}
{%- endfor %}
{%- endmacro %}


{% macro use_global(node) -%}
{% for b in node.get_buffers(bufferType.GLOBAL) %}
{{ b.name }}{{ ', ' if not loop.last else '' }}
{%- endfor %}
{%- endmacro %}

{% macro use_local(node) -%}
{% for b in node.get_buffers(bufferType.LOCAL) %}
{{ b.name }}{{ ', ' if not loop.last else '' }}
{%- endfor %}
{%- endmacro %}

{% macro use_private(node) -%}
{% for b in node.get_buffers(bufferType.PRIVATE) %}
&{{ b.name }}{{ ', ' if not loop.last else '' }}
{%- endfor %}
{%- endmacro %}

{% macro use_all(node) -%}
{% for b in node.buffers %}
{{ '&' if b.buffer_type.value == bufferType.PRIVATE.value else '' }}{{ b.name }}{{ ', ' if not loop.last else '' }}
{%- endfor %}
{%- endmacro %}
