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

# usage: goby_visualize_interfaces(TARGET_OUT <out_var> YML_DIR <yml_dir> DEPLOYMENT_YAML <yaml> IMAGE_OUT <image> [PARAMETERS <params>] [DEPENDENCIES <dep1> <dep2> ...])
# generates PDF graphviz graph from deployment yaml file
# TARGET_OUT: variable name to store output target name
# YML_DIR: directory to YML interface files
# DEPLOYMENT_YAML: deployment file
# IMAGE_OUT: output image path
# PARAMETERS: extra parameters for goby_clang_tool (optional)
# DEPENDENCIES: YML dependencies (list of targets from GOBY_EXPORT_INTERFACE())
function(GOBY_VISUALIZE_INTERFACES)
  cmake_parse_arguments(args "" "TARGET_OUT;YML_DIR;DEPLOYMENT_YAML;IMAGE_OUT;PARAMETERS" "DEPENDENCIES" ${ARGN})

  get_filename_component(ABS_IMAGE_OUT ${args_IMAGE_OUT} ABSOLUTE)
  get_filename_component(OUT_DIR ${ABS_IMAGE_OUT} DIRECTORY)
  get_filename_component(IMAGE_EXT ${ABS_IMAGE_OUT} EXT)

  # .pdf -> pdf
  string(SUBSTRING ${IMAGE_EXT} 1 -1 IMAGE_TYPE)
  
  get_filename_component(ABS_DEPLOYMENT_YAML ${args_DEPLOYMENT_YAML} ABSOLUTE)

  set(PARAMETERS_SUFFIX)
  if(args_PARAMETERS)
    string(REGEX REPLACE "[ -]" "_" PARAMETERS_SUFFIX ${args_PARAMETERS})
  endif()
  
  file(MAKE_DIRECTORY ${OUT_DIR})
  get_filename_component(DEPLOYMENT_NAME_FROM_FILENAME ${ABS_DEPLOYMENT_YAML} NAME_WE)
  set(DOT_OUT_FILE "${DEPLOYMENT_NAME_FROM_FILENAME}${PARAMETERS_SUFFIX}_${IMAGE_TYPE}.dot")
  set(ABS_DOT_OUT "${OUT_DIR}/${DOT_OUT_FILE}")
  
  add_custom_command(
    OUTPUT ${ABS_DOT_OUT}
    COMMAND goby_clang_tool
    ARGS -viz -outdir ${OUT_DIR} -o ${DOT_OUT_FILE} -p ${CMAKE_BINARY_DIR} ${ABS_DEPLOYMENT_YAML} ${args_PARAMETERS}
    DEPENDS ${args_DEPENDENCIES} ${ABS_DEPLOYMENT_YAML}
    WORKING_DIRECTORY ${args_YML_DIR}
    )

  set_source_files_properties(${ABS_DOT_OUT} PROPERTIES GENERATED TRUE)
  
  add_custom_command(
    OUTPUT ${ABS_IMAGE_OUT}
    COMMAND dot
    ARGS -T${IMAGE_TYPE} -o ${ABS_IMAGE_OUT} ${ABS_DOT_OUT}  
    DEPENDS ${ABS_DOT_OUT}
    )

  set_source_files_properties(${ABS_IMAGE_OUT} PROPERTIES GENERATED TRUE)

  set(LOCAL_TARGET_OUT ${DEPLOYMENT_NAME_FROM_FILENAME}${PARAMETERS_SUFFIX}_interface_viz_${IMAGE_TYPE})
  add_custom_target(${LOCAL_TARGET_OUT} ALL DEPENDS ${ABS_IMAGE_OUT})
  set(${args_TARGET_OUT} ${LOCAL_TARGET_OUT} PARENT_SCOPE)
endfunction()


macro(generate_interfaces) 
  cmake_parse_arguments(gi_args "" "TARGET" "" ${ARGN})
  goby_export_interface(TARGET ${gi_args_TARGET} YML_OUT_DIR ${YML_OUT_DIR} YML ${gi_args_TARGET}_YML_OUT)
  add_custom_target(${gi_args_TARGET}_interface ALL DEPENDS ${${gi_args_TARGET}_YML_OUT})
  set(project_goby_interfaces "${project_goby_interfaces};${gi_args_TARGET}_interface;${gi_args_TARGET}" CACHE INTERNAL "Goby Interface YMLS")
endmacro()
    
macro(generate_interfaces_figure) 
  cmake_parse_arguments(gif_args "" "INTERFACE_YML;YML_OUT_DIR;OUTPUT_IMAGE;PARAMETERS" "" ${ARGN})
  get_filename_component(INTERFACE_YML_NAME_WE ${gif_args_INTERFACE_YML} NAME_WE)
  set(CONFIGURED_INTERFACE_YML ${gif_args_YML_OUT_DIR}/figures/${INTERFACE_YML_NAME_WE}.yml)
  configure_file(${gif_args_INTERFACE_YML} ${CONFIGURED_INTERFACE_YML} @ONLY)
  goby_visualize_interfaces(
    TARGET_OUT TARGET_OUT
    YML_DIR ${gif_args_YML_OUT_DIR}
    DEPLOYMENT_YAML ${CONFIGURED_INTERFACE_YML}
    IMAGE_OUT ${gif_args_YML_OUT_DIR}/figures/${gif_args_OUTPUT_IMAGE}
    PARAMETERS "${gif_args_PARAMETERS}"
    DEPENDENCIES ${project_goby_interfaces})
  set(project_goby_interfaces_figures "${project_goby_interfaces_figures};${TARGET_OUT}" CACHE INTERNAL "Goby Interface Figures")
endmacro()
