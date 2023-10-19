# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/30bal/CLionProjects/grafika/src/glew"
  "C:/Users/30bal/CLionProjects/grafika/cmake-build-debug/glew-prefix/src/glew-build"
  "C:/Users/30bal/CLionProjects/grafika/cmake-build-debug/glew-prefix"
  "C:/Users/30bal/CLionProjects/grafika/cmake-build-debug/glew-prefix/tmp"
  "C:/Users/30bal/CLionProjects/grafika/cmake-build-debug/glew-prefix/src/glew-stamp"
  "C:/Users/30bal/CLionProjects/grafika/cmake-build-debug/glew-prefix/src"
  "C:/Users/30bal/CLionProjects/grafika/cmake-build-debug/glew-prefix/src/glew-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/30bal/CLionProjects/grafika/cmake-build-debug/glew-prefix/src/glew-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/30bal/CLionProjects/grafika/cmake-build-debug/glew-prefix/src/glew-stamp${cfgdir}") # cfgdir has leading slash
endif()
