# Utilities Module
import os
import logging
import jinja2 as jinja


def read_template_file(source_code_dir, path):
    templates = os.path.join(os.path.dirname(__file__), "templates")
    sources = os.path.join(os.path.dirname(__file__), source_code_dir)
    loader = jinja.FileSystemLoader(searchpath=[templates, sources])

    logging.basicConfig()
    logger = logging.getLogger('logger')
    logger = jinja.make_logging_undefined(logger=logger, base=jinja.Undefined)

    env = jinja.Environment(loader=loader, undefined=logger)
    env.lstrip_blocks = True
    env.trim_blocks = True
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
