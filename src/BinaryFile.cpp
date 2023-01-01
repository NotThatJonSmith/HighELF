#include <BinaryFile.hpp>

namespace HighELF {

BinaryFile::BinaryFile() :
    fileEndianness(BinaryFile::Endianness::Unknown) {
    hostEndianness = detectHostEndianness();
}

void BinaryFile::getByte(uint8_t* buf) {
    getEndianCorrectedChunk<uint8_t, uint8_t>(buf);
}

void BinaryFile::getByte(uint16_t* buf) {
    getEndianCorrectedChunk<uint16_t, uint8_t>(buf);
}

void BinaryFile::getByte(uint32_t* buf) {
    getEndianCorrectedChunk<uint32_t, uint8_t>(buf);
}

void BinaryFile::getByte(uint64_t* buf) {
    getEndianCorrectedChunk<uint64_t, uint8_t>(buf);
}

void BinaryFile::getHalfWord(uint16_t* buf) {
    getEndianCorrectedChunk<uint16_t, uint16_t>(buf);
}

void BinaryFile::getHalfWord(uint32_t* buf) {
    getEndianCorrectedChunk<uint32_t, uint16_t>(buf);
}

void BinaryFile::getHalfWord(uint64_t* buf) {
    getEndianCorrectedChunk<uint64_t, uint16_t>(buf);
}

void BinaryFile::getWord(uint32_t* buf) {
    getEndianCorrectedChunk<uint32_t, uint32_t>(buf);
}

void BinaryFile::getWord(uint64_t* buf) {
    getEndianCorrectedChunk<uint64_t, uint32_t>(buf);
}

void BinaryFile::getDoubleWord(uint64_t* buf) {
    getEndianCorrectedChunk<uint64_t, uint64_t>(buf);
}

std::string BinaryFile::getNullTerminatedString() {
    std::string result;
    char c;
    getByte((uint8_t*)&c);
    while (c != 0) {
        result += c;
        getByte((uint8_t*)&c);
    }
    return result;
}

std::string BinaryFile::getNullTerminatedString(unsigned int streamOffset) {
    uint32_t bookmark = inputStream.tellg();
    inputStream.seekg(streamOffset, std::ios::beg);
    std::string result = getNullTerminatedString();
    inputStream.seekg(bookmark, std::ios::beg);
    return result;
}

void BinaryFile::alignFilePointer(unsigned int alignment) {
    unsigned int modulus = inputStream.tellg() % alignment;
    uint8_t paddingByte;
    while (modulus) {
        getByte(&paddingByte);
        modulus = (modulus + 1) % alignment;
    }
}

BinaryFile::Endianness BinaryFile::detectHostEndianness() {

    union {
        uint32_t word;
        char bytes[4];
    } test = { 0x01000000 };

    if (test.bytes[0] == 0x01) {
        return BinaryFile::Endianness::Big;
    }

    return BinaryFile::Endianness::Little;
}

} // namespace HighELF