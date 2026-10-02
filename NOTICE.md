# Notices

## This repository's code

The game (`src/`), the stage generator (`tools/`), the build files and the tests are MIT licensed;
see `LICENSE`.

## No third-party game data

Everything in this project is original or synthetic: the stage is placed by code
(`tools/gen_stage.py`), the tiles, sprites and palettes are drawn by code (`src/art.c`), and no
asset, level layout, physics constant table or frame data comes from any other game. A disc built
from this repository contains only what this repository generates, plus the LibSaturn runtime
(MIT) linked into the executable.
