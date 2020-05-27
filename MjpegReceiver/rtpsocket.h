//-----------------------------------------------------------------------------
// Filename: RtpSocket.h
//
// Description: Minimal implementation of an RTP receiver.
//
// Author(s):
// Aaron Clauson (aaron@sipsorcery.com)
// 
// History:
// 26 May 2020	Aaron Clauson	  Created, Dublin, Ireland.
//
// License and Attributions: 
// Everything else Public Domain.
//-----------------------------------------------------------------------------

#ifndef SIPSORCERY_RTPSOCKET_H
#define SIPSORCERY_RTPSOCKET_H

#include "mjpeg.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include <chrono>
#include <ctime>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#define RECEIVE_TIMEOUT_MILLISECONDS 70

namespace sipsorcery
{
  class RtpSocket
  {
  public:
    int BitmapWidth;
    int BitmapHeight;

    RtpSocket(int listenPort, int bmpWidth, int bmpHeight);
    ~RtpSocket();
    void SetBitmapReadyCallback(std::function<void(std::vector<uint8_t>&)> cb);
    void Start();
    void Close();

  private:
    int _listenPort;
    bool _closed{ false };
    SOCKET _rtpSocket{ INVALID_SOCKET };
    struct sockaddr_in _listenAddr;
    struct timeval _timeout;
    std::unique_ptr<std::thread> _receiveThread{ nullptr };
    std::function<void(std::vector<uint8_t>&)> _cb{ nullptr };

    void Receive();
  };
}

#endif // SIPSORCERY_RTPSOCKET_H
