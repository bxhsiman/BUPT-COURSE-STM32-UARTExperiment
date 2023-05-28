#ifndef __TIMER_H
#define __TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

void RXtimer_process(void);
void timer_process( void );
void timer_show( void );
extern uint8_t hours, minutes , seconds;
#ifdef __cplusplus
}
#endif

#endif /* __TIMER_H */
