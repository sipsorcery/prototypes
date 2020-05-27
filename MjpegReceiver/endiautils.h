//-----------------------------------------------------------------------------
// Filename: endianutils.h
//
// Description: Endian aware read/write to buffer functions.
//
// Author(s):
// Aaron Clauson (aaron@sipsorcery.com)
// 
// History:
// 27 May 2020	Aaron Clauson	  Created, Dublin, Ireland.
//
// License and Attributions: 
// Public Domain.
//-----------------------------------------------------------------------------

#ifndef ENDIAN_UTILS_H
#define ENDIAN_UTILS_H

#include <vector>

// Given this prototype is for Windows the default is little endian.
// As always detecting endianess gets very messy very quickly and
// relying on compiling on Windows is not sufficient (Windows on
// ARM CPUs can us big endian). For more robust detection look at
// Boost or equivalent:
// https://www.boost.org/doc/libs/1_59_0/boost/predef/other/endian.h
#define ENDIAN_BIG_WORD 0

void static inline write_le16(uint16_t val, std::vector<uint8_t>& buf) {
  buf.push_back(val >> 8 & 0xff);
  buf.push_back(val & 0xff);
}

void static inline write_be16(uint16_t val, std::vector<uint8_t>& buf) {
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

void static inline write_le32(uint32_t val, std::vector<uint8_t>& buf) {
  buf.push_back(val >> 24 & 0xff);
  buf.push_back(val >> 16 & 0xff);
  buf.push_back(val >> 8 & 0xff);
  buf.push_back(val & 0xff);
}

void static inline write_be32(uint32_t val, std::vector<uint8_t>& buf) {
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

void static inline write_16(uint16_t val, std::vector<uint8_t>& buf) {
#if ENDIAN_BIG_WORD == 1
  write_be16(val, buf);
#else
  write_le16(val, buf);
#endif
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

void static inline write_32(uint32_t val, std::vector<uint8_t>& buf) {
#if ENDIAN_BIG_WORD == 1
  write_be32(val, buf);
#else
  write_le32(val, buf);
#endif
}

uint32_t static inline read_32(std::vector<uint8_t>& buf, int posn) {
#if ENDIAN_BIG_WORD == 1
  return read_be32(buf, posn);
#else
  return read_le32(buf, posn);
#endif
}

#endif // ENDIAN_UTILS_H