#include "rtpsocket.h"

#define RECEIVE_BUFFER_SIZE 2048

namespace sipsorcery
{
  RtpSocket::RtpSocket(int listenPort, int bmpWidth, int bmpHeight) :
    _listenPort(listenPort), BitmapWidth(bmpWidth), BitmapHeight(bmpHeight), _listenAddr(),
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

        int iResult = recvfrom(_rtpSocket,
          (char *)recvBuffer.data(),
          recvBuffer.size(),
          0,
          (SOCKADDR*)&SenderAddr,
          &SenderAddrSize);

        if (iResult == SOCKET_ERROR)
        {
          int lastError = WSAGetLastError();
          std::cerr << "recvfrom failed with error " << lastError << "." << std::endl;
        }
        else if(iResult > RtpHeader::RTP_MINIMUM_HEADER_LENGTH)
        {
         /* rtp_hdr* rtpHeader = (rtp_hdr*)&RecvBuf;*/
          int payloadLength = iResult - RtpHeader::RTP_MINIMUM_HEADER_LENGTH;
          RtpHeader rtpHeader;
          rtpHeader.Deserialise(recvBuffer, 0);
          JpegRtpHeader rtpJpegHeader;
          rtpJpegHeader.Deserialise(recvBuffer, RtpHeader::RTP_MINIMUM_HEADER_LENGTH);

          std::cout << "rtp version " << (int)rtpHeader.Version << ", marker " << (int)rtpHeader.MarkerBit << ", ssrc " << rtpHeader.SyncSource << 
            ", timestamp " << rtpHeader.Timestamp << ", seqnum " << rtpHeader.SeqNum << ", payload length " << payloadLength << 
            ", jpeg offset " << rtpJpegHeader.Offset << ", Q " << (int)rtpJpegHeader.Q << ", width " << rtpJpegHeader.Width * 8 << 
            ", height " << rtpJpegHeader.Height * 8 <<
            ", Q table length " << rtpJpegHeader.Length << "." << std::endl;

          //std::vector<uint8_t> buffer(&RecvBuf[0], &RecvBuf[iResult]);
          //buffer.insert(buffer.begin(), dataVec2.begin(), dataVec2.end());
          //std::copy(&RecvBuf[0], &RecvBuf[iResult], std::front_inserter(buffer));
          //buffer.resize(BitmapWidth * BitmapHeight * BYTES_PER_PIXEL);

          if (_cb != nullptr)
          {
            //_cb(buffer);
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