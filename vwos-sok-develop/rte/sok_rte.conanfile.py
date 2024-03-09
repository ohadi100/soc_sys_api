#### Aligned with CW01_S24.1.2 DevDrop ####

from conan import ConanFile 
 
class Recipe(ConanFile):
    name = "vwos-mid-sok-example"
    version = "1.3.0"
 
    settings = {
        "os": None,
        "vwos_ecu_target": [ "None", "ADAS1" ]
    }
 
    # Note: preserve scm attribute formatting
    scm = {"revision": "auto",
           "subfolder": ".",
           "type": "git",
           "url": "auto"}
    revision_mode = "scm"
 
    python_requires = "vwos-mid-integration-tools/[~4.2.3]@vwos/integration"
    python_requires_extend = "vwos-mid-integration-tools.MetaRecipeBase"
 
    def configure(self):
        if "hw_target" not in self.options["vwos-mid-integration-rte"] or ("hw_target" in self.options["vwos-mid-integration-rte"] and self.options["vwos-mid-integration-rte"].hw_target == "qemu"):
            self.options["vwos-sci-signaldispatcher-deployment"].deployment_configuration = "QNX_s32g_vt0"
 
    def requirements(self):
        # Runtime
        self.requires("vwos-mid-integration-rte/[^5.3.0]@vwos/integration")
 
        # AMSR
        self.requires("vwos-aux-state-manager/[^1.7.0]@vwos/integration")
        self.requires("vwos-aux-dm-daemon/[^1.5.0]@vwos/integration")

        # SOK demo apps
        self.requires("vwos-sok-sink-sample/[~0.5.0]@vwos/integration")
        self.requires("vwos-sok-source-sample/[~0.5.0]@vwos/integration")
