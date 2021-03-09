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
#include "PcObserver.h"

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
  _peerConnectionFactory = webrtc::CreatePeerConnectionFactory(
    nullptr /* network_thread */, nullptr /* worker_thread */,
    nullptr /* signaling_thread */, nullptr /* default_adm */,
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

  //Json::CharReaderBuilder builder;
  //Json::CharReader* reader = builder.newCharReader();
  //Json::Value jmessage;
  //std::string errors;

  //bool parsingSuccessful = reader->parse(buffer, buffer + length - 1, &jmessage, &errors);
  //delete reader;

  //if (!parsingSuccessful) {
  //  //RTC_LOG(WARNING) << "Received unknown message. " << offerMessage;
  //  return "Error parsing offer message";
  //}
  //
  //std::string sdp = jmessage["sdp"].asString();
  //if (sdp.empty()) {
  //  RTC_LOG(WARNING) << "Can't parse received session description message.";
  //  return "Error parsing sdp value from JSON offer message";
  //}

  std::string offerStr(buffer, length);
  auto offerJson = nlohmann::json::parse(offerStr);

  std::cout << offerJson.dump() << std::endl;

  auto observer = new rtc::RefCountedObject<PcObserver>();

  webrtc::PeerConnectionInterface::RTCConfiguration config;
  config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
  config.enable_dtls_srtp = true;

  rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc = _peerConnectionFactory->CreatePeerConnection(
    config, nullptr, nullptr, observer);

  if (pc == nullptr) {
    std::cerr << "Failed to get peer connection from factory." << std::endl;
    return "error";
  }
  else {
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
      pc->SetRemoteDescription(observer, remoteOffer.get());
      pc->CreateAnswer(observer, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());

      return "answer";
    }
  }
}
