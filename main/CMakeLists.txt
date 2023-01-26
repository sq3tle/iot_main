FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/main/*.*)
idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS ${app_sources} EMBED_TXTFILES ${project_dir}/lets_encrypt_cert.pem)
