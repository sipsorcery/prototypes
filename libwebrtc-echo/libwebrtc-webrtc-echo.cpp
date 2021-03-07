// gn gen out/Default --args="is_clang=false use_lld=false"
// gn args out/Default --list # Check use_lld is false. is_clang always shows true but if the false option is not set then linker errors when using webrtc.lib.
// gn clean out/Default # If previous compilation.
// ninja -C out/Default

#define NOMINMAX
#define WEBRTC_WIN

#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include <api/peer_connection_interface.h>
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"

#include <iostream>

class AppWebRTCObserver : public webrtc::PeerConnectionObserver
{
public:

  void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state)
  {
  }

  void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel)
  {
  }
  
  void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
  {
  }
  
  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
  {
  }
};

int main()
{
    std::cout << "libwebrtc echo test server" << std::endl;

    auto peer_connection_factory = webrtc::CreatePeerConnectionFactory(
      nullptr /* network_thread */, nullptr /* worker_thread */,
      nullptr /* signaling_thread */, nullptr /* default_adm */,
      webrtc::CreateBuiltinAudioEncoderFactory(),
      webrtc::CreateBuiltinAudioDecoderFactory(),
      webrtc::CreateBuiltinVideoEncoderFactory(),
      webrtc::CreateBuiltinVideoDecoderFactory(), nullptr /* audio_mixer */,
      nullptr /* audio_processing */);

    AppWebRTCObserver appWebrtcObserver;

    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    config.enable_dtls_srtp = true;

    rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc = peer_connection_factory->CreatePeerConnection(
      config, nullptr, nullptr, &appWebrtcObserver);
}
