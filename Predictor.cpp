
#include "Predictor.h"

u4 Predictor::drivePhase;
u2 Predictor::lastMagPha;
s2 Predictor::driveVelocity;

// predictor = new PositionPredictor(time_between_mag_updates)
u2 Predictor::predict(){

 u4 ph = drivePhase;
 ph += driveVelocity;
 
 const bool forward = driveVelocity > 0;
 
 const u4 MAX = ThreePhaseDriver::StepsPerCycle << drivePhaseValueShift;
 
 // Check if ph(ase) value is out of range
 if (ph > MAX) {
  // Fix it
  if (forward)
   ph -= MAX;
  else
   ph += MAX;
 }
 
 // Store new drivePhase
 drivePhase = ph;
 
 // Adjust output for velocity lag
 ph += driveVelocity * driveVelocityPhaseAdvance;
 
  // Check if ph(ase) value is out of range again
 if (ph > MAX) {
  // Fix it
  if (forward) ph -= MAX;
  else         ph += MAX;
 }
 
 return (ph >> drivePhaseValueShift);
}

void Predictor::freshPhase(u2 phase){

 
 auto tempVelocity = driveVelocity;
 
 const s2 measuredPhaseChange = phase - lastMagPha; 
 
 tempVelocity = nextVelocity(tempVelocity, measuredPhaseChange);
 
 ATOMIC_BLOCK(ATOMIC_FORCEON) {
  driveVelocity = tempVelocity;
  drivePhase = u4(phase) << drivePhaseValueShift;
 }
 
 static u1 tick = 0;

 Debug::SOUT
         << Debug::Printer::Special::Start
         << tick++
         << phase
         << Debug::Printer::Special::End;

 // Save the most recent magnetic position
 lastMagPha = phase;
 
}

s4 Predictor::nextVelocity(tempVelocity, measuredPhaseChange){

 const s2 predictedPhaseChange = (s4(tempVelocity) * cyclesPWMPerMLX) >> drivePhaseValueShift;

 //TODO make this actually reflect max acceleration
 if (measuredPhaseChange > predictedPhaseChange) {
  tempVelocity++;
 } else if (measuredPhaseChange < predictedPhaseChange) {
  tempVelocity--;
 }

 return tempVelocity;
}

void Predictor::init(){

 driveVelocity = 0;
 lastMagPha = lookupAlphaToPhase(MLX90363::getAlpha());
 drivePhase = lastMagPha << drivePhaseValueShift;
}
