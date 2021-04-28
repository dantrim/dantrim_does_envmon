add_executable(envmon_setup)
target_sources(envmon_setup PRIVATE envmon_setup.cpp)
target_link_libraries(envmon_setup PRIVATE Utils DevCom)
