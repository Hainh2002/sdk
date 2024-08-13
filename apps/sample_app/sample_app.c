#include <stdio.h>

#include "sm_logger.h"

#include "sm_bsp.h"
#include "sm_core_sch.h"

#include "sm_app.h"

#define MAIN_TAG "SAMPLE_APP"

static sm_sch_t * g_core_sch = NULL;

void sm_app_led_blink(void* _arg){
    printf("Blink LED\n");
}

int32_t sm_app_init(){
    LOG_INF(MAIN_TAG, "Initialized sample Application");
    return 0;
}

int main() {
    sm_bsp_init();

    g_core_sch = sm_sch_create_default();

    sm_app_init();

    sm_sch_start_task(g_core_sch, 200, SM_SCH_REPEAT_FOREVER, sm_app_led_blink, NULL);

    while (1){
        sm_sch_process(g_core_sch);
    }
    return 0;
}
