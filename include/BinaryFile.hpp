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

    BinaryFile();
    virtual void Load(std::string filename) = 0;

    void getByte(uint8_t* byte);
    void getByte(uint16_t* halfWord);
    void getByte(uint32_t* word);
    void getByte(uint64_t* doubleWord);
    void getHalfWord(uint16_t* halfWord);
    void getHalfWord(uint32_t* word);
    void getHalfWord(uint64_t* doubleWord);
    void getWord(uint32_t* word);
    void getWord(uint64_t* doubleWord);
    void getDoubleWord(uint64_t* doubleWord);

    std::string getNullTerminatedString();
    std::string getNullTerminatedString(unsigned int streamOffset);
    void alignFilePointer(unsigned int alignment);

protected:

    std::ifstream inputStream;
    Endianness fileEndianness;

private:

    Endianness hostEndianness;
    Endianness detectHostEndianness();

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