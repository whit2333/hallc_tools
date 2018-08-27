#ifndef hallc_calibration_shms_shower_hit_HH
#define hallc_calibration_shms_shower_hit_HH

#include <iostream>
#include "TObject.h"

// SHMS calorimeter hit class for calibration.

class THcPShHit {

  Double_t ADC;        // pedestal subtracted ADC signal.
  Double_t Edep;       // Energy deposition.
  UInt_t BlkNumber;    // Block number.

 public:

  THcPShHit();
  THcPShHit(Double_t adc, UInt_t blk_number);
  ~THcPShHit();

  void SetADC(Double_t sig) {ADC = sig;}

  void SetEdep(Double_t e) {Edep = e;}

  void SetBlkNumber(UInt_t n) {BlkNumber = n;}

  Double_t GetADC() {return ADC;}

  Double_t GetEdep() {return Edep;}

  UInt_t GetBlkNumber() {return BlkNumber;}

  void Print(std::ostream & ostrm);
};

struct pmt_hit {Double_t signal; UInt_t channel;};

#endif

