# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "F:/AI_Gen_CPlus/Auto_Generate_Tool/build/_deps/googletest-src"
  "F:/AI_Gen_CPlus/Auto_Generate_Tool/build/_deps/googletest-build"
  "F:/AI_Gen_CPlus/Auto_Generate_Tool/build/_deps/googletest-subbuild/googletest-populate-prefix"
  "F:/AI_Gen_CPlus/Auto_Generate_Tool/build/_deps/googletest-subbuild/googletest-populate-prefix/tmp"
  "F:/AI_Gen_CPlus/Auto_Generate_Tool/build/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp"
  "F:/AI_Gen_CPlus/Auto_Generate_Tool/build/_deps/googletest-subbuild/googletest-populate-prefix/src"
  "F:/AI_Gen_CPlus/Auto_Generate_Tool/build/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "F:/AI_Gen_CPlus/Auto_Generate_Tool/build/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "F:/AI_Gen_CPlus/Auto_Generate_Tool/build/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
