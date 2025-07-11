// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Boulder, Colorado, USA
// ** BSD licence applies - redistribution and use in source and binary
// ** forms, with or without modification, are permitted provided that
// ** the following conditions are met:
// ** 1) If the software is modified to produce derivative works,
// ** such modified software should be clearly marked, so as not
// ** to confuse it with the version available from UCAR.
// ** 2) Redistributions of source code must retain the above copyright
// ** notice, this list of conditions and the following disclaimer.
// ** 3) Redistributions in binary form must reproduce the above copyright
// ** notice, this list of conditions and the following disclaimer in the
// ** documentation and/or other materials provided with the distribution.
// ** 4) Neither the name of UCAR nor the names of its contributors,
// ** if any, may be used to endorse or promote products derived from
// ** this software without specific prior written permission.
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
/////////////////////////////////////////////////////////////
// Input.hh
//
// Udp input object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1999
//
/////////////////////////////////////////////////////////////

#ifndef INPUT_H
#define INPUT_H

#include <string>
#include <condition_variable>
#include <deque>
#include <thread>
#include <vector>
#include <dataport/port_types.h>
#include "Params.hh"
using namespace std;

class Input
{

  using Message = std::vector<ui08>;

  public:

    Input(string &prog_name, Params &params);

    ~Input();

    // check construction
    bool isOK() { return (_isOK); }

    // read a message
    Message readMsg() { return _pktQueue.pop(); }

    // close reader thread
    void halt();

  private:

    string _progName;
    Params &_params;
    int _udpFd;
    bool _isOK;
    std::thread _reader;
    bool _done;

    int receiveMsg();

    template<typename T> class BlockingQueue
    {
      public:
        void push(const T& v) {
            std::lock_guard<std::mutex> l(m_);
            q_.push_back(v);
            c_.notify_one();
        }

        T pop() {
            std::unique_lock<std::mutex> l(m_);
            c_.wait(l,[this]{return !q_.empty();});
            auto value=q_.front();
            q_.pop_front();
            return value;
        }

        auto empty() {
            std::lock_guard<std::mutex> l(m_);
            return q_.empty();
        }

        auto size() {
            std::lock_guard<std::mutex> l(m_);
            return q_.size();
        }

        BlockingQueue() = default;
        BlockingQueue(const BlockingQueue& rhs) = delete;
        BlockingQueue(BlockingQueue&& rhs) = delete;

      private:
        std::deque<T> q_;
        std::mutex m_;
        std::condition_variable c_;
    };

    BlockingQueue<Message> _pktQueue;
};

#endif

