#ifndef epics_PVBufferList_H
#define epics_PVBufferList_H

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <map>


#include "podd2/Logger.h"

#include "simple_epics/PVBuffer.h"
#include "simple_epics/PVList.h"


namespace hallc {

  /** PV Get-List. Continuously updated list of variables.
   *
   */
  class PVBufferList : public PVList {
  public:
    using PV_index_name_map  = std::map<std::string,int>;
    using PV_name_map        = std::map<int,std::string>;
    using PV_value_map       = std::vector<float>;
    using PV_channel_map     = std::map<int,pvac::ClientChannel>;
    using PV_provider        = std::shared_ptr<pvac::ClientProvider>;

  public:
    //int                   m_N_pvs = 0;
    //PV_provider           m_provider;
    //PV_index_name_map     m_pv_index;
    //PV_name_map           m_pv_names;
    //PV_value_map          m_pv_values;
    //PV_channel_map        m_pv_channels;
    //std::vector<int>      m_array_index;
    std::vector<PVBuffer> m_pv_buffers;

  public:
    PVBufferList();
    PVBufferList(const PVBufferList&) = default ;
    PVBufferList(const std::vector<std::string>& names);
    virtual ~PVBufferList();

    ///** Add a new process variable to list. 
    // * Returns the list index for a new or existing variable.
    // */
    virtual int AddPV(const std::string&);

    //float GetValue(const std::string& n) const; 
    //float GetValue(int n) const;

    //std::string GetName(int n) const { return m_pv_names.at(n);}

    //// This would load history at startup
    //void Init() { }

    //int          GetN() const { return m_N_pvs; }
    unsigned int GetNBuffers() const { return m_pv_buffers.size(); }
    int          GetBufferSize(int n) const ;
    int          GetBufferOffset(int n) const ;

    PVBuffer&            GetBuffer(int n) ;
    std::vector<float>&  GetBufferCopy(int n) ;

    //void TestPut(std::string pvname, double val);
    //
    ///** Put pv value. 
    // * PV put is processed in a detached thread
    // */
    //void Put(std::string pvname, double val);


    //virtual void PollAndPrintAll() ;
    //virtual void PrintAll() const;

    /** This actually goes to CA and gets new values for all variables.
     */
    virtual void Poll();

  };

}
#endif
