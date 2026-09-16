cmake_minimum_required(VERSION 3.16)
project(pompeiu_hausdorff)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
SET(CMAKE_BUILD_TYPE Release)

# option to build executable (default true)
option(BUILD_EXECUTABLE "Build executable" ON)
option(BUILD_PYTHON_BINDINGS "Build python bindings" OFF)
option(BUILD_MEX_BINDINGS "Build Matlab mex bindings" OFF)

if(BUILD_MEX_BINDINGS)
  set(LIBIGL_RESTRICTED_MATLAB ON CACHE BOOL "" FORCE)
endif()

# Libigl
include(libigl)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# library called phd (pompeiu hausdorff distance)
set(CMAKE_CXX_STANDARD 11)
set(LIBRARY_NAME phd)
add_library(${LIBRARY_NAME} STATIC
  src/PompeiuHausdorff.cpp
  src/pompeiu_hausdorff.cpp
  src/upper_bounds.cpp
  src/kang_upper_bound.cpp
  src/kang_intersect_edge_and_bisector.cpp
  src/bisector_of_two_points.cpp)
target_link_libraries(${LIBRARY_NAME} igl::core)

# Apple's clang implicitly searches /usr/local/include ahead of any
# -isystem path, which can shadow the FetchContent-downloaded Eigen with an
# incompatible system-installed copy (e.g. from Homebrew). CMake collapses
# any include directory into a single -isystem entry once it also appears
# as a SYSTEM interface include (as it does via Eigen3::Eigen), so simply
# re-adding the same physical path as a plain include has no effect. Work
# around this by symlinking the fetched Eigen headers into a private
# directory and adding *that* (a distinct path, so it isn't deduplicated
# against the -isystem entry) as an ordinary -I, which clang searches
# before the implicit /usr/local/include.
get_target_property(EIGEN_INTERFACE_INCLUDE_DIRS Eigen3::Eigen INTERFACE_INCLUDE_DIRECTORIES)
string(REGEX MATCH "\\$<BUILD_INTERFACE:([^>]+)>" _ "${EIGEN_INTERFACE_INCLUDE_DIRS}")
if(CMAKE_MATCH_1)
  set(EIGEN_REAL_DIR "${CMAKE_MATCH_1}")
  set(EIGEN_OVERRIDE_DIR "${CMAKE_BINARY_DIR}/eigen_override")
  file(MAKE_DIRECTORY "${EIGEN_OVERRIDE_DIR}")
  if(NOT EXISTS "${EIGEN_OVERRIDE_DIR}/Eigen")
    file(CREATE_LINK "${EIGEN_REAL_DIR}/Eigen" "${EIGEN_OVERRIDE_DIR}/Eigen" SYMBOLIC)
  endif()
  target_include_directories(${LIBRARY_NAME} BEFORE PUBLIC "${EIGEN_OVERRIDE_DIR}")
endif()

if(BUILD_EXECUTABLE)
  # executable called pompeiu_hausdorff
  set(EXECUTABLE_TARGET pompeiu_hausdorff_exe)
  add_executable(${EXECUTABLE_TARGET} main.cpp)
  target_link_libraries(${EXECUTABLE_TARGET} ${LIBRARY_NAME} igl::core)
  # change name to pompeiu_hausdorff
  set_target_properties(${EXECUTABLE_TARGET} PROPERTIES OUTPUT_NAME pompeiu_hausdorff)
endif()
  
  
# Download and set up nanobind
if(BUILD_PYTHON_BINDINGS)
  set(CMAKE_CXX_STANDARD 17)
  
  if (CMAKE_VERSION VERSION_LESS 3.18)
    set(DEV_MODULE Development)
  else()
    set(DEV_MODULE Development.Module)
  endif()

  find_package(Python 3.8 
    REQUIRED COMPONENTS Interpreter ${DEV_MODULE} 
    OPTIONAL_COMPONENTS Development.SABIModule)
  include(FetchContent)
  FetchContent_Declare(
    nanobind
    GIT_REPOSITORY https://github.com/wjakob/nanobind.git
    GIT_TAG        v2.2.0
  )
  FetchContent_MakeAvailable(nanobind)

  nanobind_add_module(
    cascading_upper_bounds_ext 
    STABLE_ABI
    NB_STATIC
    src/bindings.cpp)
  target_link_libraries(
    cascading_upper_bounds_ext PRIVATE ${LIBRARY_NAME} igl::core)
  #nanobind_add_stub(
  #  ${binding_name}_stub
  #  MODULE ${binding_name}
  #  OUTPUT "${binding_name}.pyi"
  #  PYTHON_PATH $<TARGET_FILE_DIR:${binding_name}>
  #  DEPENDS ${binding_name}
  #)

  # Install directive for scikit-build-core
  install(TARGETS cascading_upper_bounds_ext LIBRARY DESTINATION cascading_upper_bounds)

endif()

if(BUILD_MEX_BINDINGS)
  # igl_restricted::matlab already calls find_package(Matlab), but it does
  # so from inside a CMake function, so variables like
  # Matlab_EXTERN_LIBRARY_DIR (needed by matlab_add_mex to locate the
  # platform's exported-symbols map file) never escape into this scope.
  # Call it again here so those variables are set where matlab_add_mex
  # actually runs.
  find_package(Matlab REQUIRED COMPONENTS MEX_COMPILER MX_LIBRARY ENG_LIBRARY MAT_LIBRARY)

  matlab_add_mex(
    NAME pompeiu_hausdorff_mex
    OUTPUT_NAME pompeiu_hausdorff
    SRC src/mex_pompeiu_hausdorff.cpp
    LINK_TO ${LIBRARY_NAME} igl::core igl_restricted::matlab)

  # Matlab only shows a MEX-file's help via a same-named .m file that lives
  # in the *same* directory; build the mex binary directly into matlab/,
  # alongside pompeiu_hausdorff.m, so `help pompeiu_hausdorff` works and so
  # adding a single directory to the Matlab path is enough to use it.
  set_target_properties(pompeiu_hausdorff_mex PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/matlab"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/matlab")

  if(APPLE)
    # Xcode's new linker (default since Xcode 15) fails to resolve the
    # intentionally-undefined C++ MEX API symbols (mexCreateMexFunction,
    # etc.) that matlab_add_mex marks with -Wl,-U for dynamic lookup at
    # load time. This is a known MathWorks/Apple linker issue; forcing the
    # classic linker works around it.
    target_link_options(pompeiu_hausdorff_mex PRIVATE "-Wl,-ld_classic")
  endif()
endif()
