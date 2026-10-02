# Changelog

## 0.1.0 (unreleased)

Initial extraction from LibSaturn's `examples/high_speed_platformer` (history kept).

- Standalone CMake project that consumes LibSaturn only through its installed package
  (`find_package(LibSaturn CONFIG)`, `LibSaturn::Saturn`, `libsaturn_add_disc`), with `saturn`
  (firmware + bootable disc) and `host` (simulation and boundary tests) presets and a Conan 2 recipe.
- The simulation test compiles the game's own logic against `LibSaturn::Sim2D`, the hardware-free 2D
  modules the package ships as sources; the stage is compiled by the package's `stage2d_tool.py`.
- The boundary test checks the sources against the installed prefix: only public `saturn/*` headers,
  no `examples/common`, no sibling checkout.
- Behaviour is unchanged from the in-tree example: scripted Ymir probe runs of the old and new builds
  produce byte-identical screenshots, and the simulation test gives the same results.
