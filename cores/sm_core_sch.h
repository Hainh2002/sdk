/*
 * task.h
 *
 *  Created on: Jul 14, 2023
 *      Author: Admin
 */

#ifndef SM_CORE_SCH_H
#define SM_CORE_SCH_H

#include <stdint.h>

#define SM_SCH_TASK_MAX_SUPPORT			16
#define SM_SCH_REPEAT_FOREVER        	0xFFFF

typedef void sm_sch_t;

typedef void (*sm_sch_task_fn_t)(void *arg);

sm_sch_t* sm_sch_create_default();
sm_sch_t* sm_sch_create();
int32_t sm_sch_free(sm_sch_t* _this);

int32_t sm_sch_start_task(sm_sch_t* _this, int32_t _interval, int32_t _repeat, sm_sch_task_fn_t _task, void* _arg);
int32_t sm_sch_cancel_task(sm_sch_t* _this, int32_t _task_id);
int32_t sm_sch_cancel_taskz(sm_sch_t* _this, sm_sch_task_fn_t _fn);

int32_t sm_sch_process(sm_sch_t* _this);

#endif /* SM_CORE_SCH_H */
