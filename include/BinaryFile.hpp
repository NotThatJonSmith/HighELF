#pragma once

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
        if (hostEndianness != fileEndianness) {
            streamBuf = swapEndianness<StreamType>(streamBuf);
        }

        // Use whatever default type cast the compiler comes up with
        *memBuf = streamBuf;
    }

    template<typename T>
    T swapEndianness(T input) {
        T result = 0;
        for (unsigned int byteOffset = 0; byteOffset < sizeof(input)/2; byteOffset++) {
            unsigned int loshamt = 8*(sizeof(input)-byteOffset);
            unsigned int hishamt = 8*(sizeof(input));
            result |= (input >> hishamt) & (0xff << loshamt);
            result |= (input >> loshamt) & (0xff << hishamt);
        }
        return result;
    }

    template<typename T>
    void getStruct(T* buf) {
        // TODO template metaprogramming way of doing this?
        // Convert to tuple, getTuple, convert to struct, return...
    }

    template<size_t I = 0, class... Ts>
    std::tuple<Ts...> getTuple(std::tuple<Ts...> tup) {

        using EltType = std::tuple_element_t<I, std::tuple<Ts...>>;
        if constexpr (std::is_class<EltType>::value) {
            getStruct<EltType>(std::get<I>(tup));
        } else {
            // TODO this assumes no schroedinger's types
            getEndianCorrectedChunk<EltType, EltType>(std::get<I>(tup));
        }
        
        if constexpr (I >= sizeof...(Ts)) {
            return tup;
        } else {
            return getTuple<I+1, Ts...>(tup);
        }
    }
};

} //namespace HighELF