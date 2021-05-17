# Utilities Module
import os
import logging
import jinja2 as jinja
import re


def generate_flat_map_code(code):
    regex = r'(\s*send\()(\S+)(\);)'

    result = re.search(regex, code, re.MULTILINE)
    var_name = result.group(2)
    start_pos = result.span(1)[0]
    end_pos = result.span(2)[1]

    new_text = ("\n\n"
                "        {{ node.o_channel.tuple_type }} tuple_out = create_{{ node.o_channel.tuple_type }}(" + var_name + ");\n"
                "        {{ ch.dispatch_tuple(node, 'idx', '*w', 'tuple_out', true)|indent(8) }}")

    left = start_pos
    right = end_pos + 2
    new_code = code[:left] + new_text + code[right:]

    return new_code


def read_template_file(source_code_dir, path):
    templates = os.path.join(os.path.dirname(__file__), "templates")
    sources = os.path.join(source_code_dir)
    print('sources:' + sources)
    sources_tmp = os.path.join(sources, 'tmp')
    loader = jinja.FileSystemLoader(searchpath=[templates, sources, sources_tmp])

    logging.basicConfig()
    logger = logging.getLogger('logger')
    logger = jinja.make_logging_undefined(logger=logger, base=jinja.Undefined)

    env = jinja.Environment(loader=loader, undefined=logger)
    env.lstrip_blocks = True
    env.trim_blocks = True
    env.filters['generate_flat_map'] = generate_flat_map_code
    return env.get_template(path)


def previous_current_next(iterable):
    iterable = iter(iterable)
    prv = None
    cur = iterable.__next__()
    try:
        while True:
            nxt = iterable.__next__()
            yield (prv, cur, nxt)
            prv = cur
            cur = nxt
    except StopIteration:
        yield (prv, cur, None)

#define __NL__
#define QQ(X) #X

#define send(x) {{node.o_channel.tuple_type}} {{tuple_var_out}} = create_{{node.o_channel.tuple_type}}(QQ(x));\
# __NL__ {{ ch.dispatch_tuple(node, idx, QQ(x), idx, tuple_var_out, true) }}
# // {{node.o_channel.tuple_type}} {{tuple_var_out}} = create_{{node.o_channel.tuple_type}}('x');\
# // {{ ch.dispatch_tuple(node, idx, \' + var_name + "', idx, tuple_var_out, true) }}")
