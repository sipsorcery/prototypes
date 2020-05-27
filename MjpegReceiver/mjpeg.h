//-----------------------------------------------------------------------------
// Filename: mjpeg.h
//
// Description: Minimal header only implementation of mjpeg based on RFC2435 and
// the JPEG File Interchange Format (JFIF):
// http://www.ecma-international.org/publications/files/ECMA-TR/ECMA%20TR-098.pdf
//
// Author(s):
// Aaron Clauson (aaron@sipsorcery.com)
// 
// History:
// 26 May 2020	Aaron Clauson	  Created, Dublin, Ireland.
//
// License and Attributions:
// Some work derived from ffmpeg classes which is licensed as > GPL2.1.
// Everything else Public Domain.
//-----------------------------------------------------------------------------

#ifndef SIPSORCERY_MJPEG_H
#define SIPSORCERY_MJPEG_H

#include <stdint.h>
#include <stdexcept>
#include <vector>

// Given this prototype is for Windows the default is little endian.
// As always detecting endianess gets very messy very quickly and
// relying on compiling on Windows is not sufficient (Windows on
// ARM CPUs can us big endian). For more robust detection look at
// Boost or equivalent:
// https://www.boost.org/doc/libs/1_59_0/boost/predef/other/endian.h
#define ENDIAN_BIG_WORD 0

namespace sipsorcery
{
  void static inline write_be16(std::vector<uint8_t>& buf, uint16_t val) {
    buf.push_back(val >> 8 & 0xff);
    buf.push_back(val & 0xff);
  }

  void static inline write_le16(std::vector<uint8_t>& buf, uint16_t val) {
    buf.push_back(val & 0xff);
    buf.push_back(val >> 8 & 0xff);
  }

  uint16_t static inline read_be16(std::vector<uint8_t>& buf, int posn) {
    auto data = buf.data();
    return (uint16_t)data[posn + 1] << 8 & 0xff00 | (uint16_t)data[posn] & 0x00ff;
  }

  uint16_t static inline read_le16(std::vector<uint8_t>& buf, int posn) {
    auto data = buf.data();
    return (uint16_t)data[posn] << 8 & 0xff00 | (uint16_t)data[posn + 1] & 0x00ff;
  }

  /**
  * Extracts 3 bytes (24 bits) representing a BIG endian encoded unsigned integer.
  * @param[in] buf: the buffer to extract the 24 bits from.
  * @param[in] posn: the start posn in the buffer to extract the 24 bits from.
  * @@Returns a 32 bit unsigned integer.
  */
  uint32_t static inline read_be24(std::vector<uint8_t>& buf, int posn) {
    auto data = buf.data();
    return (uint32_t)data[posn + 2] << 16 & 0x00ff0000 |
      (uint32_t)data[posn + 1] << 8 & 0x0000ff00 |
      (uint32_t)data[posn] & 0x000000ff;
  }

  /**
    * Extracts 3 bytes (24 bits) representing a little endian encoded unsigned integer.
    * @param[in] buf: the buffer to extract the 24 bits from.
    * @param[in] posn: the start posn in the buffer to extract the 24 bits from.
    * @@Returns a 32 bit unsigned integer.
   */
  uint32_t static inline read_le24(std::vector<uint8_t>& buf, int posn) {
    auto data = buf.data();
    return (uint32_t)data[posn] << 16 & 0x00ff0000 |
      (uint32_t)data[posn + 1] << 8 & 0x0000ff00 |
      (uint32_t)data[posn + 2] & 0x000000ff;
  }

  void static inline write_be32(std::vector<uint8_t>& buf, uint32_t val) {
    buf.push_back(val >> 24 & 0xff);
    buf.push_back(val >> 16 & 0xff);
    buf.push_back(val >> 8 & 0xff);
    buf.push_back(val & 0xff);
  }

  void static inline write_le32(std::vector<uint8_t>& buf, uint32_t val) {
    buf.push_back(val & 0xff);
    buf.push_back(val >> 8 & 0xff);
    buf.push_back(val >> 16 & 0xff);
    buf.push_back(val >> 24 & 0xff);
  }

  uint32_t static inline read_be32(std::vector<uint8_t>& buf, int posn) {
    auto data = buf.data();
    return (uint32_t)data[posn + 3] << 24 & 0xff000000 |
      (uint32_t)data[posn + 2] << 16 & 0x00ff0000 |
      (uint32_t)data[posn + 1] << 8 & 0x0000ff00 |
      (uint32_t)data[posn] & 0x000000ff;
  }

  uint32_t static inline read_le32(std::vector<uint8_t>& buf, int posn) {
    auto data = buf.data();
    return (uint32_t)data[posn] << 24 & 0xff000000 |
      (uint32_t)data[posn + 1] << 16 & 0x00ff0000 |
      (uint32_t)data[posn + 2] << 8 & 0x0000ff00 |
      (uint32_t)data[posn + 3] & 0x000000ff;
  }

  uint16_t static inline read_16(std::vector<uint8_t>& buf, int posn) {
#if ENDIAN_BIG_WORD == 1
    return read_be16(buf, posn);
#else
    return read_le16(buf, posn);
#endif
  }

  uint32_t static inline read_24(std::vector<uint8_t>& buf, int posn) {
#if ENDIAN_BIG_WORD == 1
    return read_be24(buf, posn);
#else
    return read_le24(buf, posn);
#endif
  }

  uint32_t static inline read_32(std::vector<uint8_t>& buf, int posn) {
#if ENDIAN_BIG_WORD == 1
    return read_be32(buf, posn);
#else
    return read_le32(buf, posn);
#endif
  }

/**
* Minimal 12 byte RTP header as defined in
* https://tools.ietf.org/html/rfc3550.
* No facility for extensions etc.
*
* The original RTP header definition (network byte order)
*
* RTP data header
*
  typedef struct {
    unsigned int version : 2;   // protocol version
    unsigned int p : 1;         // padding flag
    unsigned int x : 1;         // header extension flag
    unsigned int cc : 4;        // CSRC count
    unsigned int m : 1;         // marker bit
    unsigned int pt : 7;        // payload type
    unsigned int seq : 16;      /* sequence number
    u_int32 ts;               // timestamp
    u_int32 ssrc;             // synchronization source
    u_int32 csrc[1];          // optional CSRC list
  } rtp_hdr_t;
*/
  class RtpHeader
  {
  public:
    static const int RTP_VERSION = 2;
    static const int RTP_MINIMUM_HEADER_LENGTH = 12;

    uint8_t Version = RTP_VERSION;    // protocol version: 2 bits.
    uint8_t PaddingFlag = 0;          // padding flag: 1 bit.
    uint8_t HeaderExtensionFlag = 0;  // header extension flag: 1 bit.
    uint8_t CSRCCount = 0;            // CSRC count: 4 bits.
    uint8_t MarkerBit = 0;            // marker bit: 1 bit.
    uint16_t PayloadType = 0;         // payload type: 7 bits.
    uint16_t SeqNum = 0;              // sequence number: 16 bits.
    uint32_t Timestamp = 0;           // timestamp: 32 bits.
    uint32_t SyncSource = 0;          // synchronization source: 32 bits.

    void Serialise(std::vector<uint8_t>& buf)
    {
      buf.push_back((Version << 6 & 0xc0) | (PaddingFlag << 5 & 0x20) | (HeaderExtensionFlag << 4 & 0x10) | (CSRCCount & 0x0f));
      buf.push_back((MarkerBit << 7 & 0x80) | (PayloadType & 0x7f));
      write_be16(buf, SeqNum);
      write_be32(buf, Timestamp);
      write_be32(buf, SyncSource);
    }

    int Deserialise(std::vector<uint8_t>& buffer, int startPosn) {
      if (buffer.size() - startPosn < RTP_MINIMUM_HEADER_LENGTH) {
        throw std::runtime_error("The available buffer size was less than the minimum RTP header length.");
      }
      else {
        auto rawBuffer = buffer.data() + startPosn;
        Version = rawBuffer[0] >> 6 & 0x03;
        PaddingFlag = rawBuffer[0] >> 5 & 0x01;
        HeaderExtensionFlag = rawBuffer[0] >> 4 & 0x01;
        CSRCCount = rawBuffer[0] & 0x0f;
        MarkerBit = rawBuffer[1] >> 7 & 0x01;
        PayloadType = rawBuffer[1] & 0x7f;
        SeqNum = read_16(buffer, startPosn + 2);
        Timestamp = read_32(buffer, startPosn + 4);
        SyncSource = read_32(buffer, startPosn + 8);

        return RTP_MINIMUM_HEADER_LENGTH;
      }
    }
  };

  /*
  Minimal RTP JPEG header class as specified in:
  https://tools.ietf.org/html/rfc2435#appendix-B.

  JPEG header

      0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   | Type-specific |              Fragment Offset                  |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      Type     |       Q       |     Width     |     Height    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  Note: Restart markers are not supported by this implementation. A Type field
  between 64 and 127 indicate restart markers are being used.

  JPEG Quantization RTP header allow inband quantization tables.
  Will be present for the first packet in a JPEG frame (offset 0) if
  Q is in the range 128-255.

    // From https://tools.ietf.org/html/rfc2435#appendix-B

      Quantization Table header

        0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |      MBZ      |   Precision   |             Length            |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                    Quantization Table Data                    |
     |                              ...                              |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  */
  class JpegRtpHeader
  {
  public:
    static const int JPEG_MIN_HEADER_LENGTH = 8;
    static const int JPEG_DEFAULT_TYPE_SPECIFIER = 0;
    static const int JPEG_TYPE_RESTART_MARKER_START = 64;
    static const int JPEG_TYPE_RESTART_MARKER_END = 127;
    static const int JPEG_QUANTIZATION_HEADER_LENGTH = 4;
    static const int Q_TABLE_INBAND_MINIMUM = 128;

    uint8_t TypeSpecifier;    // type-specific field: 8 bits.
    uint32_t Offset;          // fragment byte offset: 24 bits.
    uint8_t Type;             // id of jpeg decoder params: 8 bits.
    uint8_t Q;                // quantization factor (or table id): 8 bits. Values 128 to 255 indicate Quantization header in use.
    uint8_t Width;            // frame width in 8 pixel blocks. 8 bits.
    uint8_t Height;           // frame height in 8 pixel blocks. 8 bits.

    // Optional Quantization Table header
    uint8_t Mbz{ 0 };              //
    uint8_t Precision{ 0 };
    uint16_t Length{ 0 };          // the length in bytes of the quantization table data to follow: 16 bits

    // Optional Quantization table.
    std::vector<uint8_t> QTable;

    int Deserialise(std::vector<uint8_t>& buffer, int startPosn) {
      if (buffer.size() - startPosn < JPEG_MIN_HEADER_LENGTH) {
        throw std::runtime_error("The available buffer size was less than the minimum JPEG RTP header length.");
      }
      else {
        int headerLength = JPEG_MIN_HEADER_LENGTH;

        auto rawBuffer = buffer.data() + startPosn;
        TypeSpecifier = rawBuffer[0];
        Offset = read_24(buffer, startPosn + 1);
        Type = rawBuffer[4];
        Q = rawBuffer[5];
        Width = rawBuffer[6];
        Height = rawBuffer[7];

        // Check that the JPEG payload can be interpreted by this implementation.
        if (TypeSpecifier != JPEG_DEFAULT_TYPE_SPECIFIER) {
          throw std::runtime_error("This implementation does not support a non default RTP JPEG type specifier.");
        }
        else if (Type >= JPEG_TYPE_RESTART_MARKER_START && Type <= JPEG_TYPE_RESTART_MARKER_END) {
          throw std::runtime_error("This implementation does not support JPEG restarts.");
        }

        // Inband Q tables are only included in the first RTP packet in the frame.
        if (Offset == 0 && Q >= Q_TABLE_INBAND_MINIMUM) {
          if (buffer.size() - startPosn - JPEG_MIN_HEADER_LENGTH < JPEG_QUANTIZATION_HEADER_LENGTH) {
            throw std::runtime_error("The available buffer size was less than the minimum JPEG RTP header length.");
          }
          else {
            Mbz = rawBuffer[8];
            Precision = rawBuffer[9];
            Length = read_16(buffer, startPosn + 10);

            headerLength += JPEG_QUANTIZATION_HEADER_LENGTH + Length;

            if (Length > 0) {
              if (buffer.size() - startPosn - JPEG_MIN_HEADER_LENGTH - JPEG_QUANTIZATION_HEADER_LENGTH < Length) {
                throw std::runtime_error("The available buffer size is shorter than the JPEG Quantization header length.");
              }
              else {
                std::copy(buffer.begin() + startPosn + JPEG_MIN_HEADER_LENGTH + JPEG_QUANTIZATION_HEADER_LENGTH,
                  buffer.begin() + startPosn + JPEG_MIN_HEADER_LENGTH + JPEG_QUANTIZATION_HEADER_LENGTH + Length,
                  std::back_inserter(QTable));
              }
            }
          }
        }

        return headerLength;
      }
    }
  };

  /***
  JPEG File Interchange Format (JFIF) header.
  */
  class Jfif
  {
  public:
    /*
    JPEG marker codes
    Derived from https://github.com/FFmpeg/FFmpeg/blob/master/libavcodec/mjpeg.h
    */
    enum JpegMarker {
      /* start of frame */
      SOF0 = 0xc0,       /* baseline */
      SOF1 = 0xc1,       /* extended sequential, huffman */
      SOF2 = 0xc2,       /* progressive, huffman */
      SOF3 = 0xc3,       /* lossless, huffman */

      SOF5 = 0xc5,       /* differential sequential, huffman */
      SOF6 = 0xc6,       /* differential progressive, huffman */
      SOF7 = 0xc7,       /* differential lossless, huffman */
      JPG = 0xc8,       /* reserved for JPEG extension */
      SOF9 = 0xc9,       /* extended sequential, arithmetic */
      SOF10 = 0xca,       /* progressive, arithmetic */
      SOF11 = 0xcb,       /* lossless, arithmetic */

      SOF13 = 0xcd,       /* differential sequential, arithmetic */
      SOF14 = 0xce,       /* differential progressive, arithmetic */
      SOF15 = 0xcf,       /* differential lossless, arithmetic */

      DHT = 0xc4,       /* define huffman tables */

      DAC = 0xcc,       /* define arithmetic-coding conditioning */

      /* restart with modulo 8 count "m" */
      RST0 = 0xd0,
      RST1 = 0xd1,
      RST2 = 0xd2,
      RST3 = 0xd3,
      RST4 = 0xd4,
      RST5 = 0xd5,
      RST6 = 0xd6,
      RST7 = 0xd7,

      SOI = 0xd8,       /* start of image */
      EOI = 0xd9,       /* end of image */
      SOS = 0xda,       /* start of scan */
      DQT = 0xdb,       /* define quantization tables */
      DNL = 0xdc,       /* define number of lines */
      DRI = 0xdd,       /* define restart interval */
      DHP = 0xde,       /* define hierarchical progression */
      EXP = 0xdf,       /* expand reference components */

      APP0 = 0xe0,
      APP1 = 0xe1,
      APP2 = 0xe2,
      APP3 = 0xe3,
      APP4 = 0xe4,
      APP5 = 0xe5,
      APP6 = 0xe6,
      APP7 = 0xe7,
      APP8 = 0xe8,
      APP9 = 0xe9,
      APP10 = 0xea,
      APP11 = 0xeb,
      APP12 = 0xec,
      APP13 = 0xed,
      APP14 = 0xee,
      APP15 = 0xef,

      JPG0 = 0xf0,
      JPG1 = 0xf1,
      JPG2 = 0xf2,
      JPG3 = 0xf3,
      JPG4 = 0xf4,
      JPG5 = 0xf5,
      JPG6 = 0xf6,
      SOF48 = 0xf7,       ///< JPEG-LS
      LSE = 0xf8,       ///< JPEG-LS extension parameters
      JPG9 = 0xf9,
      JPG10 = 0xfa,
      JPG11 = 0xfb,
      JPG12 = 0xfc,
      JPG13 = 0xfd,

      COM = 0xfe,       /* comment */

      TEM = 0x01,       /* temporary private use for arithmetic coding */

      /* 0x02 -> 0xbf reserved */
    };

    // Huffman tables taken from:
    // https://github.com/FFmpeg/FFmpeg/blob/master/libavcodec/jpegtables.c

    /* Set up the standard Huffman tables (cf. JPEG standard section K.3) */
    /* IMPORTANT: these are only valid for 8-bit data precision! */
    const uint8_t avpriv_mjpeg_bits_dc_luminance[17] =
    { /* 0-base */ 0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
    const uint8_t avpriv_mjpeg_val_dc[12] =
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

    const uint8_t avpriv_mjpeg_bits_dc_chrominance[17] =
    { /* 0-base */ 0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };

    const uint8_t avpriv_mjpeg_bits_ac_luminance[17] =
    { /* 0-base */ 0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };
    const uint8_t avpriv_mjpeg_val_ac_luminance[162] =
    { 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
      0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
      0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
      0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
      0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
      0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
      0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
      0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
      0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
      0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
      0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
      0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
      0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
      0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
      0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
      0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
      0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
      0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
      0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
      0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
      0xf9, 0xfa
    };

    const uint8_t avpriv_mjpeg_bits_ac_chrominance[17] =
    { /* 0-base */ 0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };

    const uint8_t avpriv_mjpeg_val_ac_chrominance[162] =
    { 0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
      0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
      0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
      0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
      0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
      0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
      0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
      0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
      0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
      0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
      0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
      0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
      0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
      0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
      0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
      0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
      0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
      0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
      0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
      0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
      0xf9, 0xfa
    };

    /*
    Default quantization tables.
    Taken from https://github.com/FFmpeg/FFmpeg/blob/master/libavformat/rtpdec_jpeg.c
    */
    const uint8_t default_quantizers[128] = {
      /* luma table */
      16, 11, 12, 14, 12, 10, 16, 14,
      13, 14, 18, 17, 16, 19, 24, 40,
      26, 24, 22, 22, 24, 49, 35, 37,
      29, 40, 58, 51, 61, 60, 57, 51,
      56, 55, 64, 72, 92, 78, 64, 68,
      87, 69, 55, 56, 80, 109, 81, 87,
      95, 98, 103, 104, 103, 62, 77, 113,
      121, 112, 100, 120, 92, 101, 103, 99,

      /* chroma table */
      17, 18, 18, 24, 21, 24, 47, 26,
      26, 47, 99, 66, 56, 66, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99
    };

    /*
    Create the Huffman table for the JFIF header.
    Derived from https://github.com/FFmpeg/FFmpeg/blob/master/libavformat/rtpdec_jpeg.c
    */
    static int jpeg_create_huffman_table(std::vector<uint8_t> & p, int table_class,
      int table_id, const uint8_t* bits_table,
      const uint8_t* value_table)
    {
      int i, n = 0;

      p.push_back(table_class << 4 | table_id);

      for (i = 1; i <= 16; i++) {
        n += bits_table[i];
        p.push_back(bits_table[i]);
      }

      for (i = 0; i < n; i++) {
        p.push_back(value_table[i]);
      }
      return n + 17;
    }

    static void jpeg_put_marker(std::vector<uint8_t>& p, int code)
    {
      p.push_back(0xff);
      p.push_back(code);
    }

    void jpeg_create_header(std::vector<uint8_t> & buf, uint32_t type, uint32_t w,
      uint32_t h, const uint8_t* qtable, int nb_qtable, int dri)
    {
      /* Convert from blocks to pixels. */
      w <<= 3;
      h <<= 3;

      /* SOI */
      jpeg_put_marker(buf, SOI);

      /* JFIF header */
      jpeg_put_marker(buf, APP0);
      write_be16(buf, 16);
      buf.push_back('J');
      buf.push_back('F');
      buf.push_back('I');
      buf.push_back('F');
      buf.push_back(0);
      write_be16(buf, 0x0201);
      buf.push_back(0);
      write_be16(buf, 1);
      write_be16(buf, 1);
      buf.push_back(0);
      buf.push_back(0);

      if (dri) {
        jpeg_put_marker(buf, DRI);
        write_be16(buf, 4);
        write_be16(buf, dri);
      }

      /* DQT */
      jpeg_put_marker(buf, DQT);
      write_be16(buf, 2 + nb_qtable * (1 + 64));

      for (int i = 0; i < nb_qtable; i++) {
        buf.push_back(i);

        /* Each table is an array of 64 values given in zig-zag
         * order, identical to the format used in a JFIF DQT
         * marker segment. */
        // bytestream2_put_buffer(&pbc, qtable + 64 * i, 64);
        std::copy(qtable, qtable + 64, std::back_inserter(buf));
      }

      /* DHT */
      jpeg_put_marker(buf, DHT);
      // Record the position the length of the huffman tables needs to be written.
      int sizePosn = buf.size();
      // The size isn't known yet. It will be set once the tables have been written.
      write_be16(buf, 0); 

      int dht_size = 2;
      dht_size += jpeg_create_huffman_table(buf, 0, 0, avpriv_mjpeg_bits_dc_luminance,
        avpriv_mjpeg_val_dc);
      dht_size += jpeg_create_huffman_table(buf, 0, 1, avpriv_mjpeg_bits_dc_chrominance,
        avpriv_mjpeg_val_dc);
      dht_size += jpeg_create_huffman_table(buf, 1, 0, avpriv_mjpeg_bits_ac_luminance,
        avpriv_mjpeg_val_ac_luminance);
      dht_size += jpeg_create_huffman_table(buf, 1, 1, avpriv_mjpeg_bits_ac_chrominance,
        avpriv_mjpeg_val_ac_chrominance);
      //AV_WB16(dht_size_ptr, dht_size);
       buf.at(sizePosn) = dht_size >> 8 & 0xff;
       buf.at(sizePosn + 1) = dht_size & 0xff;

      /* SOF0 */
      jpeg_put_marker(buf, SOF0);
      write_be16(buf, 17); /* size */
      buf.push_back(8); /* bits per component */
      write_be16(buf, h);
      write_be16(buf, w);
      buf.push_back(3); /* number of components */
      buf.push_back(1); /* component number */
      buf.push_back((2 << 4) | (type ? 2 : 1)); /* hsample/vsample */
      buf.push_back(0); /* matrix number */
      buf.push_back(2); /* component number */
      buf.push_back(1 << 4 | 1); /* hsample/vsample */
      buf.push_back(nb_qtable == 2 ? 1 : 0); /* matrix number */
      buf.push_back(3); /* component number */
      buf.push_back(1 << 4 | 1); /* hsample/vsample */
      buf.push_back(nb_qtable == 2 ? 1 : 0); /* matrix number */

      /* SOS */
      jpeg_put_marker(buf, SOS);
      write_be16(buf, 12);
      buf.push_back(3);
      buf.push_back(1);
      buf.push_back(0);
      buf.push_back(2);
      buf.push_back(17);
      buf.push_back(3);
      buf.push_back(17);
      buf.push_back(0);
      buf.push_back(63);
      buf.push_back(0);
    }

    /**
     * Clip a signed integer value into the amin-amax range.
     * @param a value to clip
     * @param amin minimum value of the clip range
     * @param amax maximum value of the clip range
     * @return clipped value
     Taken from https://github.com/FFmpeg/FFmpeg/blob/master/libavutil/common.h
    */
    inline const int av_clip(int a, int amin, int amax)
    {
      if (a < amin) return amin;
      else if (a > amax) return amax;
      else               return a;
    }

    void create_default_qtables(uint8_t* qtables, uint8_t q)
    {
      int factor = q;
      int i;
      uint16_t S;

      factor = av_clip(q, 1, 99);

      if (q < 50)
        S = 5000 / factor;
      else
        S = 200 - factor * 2;

      for (i = 0; i < 128; i++) {
        int val = (default_quantizers[i] * S + 50) / 100;

        /* Limit the quantizers to 1 <= q <= 255. */
        val = av_clip(val, 1, 255);
        qtables[i] = val;
      }
    }
  };

}

#endif // SIPSORCERY_MJPEG_H