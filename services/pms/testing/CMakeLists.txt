cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 99)
# Set project name
project(pms_testing)

add_executable(pms_testing
        testing.c
        porting/sm_slave_nodeid_controller.c
        porting/sm_slave_nodeid_controller.h
)

find_package(Threads REQUIRED)
target_link_libraries(pms_testing Threads::Threads)
target_link_libraries(pms_testing sm_sv_pms)
#target_link_libraries(pms_testing sm_sv_charger)
