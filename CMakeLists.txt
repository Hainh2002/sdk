cmake_minimum_required(VERSION 3.22)

project(sdk)

set(CMAKE_C_STANDARD 99)

include_directories(utils
        utils/algorithms
        utils/containers
        utils/timing
        utils/unix

        libs/json-maker
        libs/tiny-json
        libs/nanoModbus

        hals
        services/button

        common/modbus
        common/modbus/mb_slave
        common/modbus/mb_master

        cores/
        cores/storage

        hals/include
)

add_subdirectory(utils)
add_subdirectory(libs/nanoModbus)

add_subdirectory(services/boot_master)
add_subdirectory(services/bp_management)
add_subdirectory(services/button)

add_subdirectory(common/modbus/mb_slave)
add_subdirectory(common/modbus/mb_master)
add_subdirectory(hals)