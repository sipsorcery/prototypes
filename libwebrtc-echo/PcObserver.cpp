/******************************************************************************
* Filename: PcObserver.cpp
*
* Description: See header file.
*
* Author :
* Aaron Clauson (aaron@sipsorcery.com)
*
* History :
* 08 Mar 2021	Aaron Clauson	  Created, Dublin, Ireland.
*
* License: Public Domain (no warranty, use at own risk)
/******************************************************************************/

#include "PcObserver.h"

#include <iostream>

void PcObserver::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state)
{
  std::cout << "OnSignalingChange " << new_state << "." << std::endl;
}

void PcObserver::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel)
{
  std::cout << "OnDataChannel " << data_channel->id() << "." << std::endl;
}

void PcObserver::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
{
  std::cout << "OnIceGatheringChange " << new_state << "." << std::endl;
}

void PcObserver::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
{
  std::cout << "OnIceCandidate " << candidate->candidate().ToString() << "." << std::endl;
}

void PcObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
  std::cout << "SDP observer on success." << std::endl;
}

void PcObserver::OnFailure(webrtc::RTCError error)
{
  std::cout << "SDP observer on failure: " << error.message() << std::endl;
}

void PcObserver::OnSuccess()
{
  std::cout << "SDP observer set remote description success." << std::endl;

}

//void AppPeerConnectionObserver::OnFailure(webrtc::RTCError error)
//{
//  std::cout << "SDP observer set remote description on failure: " << error.message() << std::endl;
//}

void PcObserver::OnAddTrack(
  rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
  const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams)
{
  std::cout << "OnAddTrack." << std::endl;
}

void PcObserver::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver)
{
  std::cout << "OnTrack." << std::endl;
}