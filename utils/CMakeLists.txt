cmake_minimum_required(VERSION 3.16)
project(ev-sdk-utils)

set(CMAKE_C_STANDARD 99)

set(SDK_UTILS_SRC
        sm_logger.c
        sm_memory.h
        sm_string_t.c
        algorithms/sm_crc.c
        algorithms/sm_uuid.c
        algorithms/sm_intel_hex.c
        containers/sm_fifo.c
        containers/sm_link_list.c
        containers/sm_map.c
        containers/sm_stack.c
        containers/sm_vector.c
        containers/sm_vector.h
        containers/sm_queue.c
        containers/sm_queue.h
        timing/sm_elapsed_timer.h
)

add_library(sdk_utils STATIC
        ${SDK_UTILS_SRC})
