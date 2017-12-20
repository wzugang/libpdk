// @copyright 2017-2018 zzu_softboy <zzu_softboy@163.com>
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Created by softboy on 2017/12/15.

#include "pdk/kernel/StringUtils.h"
#include "pdk/base/ds/ByteArray.h"

namespace pdk {

/*
#!/usr/bin/perl -l
use feature "unicode_strings";
for (0..255) {
    $up = lc(chr($_));
    $up = chr($_) if ord($up) > 0x100 || length $up > 1;
    printf "0x%02x,", ord($up);
    print "" if ($_ & 0xf) == 0xf;
}
*/
const uchar latin1Lowercased[256] = {
   0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
   0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
   0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
   0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
   0x40,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
   0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x5b,0x5c,0x5d,0x5e,0x5f,
   0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
   0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
   0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
   0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
   0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
   0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
   0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
   0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xd7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xdf,
   0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
   0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};

/*
#!/usr/bin/perl -l
use feature "unicode_strings";
for (0..255) {
    $up = uc(chr($_));
    $up = chr($_) if ord($up) > 0x100 || length $up > 1;
    printf "0x%02x,", ord($up);
    print "" if ($_ & 0xf) == 0xf;
}
*/
const uchar latin1Uppercased[256] = {
   0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
   0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
   0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
   0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
   0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
   0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
   0x60,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
   0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x7b,0x7c,0x7d,0x7e,0x7f,
   0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
   0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
   0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
   0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
   0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
   0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
   0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
   0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xf7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xff
};

namespace
{
// the CRC table below is created by the following piece of code
#if 0
void create_crc16_table()
{
   unsigned int i;
   unsigned int j;
   unsigned short crcTable[16];
   unsigned int v0, v1, v2, v3;
   for (i = 0; i < 16; i++) {
      v0 = i & 1;
      v1 = (i >> 1) & 1;
      v2 = (i >> 2) & 1;
      v3 = (i >> 3) & 1;
      j = 0;
#undef SET_BIT
#define SET_BIT(x, b, v) (x) |= (v) << (b)
      SET_BIT(j,  0, v0);
      SET_BIT(j,  7, v0);
      SET_BIT(j, 12, v0);
      SET_BIT(j,  1, v1);
      SET_BIT(j,  8, v1);
      SET_BIT(j, 13, v1);
      SET_BIT(j,  2, v2);
      SET_BIT(j,  9, v2);
      SET_BIT(j, 14, v2);
      SET_BIT(j,  3, v3);
      SET_BIT(j, 10, v3);
      SET_BIT(j, 15, v3);
      crc_tbl[i] = j;
   }
   printf("static const puint16 crcTable[16] = {\n");
   for (int i = 0; i < 16; i +=4)
      printf("    0x%04x, 0x%04x, 0x%04x, 0x%04x,\n", crcTable[i], crcTable[i+1], crcTable[i+2], crcTable[i+3]);
   printf("};\n");
}
#endif

const puint16 crcTable[16] = {
   0x0000, 0x1081, 0x2102, 0x3183,
   0x4204, 0x5285, 0x6306, 0x7387,
   0x8408, 0x9489, 0xa50a, 0xb58b,
   0xc60c, 0xd68d, 0xe70e, 0xf78f
};

}

puint16 checksum(const char *data, uint length)
{
   puint16 crc = 0xffff;
   uchar c;
   const uchar *p = reinterpret_cast<const uchar *>(data);
   while (length--) {
      c = *p++;
      crc = ((crc >> 4) & 0x0fff) ^ crcTable[((crc ^ c) & 15)];
      c >>= 4;
      crc = ((crc >> 4) & 0x0fff) ^ crcTable[((crc ^ c) & 15)];
   }
   return ~crc & 0xffff;
}

char *strdup(const char *str)
{
   if (!str) {
      return nullptr;
   }
   char *dest = new char[std::strlen(str) + 1];
   return strcopy(dest, str);
}

char *strcopy(char *dest, const char *src)
{
   if (!src) {
      return nullptr;
   }
#if defined(_MSC_VER) && _MSC_VER >= 1400
   const int length = static_cast<int>(std::strlen(str));
   // This is actually not secure!!! It will be fixed
   // properly in a later release!
   if (len >= 0 && strcpy_s(dest, length + 1, src) == 0) {
      return dest;
   }
   return nullptr;
#endif
   return std::strcpy(dest, src);
}

char *strncopy(char *dest, const char *src, uint length)
{
   if (!dest || !src) {
      return nullptr;
   }
   if (length > 0) {
#if defined(_MSC_VER) && _MSC_VER >= 1400
      strncpy_s(dest, length, src, length - 1);
#else
      std::strncpy(dest, src, length);
#endif
      dest[length - 1] = '\0';
   }
   return dest;
}

int strcmp(const char *lhs, const char *rhs)
{
   return (lhs && rhs) ? std::strcmp(lhs, rhs) : (lhs ? 1 : (rhs ? -1 : 0));
}

int stricmp(const char *lhs, const char *rhs)
{
   const uchar *s1 = reinterpret_cast<const uchar *>(lhs);
   const uchar *s2 = reinterpret_cast<const uchar *>(rhs);
   int result;
   uchar c;
   if (!s1 || !s2) {
      return s1 ? 1 : (s2 ? -1 : 0);
   }
   for (; !(result = (c = latin1Lowercased[*s1]) - latin1Lowercased[*s2]); ++s1, ++s2) {
      if (!c) {
         break;
      }
   }
   return result;
}

int strnicmp(const char *lhs, const char *rhs, uint length)
{
   const uchar *s1 = reinterpret_cast<const uchar *>(lhs);
   const uchar *s2 = reinterpret_cast<const uchar *>(rhs);
   int result;
   uchar c;
   if (!s1 || !s2) {
      return s1 ? 1 : (s2 ? -1 : 0);
   }
   for (; length--; ++s1, ++s2) {
      if ((result = (c = latin1Lowercased[*s1]) - latin1Lowercased[*s2])) {
         return result;
      }
      if (!c) {
         break;
      }
   }
   return 0;
}

int strcmp(const ds::ByteArray &lhs, const char *rhs)
{
   if (!rhs) {
      return lhs.isEmpty() ? 0 : 1;
   }
   const char *lhsData = lhs.getConstRawData();
   const char *lhsDataEnd = lhsData + lhs.length();
   for (; lhsData < lhsDataEnd && *rhs; ++lhsData, ++rhs) {
      int diff = static_cast<int>(static_cast<uchar>(*lhsData) - static_cast<uchar>(*rhs));
      if (diff) {
         return diff;
      }
   }
   if (*rhs != '\0') {
      return -1;
   }
   if (lhsData < lhsDataEnd) {
      return 1;
   }
   return 0;
}

int strcmp(const char *lhs, const ds::ByteArray &rhs)
{
   return -strcmp(rhs, lhs);
}

int strcmp(const ds::ByteArray &lhs, const ds::ByteArray &rhs)
{
   int lhsLength = lhs.length();
   int rhsLength = rhs.length();
   int ret = std::memcmp(lhs.getConstRawData(), rhs.getConstRawData(), 
                         std::min(lhsLength, rhsLength));
   if (ret != 0) {
      return ret;
   }
   return lhsLength - rhsLength;
}

} 
