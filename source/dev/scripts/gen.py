#!/usr/bin/env python3
"""scripts/gen.py

Should be run from `sndpipe/source/dev`

```
scripts/gen.py
```

"""

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


if __name__ == '__main__':

    create_project(name='chorus', 
        params=[
            # param      type       initial value
            ('time',     'float',   1.0),
            ('feedback', 'float',   0.0),
        ])
