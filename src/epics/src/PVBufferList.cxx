#include "PVBufferList.h"

#include "pv/convert.h"
#include "pv/valueBuilder.h"
#include "pva/client.h"

#include <enumerate.hpp>
#include <algorithm>
#include <sstream>
#include <thread>


namespace hallc {


  PVBufferList::PVBufferList() : PVList() {}
  //______________________________________________________________________________

  PVBufferList::PVBufferList(const std::vector<std::string>& names) : PVList(names) {
    for (const auto& name : m_pv_names) {
      m_pv_buffers.push_back(PVBuffer(0.0));
    }
      //if (m_pv_index.count(name) == 0) {
      //  int                                  index = m_N_pvs;
      //  decltype(m_pv_channels[index].get()) ret;
      //  // hanle the timeout by ignoring that variable
      //  try {
      //    m_pv_channels[index] = m_provider->connect(name);
      //    ret                  = m_pv_channels[index].get(2.0); // 5.0,epics::pvData::ValueBuilder()
      //  } catch (std::exception& e) {
      //    std::cout << e.what() << '\n';
      //    continue;
      //  }
      //  m_pv_names[index] = name;
      //  auto nfs          = ret->getNumberFields();
      //  auto val          = ret->getSubField<epics::pvData::PVDouble>("value");
      //  m_pv_values.push_back(val->getAs<float>());
      //  std::cout << m_pv_values[index] << std::endl;
      //  m_pv_index[name] = index;
      //  //m_pv_buffers.push_back(PVBuffer(0.0));
      //  // array index starting from the front
      //  // int current_array_index =  m_array_index.at(index);
      //  // create buffer of zeros
      //  // std::vector<float> buffer(2*m_buffer_max,0);
      //  // buffer.reserve(2*m_buffer_max);
      //  // buffer[current_array_index] = m_pv_values.at(index);
      //  // m_pv_buffers.push_back(buffer);
      //  // std::vector<float> buffer2;
      //  // m_buffer_copy.push_back(buffer2);
      //  m_N_pvs++;
      //}
  }
  //______________________________________________________________________________

  PVBufferList::~PVBufferList() {}
  //______________________________________________________________________________

  /** Add a new process variable to list.
   * Returns the list index for
   * new or already existing variable.
   */
  int PVBufferList::AddPV(const std::string& name) {
    //std::lock_guard<std::mutex> lockGuard(m_impl->m);
    auto N_init =  GetN(); 
    auto i_added = PVList::AddPV(name);
    if( N_init != GetN()) {
      m_pv_buffers.push_back(PVBuffer(0.0));
    }
    return i_added;
  }
  //______________________________________________________________________________

  //void PVList::PollAndPrintAll() {
  //  std::lock_guard<std::mutex> lockGuard(m_impl->m);
  //  if (m_N_pvs > 0) {
  //    for (auto& channel_pair : m_pv_channels) {
  //      int  index         = channel_pair.first;
  //      auto ret           = channel_pair.second.get();
  //      auto val           = ret->getSubField<epics::pvData::PVDouble>("value");
  //      m_pv_values[index] = val->getAs<float>();
  //      //m_pv_buffers[index].Add(m_pv_values[index]);
  //    }
  //  }
  //  for (const auto& index_pair : m_pv_index) {
  //    int         index = index_pair.second;
  //    const auto& name  = index_pair.first;
  //    std::cout << name << " \t " << m_pv_values.at(index) << "\n";
  //  }
  //}
  ////______________________________________________________________________________

  //void PVList::PrintAll() const {
  //  std::lock_guard<std::mutex> lockGuard(m_impl->m);
  //  for (const auto& index_pair : m_pv_index) {
  //    int         index = index_pair.second;
  //    const auto& name  = index_pair.first;
  //    std::cout << name << " \t " << m_pv_values.at(index) << "\n";
  //  }
  //}
  ////______________________________________________________________________________

  void PVBufferList::Poll() {
    PVList::Poll();
    //std::lock_guard<std::mutex> lockGuard(m_impl->m);
    for (auto&& [i, n] : iter::enumerate(m_pv_values)) { 
        m_pv_buffers[i].Add(n);
    }
    //if (m_N_pvs > 0) {
    //  for (auto& channel_pair : m_pv_channels) {
    //    int  index         = channel_pair.first;
    //    auto ret           = channel_pair.second.get();
    //    auto val           = ret->getSubField<epics::pvData::PVDouble>("value");
    //    m_pv_values[index] = val->getAs<float>();
    //  }
    //}
  }
  //______________________________________________________________________________

  // Now we buffer the variable (to view on a strip chart)
  // Check that we are not at the end of the array

  // if (m_array_index[index]+1 == m_buffer_max*2) {
  //  // Rotate about the middle
  //  std::rotate(m_pv_buffers[index].begin(),
  //              m_pv_buffers[index].begin()+m_buffer_max,
  //              m_pv_buffers[index].end() );
  //  m_array_index[index] = m_buffer_max-1;
  //}
  // m_array_index[index]++;
  // std::cout << m_pv_buffers[index].size() << std::endl;
  // std::cout << " index  " << m_array_index.at(index) << "\n";
  // std::cout << "Rotating : " << m_pv_buffers[index][m_array_index.at(index)] <<"\n";
  // std::cout << "Rotating : " << m_pv_buffers[index][m_array_index.at(index)-1] <<"\n";

  // std::rotate(m_pv_buffers[index].begin(),
  //            m_pv_buffers[index].begin()+m_buffer_max,
  //            m_pv_buffers[index].end() );
  // m_array_index[index] = m_buffer_max-1;
  // std::cout << " rotated : " << m_pv_buffers[index][m_array_index.at(index)] <<"\n";
  // std::cout << " rotated : " << m_pv_buffers[index][m_array_index.at(index)-1] <<"\n";
  // if(m_pv_buffers[index].size() > (m_buffer_max + m_buffer_extra)) {
  //  m_pv_buffers[index].erase(m_pv_buffers[index].begin(),m_pv_buffers[index].begin()+m_buffer_extra);
  //}

//  PVBuffer& PVList::GetBuffer(int n) {
//    // if ( (n >= 0) && (n < GetNBuffers()) ) {
//    return m_pv_buffers.at(n);
//    //}
//  }
//  //______________________________________________________________________________
//
//  std::vector<float>& PVList::GetBufferCopy(int n) {
//    std::lock_guard<std::mutex> lockGuard(m_impl->m);
//    // if ( (n >= 0) && (n < m_buffer_copy.size()) ) {
//    return m_pv_buffers.at(n).GetBufferCopy();
//    //  m_buffer_copy[n].clear();
//    //  std::copy(m_pv_buffers[n].begin()+GetBufferOffset(n),
//    //            m_pv_buffers[n].begin()+GetBufferOffset(n)+m_buffer_max,
//    //            std::back_inserter(m_buffer_copy[n]));
//    //  return m_buffer_copy[n];
//    //}
//    // std::cout << "error\n";
//    // std::vector<float> buffer;
//    // m_buffer_copy.push_back(buffer);
//    // return buffer;
//  }
//  //______________________________________________________________________________
//
//  int PVList::GetBufferSize(int n) const {
//    int  sz      = m_pv_buffers.at(n).GetIndex();
//    auto buf_max = m_pv_buffers.at(n).GetMaxSize();
//    if (sz < buf_max) {
//      return sz;
//    }
//    return buf_max;
//    // auto buf_size = m_pv_buffers.at(n).size();
//    // if( buf_size > m_buffer_max  ) {
//    //  return m_buffer_max;
//    //}
//    // return buf_size;
//  }
//  //______________________________________________________________________________
//
//  int PVList::GetBufferOffset(int n) const {
//    // int off = (int)m_pv_buffers.at(n).size() - m_buffer_max;
//    int  current_index = m_pv_buffers.at(n).GetIndex();
//    auto buf_max       = m_pv_buffers.at(n).GetMaxSize();
//    if (current_index > buf_max) {
//      return current_index - buf_max;
//    }
//    return 0;
//    // if (off > 0) {
//    //  return off;
//    //}
//    // return 0;
//  }

  //void PVBufferList::Put(std::string pvname, double val) {
  //  if (m_pv_index.count(pvname) != 0) {
  //    int ind1 = m_pv_index[pvname];

  //    std::thread thObj(
  //      [&, ind1, val]() { 
  //        try {
  //          auto res =  m_pv_channels.at(ind1).put().set("value", val);
  //          res.exec(); 
  //        } catch (const std::exception& e) {
  //          std::cout << "error : " << e.what() << ", PV " << ind1 <<  " not put\n";
  //        }
  //      });
  //    thObj.detach();
  //  }
  //}

  //void PVList::TestPut(std::string pvname, double val) {
  //  if (m_pv_index.count(pvname) != 0) {
  //    int ind1 = m_pv_index[pvname];
  //    std::cout << "Before " << m_pv_channels.at(ind1).name() << " : "
  //              << m_pv_channels.at(ind1).get() << "\n";

  //    std::thread thObj([&, ind1, val]() {
  //      //
  //      // m_pv_channels.at(ind1).put().set("valueAlarm.lowWarningLimit", val-1.0).exec();
  //      // m_pv_channels.at(ind1).put().set("valueAlarm.highWarningLimit", val+1.0).exec();
  //      m_pv_channels.at(ind1).put().set("value", val).exec();

  //      std::cout << "After  " << m_pv_channels.at(ind1).name() << " : "
  //                << m_pv_channels.at(ind1)
  //                       .get()
  //                       ->getSubField<epics::pvData::PVDouble>("value")
  //                       ->getAs<double>()
  //                << ", "
  //                << m_pv_channels.at(ind1)
  //                       .get()
  //                       ->getSubField<epics::pvData::PVDouble>("valueAlarm.highWarningLimit")
  //                       ->getAs<double>()
  //                << "\n";
  //    });
  //    thObj.detach();
  //  }
  //}
} // namespace hallc

