/******************************************************************************
* Filename: PcFactory.cpp
*
* Description: See header file.
*
* Author:
* Aaron Clauson (aaron@sipsorcery.com)
*
* History:
* 08 Mar 2021	Aaron Clauson	  Created, Dublin, Ireland.
*
* License: Public Domain (no warranty, use at own risk)
/******************************************************************************/

#include "PcFactory.h"

#include <nlohmann/json.hpp>

#include <api/audio_codecs/audio_decoder_factory.h>
#include <api/audio_codecs/audio_encoder_factory.h>
#include <api/audio_codecs/builtin_audio_decoder_factory.h>
#include <api/audio_codecs/builtin_audio_encoder_factory.h>
#include <api/create_peerconnection_factory.h>
#include <api/peer_connection_interface.h>
#include <api/video_codecs/builtin_video_decoder_factory.h>
#include <api/video_codecs/builtin_video_encoder_factory.h>
#include <api/video_codecs/video_decoder_factory.h>
#include <api/video_codecs/video_encoder_factory.h>
#include <rtc_base/strings/json.h>

#include <iostream>
#include <sstream>

class CSDO : public webrtc::CreateSessionDescriptionObserver {

public:
  CSDO()  {
  }

  void OnSuccess(webrtc::SessionDescriptionInterface* desc) override {
    std::cout << std::this_thread::get_id() << ":"
      << "CreateSessionDescriptionObserver::OnSuccess" << std::endl;
  };

  void OnFailure(webrtc::RTCError error) override {
    std::cout << std::this_thread::get_id() << ":"
      << "CreateSessionDescriptionObserver::OnFailure" << std::endl
      << error.message() << std::endl;
  };
};

class SSDO : public webrtc::SetSessionDescriptionObserver {

public:
  SSDO() {
  }

  void OnSuccess() override {
    std::cout << std::this_thread::get_id() << ":"
      << "SetSessionDescriptionObserver::OnSuccess" << std::endl;
  };

  void OnFailure(webrtc::RTCError error) override {
    std::cout << std::this_thread::get_id() << ":"
      << "SetSessionDescriptionObserver::OnFailure" << std::endl
      << error.message() << std::endl;
  };
};

PcFactory::PcFactory() :
  _peerConnections()
{
  _networkThread = rtc::Thread::CreateWithSocketServer();
  _networkThread->Start();
  _workerThread = rtc::Thread::Create();
  _workerThread->Start();
  _signalingThread = rtc::Thread::Create();
  _signalingThread->Start();

  _peerConnectionFactory = webrtc::CreatePeerConnectionFactory(
    _networkThread.get() /* network_thread */,
    _workerThread.get() /* worker_thread */,
    _signalingThread.get() /* signaling_thread */,
    nullptr /* default_adm */,
    webrtc::CreateBuiltinAudioEncoderFactory(),
    webrtc::CreateBuiltinAudioDecoderFactory(),
    webrtc::CreateBuiltinVideoEncoderFactory(),
    webrtc::CreateBuiltinVideoDecoderFactory(),
    nullptr /* audio_mixer */,
    nullptr /* audio_processing */);
}

void PcFactory::WaitForAnswer(std::condition_variable cv) {

}

std::string PcFactory::CreatePeerConnection(const char* buffer, int length) {
  /*std::mutex m;
  std::condition_variable cv;
  std::unique_lock<std::mutex> lk(m);*/

  /*  std::thread worker([]() {
      WaitForAnswer(cv);
      });*/

      //cv.wait(lk);

  std::string offerStr(buffer, length);
  auto offerJson = nlohmann::json::parse(offerStr);

  std::cout << offerJson.dump() << std::endl;

  webrtc::PeerConnectionInterface::RTCConfiguration config;
  config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
  config.enable_dtls_srtp = true;

  auto observer = new rtc::RefCountedObject<PcObserver>();

  auto pcOrError = _peerConnectionFactory->CreatePeerConnectionOrError(
    config, webrtc::PeerConnectionDependencies(observer));

  rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc = nullptr;

  if (!pcOrError.ok()) {
    std::cerr << "Failed to get peer connection from factory. " << pcOrError.error().message() << std::endl;
    return "error";
  }
  else {
    pc = pcOrError.MoveValue();

    _peerConnections.push_back(pc);

    //pc->CreateOffer(&observer, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    //auto offer = webrtc::CreateSessionDescription(webrtc::SdpType::kOffer, offer);

    webrtc::SdpParseError sdpError;
    auto remoteOffer = webrtc::CreateSessionDescription(webrtc::SdpType::kOffer, offerJson["sdp"], &sdpError);

    if (remoteOffer == nullptr) {
      std::cerr << "Failed to get parse remote SDP." << std::endl;
      return "error";
    }
    else {
      std::cout << "Setting remote description on peer connection." << std::endl;
      auto ssdo = new rtc::RefCountedObject<PcObserver>();
      pc->SetRemoteDescription(ssdo, remoteOffer.get());
      //auto ao = new rtc::RefCountedObject<PcObserver>();

      std::mutex mtx;
      std::condition_variable cv;
      bool isReady = false;
      std::string answerSdp;
      std::string error;

      auto createObs = new rtc::RefCountedObject<CreateSdpObserver>(mtx, cv, isReady, answerSdp, error);
      pc->SetLocalDescription(createObs);

      std::unique_lock<std::mutex> lck(mtx);

      if (!isReady) {
        std::cout << "Waiting for create answer to complete..." << std::endl;
        cv.wait(lck);
      }

      auto localDescription = pc->local_description();

      if (localDescription == nullptr) {
        return error;
      }
      else {
        std::cout << "Create answer complete." << std::endl;

        localDescription->ToString(&answerSdp);

        std::cout << answerSdp << std::endl;

        nlohmann::json answerJson;
        answerJson["type"] = "answer";
        answerJson["sdp"] = answerSdp;

        return answerJson.dump();
      }
    }
  }
}
