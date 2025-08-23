class Qtplugin < Formula
  desc "Modern C++ Plugin System for Qt Applications"
  homepage "https://github.com/QtForge/QtPlugin"
  url "https://github.com/QtForge/QtPlugin/archive/refs/tags/v3.0.0.tar.gz"
  sha256 "0000000000000000000000000000000000000000000000000000000000000000"
  license "MIT"
  head "https://github.com/QtForge/QtPlugin.git", branch: "main"

  depends_on "cmake" => :build
  depends_on "ninja" => :build
  depends_on "qt@6"

  def install
    args = %W[
      -DCMAKE_BUILD_TYPE=Release
      -DQTPLUGIN_BUILD_EXAMPLES=ON
      -DQTPLUGIN_BUILD_TESTS=OFF
      -DQTPLUGIN_BUILD_NETWORK=ON
      -DQTPLUGIN_BUILD_UI=ON
      -DCMAKE_INSTALL_PREFIX=#{prefix}
    ]

    system "cmake", "-S", ".", "-B", "build", "-G", "Ninja", *args
    system "cmake", "--build", "build", "--parallel"
    system "cmake", "--install", "build"
  end

  test do
    # Test pkg-config
    assert_match version.to_s, shell_output("pkg-config --modversion qtplugin")
    
    # Test CMake integration
    (testpath/"CMakeLists.txt").write <<~EOS
      cmake_minimum_required(VERSION 3.21)
      project(test_qtplugin)
      
      set(CMAKE_CXX_STANDARD 20)
      set(CMAKE_CXX_STANDARD_REQUIRED ON)
      
      find_package(QtPlugin REQUIRED COMPONENTS Core)
      
      add_executable(test_qtplugin test.cpp)
      target_link_libraries(test_qtplugin QtPlugin::Core)
    EOS

    (testpath/"test.cpp").write <<~EOS
      #include <qtplugin/qtplugin.hpp>
      #include <iostream>
      
      int main() {
          std::cout << "QtPlugin version: " << QTPLUGIN_VERSION_STRING << std::endl;
          return 0;
      }
    EOS

    system "cmake", "-S", ".", "-B", "build", "-DCMAKE_BUILD_TYPE=Release"
    system "cmake", "--build", "build"
    
    # Run the test
    output = shell_output("./build/test_qtplugin")
    assert_match "QtPlugin version:", output
  end
end
