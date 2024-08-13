/*
 * sm_task.c
 *
 *  Created on: Jul 14, 2023
 *      Author: Admin
 */

#include <stdio.h>
#include <stdlib.h>

#include "sm_core_sch.h"
#include "sm_elapsed_timer.h"

#define _impl(p)    ((sm_scheduler_t*)(p))

typedef struct {
    int32_t m_repeat_count;
    int32_t m_interval;
    sm_sch_task_fn_t m_fn;
    void* m_arg;
}sm_sch_task_t;

typedef struct{
    struct {
        elapsed_timer_t m_timeout;
        int32_t m_count;
        sm_sch_task_t m_task;
    }m_tasks[SM_SCH_TASK_MAX_SUPPORT];
}sm_scheduler_t;

static sm_scheduler_t g_sch_default;

sm_sch_t* sm_sch_create(){
    sm_scheduler_t* sch = malloc (sizeof(sm_scheduler_t));
    if(!sch){
        return NULL;
    }
    for(int index = 0; index < SM_SCH_TASK_MAX_SUPPORT; ++index){
        sch->m_tasks->m_count = 0;
        sch->m_tasks->m_task.m_arg = NULL;
        sch->m_tasks[index].m_task.m_fn = NULL;
        sch->m_tasks[index].m_task.m_interval = 0;
        sch->m_tasks[index].m_task.m_repeat_count = 0;
    }

    return (sm_sch_t*)sch;
}

sm_sch_t* sm_sch_create_default(){
	 for(int index = 0; index < SM_SCH_TASK_MAX_SUPPORT; ++index){
		 g_sch_default.m_tasks->m_count = 0;
		 g_sch_default.m_tasks->m_task.m_arg = NULL;
		 g_sch_default.m_tasks[index].m_task.m_fn = NULL;
		 g_sch_default.m_tasks[index].m_task.m_interval = 0;
		 g_sch_default.m_tasks[index].m_task.m_repeat_count = 0;
	 }
	 return &g_sch_default;
}

int32_t sm_sch_free(sm_sch_t* _this){
    if(!_this){
        return -1;
    }
    for (int index = 0; index < SM_SCH_TASK_MAX_SUPPORT; ++index) {
        _impl(_this)->m_tasks[index].m_task.m_arg = NULL;
        _impl(_this)->m_tasks[index].m_task.m_fn = NULL;
        _impl(_this)->m_tasks[index].m_task.m_interval = 0;
        _impl(_this)->m_tasks[index].m_task.m_repeat_count = 0;
    }
}

int32_t sm_sch_start_task(sm_sch_t* _this, int32_t _interval, int32_t _repeat, sm_sch_task_fn_t _task, void* _arg){
    if(!_this){
        return -1;
    }
    for (int index = 0; index < SM_SCH_TASK_MAX_SUPPORT; ++index) {
        if(!_impl(_this)->m_tasks[index].m_task.m_fn){
            _impl(_this)->m_tasks[index].m_task.m_arg = _arg;
            _impl(_this)->m_tasks[index].m_task.m_fn = _task;
            _impl(_this)->m_tasks[index].m_task.m_interval = _interval;
            _impl(_this)->m_tasks[index].m_task.m_repeat_count = _repeat;
            _impl(_this)->m_tasks[index].m_count = 0;
            elapsed_timer_resetz(&_impl(_this)->m_tasks[index].m_timeout, _interval);
            for (index = index + 1; index < SM_SCH_TASK_MAX_SUPPORT; index++)
                if (_impl(_this)->m_tasks[index].m_task.m_fn == _task)
                    _impl(_this)->m_tasks[index].m_task.m_fn = NULL;
            return index;
        }
    }
    return -1;
}

int32_t sm_sch_cancel_task(sm_sch_t* _this,  int32_t _task_id){
    if(!_this || _task_id >= SM_SCH_TASK_MAX_SUPPORT){
        return -1;
    }
    _impl(_this)->m_tasks[_task_id].m_task.m_arg = NULL;
    _impl(_this)->m_tasks[_task_id].m_task.m_fn = NULL;
    _impl(_this)->m_tasks[_task_id].m_task.m_interval = 0;
    _impl(_this)->m_tasks[_task_id].m_task.m_repeat_count = 0;
    _impl(_this)->m_tasks[_task_id].m_count = 0;
    return 0;
}

int32_t sm_sch_cancel_taskz(sm_sch_t *_this, sm_sch_task_fn_t _fn) {
	if (!_this || !_fn) {
		return -1;
	}
	for (int index = 0; index < SM_SCH_TASK_MAX_SUPPORT; ++index) {
		if (_impl(_this)->m_tasks[index].m_task.m_fn == _fn) {
			_impl(_this)->m_tasks[index].m_task.m_fn = NULL;
			_impl(_this)->m_tasks[index].m_task.m_arg = NULL;
			_impl(_this)->m_tasks[index].m_task.m_interval = 0;
			_impl(_this)->m_tasks[index].m_task.m_repeat_count = 0;
			_impl(_this)->m_tasks[index].m_count = 0;
		}
	}
	return 0;
}

int32_t sm_sch_process(sm_sch_t* _this){
    sm_sch_task_fn_t callback_fn = NULL;

    if(!_this){
        return -1;
    }
    for (int index = 0; index < SM_SCH_TASK_MAX_SUPPORT; ++index) {
        if (!elapsed_timer_get_remain(&_impl(_this)->m_tasks[index].m_timeout) && _impl(_this)->m_tasks[index].m_task.m_fn) {
            callback_fn = _impl(_this)->m_tasks[index].m_task.m_fn;
            callback_fn(_impl(_this)->m_tasks[index].m_task.m_arg);

            if ((_impl(_this)->m_tasks[index].m_count < _impl(_this)->m_tasks[index].m_task.m_repeat_count - 1)
                || (_impl(_this)->m_tasks[index].m_task.m_repeat_count == SM_SCH_REPEAT_FOREVER)) {
                _impl(_this)->m_tasks[index].m_count++;
                elapsed_timer_reset(&_impl(_this)->m_tasks[index].m_timeout);
            }else{
                _impl(_this)->m_tasks[index].m_task.m_fn = NULL;
                _impl(_this)->m_tasks[index].m_task.m_arg = NULL;
            }
        }
    }
    return 0;
}
