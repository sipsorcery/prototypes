/******************************************************************************
* Filename: PcFactory.h
*
* Description:
* Factory to allow the creation of new WebRTC peer connection instances.
*
* Author:
* Aaron Clauson (aaron@sipsorcery.com)
*
* History:
* 08 Mar 2021	Aaron Clauson	  Created, Dublin, Ireland.
*
* License: Public Domain (no warranty, use at own risk)
/******************************************************************************/

#ifndef __PEER_CONNECTION_FACTORY__
#define __PEER_CONNECTION_FACTORY__

#include <api/peer_connection_interface.h>
#include <api/scoped_refptr.h>

#include <condition_variable>
#include <memory>
#include <string>
#include <vector>

class PcFactory {
public:
  PcFactory();
  void WaitForAnswer(std::condition_variable cv);
  std::string CreatePeerConnection(const char* buffer, int length);

private:
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _peerConnectionFactory;
  std::vector<rtc::scoped_refptr<webrtc::PeerConnectionInterface>> _peerConnections;
};

#endif