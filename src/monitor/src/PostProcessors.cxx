#include "scandalizer/PostProcessors.h"

#include "TSystem.h"
#include "THaEvData.h"


namespace hallc {
  namespace scandalizer {

      Int_t SimplePostProcess::Init(const TDatime& ) {
        return _init_lambda();
      }
      Int_t SimplePostProcess::Process( const THaEvData* evt, const THaRunBase*, Int_t code ){ 
        _event_lambda(evt);
        //_output_file << " Event : " << evt->GetEvNum() << "  ( " << evt->GetEvType() << ")\n";
        if( evt->GetEvNum()%100 == 0) {
          gSystem->ProcessEvents();
          //_output_file.flush();
        }
        return 0;
      }
      Int_t SimplePostProcess::Close(){ 
        std::cout << "close\n";
        //_output_file.flush();
        //_output_file.close();
        return 0; 
      }
      //____________________________________________________________
  }
}
