//
// Created by NgHuyHai on 8/12/2024.
//
#include <stdint.h>
#include <stddef.h>
#include "sm_sv_bt.h"

#include <stdio.h>

#include "sm_elapsed_timer.h"

#define SM_SV_BTN_TIME_DEBOUNCE                         10UL //ms

static void bt_reset_data(sm_sv_bt_t *_this, uint8_t _bt_id);

struct sm_bt{
    sm_bt_if            m_get_state;
    sm_sv_bt_config_t   m_cfg;
    SM_SV_BT_STATE      m_state;
    uint32_t            m_time_pressed;
    uint32_t            m_time_released;
};

struct sm_bt g_btns[SM_SV_BT_MAX_BT_NUM];

typedef struct {
    struct sm_bt *m_btns;
    uint8_t     m_btn_num;
    struct event_cb {
        sm_sv_bt_event_callback_fn_t    m_fn;
        void                            *m_arg;
    } m_event_cb[SM_SV_BT_MAX_BT_NUM];
    elapsed_timer_t     m_time_debounce;
} sm_sv_bt_impl_t;

sm_sv_bt_impl_t g_sv_btn;

sm_sv_bt_config_t g_cfg_default = {
    .m_tapping_time = 7*SM_SV_BTN_TIME_DEBOUNCE,
    .m_holding_time = 100*SM_SV_BTN_TIME_DEBOUNCE,
    .m_double_tapping_time = 0,
    .m_margin_time = 2*SM_SV_BTN_TIME_DEBOUNCE,
};
#define _impl(x) ((sm_sv_bt_impl_t*)(x))

sm_sv_bt_t* sm_sv_bt_create(sm_bt_if _bsp_bt_if[], uint8_t _button_num) {
    if (!_button_num || _button_num > SM_SV_BT_MAX_BT_NUM || _bsp_bt_if == NULL)
         return NULL;
    g_sv_btn.m_btns = g_btns;
    g_sv_btn.m_btn_num = _button_num;
    for (int i=0; i< _button_num; i++) {
        g_sv_btn.m_btns[i].m_get_state = _bsp_bt_if[i];
        bt_reset_data(&g_sv_btn, i);
    }

    elapsed_timer_resetz(&g_sv_btn.m_time_debounce, SM_SV_BTN_TIME_DEBOUNCE);
    return (sm_sv_bt_t*)&g_sv_btn;
}

int32_t sm_sv_bt_destroy(sm_sv_bt_t *_this) {
    return 0;
}

int32_t sm_sv_bt_reg_event(sm_sv_bt_t *_this,
                        uint8_t _button_id,
                        sm_sv_bt_event_callback_fn_t _fn_callback,
                        void *_arg) {
    if (!_this || _button_id > SM_SV_BT_MAX_BT_NUM) return -1;
    _impl(_this)->m_event_cb[_button_id].m_fn = _fn_callback;
    _impl(_this)->m_event_cb[_button_id].m_arg = _arg;
    return 0;
}

void sm_sv_bt_set_common_config(sm_sv_bt_t *_this, sm_sv_bt_config_t *_bt_config) {
    if (!_this || !_bt_config) return;
    for (int i=0; i<_impl(_this)->m_btn_num; i++) {
        _impl(_this)->m_btns[i].m_cfg.m_holding_time           = _bt_config->m_holding_time;
        _impl(_this)->m_btns[i].m_cfg.m_tapping_time           = _bt_config->m_tapping_time;
        _impl(_this)->m_btns[i].m_cfg.m_double_tapping_time    = _bt_config->m_double_tapping_time;
        _impl(_this)->m_btns[i].m_cfg.m_margin_time            = _bt_config->m_margin_time;
    }
}
void sm_sv_bt_set_config(sm_sv_bt_t *_this, uint8_t _button_id, sm_sv_bt_config_t *_bt_config) {
    if (!_this || !_bt_config || _button_id > _impl(_this)->m_btn_num) return;
    _impl(_this)->m_btns[_button_id].m_cfg.m_holding_time           = _bt_config->m_holding_time;
    _impl(_this)->m_btns[_button_id].m_cfg.m_tapping_time           = _bt_config->m_tapping_time;
    _impl(_this)->m_btns[_button_id].m_cfg.m_double_tapping_time    = _bt_config->m_double_tapping_time;
    _impl(_this)->m_btns[_button_id].m_cfg.m_margin_time            = _bt_config->m_margin_time;
}
void sm_sv_bt_get_config(sm_sv_bt_t *_this, bool _is_common, uint8_t _button_id, sm_sv_bt_config_t *_bt_config) {
    if (!_this || _button_id > _impl(_this)->m_btn_num) return;
    if (_is_common) {

    }else {

    }
}
void sm_sv_bt_process(sm_sv_bt_t *_this) {
    if (!_this)
        return;

    struct sm_bt *p_bt = NULL;
    if (!elapsed_timer_get_remain(&_impl(_this)->m_time_debounce)) {
        for (int i=0 ;i< _impl(_this)->m_btn_num; i++) {
            p_bt = &_impl(_this)->m_btns[i];
            p_bt->m_state = p_bt->m_get_state();
            // printf("button state: %d\n",  p_bt->m_state);
            if (p_bt->m_state == SM_SV_BT_STATE_PRESSED) {
                p_bt->m_time_pressed += SM_SV_BTN_TIME_DEBOUNCE;
                if (p_bt->m_time_pressed > p_bt->m_cfg.m_holding_time && p_bt->m_time_released == p_bt->m_cfg.m_margin_time){
                    if (_impl(_this)->m_event_cb[i].m_fn)
                        _impl(_this)->m_event_cb[i].m_fn(i, SM_SV_BT_EVENT_HOLD, _impl(_this)->m_event_cb[i].m_arg);
                    p_bt->m_time_released = 0;
                }
            }else{
                p_bt->m_time_released += SM_SV_BTN_TIME_DEBOUNCE;
                if (p_bt->m_time_released >= p_bt->m_cfg.m_margin_time) {
                    p_bt->m_time_released = p_bt->m_cfg.m_margin_time;
                    if (p_bt->m_time_pressed >= p_bt->m_cfg.m_tapping_time && p_bt->m_time_pressed <= p_bt->m_cfg.m_holding_time) {
                        if (_impl(_this)->m_event_cb[i].m_fn)
                            _impl(_this)->m_event_cb[i].m_fn(i, SM_SV_BT_EVENT_TAP, _impl(_this)->m_event_cb[i].m_arg);
                    }
                    p_bt->m_time_pressed = 0;
                }
            }
        }

        elapsed_timer_reset(&_impl(_this)->m_time_debounce);
    }

}

static void bt_scan_state(sm_sv_bt_t *_this);
static void bt_reset_data(sm_sv_bt_t *_this, uint8_t _bt_id) {
    _impl(_this)->m_btns[_bt_id].m_state = SM_SV_BT_STATE_RELEASED;
    _impl(_this)->m_btns[_bt_id].m_cfg.m_holding_time           = g_cfg_default.m_holding_time;
    _impl(_this)->m_btns[_bt_id].m_cfg.m_tapping_time           = g_cfg_default.m_tapping_time;
    _impl(_this)->m_btns[_bt_id].m_cfg.m_double_tapping_time    = g_cfg_default.m_double_tapping_time;
    _impl(_this)->m_btns[_bt_id].m_cfg.m_margin_time            = g_cfg_default.m_margin_time;
    _impl(_this)->m_btns[_bt_id].m_time_pressed = 0;
    _impl(_this)->m_btns[_bt_id].m_time_released = 0;
}