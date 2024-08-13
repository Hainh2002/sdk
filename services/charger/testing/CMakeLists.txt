cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 99)
# Set project name
project(charger_testing)
add_executable(testing_charger
        testing.c
        porting/sm_slave_nodeid_controller.c
        porting/sm_slave_nodeid_controller.h
)
target_link_libraries(testing_charger sm_sv_charger)
target_link_libraries(testing_charger pthread)