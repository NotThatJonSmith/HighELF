#pragma once

#include <iostream>

#include <string>
#include <fstream>
#include <stdint.h>

#include <tuple>

namespace HighELF {

class BinaryFile {

public:

    enum class Endianness { Big, Little, Unknown };

public:

    BinaryFile() :
        fileEndianness(BinaryFile::Endianness::Unknown) {
        hostEndianness = detectHostEndianness();
    }

    virtual void Load(std::string filename) = 0;

    void getByte(uint8_t* buf) { getEndianCorrectedChunk<uint8_t, uint8_t>(buf); }
    void getByte(uint16_t* buf) { getEndianCorrectedChunk<uint16_t, uint8_t>(buf); }
    void getByte(uint32_t* buf) { getEndianCorrectedChunk<uint32_t, uint8_t>(buf); }
    void getByte(uint64_t* buf) { getEndianCorrectedChunk<uint64_t, uint8_t>(buf); }
    void getHalfWord(uint16_t* buf) { getEndianCorrectedChunk<uint16_t, uint16_t>(buf); }
    void getHalfWord(uint32_t* buf) { getEndianCorrectedChunk<uint32_t, uint16_t>(buf); }
    void getHalfWord(uint64_t* buf) { getEndianCorrectedChunk<uint64_t, uint16_t>(buf); }
    void getWord(uint32_t* buf) { getEndianCorrectedChunk<uint32_t, uint32_t>(buf); }
    void getWord(uint64_t* buf) { getEndianCorrectedChunk<uint64_t, uint32_t>(buf); }
    void getDoubleWord(uint64_t* buf) { getEndianCorrectedChunk<uint64_t, uint64_t>(buf); }

    std::string getNullTerminatedString() {
        std::string result;
        char c;
        getByte((uint8_t*)&c);
        while (c != 0) {
            result += c;
            getByte((uint8_t*)&c);
        }
        return result;
    }

    std::string getNullTerminatedString(unsigned int streamOffset) {
        uint32_t bookmark = inputStream.tellg();
        inputStream.seekg(streamOffset, std::ios::beg);
        std::string result = getNullTerminatedString();
        inputStream.seekg(bookmark, std::ios::beg);
        return result;
    }

    void alignFilePointer(unsigned int alignment) {
        unsigned int modulus = inputStream.tellg() % alignment;
        uint8_t paddingByte;
        while (modulus) {
            getByte(&paddingByte);
            modulus = (modulus + 1) % alignment;
        }
    }

protected:

    std::ifstream inputStream;
    Endianness fileEndianness;

private:

    Endianness hostEndianness;
    Endianness detectHostEndianness() {
        union {
            uint32_t word;
            char bytes[4];
        } test = { 0x01000000 };

        if (test.bytes[0] == 0x01) {
            return BinaryFile::Endianness::Big;
        }

        return BinaryFile::Endianness::Little;
    }

    template<typename MemType, typename StreamType>
    void getEndianCorrectedChunk(MemType* memBuf) {

        if (sizeof(StreamType) > 1 && fileEndianness == Endianness::Unknown) {
            // TODO complain that we're pulling >1 byte before knowing endian
        }

        StreamType streamBuf;
        inputStream.read((char*)&streamBuf, sizeof(StreamType));
        // TODO check file status every single time

        // Convert stream endianness if it's different
        if (sizeof(StreamType) > 1 && hostEndianness != fileEndianness) {
            streamBuf = swapEndianness<StreamType>(streamBuf);
        }

        // Use whatever default type cast the compiler comes up with
        *memBuf = streamBuf;
    }

    template<typename T>
    T swapEndianness(T input) {

        union ByteOverlay {
            T object;
            char bytes[sizeof(T)];
        };
    
        ByteOverlay inputBytes;
        inputBytes.object = input;

        ByteOverlay outputBytes;
        outputBytes.object = 0;

        for (unsigned int i = 0; i < sizeof(T); i++) {
            outputBytes.bytes[i] = inputBytes.bytes[sizeof(T)-i-1];
        }

        // std::cout << "Swap: " << std::hex << inputBytes.object << " -> "
        //           << outputBytes.object << std::endl;

        return outputBytes.object;
    }
};

} //namespace HighELF