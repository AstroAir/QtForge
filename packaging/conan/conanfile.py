from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy, get
import os

class QtPluginConan(ConanFile):
    name = "qtplugin"
    version = "3.0.0"
    
    # Package metadata
    description = "Modern C++ Plugin System for Qt Applications"
    license = "MIT"
    author = "QtForge <support@qtforge.dev>"
    url = "https://github.com/QtForge/QtPlugin"
    homepage = "https://github.com/QtForge/QtPlugin"
    topics = ("qt", "plugin", "c++", "modular", "architecture")
    
    # Package configuration
    package_type = "library"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "network": [True, False],
        "ui": [True, False],
        "examples": [True, False],
        "tests": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "network": False,
        "ui": False,
        "examples": True,
        "tests": False
    }
    
    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*", "include/*", "cmake/*", "examples/*", "tests/*", "LICENSE", "README.md"
    
    def config_options(self) -> None:
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")
    
    def configure(self) -> None:
        if self.options.shared:
            self.options.rm_safe("fPIC")
    
    def layout(self) -> None:
        cmake_layout(self)
    
    def requirements(self) -> None:
        self.requires("qt/6.5.3")
        
        if self.options.network:
            # Qt Network is included in qt/6.5.3
            pass
        
        if self.options.ui:
            # Qt Widgets is included in qt/6.5.3
            pass
    
    def build_requirements(self) -> None:
        self.tool_requires("cmake/[>=3.21]")
    
    def generate(self) -> None:
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        
        # Set CMake options based on Conan options
        tc.variables["QTPLUGIN_BUILD_NETWORK"] = self.options.network
        tc.variables["QTPLUGIN_BUILD_UI"] = self.options.ui
        tc.variables["QTPLUGIN_BUILD_EXAMPLES"] = self.options.examples
        tc.variables["QTPLUGIN_BUILD_TESTS"] = self.options.tests
        tc.variables["BUILD_SHARED_LIBS"] = self.options.shared
        
        tc.generate()
    
    def build(self) -> None:
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        
        if self.options.tests:
            cmake.test()
    
    def package(self) -> None:
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
        cmake = CMake(self)
        cmake.install()
    
    def package_info(self) -> None:
        # Core library is always available
        self.cpp_info.components["core"].libs = ["qtplugin-core"]
        self.cpp_info.components["core"].requires = ["qt::qt6Core"]
        self.cpp_info.components["core"].names["cmake_find_package"] = "Core"
        self.cpp_info.components["core"].names["cmake_find_package_multi"] = "Core"
        
        # Security component
        self.cpp_info.components["security"].libs = ["qtplugin-security"]
        self.cpp_info.components["security"].requires = ["core"]
        self.cpp_info.components["security"].names["cmake_find_package"] = "Security"
        self.cpp_info.components["security"].names["cmake_find_package_multi"] = "Security"
        
        # Optional components
        if self.options.network:
            self.cpp_info.components["network"].libs = ["qtplugin-network"]
            self.cpp_info.components["network"].requires = ["core", "qt::qt6Network"]
            self.cpp_info.components["network"].names["cmake_find_package"] = "Network"
            self.cpp_info.components["network"].names["cmake_find_package_multi"] = "Network"
        
        if self.options.ui:
            self.cpp_info.components["ui"].libs = ["qtplugin-ui"]
            self.cpp_info.components["ui"].requires = ["core", "qt::qt6Widgets"]
            self.cpp_info.components["ui"].names["cmake_find_package"] = "UI"
            self.cpp_info.components["ui"].names["cmake_find_package_multi"] = "UI"
        
        # Global package info
        self.cpp_info.names["cmake_find_package"] = "QtPlugin"
        self.cpp_info.names["cmake_find_package_multi"] = "QtPlugin"
        
        # Set C++ standard requirement
        self.cpp_info.cppstd = "20"
        
        # pkg-config support
        self.cpp_info.names["pkg_config"] = "qtplugin"
