/* 
 * File:   ThreePhaseDriver.h
 * Author: Cameron
 *
 * Created on December 17, 2014, 3:20 PM
 */

#ifndef THREEPHASEDRIVER_H
#define	THREEPHASEDRIVER_H

#include <AVR++/basicTypes.h>
#include <avr/interrupt.h>

using namespace AVR;

class ThreePhaseDriver {
 
 /**
  * Read the sin table
  * @param phase
  * @return 
  */
 static inline u2 getPhasePWM(u1 const phase) __attribute__((const));
 
 static u1 amplitude;
 
// static u1 cacheA;
// static register u1 cacheB asm("r11");
// static register u1 cacheC asm("r12");
public:
 enum class Phase : u1 {A=0, B=1, C=2, INIT};

protected:
 static Phase currentPhase;

public:
 /**
  * Internal granularity of sin wave for each phase
  */
 static u2 constexpr StepsPerPhase = 256;
 
 /**
  * One for each of A, B, and C.
  */
 static u1 constexpr PhasesPerCycle = 3;
 
 /**
  * One Cycle is one full commutation "revolution" of the motor. This is almost
  * certainly not one actual revolution of the motor shaft.
  */
 static u2 constexpr StepsPerCycle = StepsPerPhase * PhasesPerCycle;
 
 /**
  * Highest possible timer value
  */
 static u2 constexpr MAX = 0x7ff;

 static void init();
 
 /**
  * Convenience function with its own internal step counter
  */
 static void advance();
 
 /**
  * Version of advanceTo() that takes a single u2 between 0 and 0x2ff inclusive
  * @param step
  */
 inline static void advanceToFullSine  (u2 const step) {advanceToFullSine  ((Phase)(step >> 8), step);}
 inline static void advanceToFullSquare(u2 const step) {advanceToFullSquare((Phase)(step >> 8), step);}
 
 /**
  * Advance the pwm outputs to a new commutation
  * 
  * @param phase
  * @param step
  */
 static void advanceToFullSine  (Phase const phase, u1 const step);
 static void advanceToFullSquare(Phase const phase, u1 const step);
 
 static constexpr u1 maxAmplitude = 0xff - 30;
 
 static inline void setAmplitude(u1 const a) {amplitude = a > maxAmplitude ? maxAmplitude : a;}

 static void advanceToBackEMF(Phase const phase, u1 const step);
};

#endif	/* THREEPHASEDRIVER_H */

