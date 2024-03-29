#pragma once

#include <fstream>
#include <string>
#include <vector>

#include <BinaryFile.hpp>

namespace HighELF {

class ElfFile : private BinaryFile {

public:

    struct ElfIdentification {
        uint8_t ei_magic0;
        uint8_t ei_magic1;
        uint8_t ei_magic2;
        uint8_t ei_magic3;
        uint8_t ei_class;
        uint8_t ei_data;
        uint8_t ei_version;
        uint8_t ei_osabi;
        uint8_t ei_abiversion;
        uint8_t ei_pad[7];
    };

    struct ElfHeader {
        uint16_t e_type;
        uint16_t e_machine;
        uint32_t e_version;
        uint64_t e_entry;
        uint64_t e_phoff;
        uint64_t e_shoff;
        uint32_t e_flags;
        uint16_t e_ehsize;
        uint16_t e_phentsize;
        uint16_t e_phnum;
        uint16_t e_shentsize;
        uint16_t e_shnum;
        uint16_t e_shstrndx;
    };

    struct ProgramHeader {
        uint32_t p_type;
        uint32_t p_flags;
        uint64_t p_offset;
        uint64_t p_vaddr;
        uint64_t p_paddr;
        uint64_t p_filesz;
        uint64_t p_memsz;
        uint64_t p_align;
    };

    struct SectionHeader {
        uint32_t sh_name;
        uint32_t sh_type;
        uint64_t sh_flags;
        uint64_t sh_addr;
        uint64_t sh_offset;
        uint64_t sh_size;
        uint32_t sh_link;
        uint32_t sh_info;
        uint64_t sh_addralign;
        uint64_t sh_entsize;
    };

    struct Section {
        std::string name;
        std::vector<char> bytes;
    };

public:

    ElfFile() { }
    ElfFile(std::string filename) {
        Load(filename);
    }

    virtual void Load(std::string filename) override  {

        inputStream.open(filename, std::ios::in | std::ios::binary);

        if (!inputStream.is_open()) {
            status = Status::BadFile;
            return;
        }

        inputStream.seekg(0, std::ios::beg);

        getByte(&elfIdentification.ei_magic0);
        getByte(&elfIdentification.ei_magic1);
        getByte(&elfIdentification.ei_magic2);
        getByte(&elfIdentification.ei_magic3);
        getByte(&elfIdentification.ei_class);
        getByte(&elfIdentification.ei_data);
        getByte(&elfIdentification.ei_version);
        getByte(&elfIdentification.ei_osabi);
        getByte(&elfIdentification.ei_abiversion);

        for (int i = 0; i < 7; i++) {
            getByte(&elfIdentification.ei_pad[i]);
        }

        if (elfIdentification.ei_magic0 != 0x7F ||
            elfIdentification.ei_magic1 != 'E'  ||
            elfIdentification.ei_magic2 != 'L'  ||
            elfIdentification.ei_magic3 != 'F') {
            status = Status::BadIdentMagic;
            inputStream.close();
            return;
        }

        if (elfIdentification.ei_class != 1 && elfIdentification.ei_class != 2) {
            status = Status::BadIdentClass;
            inputStream.close();
            return;
        }

        bool fileIs64Bit = elfIdentification.ei_class == 2;

        if (elfIdentification.ei_data != 1 && elfIdentification.ei_data != 2) {
            status = Status::BadIdentData;
            inputStream.close();
            return;
        }

        if (elfIdentification.ei_data == 1) {
            fileEndianness = Endianness::Little;
        }

        if (elfIdentification.ei_data == 2) {
            fileEndianness = Endianness::Big;
        }

        if (elfIdentification.ei_version != 1) {
            status = Status::BadIdentVersion;
            inputStream.close();
            return;
        }

        getHalfWord(&elfHeader.e_type);
        getHalfWord(&elfHeader.e_machine);
        getWord(&elfHeader.e_version);

        if (elfHeader.e_version != 1) {
            status = Status::BadHeaderVersion;
            inputStream.close();
            return;
        }

        if (fileIs64Bit) {
            getDoubleWord(&elfHeader.e_entry);
            getDoubleWord(&elfHeader.e_phoff);
            getDoubleWord(&elfHeader.e_shoff);
        } else {
            getWord(&elfHeader.e_entry);
            getWord(&elfHeader.e_phoff);
            getWord(&elfHeader.e_shoff);
        }

        getWord(&elfHeader.e_flags);
        getHalfWord(&elfHeader.e_ehsize);
        getHalfWord(&elfHeader.e_phentsize);
        getHalfWord(&elfHeader.e_phnum);
        getHalfWord(&elfHeader.e_shentsize);
        getHalfWord(&elfHeader.e_shnum);
        getHalfWord(&elfHeader.e_shstrndx);

        // TODO check that the ELF header size matches how much we've read so far
        // TODO check that we're still in a sane file location
        // TODO check that phentsize matches expectation for 32/64 bit class ID
        // TODO check that phnum and shnum are reasonable

        programHeaders.resize(elfHeader.e_phnum);
        sectionHeaders.resize(elfHeader.e_shnum);
        sections.resize(elfHeader.e_shnum);

        // TODO be skeptical about how much you're supposed to read w/ the following
        // unsigned int phtBytesRead = 0;
        // unsigned int phtBytesMax = phnum * phentsize;

        inputStream.seekg(elfHeader.e_phoff, std::ios::beg);

        if (fileIs64Bit) {
            for (int i = 0; i < elfHeader.e_phnum; i++) {
                getWord(&programHeaders[i].p_type);
                getWord(&programHeaders[i].p_flags);
                getDoubleWord(&programHeaders[i].p_offset);
                getDoubleWord(&programHeaders[i].p_vaddr);
                getDoubleWord(&programHeaders[i].p_paddr);
                getDoubleWord(&programHeaders[i].p_filesz);
                getDoubleWord(&programHeaders[i].p_memsz);
                getDoubleWord(&programHeaders[i].p_align);
            }
        } else {
            for (int i = 0; i < elfHeader.e_phnum; i++) {
                getWord(&programHeaders[i].p_type);
                getWord(&programHeaders[i].p_offset);
                getWord(&programHeaders[i].p_vaddr);
                getWord(&programHeaders[i].p_paddr);
                getWord(&programHeaders[i].p_filesz);
                getWord(&programHeaders[i].p_memsz);
                getWord(&programHeaders[i].p_flags);
                getWord(&programHeaders[i].p_align);
            }
        }

        inputStream.seekg(elfHeader.e_shoff, std::ios::beg);

        if (fileIs64Bit) {
            for (int i = 0; i < elfHeader.e_shnum; i++) {
                getWord(&sectionHeaders[i].sh_name);
                getWord(&sectionHeaders[i].sh_type);
                getDoubleWord(&sectionHeaders[i].sh_flags);
                getDoubleWord(&sectionHeaders[i].sh_addr);
                getDoubleWord(&sectionHeaders[i].sh_offset);
                getDoubleWord(&sectionHeaders[i].sh_size);
                getWord(&sectionHeaders[i].sh_link);
                getWord(&sectionHeaders[i].sh_info);
                getDoubleWord(&sectionHeaders[i].sh_addralign);
                getDoubleWord(&sectionHeaders[i].sh_entsize);
            }
        } else {
            for (int i = 0; i < elfHeader.e_shnum; i++) {
                getWord(&sectionHeaders[i].sh_name);
                getWord(&sectionHeaders[i].sh_type);
                getWord(&sectionHeaders[i].sh_flags);
                getWord(&sectionHeaders[i].sh_addr);
                getWord(&sectionHeaders[i].sh_offset);
                getWord(&sectionHeaders[i].sh_size);
                getWord(&sectionHeaders[i].sh_link);
                getWord(&sectionHeaders[i].sh_info);
                getWord(&sectionHeaders[i].sh_addralign);
                getWord(&sectionHeaders[i].sh_entsize);
            }
        }

        for (int i = 0; i < elfHeader.e_shnum; i++) {

            // SHT_NOBITS and SHT_NULL have no data to read, regardless of size
            if (sectionHeaders[i].sh_type == 8 || sectionHeaders[i].sh_type == 0) {
                continue;
            }

            inputStream.seekg(sectionHeaders[i].sh_offset, std::ios::beg);

            sections[i].bytes.resize(sectionHeaders[i].sh_size);
            inputStream.read(sections[i].bytes.data(), sectionHeaders[i].sh_size);
            // TODO safety check, this is the only naked read
        }

        // TODO bounds check names against section size.
        for (int i = 0; i < elfHeader.e_shnum; i++) {
            char* name = &sections[elfHeader.e_shstrndx].bytes[sectionHeaders[i].sh_name];
            sections[i].name = name;
        }

        status = Status::Loaded;
        inputStream.close();
    }

    ElfIdentification elfIdentification;
    ElfHeader elfHeader;
    std::vector<ProgramHeader> programHeaders;
    std::vector<SectionHeader> sectionHeaders;
    std::vector<Section> sections;

    enum class Status {
        Unloaded, BadFile, BadIdentMagic, BadIdentClass, BadIdentData,
        BadIdentVersion, BadHeaderVersion, Loaded
    };

    Status status = Status::Unloaded;

};

} // namespace HighELF