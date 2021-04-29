#!/usr/bin/env python3
"""scripts/gen.py

The main function is `create_project`:


    create_project(name='chorus', 
        params=[
            # param      type       initial value
            ('time',     'float',   1.0),
            ('feedback', 'float',   0.0),
        ])


The command-line interface to the above function,
should be run from `sndpipe/source/dev`:

    $ ./scripts/gen.py --help
    usage: gen.py [-h] [--shortname SHORTNAME] [--param PARAM [PARAM ...]] name

    Start a subproject.

    positional arguments:
    name                  name of external

    optional arguments:
    -h, --help            show this help message and exit
    --shortname SHORTNAME, -s SHORTNAME
                            shortname of module
    --param PARAM [PARAM ...], -p PARAM [PARAM ...]
                            add a parameter triple (name, type, default)

Please note that `--param` or `-p` can used several times to add as many
params as required.

"""

import argparse
import termios
import tty
import sys
import os 
from pathlib import Path

from jinja2 import Environment, FileSystemLoader

TEMPLATE_DIR = os.path.join(os.getcwd(), 'template~')
assert os.path.exists(TEMPLATE_DIR)

env = Environment(loader=FileSystemLoader(TEMPLATE_DIR))

def getch(txt):
    print(txt, end=' ')
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(sys.stdin.fileno())
        ch = sys.stdin.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    return ch

def create_project(name, shortname=None, params=None):
    if not shortname:
        shortname = name
    if not params:
        params = []
    template_paths = [
        'src/sp_template~.c.j2',
        'project.yml.j2',
        'project.xcconfig.j2',
        'build.sh.j2',
        'sp_template~.maxhelp.j2',
        'README.md.j2',
    ]
    project = Path(f"sp_{name}~")
    if not project.exists():
        project.mkdir()
        (project / 'src').mkdir()
    for path in template_paths:
        if 'sp_template~' in path:
            target = project / path.replace('template', name).strip('.j2')
        else:
            target = project / path.strip('.j2')
        template = env.get_template(path)
        rendered = template.render(name=name, shortname=shortname, params=params)
        if target.exists():
            answer = getch(f"\n overwrite '{target}' (y/n/q)?")
            if answer == 'y':
                target.write_text(rendered)
            if answer == 'q':
                sys.exit()
            continue
        else:
            target.write_text(rendered)
    print('done')


def commandline():
    parser = argparse.ArgumentParser(description='Start a subproject.')
    parser.add_argument('name', help='name of external')
    parser.add_argument('--shortname', '-s', required=False,
                        help='shortname of module')
    parser.add_argument('--param', '-p', nargs='+', action='append',
                        help = 'add a param (name, default) or (name, type, default)')

    args = parser.parse_args()
    # print(args)
    name = args.name
    shortname = args.shortname
    types = dict(float=float, long=int)
    params = []
    for entry in args.param:
        if len(entry) == 2:
            if '.' in entry[1]:
                params.append((entry[0], 'float', float(entry[1])))
            else:
                params.append((entry[0], 'long', int(entry[1])))
        elif len(entry) == 3:
            param, typ, initial_value = entry
            params.append((param, typ, types[typ](initial_value)))
        else:
            print('skipping', entry)
            print("can only process 2-element or 3-element params")
            continue
    create_project(name, shortname, params)


if __name__ == '__main__': commandline()
