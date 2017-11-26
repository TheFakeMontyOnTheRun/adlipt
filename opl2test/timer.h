#ifndef TIMER_H_
#define TIMER_H_

void timer_setup(unsigned short frequency);
void timer_shutdown();
unsigned long timer_get();

void hlt(void);
#pragma aux hlt = "hlt";

#endif
