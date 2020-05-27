#include "rtpsocket.h"
#include "strutils.h"

#include <fstream>
#include <iterator>
#include <string>

#define RECEIVE_BUFFER_SIZE 2048

namespace sipsorcery
{
  RtpSocket::RtpSocket(int listenPort) :
    _listenPort(listenPort), _listenAddr(),
    _timeout()
  { }

  RtpSocket::~RtpSocket()
  {
    Close();
  }

  void RtpSocket::Start()
  {
    WSADATA wsaData;

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
      printf("WSAStartup failed: %d\n", iResult);
      goto done;
    }

    _rtpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_rtpSocket == INVALID_SOCKET)
    {
      wprintf(L"socket function failed with error: %u\n", WSAGetLastError());
      WSACleanup();
      goto done;
    }

    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.
    _listenAddr.sin_family = AF_INET;
    _listenAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    _listenAddr.sin_port = htons((short)_listenPort);

    //----------------------
    // Bind the socket.
    iResult = bind(_rtpSocket, (SOCKADDR*)&_listenAddr, sizeof(_listenAddr));
    if (iResult == SOCKET_ERROR)
    {
      wprintf(L"bind failed with error %u\n", WSAGetLastError());
      closesocket(_rtpSocket);
      WSACleanup();
      goto done;
    }
    else
    {
      wprintf(L"bind returned success\n");
    }

    _timeout.tv_sec = 0;
    _timeout.tv_usec = RECEIVE_TIMEOUT_MILLISECONDS * 1000;

    _receiveThread = std::make_unique<std::thread>(std::thread(&RtpSocket::Receive, this));

  done:

    printf("finished.\n");
  }

  void RtpSocket::SetBitmapReadyCallback(std::function<void(std::vector<uint8_t>&)> cb)
  {
    _cb = cb;
  }

  void RtpSocket::Receive()
  {
    std::vector<uint8_t> recvBuffer(RECEIVE_BUFFER_SIZE);

    struct sockaddr_in SenderAddr;
    int SenderAddrSize = sizeof(SenderAddr);
    struct fd_set fds;
    std::vector<uint8_t> frame;
    int frameCounter = 0;

    while (!_closed)
    {
      FD_ZERO(&fds);
      FD_SET(_rtpSocket, &fds);

      //auto now = std::chrono::system_clock::now();
      //time_t tt = std::chrono::system_clock::to_time_t(now);
      //tm utc_tm;
      //gmtime_s(&utc_tm, &tt);

      //for (int i = 0; i < buffer.size(); i += 4)
      //{
      //    buffer[i] = (utc_tm.tm_sec % 4 == 0) ? 0xff : 0;
      //    buffer[i + 1] = (utc_tm.tm_sec % 3 == 0) ? 0xff : 0;
      //    buffer[i + 2] = (utc_tm.tm_sec % 2 == 0) ? 0xff : 0;
      //    buffer[i + 3] = 0xff; // Transparency is already being applied to the whole Bitmap by WPF.
      //}

      // Return value:
      // -1: error occurred
      // 0: timed out
      // > 0: data ready to be read
      int selectResult = select(0, &fds, 0, 0, &_timeout);

      if (selectResult < 0) {
        wprintf(L"select failed with error %u\n", WSAGetLastError());
        break;
      }
      else if (selectResult > 0) {

        int bytesRead = recvfrom(_rtpSocket,
          (char *)recvBuffer.data(),
          recvBuffer.size(),
          0,
          (SOCKADDR*)&SenderAddr,
          &SenderAddrSize);

        if (bytesRead == SOCKET_ERROR)
        {
          int lastError = WSAGetLastError();
          std::cerr << "recvfrom failed with error " << lastError << "." << std::endl;
        }
        else if(bytesRead > RtpHeader::RTP_MINIMUM_HEADER_LENGTH)
        {
         /* rtp_hdr* rtpHeader = (rtp_hdr*)&RecvBuf;*/
          int payloadLength = bytesRead - RtpHeader::RTP_MINIMUM_HEADER_LENGTH;
          RtpHeader rtpHeader;
          int rtpHdrLen = rtpHeader.Deserialise(recvBuffer, 0);
          JpegRtpHeader jpegHeader;
          int jpegHdrLen = jpegHeader.Deserialise(recvBuffer, RtpHeader::RTP_MINIMUM_HEADER_LENGTH);

          std::cout << "rtp version " << (int)rtpHeader.Version << ", marker " << (int)rtpHeader.MarkerBit << ", ssrc " << rtpHeader.SyncSource << 
            ", timestamp " << rtpHeader.Timestamp << ", seqnum " << rtpHeader.SeqNum << ", payload length " << payloadLength << 
            ", jpeg offset " << jpegHeader.Offset << ", Q " << (int)jpegHeader.Q << ", width " << jpegHeader.Width * 8 << 
            ", height " << jpegHeader.Height * 8 << ", Q table length " << jpegHeader.Length << "." << std::endl;
          std::cout << "Q Table: " << toHex(jpegHeader.QTable) << std::endl;

          //std::vector<uint8_t> buffer(&RecvBuf[0], &RecvBuf[iResult]);
          //buffer.insert(buffer.begin(), dataVec2.begin(), dataVec2.end());
          //std::copy(&RecvBuf[0], &RecvBuf[iResult], std::front_inserter(buffer));
          //buffer.resize(BitmapWidth * BitmapHeight * BYTES_PER_PIXEL);

          if (jpegHeader.Offset == 0) {
            frame.clear();

            // Add the JFIF header at the top of the frame.
            sipsorcery::Jfif jfif;
            jfif.jpeg_create_header(frame, jpegHeader.Type, jpegHeader.Width, jpegHeader.Height, jpegHeader.QTable.data(), 1, 0);
          }

          int hdrLen = rtpHdrLen + jpegHdrLen;
          int payloadLen = bytesRead - hdrLen;
          if (payloadLen > 0) {
            std::copy(recvBuffer.begin() + hdrLen, recvBuffer.begin() + bytesRead, std::back_inserter(frame));
            std::cout << payloadLen << " bytes written to frame." << std::endl;
          }
          else {
            std::cout << "No payload bytes in RTP packet." << std::endl;
          }

          if (rtpHeader.MarkerBit == 1) {
            
            // Need to write the jpeg end of data tag.
            sipsorcery::Jfif::jpeg_put_marker(frame, sipsorcery::Jfif::JpegMarker::EOI);

            // This is the last packet in the JPEG frame.
            std::cout << "frame ready total length " << frame.size() << "." << std::endl;

            std::ofstream output_file("frame_" + std::to_string(frameCounter) + ".jpeg", std::ios::out | std::ofstream::binary);
            std::ostream_iterator<uint8_t> output_iterator(output_file);
            std::copy(frame.begin(), frame.end(), output_iterator);
            output_file.close();

            if (_cb != nullptr)
            {
              //_cb(buffer);
            }

            frame.clear();
            frameCounter++;
          }
        }
      }
    }

    closesocket(_rtpSocket);
    WSACleanup();
  }

  void RtpSocket::Close()
  {
    _closed = true;

    if (_receiveThread != nullptr)
    {
      _receiveThread->join();
    }
  }
}