from conans import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain
from conan.tools.scm import Git
from conan.tools.files import load
import re

from os.path import join


class Recipe(ConanFile):
    name = "vwos-sok-sink-sample"

    def set_version(self):
        content = load(self, f"{self.recipe_folder}/conanfile.py")
        version = re.search("version *= *\"([^\"]*)\"", content).group(1)
        self.version = version.strip()

    license = "<VW.os license>"
    description = "Sample application for SOK sink"
    url = "https://git.hub.vwgroup.com/swp-vwos/vwos-sok.git"
    topics = ("VW.os", "SCI", "signal", "communication")

    settings = "os", "arch", "compiler", "build_type", "vwos_build_mode"

    # Note: preserve scm attribute formatting
    scm = {"revision": "auto",
           "subfolder": ".",
           "type": "git",
           "url": "auto"}
    revision_mode = "scm"

    generators = "CMakeDeps"

    @property
    def namespace(self):
        return f"{self.user}/{self.channel}" if self._conan_user else ""

    def layout(self):
        self.folders.build = "build_sample_sink"
        self.folders.generators = join(self.folders.build, "conan")
        self.folders.source = "tests/samples/sok_demo"

    def requirements(self):
        self.requires(f"vwos-sok-fm/{self.version}@{self.namespace}")
        self.requires("vwos-mid-integration-model/[~3.2.3]@vwos/integration")
        self.requires("protobuf/[~3.21.6]@vwos/integration")

    def build_requirements(self):
        self.build_requires("vwos-mid-vector-amsr-generators/[~1.0.0]@vwos/integration")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables['SOK_FVM_SERVER'] = False
        tc.cache_variables["CONAN_PKG_NAME"] = self.name
        tc.cache_variables["CONAN_PKG_VERSION"] = self.version
        if self.settings.os == 'Neutrino':
            tc.preprocessor_definitions["NEUTRINO_BUILD"] = 1
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.install()
        self.copy("deploy/*")

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "module")
        self.cpp_info.includedirs = ["model"]
