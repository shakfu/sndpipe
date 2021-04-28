#!/usr/bin/env python3

import os 
from pathlib import Path

from jinja2 import Environment, FileSystemLoader

TEMPLATE_DIR = os.path.join(os.getcwd(), 'template~')
assert os.path.exists(TEMPLATE_DIR)

env = Environment(loader=FileSystemLoader(TEMPLATE_DIR))

def create_project(name, shortname, params):
    template_paths = [
        'src/sp_template~.c.j2',
        'project.yml.j2',
        'build.sh.j2',
    ]
    project = Path(f"sp_{name}~")
    if not project.exists():
        project.mkdir()
        (project / 'src').mkdir()
    for path in template_paths:
        if 'sp_template~.c' in path:
            target = project / path.replace('template', name).strip('.j2')
        else:
            target = project / path.strip('.j2')
        template = env.get_template(path)
        rendered = template.render(name=name, shortname=shortname, params=params)
        target.write_text(rendered)
    print('done')


if __name__ == '__main__':

    create_project(name='delay', shortname='del', 
        params=[
            # param     type        default value
            ('time',    'float',    500.0),
        ])
