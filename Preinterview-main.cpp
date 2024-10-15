#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

uint32_t convertPackDataToUint(std::vector<char> &packData, size_t cursorPos,
                               size_t endPos, bool flip = false);

uint8_t cutoutByte(uint64_t sequence, size_t place) {
  return uint8_t(sequence << (8 * place) >> 56);
}

class IpAdr {
protected:
  friend class packet;
  IpAdr() { ip = 0; }
  IpAdr(std::vector<char> &ipSeq, int ipStart) {
    ip = convertPackDataToUint(ipSeq, ipStart + 2, ipStart + 5);
  }
  uint32_t ip;
};

class packet {
public:
  static size_t getSizeLength() { return 2; }
  // returns byte length of packet size preamble

  static uint32_t getVersion(std::vector<char> &packData, size_t cursorPos) {
    return convertPackDataToUint(packData, cursorPos + 12, cursorPos + 13);
  }

  static uint64_t getIpSignature(std::vector<char> &packData, size_t cursorPos)
  // returns both destination&sender IPs stored in one 64-bit int
  {
    IpAdr source(packData, cursorPos + 6);
    IpAdr destination(packData, cursorPos);
    return (uint64_t)destination.ip << 32 | (uint64_t)source.ip;
  }
};

class Packet : public packet {
public:
  Packet() {
    ver_ = 0;
    transCtr_ = 1;
  }
  Packet(std::vector<char> &packData, size_t cursorPos /*, size_t endPos*/) {
    ver_ = packet::getVersion(packData, cursorPos);
    ipSig_ = packet::getIpSignature(packData, cursorPos);
    transCtr_ = 1;
  }

  std::string getIpToString()
  // returns string of type "sender -> destintion" for current packet IPs
  {
    std::string toReturn = std::to_string(cutoutByte(ipSig_, 4));
    for (int i = 5; i < 8; ++i) {
      toReturn = toReturn + "." + std::to_string(cutoutByte(ipSig_, i));
    }
    toReturn = toReturn + " -> " + std::to_string(cutoutByte(ipSig_, 0));
    for (int i = 1; i < 4; ++i) {
      toReturn = toReturn + "." + std::to_string(cutoutByte(ipSig_, i));
    }
    return toReturn;
  }

  unsigned int getVersion() { return ver_; }

  uint64_t getIpSignature() { return ipSig_; }

  void incrementTransitCtr() {
    ++transCtr_;
    return;
  }

  unsigned getTransitCtr() { return transCtr_; }

private:
  unsigned int ver_;
  uint64_t ipSig_;
  unsigned int transCtr_;
};

uint32_t convertPackDataToUint(std::vector<char> &packData, size_t cursorPos,
                               size_t endPos, bool flip /*= false*/)
// converts given char(byte) vector sequence into a 32-bit int
{
  unsigned int sum = 0;
  if (flip) {
    for (int i = endPos; i >= (int)cursorPos; --i) {
      char tmp = packData[i];
      sum = sum | (unsigned char)packData[i] << ((i - cursorPos) * 8);
    }
  } else {
    for (int i = cursorPos; i <= (int)endPos; ++i) {
      sum = sum | (unsigned char)packData[i] << ((endPos - i) * 8);
    }
  }
  return sum;
}

int main() {
  std::ifstream file;
  std::string inputFile;
  std::vector<char> wholeFile;
  size_t cursorPos = 0;
  size_t dataLen = 0;
  std::vector<Packet> ipv4Packs; // вектор только для пакетов v4, согласно
  // задаче. если надо хранить другие пакеты, дополняется добавлением вектора
  // other и записи туда при непрохождении проверки на v4
  std::vector<Packet>::iterator packsIter;
  int nIpv4Packs = 0;
  int nOtherPacks = 0;

  std::cin >> inputFile; // getting the file
  file.open(inputFile,
            std::ios::in |
                std::ios::binary); // D:\Docs\c++\preinterview\packets.sig

  if (!file) // check if file opened alright
  {
    std::cout << "File didn't open\n\n";
    return -1;
  }

  file.seekg(0, std::ios::end);
  std::streampos length(file.tellg());
  // set length of the file

  if (length)
  // if file contains anything, char vector gets resized after file length and
  // read into
  {
    file.seekg(0, std::ios::beg);
    wholeFile.resize(static_cast<std::size_t>(length));
    file.read(&wholeFile.front(), static_cast<std::size_t>(length));
  } else {
    std::cout << "File is empty\n\n";
    return -1;
  }

  while (cursorPos < wholeFile.size()) {
    bool isWritten = false;

    dataLen = (size_t)convertPackDataToUint(
        wholeFile, cursorPos, cursorPos + Packet::getSizeLength() - 1, true);
    cursorPos = cursorPos + packet::getSizeLength();
    packsIter = ipv4Packs.begin();

    if (packet::getVersion(wholeFile, cursorPos) == 2048)
    // if current scanned packet version is IPv4, ++counter and add to ipv4
    // packs list
    {
      ++nIpv4Packs;
      for (; packsIter != ipv4Packs.end(); packsIter++) {
        if (packsIter->getIpSignature() ==
            packet::getIpSignature(wholeFile, cursorPos)) {
          packsIter->incrementTransitCtr();
          isWritten = true;
          break;
        }
      }
      if (!isWritten)
      // if current ipv4 packet doesn't repeats IP signature of previous one, it
      // gets pushed into a vector
      {
        ipv4Packs.push_back(Packet(wholeFile, cursorPos));
      }
    } else {
      ++nOtherPacks;
    }
    cursorPos = cursorPos + dataLen;
  }

  // outputting the results
  packsIter = ipv4Packs.begin();
  std::cout << std::setw(40) << "Packets containing IPv4:" << "\t" << nIpv4Packs
            << std::endl
            << std::setw(40) << "Packets without IPv4:" << "\t" << nOtherPacks
            << std::endl
            << std::endl;
  for (; packsIter != ipv4Packs.end(); packsIter++) {
    std::cout << std::setw(40) << packsIter->getIpToString() << "\t"
              << packsIter->getTransitCtr() << std::endl;
  }

  return 0;
}