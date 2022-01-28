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

    ElfFile();
    ElfFile(std::string filename);
    virtual void Load(std::string filename) override;

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