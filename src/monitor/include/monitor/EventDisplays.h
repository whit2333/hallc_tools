#ifndef hallc_EventDisplays_HH
#define hallc_EventDisplays_HH

#include "CodaDecoder.h"
#include "Scandalizer.h"
#include "THaEvData.h"
#include "THaPostProcess.h"
#include "THcParmList.h"

#include "THcDC.h"
#include "THcHodoscope.h"

#include <functional>
#include <iostream>

#include "monitor/DetectorDisplay.h"
#include "podd2/Logger.h"

namespace hallc {

  namespace event_display {

    /**  Basic Event display.
     *
     */
    class BasicEventDisplay : public podd2::AnalysisLogging<THaPostProcess> {
    private:
      int _counter = 0;

    public:
      int              _run_number  = 0;
      DetectorDisplay* _det_display = nullptr;
      THcHodoscope*    _hod         = nullptr;
      THcCherenkov*    _hgcer       = nullptr;
      THcDC*           _dc          = nullptr;

    public:
      BasicEventDisplay(DetectorDisplay* d)
          : podd2::AnalysisLogging<THaPostProcess>(), _det_display(d) {}

      BasicEventDisplay() : podd2::AnalysisLogging<THaPostProcess>() {}
      virtual ~BasicEventDisplay() {}

      virtual Int_t Init(const TDatime&);
      virtual Int_t Process(const THaEvData* evt, const THaRunBase*, Int_t code);
      virtual Int_t Close();
      ClassDef(BasicEventDisplay,1)
    };


    /** SHMS Event Display.
     *
     */
    class SHMSEventDisplay : public BasicEventDisplay {
    public:
      SHMSEventDisplay(DetectorDisplay* d) : BasicEventDisplay(d) {}
      SHMSEventDisplay() : BasicEventDisplay() {}
      virtual ~SHMSEventDisplay() {}
      virtual Int_t Init(const TDatime&);
      ClassDef(SHMSEventDisplay,1)
    };

    /** HMS Event Display.
     *
     */
    class HMSEventDisplay : public BasicEventDisplay {
    public:
      HMSEventDisplay(DetectorDisplay* d) : BasicEventDisplay(d) {}
      HMSEventDisplay() : BasicEventDisplay() {}
      virtual ~HMSEventDisplay() {}
      virtual Int_t Init(const TDatime&);
      ClassDef(HMSEventDisplay,1)
    };

  } // namespace event_display

} // namespace hallc

#endif
