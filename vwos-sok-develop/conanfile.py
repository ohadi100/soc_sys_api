from conans import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain
from conan.tools.scm import Git

from os.path import join


class Recipe(ConanFile):
    name = "vwos-sok-fm"
    version = "0.5.0"
    license = "<VW.os license>"
    description = "VW.os Mid library for freshness manager"
    url = "https://git.hub.vwgroup.com/swp-vwos/vwos-sok.git"
    topics = ("VW.os", "Fvm", "Autosar", "Adaptive")

    settings = "os", "arch", "compiler", "build_type", "vwos_build_mode"

    # Note: preserve scm attribute formatting
    scm = {"revision": "auto",
           "subfolder": ".",
           "type": "git",
           "url": "auto"}
    revision_mode = "scm"
    options = {
        "gtest": [True, False],
    }
    default_options = {
        "gtest": False,
    }
    generators = "CMakeDeps"

    def layout(self):
        self.folders.build = "build_fvm"
        self.folders.generators = join(self.folders.build, "conan")
        self.folders.source = "."

    def requirements(self):
        self.requires("vwos-mid-vector-amsr/[~1.6.0]@vwos/integration")
        self.requires("vwos-sci-libbackend/[~6.10.0]@vwos/integration")
        self.requires("vwos-sci-libutils/[~0.6.0]@vwos/integration")
        # self.requires("vwos-crypto-libcrypto/[~7.9.0]@vwos/integration") #due to conan packages conflicts
        self.requires("rapidjson/[~1.1.1]@vwos/integration")
        self.requires("vwos-mid-integration-interfaces/[~3.5.0]@vwos/integration")
        self.requires("system-diag-lib/[~0.8.0]@vwos/integration")

    def build_requirements(self):
        self.test_requires("gtest/[~1.11.0]@vwos/integration")
        self.tool_requires("vwos-mid-clang-tools/[^1.0.0]@vwos/integration")
        self.tool_requires("vwos-mid-parasoft-tools/[^1.3.2]@vwos/integration")


    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["CONAN_PKG_NAME"] = self.name
        tc.cache_variables["CONAN_PKG_VERSION"] = self.version
        tc.cache_variables['ENABLE_UNIT_TESTS'] = self.options.gtest
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
        self.copy("FreshnessValueManager.hpp", dst="include/sok/fvm", src="include/sok/fvm/")
        self.copy("FreshnessValueManagerConstants.hpp", dst="include/sok/fvm", src="include/sok/fvm/")
        self.copy("FreshnessValueManagerDefinitions.hpp", dst="include/sok/fvm", src="include/sok/fvm/")
        self.copy("FreshnessValueManagerError.hpp", dst="include/sok/fvm", src="include/sok/fvm/")
        self.copy("IFreshnessValueManagerConfigAccessor.hpp", dst="include/sok/fvm", src="include/sok/fvm/")
        self.copy("IFvmRuntimeAttributesManager.hpp", dst="include/sok/fvm", src="include/sok/fvm/")
        self.copy("ISignalManager.hpp", dst="include/sok/fvm", src="include/sok/fvm/")
        self.copy("SokFmInternalFactory.hpp", dst="include/sok/fvm", src="include/sok/fvm/")
        self.copy("CommonDefinitions.hpp", dst="include/sok/common", src="include/sok/common/")
        self.copy("CommonError.hpp", dst="include/sok/common", src="include/sok/common/")
        self.copy("ErrorOrObjectResult.hpp", dst="include/sok/common", src="include/sok/common/")
        self.copy("ICsmAccessor.hpp", dst="include/sok/common", src="include/sok/common/")
        self.copy("SokUtilities.hpp", dst="include/sok/common", src="include/sok/common/")
        self.copy("Logger.hpp", dst="include/sok/common", src="include/sok/common/")
        self.copy("deploy/deploy.sh")

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "module")
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.set_property("cmake_target_name", "sok_fm::sok_fm")

        self.cpp_info.components["sok_fm"].set_property("cmake_target_name", "sok_fm::sok_fm")
        self.cpp_info.components["sok_fm"].libs.append("libsok_fm.so")

        self.cpp_info.components["fm_server"].set_property("cmake_target_name", "sok_fm::fm_server")
        self.cpp_info.components["fm_server"].libs.append("libsok_fm_server.so")
