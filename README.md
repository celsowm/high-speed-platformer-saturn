# high_speed_platformer

A stage-based 2D platformer built from the generic 2D runtime modules of
[LibSaturn](https://github.com/celsowm/libsaturn), and nothing else: no module in the library knows
this game. It was the acceptance example of LibSaturn's 2D runtime refactor and lives here as an
independent consumer of the installed LibSaturn package: it reaches the library only through
`find_package(LibSaturn CONFIG)`, `LibSaturn::Saturn`, `LibSaturn::Sim2D` and the helpers the package
ships. There is no sibling checkout, no `add_subdirectory`, and no LibSaturn private header or
build-tree path anywhere in this repository.

Everything in it is original or synthetic: the stage is placed by code, the art is drawn by code,
and no data comes from any other game.

## Controls

| Button | Action |
| --- | --- |
| D-pad left / right | run |
| A / B / C | jump (hold for a higher jump; also leaves a platform or the rail) |
| D-pad down | roll (needs some speed; keeps the speed on slopes) |

The stage runs from the left wall to the finish line at the right. Falling into a pit respawns at the
last checkpoint flag.

## What it exercises

| Stress item | Where |
| --- | --- |
| high horizontal speed | the dash pads set the ground speed to 14 px/step; a hill run reaches 17 |
| linear and profiled slopes | 45 degree and 22.5 degree ramps made of `columns` profiles |
| floor / wall / ceiling traversal | the loop: an octagon, 16 x 16 tiles |
| moving support | two platforms follow Path2 curves (a Bezier and a circle) |
| one-way support | the planks and the top-solid platform |
| layer switching | three Physics2 sensors flip the hero's terrain layer around the loop |
| generic path | the platforms; the rail is a cubic Bezier |
| rail-like path follower | the hero hangs from the rail and rides the curve |
| generic collectibles | rings, with a sparkle clip on pickup |
| regional entity activation | all rings, pads, springs and flags stream through `entity_stream2` |
| large logical map | 3072 x 384 px, three VDP2 layers kept resident by `stage_map2` |
| camera look-ahead, bounds, shake | `follow_camera2d`: dead zone, speed look-ahead, a locked finish room, shake on hard landings and pads |
| sprite animation | `sprite_clip` clips with pivots, events (footsteps), rates and shapes (ring pickup boxes) |
| multiple VDP2 layers / parallax | NBG0 terrain, NBG1 hills at 1/2 speed, NBG2 clouds at 1/4 |

## Layout

| Path | What it is |
| --- | --- |
| `src/game.c`, `src/game.h` | the game: acceleration, slopes, jumps, rail, pickups, camera and entity glue. No hardware. |
| `src/view.c`, `src/view.h` | VDP2 layers, texture, VDP1 draws and the HUD |
| `src/art.c`, `src/art.h` | procedural cells, palettes and the sprite sheet |
| `src/main.c` | the frame loop (60 Hz fixed steps, up to three catch-up steps) |
| `tools/gen_stage.py` | writes the stage2d spec and the layout (spawn, triggers, platforms) |
| `cmake/StageData.cmake` | runs `gen_stage.py`, then LibSaturn's installed `stage2d_tool.py`, into the build tree |
| `tests/host/test_sim.cpp` | plays the game's `game.c` with no video chip and asserts the stress items |
| `tests/host/test_boundary.cpp` | the consumer-boundary check against the installed prefix |
| `harness/high_speed_platformer.pad`, `tools/probe_check.py` | the scripted probe run and its check |

`gen_stage.py` and `stage2d_tool.py` produce `stage.h/.c` and `layout.h/.c` into
`build/<preset>/generated/`; nothing generated is checked in.

## Requirements

- An installed LibSaturn package (a `cmake --install` prefix, or the Conan package), version 0.1 or
  newer: it provides the runtime, the stage2d tool and `LibSaturn::Sim2D`. Set `LIBSATURN_PREFIX` to it.
- The `sh2eb-elf` GCC toolchain on `PATH`, CMake 3.24+, Ninja, Python 3.9+ (standard library only, see
  `requirements.txt`), and `mkisofs`/`genisoimage`/`xorrisofs` for the disc image.

## Build and test

```sh
export LIBSATURN_PREFIX=/path/to/libsaturn   # the installed package

cmake --preset host                          # simulation + boundary tests (native compiler)
cmake --build --preset host
ctest --preset host

cmake --preset saturn                        # firmware and the bootable disc (SH-2 cross build)
cmake --build --preset saturn
```

Outputs under `build/saturn/`: `high_speed_platformer.elf`, `high_speed_platformer.app.bin` (must stay
under 983040 bytes) and `disc/high_speed_platformer.{iso,bin,cue}`.

With Conan instead of a CMake prefix:

```sh
conan config install <libsaturn>/packaging/conan/config
conan create <libsaturn> --profile:host=saturn-sh2eb --profile:build=default
conan create .           --profile:host=saturn-sh2eb --profile:build=default
```

## Run it

Open `build/saturn/disc/high_speed_platformer.cue` in an emulator. LibSaturn's Ymir harness probe (a
separate tool, not vendored here) plays the whole stage with `harness/high_speed_platformer.pad` (run
right, one short jump over the first pit). The game publishes counters in `g_hsp_telemetry`, and
`tools/probe_check.py` reads them out of work RAM and fails unless the stage was cleared without a death,
the loop switched layers twice and the speed passed the dash-pad speed:

```sh
python tools/probe_check.py --probe path/to/probe --bios path/to/your_saturn_bios.bin     --nm sh2eb-elf-nm --shots build/shots
```

## Boundary

The game includes public `saturn/*` headers only and defines its own check macro. The `boundary` test
fails the build on a header the installed package does not ship, a private include, a build rule that
adds a sibling directory, or a script that reaches into a LibSaturn checkout.
