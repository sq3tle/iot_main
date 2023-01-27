FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/main/*.*)
idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS ${app_sources})
