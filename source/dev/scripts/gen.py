#!/usr/bin/env python3
"""scripts/gen.py

The main function is `create_project`:

    create_project(name='chorus', 
        params=[
            # param      type       initial     min     max
            ('time',     'int',     1,          0,      500),
            ('feedback', 'float',   0.0,        0.0,    1.0),
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
from dataclasses import dataclass
from typing import Optional, Union

from jinja2 import Environment, FileSystemLoader

TEMPLATE_DIR = os.path.join(os.getcwd(), 'template~')
assert os.path.exists(TEMPLATE_DIR), "Must be run from dev directory"

env = Environment(loader=FileSystemLoader(TEMPLATE_DIR))

@dataclass
class Param:
    """Max/MSP External parameter class"""
    name: str
    type: str
    value: Union[float, int]
    min: Union[float, int] = 0.0
    max: Union[float, int] = 1.0

class Project:
    """Generates soundpipe module-based MSP external project4"""

    template_paths = [
            'src/sp_template~.c.j2',
            'project.yml.j2',
            'project.xcconfig.j2',
            'build.sh.j2',
            'sp_template~.maxhelp.j2',
            'README.md.j2',
        ]

    def __init__(self, name, shortname=None, params=None):
        self.name = name
        self.shortname = shortname or name
        self.params = params or []

    def getch(self, txt):
        print(txt, end=' ')
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(sys.stdin.fileno())
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch

    def create(self):
        """create project structure"""
        project = Path(f"sp_{self.name}~")
        if not project.exists():
            project.mkdir()
            (project / 'src').mkdir()
        for path in self.template_paths:
            if 'sp_template~' in path:
                target = project / path.replace('template', self.name).strip('.j2')
            else:
                target = project / path.strip('.j2')
            template = env.get_template(path)
            rendered = template.render(
                name=self.name, shortname=self.shortname, params=self.params)
            if target.exists():
                answer = self.getch(f"\n overwrite '{target}' (y/n/q)?")
                if answer == 'y':
                    target.write_text(rendered)
                if answer == 'q':
                    sys.exit()
                continue
            else:
                target.write_text(rendered)


    @classmethod
    def commandline(cls):
        parser = argparse.ArgumentParser(description='generate a project.')
        parser.add_argument('name', help='name of external')
        parser.add_argument('--shortname', '-s', required=False,
                            help='shortname of module')
        parser.add_argument('--param', '-p', nargs='+', action='append',
                            help = 'add a param (name, default) or (name, type, default)')

        args = parser.parse_args()
        print(args)
        name = args.name
        shortname = args.shortname
        params = []
        for entry in args.param:
            if len(entry) == 2:
                # (name, initial)
                param, initial = entry
                if '.' in initial:
                    p = Param(name=param, type='float', value=float(initial))
                else:
                    p = Param(name=param, type='long', value=int(initial), min=0, max=100)
                params.append(p)
            elif len(entry) == 4:
                # (name, initial, min, max)
                param, initial, min, max = entry
                if '.' in initial:
                    p = Param(name=param, type='float', value=float(initial),
                            min=float(min), max=float(max))
                else:
                    p = Param(name=param, type='long', value=int(initial),
                            min=int(min), max=int(max))
                params.append(p)
            else:
                print('skipping', entry)
                print("can only process 2-element or 4-element params")
                continue
        cls(name, shortname, params).create()


if __name__ == '__main__': 
    Project.commandline()
