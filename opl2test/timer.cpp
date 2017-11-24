#ifndef __DJGPP
#include <i86.h>
#else
#include <cstdlib>
#include <sys/farptr.h>
#include <conio.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <time.h>
#include <unistd.h>

#define LOCK_VARIABLE(x)    _go32_dpmi_lock_data((void *)&x,(long)sizeof(x));
#define LOCK_FUNCTION(x)    _go32_dpmi_lock_code(x,(long)sizeof(x));

#define TIMER 0x1C
 _go32_dpmi_seginfo OldISR, NewISR;

#endif

#include <dos.h>
#include <conio.h>

#ifndef __DJGPP
static void (__interrupt __far *prev_timer_handler)();
#endif

static volatile unsigned long timer_ticks;
static unsigned timer_counter;
static unsigned timer_sum;

#ifndef __DJGPP
void __interrupt __far timer_handler()
#else
void timer_handler()
#endif
{
    unsigned old_sum = timer_sum;

    ++timer_ticks;

    timer_sum += timer_counter;


    if (timer_sum < old_sum) {
#ifndef __DJGPP
    _chain_intr(prev_timer_handler);
#else
        _go32_dpmi_chain_protected_mode_interrupt_vector(TIMER,&OldISR);
#endif
  } else {
    outp(0x20, 0x20);
  }
}

void timer_setup(unsigned frequency)
{
  timer_ticks = 0;
  timer_counter = 0x1234DD / frequency;
  timer_sum = 0;

#ifndef __DJGPP
  prev_timer_handler = _dos_getvect(0x1C);
  _dos_setvect(0x1C, timer_handler);

  _disable();
  outp(0x43, 0x34);
  outp(0x40, timer_counter & 256);
  outp(0x40, timer_counter >> 8);
  _enable();
#else
    LOCK_FUNCTION(timer_handler);
    LOCK_VARIABLE(timer_ticks);
    LOCK_VARIABLE(timer_sum);

  _go32_dpmi_get_protected_mode_interrupt_vector(TIMER, &OldISR);
  NewISR.pm_offset = (int)timer_handler;
  NewISR.pm_selector = _go32_my_cs();
 _go32_dpmi_chain_protected_mode_interrupt_vector(TIMER,&NewISR);

  disable();
  outp(0x43, 0x34);
  outp(0x40, timer_counter & 256);
  outp(0x40, timer_counter >> 8);
  enable();
#endif
}

void timer_shutdown()
{
#ifndef __DJGPP
  _disable();
  outp(0x43, 0x34);
  outp(0x40, 0);
  outp(0x40, 0);
  _enable();

  _dos_setvect(0x1C, prev_timer_handler);
#else
    _go32_dpmi_set_protected_mode_interrupt_vector(TIMER,&OldISR);
#endif
}

unsigned long timer_get() {
  unsigned long result = 0;
#ifndef __DJGPP
  _disable();
  result = timer_ticks;
  _enable();
#else
    disable();
    result = timer_ticks;
    enable();
#endif
  return result;
}

#ifdef __DJGPP
void hlt() {
}
#endif