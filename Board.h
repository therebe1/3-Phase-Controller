/* 
 * File:   Board.h
 * Author: bztacklind
 *
 * Created on March 25, 2014, 12:46 PM
 */

#ifndef BOARD_H
#define	BOARD_H

#include <AVR++/IOpin.h>
#include "Config.h"
#include <avr/io.h>

namespace ThreePhaseControllerNamespace {

using namespace AVR;

namespace Board {
    
  using namespace Basic;
#ifdef BED_CONTROLLER
 using Sw1 = Input<Ports::F, 1>; // start switch pin
 using LED_L = Output<Ports::F, 4>; // On breakout
 using LED_TX = Output<Ports::D, 5>; // On Simple Controller
 // LED_RX is SPI_SS (PB0)
 using LED = LED_L;
#endif

#ifdef QUANTUM_DRIVE
 using LED = Output<Ports::B, 7>;
#endif

 using MagSel = Output<Ports::B, 0, true>;

#ifdef QUANTUM_DRIVE
 using H1 = Input<Ports::D, 0>; // INT0
 using H2 = Input<Ports::D, 1>; // INT1
 using H3 = Input<Ports::B, 4>; // PCINT4
#endif

#ifdef BED_CONTROLLER
 using H1 = Input<Ports::E, 6, false, true>; // INT6
 using H2 = Input<Ports::B, 7, false, true>; // PCINT7
 using H3 = Input<Ports::B, 4, false, true>; // PCINT4
#endif
 
 namespace SPI {
  using SCLK = Output<Ports::B, 1>;
  using MOSI = Output<Ports::B, 2>;
  using MISO = Input<Ports::B, 3>;

  inline void slaveDeselect() __attribute__((always_inline, hot));
  inline void slaveSelect() __attribute__((always_inline, hot));
  inline void slaveDeselect() {MagSel::off();}
  inline void slaveSelect  () {MagSel::on();}

  /**
   * Check if we're still talking on the SPI bus
   * @return 
   */
  inline bool isSlaveSelected() {
   return MagSel::isOn();
  }
 };
 
 namespace SER {
  using Rx = Input<Ports::D, 2>;
  using Tx = Output<Ports::D, 3>;
 };
 
 namespace DRV {
  using AH = Output<Ports::C, 7>;
  using AL = Output<Ports::C, 6>;
  using BH = Output<Ports::B, 6>;
  using BL = Output<Ports::B, 5>;
  using CH = Output<Ports::D, 7>;
  using CL = Output<Ports::D, 6>;
 };
 
#ifdef QUANTUM_DRIVE
 namespace SEN {
  using AS = Input<Ports::F, 4, false>; // ADC4
  using BS = Input<Ports::F, 7, false>; // ADC7
  using CS = Input<Ports::D, 4, false>; // ADC8
  using VBATS = Input<Ports::F, 5, false>; // ADC5
 };
 
 namespace MUX {
  constexpr u1 AS = 4; // ADC4
  constexpr u1 BS = 7; // ADC7
  constexpr u1 CS = 0b100000; // ADC8
  constexpr u1 VBATS = 5; // ADC5
 }
#endif

#ifdef BED_CONTROLLER
 namespace SEN {
  using VBATS = Input<Ports::D, 4, false>; // ADC8
 };

 namespace MUX {
  constexpr u1 VBATS = 0b100000; // ADC8
 }
#endif
 
 constexpr u4 ClockFrequency = F_CPU;
};

};

#endif	/* BOARD_H */
