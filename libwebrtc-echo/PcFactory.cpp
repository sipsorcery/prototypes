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

PcFactory::~PcFactory()
{
  for (auto pc : _peerConnections) {
    pc->Close();
  }
  _peerConnections.clear();
  _peerConnectionFactory = nullptr;

  _networkThread->Stop();
  _workerThread->Stop();
  _signalingThread->Stop();
}

std::string PcFactory::CreatePeerConnection(const char* buffer, int length) {

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

    webrtc::SdpParseError sdpError;
    auto remoteOffer = webrtc::CreateSessionDescription(webrtc::SdpType::kOffer, offerJson["sdp"], &sdpError);

    if (remoteOffer == nullptr) {
      std::cerr << "Failed to get parse remote SDP." << std::endl;
      return "error";
    }
    else {
      std::cout << "Setting remote description on peer connection." << std::endl;
      auto setRemoteObserver = new rtc::RefCountedObject<SetRemoteSdpObserver>();
      pc->SetRemoteDescription(remoteOffer->Clone(), setRemoteObserver);

      std::mutex mtx;
      std::condition_variable cv;
      bool isReady = false;

      auto createObs = new rtc::RefCountedObject<CreateSdpObserver>(mtx, cv, isReady);
      pc->SetLocalDescription(createObs);

      std::unique_lock<std::mutex> lck(mtx);

      if (!isReady) {
        std::cout << "Waiting for create answer to complete..." << std::endl;
        cv.wait(lck);
      }

      auto localDescription = pc->local_description();

      if (localDescription == nullptr) {
        return "Failed to set local description.";
      }
      else {
        std::cout << "Create answer complete." << std::endl;

        std::string answerSdp;
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
