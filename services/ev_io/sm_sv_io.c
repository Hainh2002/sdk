#include "sm_sv_io.h"
#include "sm_logger.h"

#define TAG "SM_EV_IO"

const uint32_t sys_tick_ms = 10;

typedef enum {
    IN_KEY_ID = 0,
    IN_LEFT_LIGHT_ID,
    IN_RIGHT_LIGHT_ID,
    IN_PHARE_LIGHT_ID,
    IN_COS_LIGHT_ID,
    IN_PARKING_ID,
    IN_DRIVING_MODE_ID,
    IN_REVERSE_ID,
    IN_HORN_ID,
    IN_LEFT_BREAK_ID,
    IN_RIGHT_BREAK_ID,
    OUT_LEFT_LIGHT_ID,
    OUT_RIGHT_LIGHT_ID,
    OUT_PHARE_LIGHT_ID,
    OUT_COS_LIGHT_ID,
    OUT_HORN_ID,
    OUT_TAIL_LIGHT_ID,
} IO_INDEX;

typedef enum {
    RELEASED_OFF = 0,   // input, output
    RELEASED_ON,        // input, output
    FORCED_OFF,         // output
    FORCED_ON,          // output
    FORCED_BLINK        // output
} IO_STATE;

struct blink_cnt_t {
    uint64_t m_cnt;
    uint32_t m_dur;
    uint32_t m_per;
};

typedef struct{
    sm_sv_io_if_t *m_if;
    uint8_t m_io_st[SM_SV_IO_MAX_IO_NUM];
    sm_sv_io_event_cb_fn_t event_cb_fn_t;
    void* m_event_arg;
}sm_sv_io;

#define impl(x) ((sm_sv_io*)(x))

static void get_input_process(sm_sv_io_t *);
static void set_output_process(sm_sv_io_t *);
static void blink(void (*_set_out_put_fn_t)(uint8_t), uint32_t, uint32_t, uint64_t*);
static void right_light_process(sm_sv_io_t*);
static void left_light_process(sm_sv_io_t*);
static void phare_light_process(sm_sv_io_t*);
static void cos_light_process(sm_sv_io_t*);
static void tail_light_process(sm_sv_io_t*);
static void horn_process(sm_sv_io_t*);
static void reverse_mode_process(sm_sv_io_t *);
static void reset_io(sm_sv_io_t *_this);

static struct blink_cnt_t left_light_blink_cnt;
static struct blink_cnt_t right_light_blink_cnt;
static struct blink_cnt_t horn_blink_cnt;

sm_sv_io_t* sm_sv_io_create(void* _if){
    sm_sv_io* _this = malloc(sizeof(sm_sv_io));
    if (_this == NULL)
        return NULL;
    for (int i = 0; i < SM_SV_IO_MAX_IO_NUM; ++i) {
        _this->m_io_st[i] = 0;
    }
    _this->m_if = (sm_sv_io_if_t*) _if;
    _this->event_cb_fn_t = NULL;
    _this->m_event_arg = NULL;

    LOG_DBG(TAG, "Created IO service SUCCESS");

    return (sm_sv_io_t*) _this;
}

int32_t sm_sv_io_destroy(sm_sv_io_t* _this){
    if (_this == NULL)
        return -1;
    for (int i = 0; i < SM_SV_IO_MAX_IO_NUM; ++i) {
        impl(_this)->m_io_st[i] = 0;
    }
    impl(_this)->m_if = NULL;
    impl(_this)->event_cb_fn_t = NULL;
    impl(_this)->m_event_arg = NULL;
    free(_this);
    _this = NULL;
    return 0;
}
int32_t sm_sv_io_reg_event(sm_sv_io_t *_this, sm_sv_io_event_cb_fn_t _cb_fn, void *_arg){
    impl(_this)->event_cb_fn_t = _cb_fn;
    impl(_this)->m_event_arg = _arg;
    return 0;
}
int32_t sm_sv_io_set_if(sm_sv_io_t *_this, sm_sv_io_if_t *_io_if){
    if (_this == NULL || _io_if == NULL)
        return -1;

    impl(_this)->m_if = _io_if;
    return 0;
}

uint8_t sm_sv_io_get_input_key(sm_sv_io_t *_this){
    return (uint8_t) impl(_this)->m_if->in_key_fn_t();
}
uint8_t sm_sv_io_get_input_left_light(sm_sv_io_t *_this){
    return (uint8_t) impl(_this)->m_if->in_left_light_fn_t();
}
uint8_t sm_sv_io_get_input_right_light(sm_sv_io_t *_this){
    return (uint8_t) impl(_this)->m_if->in_right_light_fn_t();
}
uint8_t sm_sv_io_get_input_phare_light(sm_sv_io_t *_this){
    return (uint8_t) impl(_this)->m_if->in_phase_light_fn_t();
}
uint8_t sm_sv_io_get_input_parking(sm_sv_io_t *_this){
    return (uint8_t) impl(_this)->m_if->in_parking_fn_t();
}
uint8_t sm_sv_io_get_input_driving_mode(sm_sv_io_t *_this){
    return (uint8_t) impl(_this)->m_if->in_driving_mode_fn_t();
}
uint8_t sm_sv_io_get_input_left_break(sm_sv_io_t *_this){
    return (uint8_t) impl(_this)->m_if->in_left_break_fn_t();
}
uint8_t sm_sv_io_get_input_right_break(sm_sv_io_t *_this){
    return (uint8_t) impl(_this)->m_if->in_right_break_fn_t();
}
uint8_t sm_sv_io_get_input_reverse(sm_sv_io_t *_this){
    return (uint8_t) impl(_this)->m_if->in_reverse_fn_t();
}
uint8_t sm_sv_io_get_input_horn(sm_sv_io_t *_this){
    return (uint8_t) impl(_this)->m_if->in_horn_fn_t();
}

void sm_sv_io_set_output_left_light(sm_sv_io_t *_this, uint8_t _value){
    if (_value == 0){
        if (impl(_this)->m_io_st[OUT_LEFT_LIGHT_ID] != FORCED_OFF)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_LEFT_CHANGED_OFF, impl(_this)->m_event_arg);
        impl(_this)->m_io_st[OUT_LEFT_LIGHT_ID] = FORCED_OFF;
    }
    else{
        if (impl(_this)->m_io_st[OUT_LEFT_LIGHT_ID] != FORCED_ON)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_LEFT_CHANGED_ON, impl(_this)->m_event_arg);
        impl(_this)->m_io_st[OUT_LEFT_LIGHT_ID] = FORCED_ON;
    }
}
void sm_sv_io_release_output_left_light(sm_sv_io_t *_this){
    impl(_this)->m_if->out_left_light_fn_t(0);
    impl(_this)->m_io_st[OUT_LEFT_LIGHT_ID] = RELEASED_OFF;
}

void sm_sv_io_set_output_right_light(sm_sv_io_t *_this, uint8_t _value){
    if (_value == 0)
        impl(_this)->m_io_st[OUT_RIGHT_LIGHT_ID] = FORCED_OFF;
    else
        impl(_this)->m_io_st[OUT_RIGHT_LIGHT_ID] = FORCED_ON;
    impl(_this)->m_if->out_right_light_fn_t(_value);
}
void sm_sv_io_release_output_right_light(sm_sv_io_t *_this){
    impl(_this)->m_if->out_right_light_fn_t(0);
    impl(_this)->m_io_st[OUT_RIGHT_LIGHT_ID] = RELEASED_OFF;
}

void sm_sv_io_set_output_phare_light(sm_sv_io_t *_this, uint8_t _value){
    if (_value == 0)
        impl(_this)->m_io_st[OUT_PHARE_LIGHT_ID] = FORCED_OFF;
    else
        impl(_this)->m_io_st[OUT_PHARE_LIGHT_ID] = FORCED_ON;
    impl(_this)->m_if->out_phase_light_fn_t(_value);
}
void sm_sv_io_release_output_phare_light(sm_sv_io_t *_this){
    impl(_this)->m_if->out_phase_light_fn_t(0);
    impl(_this)->m_io_st[OUT_PHARE_LIGHT_ID] = RELEASED_OFF;
}

void sm_sv_io_set_output_horn(sm_sv_io_t *_this, uint8_t _value){
    if (_value == 0)
        impl(_this)->m_io_st[OUT_HORN_ID] = FORCED_OFF;
    else
        impl(_this)->m_io_st[OUT_HORN_ID] = FORCED_ON;
    impl(_this)->m_if->out_horn_fn_t(_value);
}
void sm_sv_io_release_output_horn(sm_sv_io_t *_this){
    impl(_this)->m_if->out_horn_fn_t(0);
    impl(_this)->m_io_st[OUT_HORN_ID] = RELEASED_OFF;
}

void sm_sv_io_set_output_tail_light(sm_sv_io_t *_this, uint8_t _value){
    if (_value == 0)
        impl(_this)->m_io_st[OUT_TAIL_LIGHT_ID] = FORCED_OFF;
    else
        impl(_this)->m_io_st[OUT_TAIL_LIGHT_ID] = FORCED_ON;
    impl(_this)->m_if->out_tail_light_fn_t(_value);
}
void sm_sv_io_release_output_tail_light(sm_sv_io_t *_this, uint8_t _value){
    impl(_this)->m_if->out_tail_light_fn_t(0);
    impl(_this)->m_io_st[OUT_TAIL_LIGHT_ID] = RELEASED_OFF;
}

void sm_sv_io_set_output_code_light(sm_sv_io_t *_this, uint8_t _value){
    if (_value == 0)
        impl(_this)->m_io_st[OUT_COS_LIGHT_ID] = FORCED_OFF;
    else
        impl(_this)->m_io_st[OUT_COS_LIGHT_ID] = FORCED_ON;
    impl(_this)->m_if->out_code_light_fn_t(_value);
}
void sm_sv_io_release_output_code_light(sm_sv_io_t *_this){
    impl(_this)->m_if->out_code_light_fn_t(0);
    impl(_this)->m_io_st[OUT_COS_LIGHT_ID] = RELEASED_OFF;
}

void sm_sv_io_set_left_light_blink(sm_sv_io_t *_this, uint32_t _duration, uint32_t _period){
    if (impl(_this)->m_io_st[OUT_LEFT_LIGHT_ID] != FORCED_BLINK){
        impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_LEFT_CHANGED_BLINK, impl(_this)->m_event_arg);
        impl(_this)->m_io_st[OUT_LEFT_LIGHT_ID] = FORCED_BLINK;
        left_light_blink_cnt.m_cnt = 0;
    }
    left_light_blink_cnt.m_dur = _duration;
    left_light_blink_cnt.m_per = _period;
}
void sm_sv_io_release_left_light_blink(sm_sv_io_t *_this){
    if (impl(_this)->m_io_st[OUT_LEFT_LIGHT_ID] == FORCED_BLINK){
        impl(_this)->m_if->out_left_light_fn_t(0);
        impl(_this)->m_io_st[OUT_LEFT_LIGHT_ID] = RELEASED_OFF;
        impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_LEFT_CHANGED_OFF, impl(_this)->m_event_arg);
        left_light_blink_cnt.m_dur = 0;
        left_light_blink_cnt.m_per = 0;
    }
}

void sm_sv_io_set_right_light_blink(sm_sv_io_t *_this, uint32_t _duration, uint32_t _period){
    if (impl(_this)->m_io_st[OUT_RIGHT_LIGHT_ID] != FORCED_BLINK){
        impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_RIGHT_CHANGED_BLINK, impl(_this)->m_event_arg);
        impl(_this)->m_io_st[OUT_RIGHT_LIGHT_ID] = FORCED_BLINK;
        right_light_blink_cnt.m_cnt = 0;
    }
    right_light_blink_cnt.m_dur = _duration;
    right_light_blink_cnt.m_per  = _period;
}
void sm_sv_io_release_right_light_blink(sm_sv_io_t *_this){
    if (impl(_this)->m_io_st[OUT_RIGHT_LIGHT_ID] == FORCED_BLINK){
        impl(_this)->m_if->out_right_light_fn_t(0);
        impl(_this)->m_io_st[OUT_RIGHT_LIGHT_ID] = RELEASED_OFF;
        impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_RIGHT_CHANGED_OFF, impl(_this)->m_event_arg);
        right_light_blink_cnt.m_dur  = 0;
        right_light_blink_cnt.m_per  = 0;
    }
}

void sm_sv_io_set_horn_blink(sm_sv_io_t *_this, uint32_t _duration, uint32_t _period){
    if (impl(_this)->m_io_st[OUT_HORN_ID] != FORCED_BLINK){
        impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_HORN_CHANGED_BLINK, impl(_this)->m_event_arg);
        impl(_this)->m_io_st[OUT_HORN_ID] = FORCED_BLINK;
        horn_blink_cnt.m_cnt = 0;
    }
    horn_blink_cnt.m_dur = _duration;
    horn_blink_cnt.m_per = _period;
}
void sm_sv_io_release_output_horn_blink(sm_sv_io_t *_this){
    impl(_this)->m_if->out_horn_fn_t(0);
    impl(_this)->m_io_st[OUT_HORN_ID] = RELEASED_OFF;
    horn_blink_cnt.m_dur = 0;
    horn_blink_cnt.m_per = 0;
}

void sm_sv_io_process(sm_sv_io_t *_this){
    get_input_process(_this);
    set_output_process(_this);
}
static void get_input_process(sm_sv_io_t *_this){
    IO_STATE temp_st = RELEASED_OFF;
    /* Key */
    temp_st = impl(_this)->m_if->in_key_fn_t();
    if ((temp_st != impl(_this)->m_io_st[IN_KEY_ID]) && (impl(_this)->event_cb_fn_t != NULL)){
        if (temp_st == RELEASED_OFF)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_KEY_CHANGED_OFF,
                                        impl(_this)->m_event_arg);
        if (temp_st == RELEASED_ON)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_KEY_CHANGED_ON,
                                        impl(_this)->m_event_arg);
    }
    impl(_this)->m_io_st[IN_KEY_ID] = (uint8_t)temp_st;
    /* Left */
    temp_st = impl(_this)->m_if->in_left_light_fn_t();
    if ((temp_st != impl(_this)->m_io_st[IN_LEFT_LIGHT_ID]) && (impl(_this)->event_cb_fn_t != NULL)){
        if (temp_st == RELEASED_OFF)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_LEFT_CHANGED_OFF,
                                        impl(_this)->m_event_arg);
        if (temp_st == RELEASED_ON)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_LEFT_CHANGED_ON,
                                        impl(_this)->m_event_arg);
    }
    impl(_this)->m_io_st[IN_LEFT_LIGHT_ID] =(uint8_t)temp_st;
    /* Right */
    temp_st = impl(_this)->m_if->in_right_light_fn_t();
    if ((temp_st != impl(_this)->m_io_st[IN_RIGHT_LIGHT_ID]) && (impl(_this)->event_cb_fn_t != NULL)){
        if (temp_st == RELEASED_OFF)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_RIGHT_CHANGED_OFF,
                                        impl(_this)->m_event_arg);
        if (temp_st == RELEASED_ON)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_RIGHT_CHANGED_ON,
                                        impl(_this)->m_event_arg);
    }
    impl(_this)->m_io_st[IN_RIGHT_LIGHT_ID] = (uint8_t)temp_st;
    /* Phare */
    temp_st = impl(_this)->m_if->in_phase_light_fn_t();
    if ((temp_st != impl(_this)->m_io_st[IN_PHARE_LIGHT_ID]) && (impl(_this)->event_cb_fn_t != NULL)){
        if (temp_st == RELEASED_OFF)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_PHASE_CHANGED_OFF,
                                       impl(_this)->m_event_arg);
        if (temp_st == RELEASED_ON)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_PHASE_CHANGED_ON,
                                        impl(_this)->m_event_arg);
    }
    impl(_this)->m_io_st[IN_PHARE_LIGHT_ID] = (uint8_t)temp_st;
    /* Cos */
    temp_st = impl(_this)->m_if->in_cos_light_fn_t();
    if ((temp_st != impl(_this)->m_io_st[IN_COS_LIGHT_ID]) && (impl(_this)->event_cb_fn_t != NULL)){
        if (temp_st == RELEASED_OFF)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_COS_CHANGED_OFF,
                                        impl(_this)->m_event_arg);
        if (temp_st == RELEASED_ON)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_COS_CHANGED_ON,
                                        impl(_this)->m_event_arg);
    }
    impl(_this)->m_io_st[IN_COS_LIGHT_ID] = (uint8_t)temp_st;
    /* Horn */
    temp_st = impl(_this)->m_if->in_horn_fn_t();
    if ((temp_st != impl(_this)->m_io_st[IN_HORN_ID]) && (impl(_this)->event_cb_fn_t != NULL)){
        if (temp_st == RELEASED_OFF)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_HORN_CHANGED_OFF,
                                       impl(_this)->m_event_arg);
        if (temp_st == RELEASED_ON)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_HORN_CHANGED_ON,
                                       impl(_this)->m_event_arg);
    }
    impl(_this)->m_io_st[IN_HORN_ID] = (uint8_t)temp_st;
    /* Parking */
    temp_st = impl(_this)->m_if->in_parking_fn_t();
    if ((temp_st != impl(_this)->m_io_st[IN_PARKING_ID]) && (impl(_this)->event_cb_fn_t != NULL)){
        if (temp_st == RELEASED_ON)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_PARKING_PRESSED,
                                       impl(_this)->m_event_arg);
    }
    impl(_this)->m_io_st[IN_PARKING_ID] = (uint8_t) temp_st;
    /* Driving */
    temp_st = impl(_this)->m_if->in_driving_mode_fn_t();
    if ((temp_st != impl(_this)->m_io_st[IN_DRIVING_MODE_ID]) && (impl(_this)->event_cb_fn_t != NULL)){
        if (temp_st == RELEASED_ON)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_DRV_MODE_PRESSED,
                                       impl(_this)->m_event_arg);
    }
    impl(_this)->m_io_st[IN_DRIVING_MODE_ID] = (uint8_t) temp_st;

    /* Left Break */
    impl(_this)->m_io_st[IN_LEFT_BREAK_ID] = impl(_this)->m_if->in_left_break_fn_t();
    /* Right Break */
    impl(_this)->m_io_st[IN_RIGHT_BREAK_ID] = impl(_this)->m_if->in_right_break_fn_t();
    /* Reverse */
    temp_st = impl(_this)->m_if->in_reverse_fn_t();
    if ((temp_st != impl(_this)->m_io_st[IN_REVERSE_ID]) && (impl(_this)->event_cb_fn_t != NULL)){
        if (temp_st == RELEASED_OFF)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_REVERSE_CHANGED_OFF,
                                        impl(_this)->m_event_arg);
        if (temp_st == RELEASED_ON)
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_IN_REVERSE_CHANGED_ON,
                                        impl(_this)->m_event_arg);
    }
    impl(_this)->m_io_st[IN_REVERSE_ID] = (uint8_t)temp_st;
}
static void set_output_process(sm_sv_io_t *_this){
    if (impl(_this)->m_io_st[IN_KEY_ID] == RELEASED_OFF){  /* key off */
        reset_io(_this);
        return;
    }
    else{ /* key on */
        reverse_mode_process(_this);
        left_light_process(_this);
        right_light_process(_this);
        phare_light_process(_this);
        cos_light_process(_this);
        tail_light_process(_this);
        horn_process(_this);
    }
}
static void blink(void (*_set_out_put_fn_t)(uint8_t), uint32_t _duration, uint32_t _period, uint64_t *_blink_cnt){
    (*_blink_cnt) += sys_tick_ms;
    if ((*_blink_cnt) <= _duration)
        _set_out_put_fn_t(1);
    if ((*_blink_cnt) > _duration)
        _set_out_put_fn_t(0);
    if ((*_blink_cnt) > _period) {
//       _set_out_put_fn_t(1);
        (*_blink_cnt) = 0;
    }
}
static void left_light_process(sm_sv_io_t *_this){
    IO_STATE temp_st_in = impl(_this)->m_io_st[IN_LEFT_LIGHT_ID];
    IO_STATE temp_st_out = impl(_this)->m_io_st[OUT_LEFT_LIGHT_ID];

    switch (temp_st_out){
    case FORCED_BLINK:
        blink( impl(_this)->m_if->out_left_light_fn_t,
               left_light_blink_cnt.m_dur,
               left_light_blink_cnt.m_per,
               &left_light_blink_cnt.m_cnt);
        break;
    case FORCED_ON:
        impl(_this)->m_if->out_left_light_fn_t(1);
        break;
    case FORCED_OFF:
        impl(_this)->m_if->out_left_light_fn_t(0);
        break;
    case RELEASED_OFF:
        impl(_this)->m_if->out_left_light_fn_t(0);
        if (temp_st_in == RELEASED_ON){
            impl(_this)->m_io_st[OUT_LEFT_LIGHT_ID] = RELEASED_ON;
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_LEFT_CHANGED_BLINK, impl(_this)->m_event_arg);
        }
            left_light_blink_cnt.m_cnt = 0;
        break;
    case RELEASED_ON:
        blink( impl(_this)->m_if->out_left_light_fn_t,
               left_light_blink_cnt.m_dur,
               left_light_blink_cnt.m_per,
               &left_light_blink_cnt.m_cnt);
        if (temp_st_in == RELEASED_OFF){
            impl(_this)->m_io_st[OUT_LEFT_LIGHT_ID] = RELEASED_OFF;
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_LEFT_CHANGED_OFF, impl(_this)->m_event_arg);
        }
        break;
    default:
        break;
    }
}
static void right_light_process(sm_sv_io_t *_this){
    IO_STATE temp_st_in = impl(_this)->m_io_st[IN_RIGHT_LIGHT_ID];
    IO_STATE temp_st_out = impl(_this)->m_io_st[OUT_RIGHT_LIGHT_ID];

    switch (temp_st_out){
    case FORCED_BLINK:
        blink( impl(_this)->m_if->out_right_light_fn_t,
               SM_SV_IO_BLINK_DUR_MS,
               SM_SV_IO_BLINK_PER_MS,
               &right_light_blink_cnt.m_cnt);
        break;
    case FORCED_ON:
        impl(_this)->m_if->out_right_light_fn_t(1);
        break;
    case FORCED_OFF:
        impl(_this)->m_if->out_right_light_fn_t(0);
        break;
    case RELEASED_OFF:
        impl(_this)->m_if->out_right_light_fn_t(0);
        if (temp_st_in == RELEASED_ON){
            impl(_this)->m_io_st[OUT_RIGHT_LIGHT_ID] = RELEASED_ON;
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_RIGHT_CHANGED_BLINK, impl(_this)->m_event_arg);
        }
        break;
    case RELEASED_ON:
        blink( impl(_this)->m_if->out_right_light_fn_t,
               SM_SV_IO_BLINK_DUR_MS,
               SM_SV_IO_BLINK_PER_MS,
               &right_light_blink_cnt.m_cnt);
        if (temp_st_in == RELEASED_OFF){
            impl(_this)->m_io_st[OUT_RIGHT_LIGHT_ID] = RELEASED_OFF;
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_RIGHT_CHANGED_OFF, impl(_this)->m_event_arg);
        }
        break;
    default:
        break;
    }
}
static void phare_light_process(sm_sv_io_t *_this){
    IO_STATE temp_st_in = impl(_this)->m_io_st[IN_PHARE_LIGHT_ID];
    IO_STATE temp_st_out = impl(_this)->m_io_st[OUT_PHARE_LIGHT_ID];

    switch (temp_st_out){
    case FORCED_BLINK:
//        blink( impl(_this)->m_if->out_phase_light_fn_t, right_blink_dur_ms, right_blink_dur_ms, &right_blink_cnt_ms);
        break;
    case FORCED_ON:
        impl(_this)->m_if->out_phase_light_fn_t(1);
        break;
    case FORCED_OFF:
        impl(_this)->m_if->out_phase_light_fn_t(0);
        break;
    case RELEASED_OFF:
        impl(_this)->m_if->out_phase_light_fn_t(0);
        if (temp_st_in == RELEASED_ON){
            impl(_this)->m_io_st[OUT_PHARE_LIGHT_ID] = RELEASED_ON;
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_PHASE_CHANGED_ON, impl(_this)->m_event_arg);
        }
        break;
    case RELEASED_ON:
        impl(_this)->m_if->out_phase_light_fn_t(1);
        if (temp_st_in == RELEASED_OFF){
            impl(_this)->m_io_st[OUT_PHARE_LIGHT_ID] = RELEASED_OFF;
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_PHASE_CHANGED_OFF, impl(_this)->m_event_arg);
        }
        break;
    default:
        break;
    }
}
static void cos_light_process(sm_sv_io_t *_this){
    IO_STATE temp_st_in = impl(_this)->m_io_st[IN_COS_LIGHT_ID];
    IO_STATE temp_st_out = impl(_this)->m_io_st[OUT_COS_LIGHT_ID];

    switch (temp_st_out){
    case FORCED_BLINK:
//        blink( impl(_this)->m_if->out_code_light_fn_t, right_blink_dur_ms, right_blink_dur_ms, &right_blink_cnt_ms);
        break;
    case FORCED_ON:
        impl(_this)->m_if->out_code_light_fn_t(1);
        break;
    case FORCED_OFF:
        impl(_this)->m_if->out_code_light_fn_t(0);
        break;
    case RELEASED_OFF:
        impl(_this)->m_if->out_code_light_fn_t(0);
        if (temp_st_in == RELEASED_ON){
            impl(_this)->m_io_st[OUT_COS_LIGHT_ID] = RELEASED_ON;
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_COS_CHANGED_ON, impl(_this)->m_event_arg);
        }
        break;
    case RELEASED_ON:
        impl(_this)->m_if->out_code_light_fn_t(1);
        if (temp_st_in == RELEASED_OFF){
            impl(_this)->m_io_st[OUT_COS_LIGHT_ID] = RELEASED_OFF;
            impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_COS_CHANGED_OFF, impl(_this)->m_event_arg);
        }
        break;
    default:
        break;
    }
}
static void tail_light_process(sm_sv_io_t *_this){
    // IO_STATE temp_st_in = impl(_this)->m_io_st[IN_RIGHT_LIGHT_ID];
    IO_STATE temp_st_out = impl(_this)->m_io_st[OUT_TAIL_LIGHT_ID];

    switch (temp_st_out){
    case FORCED_BLINK:
        break;
    case FORCED_ON:
        impl(_this)->m_if->out_tail_light_fn_t(1);
        break;
    case FORCED_OFF:
        impl(_this)->m_if->out_tail_light_fn_t(0);
        break;
    case RELEASED_OFF:

        break;
    case RELEASED_ON:

        break;
    default:
        break;
    }
}
static void horn_process(sm_sv_io_t *_this){
    IO_STATE temp_st_in = impl(_this)->m_io_st[IN_HORN_ID];
    IO_STATE temp_st_out = impl(_this)->m_io_st[OUT_HORN_ID];
    switch (temp_st_out){
        case FORCED_BLINK:
            blink(impl(_this)->m_if->out_horn_fn_t,
                  horn_blink_cnt.m_dur,
                  horn_blink_cnt.m_per,
                  &horn_blink_cnt.m_cnt);
            break;
        case FORCED_ON:
            impl(_this)->m_if->out_horn_fn_t(1);
            break;
        case FORCED_OFF:
            impl(_this)->m_if->out_horn_fn_t(0);
            break;
        case RELEASED_OFF:
            impl(_this)->m_if->out_horn_fn_t(0);
            if (temp_st_in == RELEASED_ON){
                impl(_this)->m_io_st[OUT_HORN_ID] = RELEASED_ON;
                impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_HORN_CHANGED_ON, impl(_this)->m_event_arg);
            }
            break;
        case RELEASED_ON:
            impl(_this)->m_if->out_horn_fn_t(1);
            if (temp_st_in == RELEASED_OFF){
                impl(_this)->m_io_st[OUT_HORN_ID] = RELEASED_OFF;
                impl(_this)->event_cb_fn_t(SM_SV_IO_EVENT_OUT_HORN_CHANGED_OFF, impl(_this)->m_event_arg);
            }
            break;
        default:
            break;
    }
}
static void reverse_mode_process(sm_sv_io_t *_this){
    IO_STATE temp_st_in = impl(_this)->m_io_st[IN_REVERSE_ID];
    if ( temp_st_in == 1){
        sm_sv_io_set_left_light_blink(_this, SM_SV_IO_BLINK_DUR_MS, SM_SV_IO_BLINK_PER_MS);
        sm_sv_io_set_right_light_blink(_this, SM_SV_IO_BLINK_DUR_MS, SM_SV_IO_BLINK_PER_MS);
    } else {
//        if (impl(_this)->m_io_st[IN_LEFT_LIGHT_ID] == 1)
            sm_sv_io_release_left_light_blink(_this);
//        if (impl(_this)->m_io_st[IN_RIGHT_LIGHT_ID] == 1)
            sm_sv_io_release_right_light_blink(_this);
    }
}
static void reset_io(sm_sv_io_t *_this){
    impl(_this)->m_if->out_left_light_fn_t(0);
    impl(_this)->m_if->out_right_light_fn_t(0);
    impl(_this)->m_if->out_phase_light_fn_t(0);
    impl(_this)->m_if->out_code_light_fn_t(0);
    impl(_this)->m_if->out_horn_fn_t(0);
    impl(_this)->m_if->out_tail_light_fn_t(0);

    for(uint8_t i=OUT_LEFT_LIGHT_ID; i<SM_SV_IO_MAX_IO_NUM; i++){
        impl(_this)->m_io_st[i] = 0;
    }
}