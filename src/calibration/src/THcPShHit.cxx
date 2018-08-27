
#include "calibration/THcPShHit.h"

#include <iostream>

//------------------------------------------------------------------------------

THcPShHit::THcPShHit() {
  ADC = -99999.;
  Edep = -99999.;
  BlkNumber = 99999;
};

THcPShHit::THcPShHit(Double_t adc, UInt_t blk_number) {
  ADC = adc;
  Edep = 0.;
  BlkNumber = blk_number;
};

THcPShHit::~THcPShHit() { };

//------------------------------------------------------------------------------

void THcPShHit::Print(std::ostream & ostrm) {

  // Output hit data through the stream ostrm.

  ostrm << ADC << " " << Edep << " " << BlkNumber << "\n";
};

//------------------------------------------------------------------------------

