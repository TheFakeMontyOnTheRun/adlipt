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
long ms;
long timeToSleep = 0;
void timer_handler()
#endif
{
#ifndef __DJGPP
    unsigned old_sum = timer_sum;

    ++timer_ticks;

    timer_sum += timer_counter;

    if (timer_sum < old_sum) {
    _chain_intr(prev_timer_handler);
  } else {
    outp(0x20, 0x20);
  }
#else

    auto t0 = uclock();
    usleep( 12500 );
    auto t1 = uclock();
    ms = (t1 - t0) / UCLOCKS_PER_SEC;
    ++timer_ticks;
    timeToSleep -= ms;
    if ( timeToSleep <= 0 ) {

        timer_sum += timer_counter;
        timeToSleep = ms;
    }
#endif
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
  ms = 1000000 / frequency;
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
#endif
}

unsigned long timer_get() {
  unsigned long result = 0;
#ifndef __DJGPP
  _disable();
  result = timer_ticks;
  _enable();
#else
  result = timer_ticks;
#endif
  return result;
}

#ifdef __DJGPP
void hlt() {
  timer_handler();
}
#endif