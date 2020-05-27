// CppRtpReceiver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "RtpSocket.h"
#include "strutils.h"

#include <cassert>
#include <iostream>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define RTP_LISTEN_PORT 10100
#define RTP_BITMAP_WIDTH 540   
#define RTP_BITMAP_HEIGHT 360

void OnRtpBitmapReady(std::vector<uint8_t>& bmp);

int main()
{
  std::cout << "RTP Receiver Test Console" << std::endl;
  
  auto _rtpSocket = std::make_unique<sipsorcery::RtpSocket>(RTP_LISTEN_PORT, RTP_BITMAP_WIDTH, RTP_BITMAP_HEIGHT);
  //auto fp = std::bind(&TermControl::_OnRtpBitmapReady, this, std::placeholders::_1);
  _rtpSocket->SetBitmapReadyCallback(OnRtpBitmapReady);

  _rtpSocket->Start();

  /*auto buffer = ParseHex("801a02546364c3ce95e95fde0000000001ffa05a00000040080c0c0e0c0e1010101010101312131414141313131314141415151519191915151514141515181819191b1c1b1a1a191a1c1c1e1e1e242422222a2a2b33333e");

  sipsorcery::RtpHeader rtpHeader;
  rtpHeader.Deserialise(buffer, 0);

  std::cout << "rtp version " << (int)rtpHeader.Version << ", marker " << (int)rtpHeader.MarkerBit << ", ssrc " << rtpHeader.SyncSource <<
    ", timestamp " << rtpHeader.Timestamp << ", seqnum " << rtpHeader.SeqNum << "." << std::endl;

  std::vector<uint8_t> sendBuffer;
  rtpHeader.Serialise(sendBuffer);

  std::cout << toHex(sendBuffer) << std::endl;

  sipsorcery::JpegRtpHeader jpegRtpHeader;
  jpegRtpHeader.Deserialise(buffer, sipsorcery::RtpHeader::RTP_MINIMUM_HEADER_LENGTH);

  std::cout << "jpeg type specifier " << (int)jpegRtpHeader.TypeSpecifier << ", offset " << jpegRtpHeader.Offset << ", type " << (int)jpegRtpHeader.Type <<
    ", Q " << (int)jpegRtpHeader.Q << ", width " << jpegRtpHeader.Width * 8 << ", height " << jpegRtpHeader.Height * 8 << ", Q table length " << jpegRtpHeader.Length <<
    "." << std::endl;

  if (jpegRtpHeader.Length > 0) {
    std::cout << "Inband Quantization table " << toHex(jpegRtpHeader.QTable) << "." << std::endl;
  }*/

  //auto buffer = ParseHex("00000b1401ffa05a");
  //sipsorcery::JpegRtpHeader jpegRtpHeader;
  //jpegRtpHeader.Deserialise(buffer, 0);

  //std::cout << "jpeg type specifier " << (int)jpegRtpHeader.TypeSpecifier << ", offset " << jpegRtpHeader.Offset << ", type " << (int)jpegRtpHeader.Type <<
  //  ", Q " << (int)jpegRtpHeader.Q << ", width " << jpegRtpHeader.Width * 8 << ", height " << jpegRtpHeader.Height * 8 << ", Q table length " << jpegRtpHeader.Length <<
  //  "." << std::endl;

  //if (jpegRtpHeader.Length > 0) {
  //  std::cout << "Inband Quantization table " << toHex(jpegRtpHeader.QTable) << "." << std::endl;
  //}

  std::cout << "Press any key to exit..." << std::endl;
  getchar();
}

void OnRtpBitmapReady(std::vector<uint8_t>& bmp)
{
  //std::cout << "OnRtpBitmapReady" << std::endl;
}

