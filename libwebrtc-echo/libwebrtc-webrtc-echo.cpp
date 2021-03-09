/******************************************************************************
* Filename: libwebrtc-webrtc-echo.cpp
*
* Description:
* Main program for a test program that creates a echo peer using Google's
* webrtc library, https://webrtc.googlesource.com/src/webrtc/.
* 
* Dependencies:
* vcpkg install nlohmann-json:x64-windows # Too much time sent troubleshooting jsoncpp.
*
* Author:
* Aaron Clauson (aaron@sipsorcery.com)
*
* History:
* 08 Mar 2021	Aaron Clauson	  Created, Dublin, Ireland.
*
* License: Public Domain (no warranty, use at own risk)
/******************************************************************************/

// gn gen out/Default --args="is_clang=false use_lld=false"
// gn args out/Default --list # Check use_lld is false. is_clang always shows true but if the false option is not set then linker errors when using webrtc.lib.
// gn clean out/Default # If previous compilation.
// ninja -C out/Default

#include "HttpSimpleServer.h"
#include "PcFactory.h"

#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

#define HTTP_SERVER_ADDRESS "0.0.0.0"
#define HTTP_SERVER_PORT 8080
#define HTTP_OFFER_URL "/offer"

int main()
{
  std::cout << "libwebrtc echo test server" << std::endl;

#ifdef _WIN32
  {
    /* If running on Windows need to initialise sockets. */
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsaData);
  }
#endif

  HttpSimpleServer httpSvr;
  httpSvr.Init(HTTP_SERVER_ADDRESS, HTTP_SERVER_PORT, HTTP_OFFER_URL);

  PcFactory pcFactory;
  HttpSimpleServer::SetPeerConnectionFactory(&pcFactory);

  httpSvr.Run();

  std::cout << "Stopping HTTP server..." << std::endl;

  httpSvr.Stop();

#ifdef _WIN32
  WSACleanup();
#endif

  std::cout << "Exiting..." << std::endl;
}
