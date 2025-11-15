#ifndef MOTION_CONTROL_H
#define MOTION_CONTROL_H

#include <stdint.h>

extern struct display_control display_cntrl;


#define motionControl_indicator_pin PB3
#define motionControl_indicator_port PORTB
#define motionControl_ddr DDRB

void motionControl_init(void);
void motionControl_toggleIndicator(void);

void init_timer1(void);

void analog_comp_init(void);

#endif