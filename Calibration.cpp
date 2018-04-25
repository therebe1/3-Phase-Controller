/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Calibration.cpp
 * Author: Cameron
 * 
 * Created on April 11, 2017, 6:49 PM
 */

#include <util/delay.h>

#include "Calibration.h"
#include "ThreePhaseDriver.h"
#include "MLX90363.h"
#include "Debug.h"
#include "HallWatcher.h"

using namespace ThreePhaseControllerNamespace;
using namespace Calibration;

void Calibration::main() {
  if (!enabled) return;
  
  ThreePhaseDriver::init();
  HallWatcher::init();
  MLX90363::init();
  MLX90363::prepareGET1Message(MLX90363::MessageType::Alpha);
  ThreePhaseDriver::setAmplitude(0);
  ThreePhaseDriver::setDeadTimes({15, 15});

  auto magRoll = MLX90363::getRoll();
  
  Board::LED::on();

  do {
    MLX90363::startTransmitting();

    // Hang while transmitting
    while (MLX90363::isTransmitting());

    // Delay long enough to guarantee data is ready
    _delay_ms(2);

    // Loop until we actually receive real data
  } while (!MLX90363::hasNewData(magRoll));

  // If numberOfSpins is too large, we should get a compile time overflow error
  constexpr u2 steps = ThreePhaseDriver::StepsPerCycle * numberOfSpins;

  constexpr u1 stepSize = 1;

  using namespace Debug;

  u2 i = 0;

  for (; i < ThreePhaseDriver::StepsPerCycle; i += stepSize) {
    ThreePhaseDriver::setAmplitude(i * amplitude / rampSteps);
    step(i);
  }

  ThreePhaseDriver::setAmplitude(amplitude);
  
  for (; i < steps + ThreePhaseDriver::StepsPerCycle; i += stepSize) {
    step(i);
    // Send data via debug serial port
    SOUT
      << Printer::Special::Start
      << i << MLX90363::getAlpha() << HallWatcher::getState()
      << Printer::Special::End;
  }


  for (; i > steps; i -= stepSize) {
    step(i);
  }
  
  
  for (; i; i -= stepSize) {
    step(i);
    // Send data via debug serial port
    SOUT
      << Printer::Special::Start
      << i << MLX90363::getAlpha() << HallWatcher::getState()
      << Printer::Special::End;
  }

  ThreePhaseDriver::setAmplitude(0);
  ThreePhaseDriver::advanceTo(0);

  // Don't continue
  while (1);
}

void Calibration::step(uint16_t i) {
  // Move to next position
  ThreePhaseDriver::advanceTo(i);

  // Give the motor some time to move
  _delay_ms(2);
  // Start the ADC sample on the MLX. We're going to throw away the data from this reading
  MLX90363::startTransmitting();

  // Hang while transmitting
  while (MLX90363::isTransmitting());

  // Wait for a reading to be ready
  _delay_ms(2);
  // Record current roll value
  auto magRoll = MLX90363::getRoll();

  // Start SPI sequence
  MLX90363::startTransmitting();

  // Hang while transmitting
  while (MLX90363::isTransmitting());

  // Wait for SPI sequence to finish
  // TODO: check in case crc fails and we'd be sitting here forever
  while (!MLX90363::hasNewData(magRoll));
}