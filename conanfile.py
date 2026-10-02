"""Conan 2 recipe for the high-speed platformer.

An ordinary LibSaturn consumer: `requires("libsaturn")` and nothing else. The recipe configures the
project's own CMakeLists.txt with the Saturn host profile; CMakeToolchain puts the libsaturn package
folder on CMAKE_PREFIX_PATH and the project's unmodified `find_package(LibSaturn CONFIG REQUIRED)`
resolves the installed config, exactly as it does for a `cmake --install` prefix.

    conan config install <libsaturn>/packaging/conan/config   # saturn-sh2eb profile
    conan create <libsaturn> --profile:host=saturn-sh2eb --profile:build=default
    conan create . --profile:host=saturn-sh2eb --profile:build=default

The stage is generated at build time; no game data is stored in the package sources.
"""
import os

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy, load


class HighSpeedPlatformerConan(ConanFile):
    name = "high-speed-platformer"
    description = "A stage-based 2D platformer for the Sega Saturn built from LibSaturn's generic 2D runtime"
    license = "MIT"
    url = "https://github.com/celsowm/high-speed-platformer-saturn"
    homepage = "https://github.com/celsowm/high-speed-platformer-saturn"
    topics = ("sega-saturn", "platformer", "sh2", "libsaturn")

    package_type = "application"
    settings = "os", "arch", "compiler", "build_type"

    exports_sources = (
        "CMakeLists.txt", "CMakePresets.json", "VERSION", "LICENSE", "NOTICE.md",
        "cmake/*", "src/*", "tools/*", "tests/*", "harness/*", "requirements.txt",
    )

    def set_version(self):
        self.version = load(self, os.path.join(self.recipe_folder, "VERSION")).strip()

    def requirements(self):
        self.requires("libsaturn/[>=0.1 <0.2]")

    def validate(self):
        if str(self.settings.os) != "baremetal" or str(self.settings.arch) != "sh2eb":
            raise ConanInvalidConfiguration(
                "high-speed-platformer builds for the Sega Saturn: use --profile:host=saturn-sh2eb "
                "(installed by `conan config install` from LibSaturn's packaging/conan/config)."
            )

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["CMAKE_C_FLAGS_RELEASE"] = "-O2"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE", self.source_folder, os.path.join(self.package_folder, "licenses"))
        copy(self, "NOTICE.md", self.source_folder, os.path.join(self.package_folder, "licenses"))
        copy(self, "high_speed_platformer.app.bin", self.build_folder, os.path.join(self.package_folder, "bin"))
        copy(self, "high_speed_platformer.elf", self.build_folder, os.path.join(self.package_folder, "bin"))
        for pattern in ("high_speed_platformer.cue", "high_speed_platformer.bin", "high_speed_platformer.iso"):
            copy(self, pattern, os.path.join(self.build_folder, "disc"), os.path.join(self.package_folder, "disc"))

    def package_info(self):
        self.cpp_info.bindirs = ["bin"]
        self.cpp_info.libdirs = []
        self.cpp_info.includedirs = []
