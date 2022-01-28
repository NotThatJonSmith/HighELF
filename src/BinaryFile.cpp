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