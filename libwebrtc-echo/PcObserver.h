/******************************************************************************
* Filename: PcObserver.h
*
* Description:
* Observer to receive notifications of peer connection lifetime events.
*
* Author:
* Aaron Clauson (aaron@sipsorcery.com)
*
* History:
* 08 Mar 2021	Aaron Clauson	  Created, Dublin, Ireland.
*
* License: Public Domain (no warranty, use at own risk)
/******************************************************************************/

#ifndef __PEER_CONNECTION_OBSERVER__
#define __PEER_CONNECTION_OBSERVER__

#include <api/peer_connection_interface.h>

class PcObserver :
  public webrtc::CreateSessionDescriptionObserver,
  public webrtc::PeerConnectionObserver,
  public webrtc::SetSessionDescriptionObserver
{ 
public:
  /* CreateSessionDescriptionObserver methods. */
  void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state);
  void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel);
  void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state);
  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);
  void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
  void OnFailure(webrtc::RTCError error) override;

  /* PeerConnectionObserver methods. */
  void OnAddTrack(
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
    const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams);

  void OnTrack(
    rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver);

  /* SetSessionDescriptionObserver methods. */
  void OnSuccess() override;
  //void OnFailure(webrtc::RTCError error) override;
};

#endif