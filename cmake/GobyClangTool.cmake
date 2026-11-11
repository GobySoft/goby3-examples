# must output json compile commands for goby_clang_tool to work
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Enable/Disable output of compile commands during generation." FORCE)
unset(project_goby_interfaces CACHE)
unset(project_goby_interfaces_figures CACHE)

# usage: goby_export_interface(TARGET <target_name> YML_OUT_DIR <output_dir> YML <yml_var>)
# TARGET: target name
# YML_OUT_DIR: directory for output YML file
# YML: variable name to store the path to the yml file
function(GOBY_EXPORT_INTERFACE)
  cmake_parse_arguments(args "" "TARGET;YML_OUT_DIR;YML" "" ${ARGN})

  get_target_property(TARGET_SOURCES ${args_TARGET} SOURCES)

  file(MAKE_DIRECTORY ${args_YML_OUT_DIR})
  
  set(ABS_TARGET_SOURCES)
  foreach(SOURCE ${TARGET_SOURCES})
    get_filename_component(ABS_TARGET_SOURCE ${SOURCE} ABSOLUTE)
    get_filename_component(SOURCE_EXTENSION ${SOURCE} EXT)

    # omit protobuf header files
    if(NOT SOURCE_EXTENSION STREQUAL ".pb.h")
      list(APPEND ABS_TARGET_SOURCES ${ABS_TARGET_SOURCE})
    endif()
  endforeach()  
  
  set(${args_YML} "${args_YML_OUT_DIR}/${args_TARGET}_interface.yml")
  add_custom_command(
    OUTPUT "${args_YML_OUT_DIR}/${args_TARGET}_interface.yml"
    COMMAND goby_clang_tool
    ARGS -gen -target ${args_TARGET} -outdir ${args_YML_OUT_DIR} -p ${CMAKE_BINARY_DIR} ${ABS_TARGET_SOURCES}
    COMMENT "Running goby_clang_tool on ${args_TARGET}"
    DEPENDS ${ABS_TARGET_SOURCES} ${args_TARGET}
    VERBATIM)

  set_source_files_properties(${${args_YML}} PROPERTIES GENERATED TRUE)
  set(${args_YML} ${${args_YML}} PARENT_SCOPE)
endfunction()

macro(generate_interfaces) 
  cmake_parse_arguments(gi_args "" "TARGET" "" ${ARGN})
  goby_export_interface(TARGET ${gi_args_TARGET} YML_OUT_DIR ${YML_OUT_DIR} YML ${gi_args_TARGET}_YML_OUT)
  add_custom_target(${gi_args_TARGET}_interface ALL DEPENDS ${${gi_args_TARGET}_YML_OUT})
  set(project_goby_interfaces "${project_goby_interfaces};${gi_args_TARGET}_interface;${gi_args_TARGET}" CACHE INTERNAL "Goby Interface YMLS")
endmacro()
    
# usage: generate_interfaces_figure(DEPLOYMENT <deployment.yml> NAME <name>)
# Draws the deployment with 'goby interface visualize' as YML_OUT_DIR/figures/<name>.pdf (Graphviz)
# and, with build_interface_figures_d2, as <name>.d2 (rendered to <name>.svg when d2 is found).
function(generate_interfaces_figure)
  cmake_parse_arguments(gif_args "" "DEPLOYMENT;NAME" "" ${ARGN})
  get_filename_component(deployment ${gif_args_DEPLOYMENT} ABSOLUTE)
  set(figures_dir ${YML_OUT_DIR}/figures)
  file(MAKE_DIRECTORY ${figures_dir})
  set(visualize ${GOBY_EXECUTABLE} interface visualize ${deployment} -I ${YML_OUT_DIR})
  set(base ${figures_dir}/${gif_args_NAME})

  add_custom_command(
    OUTPUT ${base}.dot
    COMMAND ${visualize} --output_file ${base}.dot
    DEPENDS ${project_goby_interfaces} ${deployment}
    COMMENT "Drawing ${gif_args_NAME} (Graphviz)"
    VERBATIM)
  add_custom_command(
    OUTPUT ${base}.pdf
    COMMAND dot -Tpdf -o ${base}.pdf ${base}.dot
    DEPENDS ${base}.dot
    VERBATIM)
  set(outputs ${base}.pdf)

  if(build_interface_figures_d2)
    add_custom_command(
      OUTPUT ${base}.d2
      COMMAND ${visualize} --format D2 --output_file ${base}.d2
      DEPENDS ${project_goby_interfaces} ${deployment}
      COMMENT "Drawing ${gif_args_NAME} (D2)"
      VERBATIM)
    list(APPEND outputs ${base}.d2)
    if(D2_EXECUTABLE)
      add_custom_command(
        OUTPUT ${base}.svg
        COMMAND ${D2_EXECUTABLE} ${base}.d2 ${base}.svg
        DEPENDS ${base}.d2
        VERBATIM)
      list(APPEND outputs ${base}.svg)
    endif()
  endif()

  add_custom_target(${gif_args_NAME}_interface_figure ALL DEPENDS ${outputs})
endfunction()
