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

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

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

class CreateSdpObserver :
  public webrtc::SetLocalDescriptionObserverInterface
{
public:
  
  CreateSdpObserver(std::mutex& mtx, std::condition_variable& cv, bool& isReady, std::string& answerSdp, std::string& error)
    : _mtx(mtx), _cv(cv), _isReady(isReady), _answerSdp(answerSdp), _error(error) {
  }

  ~CreateSdpObserver() {
    std::cout << "CreateSdpObserver Destructor." << std::endl;
  }

  void OnSetLocalDescriptionComplete(webrtc::RTCError error) {
    std::cout << "OnSetLocalDescriptionComplete." << std::endl;

    std::unique_lock<std::mutex> lck(_mtx);
    _isReady = true;
    _cv.notify_all();
  }

  //void OnSuccess(webrtc::SessionDescriptionInterface* desc) {

  //  std::cout << "CreateSdpObserver OnSuccess." << std::endl;

  //  desc->ToString(&_answerSdp);

  //  std::unique_lock<std::mutex> lck(_mtx);
  //  _isReady = true;
  //  _cv.notify_all();

  //  std::cout << "CreateSdpObserver OnSuccess end." << std::endl;
  //}

  //void OnFailure(webrtc::RTCError error) {

  //  _error = error.message();

  //  std::cout << "CreateSdpObserver OnFailure." << std::endl;

  //  std::unique_lock<std::mutex> lck(_mtx);
  //  _isReady = true;
  //  _cv.notify_all();
  //}

private:
  std::mutex& _mtx;
  std::condition_variable& _cv;
  bool& _isReady;
  std::string& _answerSdp;
  std::string& _error;

};

#endif