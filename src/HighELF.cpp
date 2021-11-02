#include <HighELF.hpp>

namespace HighELF {

ElfFile::ElfFile() {
}

ElfFile::ElfFile(std::string filename) {
    Load(filename);
}

void ElfFile::Load(std::string filename) {

    elfIfStream.open(filename, std::ios::in | std::ios::binary);

    if (!elfIfStream.is_open()) {
        status = Status::BadFile;
        return;
    }

    elfIfStream.seekg(0, std::ios::beg);

    getByte(&ei_magic0);
    getByte(&ei_magic1);
    getByte(&ei_magic2);
    getByte(&ei_magic3);
    getByte(&ei_class);
    getByte(&ei_data);
    getByte(&ei_version);
    getByte(&ei_osabi);
    getByte(&ei_abiversion);

    for (int i = 0; i < 7; i++) {
        getByte(&ei_pad[i]);
    }

    if (ei_magic0 != 0x7F || ei_magic1 != 'E'  ||
        ei_magic2 != 'L'  || ei_magic3 != 'F') {
        status = Status::BadIdentMagic;
        elfIfStream.close();
        return;
    }

    if (ei_class != 1 && ei_class != 2) {
        status = Status::BadIdentClass;
        elfIfStream.close();
        return;
    }

    bool fileIs64Bit = ei_class == 2;

    if (ei_data != 1 && ei_data != 2) {
        status = Status::BadIdentData;
        elfIfStream.close();
        return;
    }

    bool fileIsBigEndian = ei_data == 2;
    endiansMismatch = fileIsBigEndian != ElfFile::isHostBigEndian();

    if (ei_version != 1) {
        status = Status::BadIdentVersion;
        elfIfStream.close();
        return;
    }

    getHalfWord(&e_type);
    getHalfWord(&e_machine);
    getWord(&e_version);

    if (e_version != 1) {
        status = Status::BadHeaderVersion;
        elfIfStream.close();
        return;
    }

    if (fileIs64Bit) {
        getDoubleWord(&e_entry);
        getDoubleWord(&e_phoff);
        getDoubleWord(&e_shoff);
    } else {
        uint32_t tmp;
        getWord(&tmp); e_entry = tmp;
        getWord(&tmp); e_phoff = tmp;
        getWord(&tmp); e_shoff = tmp;
    }

    getWord(&e_flags);
    getHalfWord(&e_ehsize);
    getHalfWord(&e_phentsize);
    getHalfWord(&e_phnum);
    getHalfWord(&e_shentsize);
    getHalfWord(&e_shnum);
    getHalfWord(&e_shstrndx);

    // TODO check that the ELF header size matches how much we've read so far
    // TODO check that we're still in a sane file location
    // TODO check that phentsize matches expectation for 32/64 bit class ID
    // TODO check that phnum and shnum are reasonable

    programHeaders.resize(e_phnum);
    sectionHeaders.resize(e_shnum);
    sections.resize(e_shnum);

    // TODO be skeptical about how much you're supposed to read w/ the following
    // unsigned int phtBytesRead = 0;
    // unsigned int phtBytesMax = phnum * phentsize;

    elfIfStream.seekg(e_phoff, std::ios::beg);

    if (fileIs64Bit) {
        for (int i = 0; i < e_phnum; i++) {
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
        for (int i = 0; i < e_phnum; i++) {
            uint32_t tmp;
            getWord(&programHeaders[i].p_type);
            getWord(&tmp); programHeaders[i].p_offset = tmp;
            getWord(&tmp); programHeaders[i].p_vaddr = tmp;
            getWord(&tmp); programHeaders[i].p_paddr = tmp;
            getWord(&tmp); programHeaders[i].p_filesz = tmp;
            getWord(&tmp); programHeaders[i].p_memsz = tmp;
            getWord(&tmp); programHeaders[i].p_flags = tmp;
            getWord(&tmp); programHeaders[i].p_align = tmp;
        }
    }

    elfIfStream.seekg(e_shoff, std::ios::beg);

    if (fileIs64Bit) {
        for (int i = 0; i < e_shnum; i++) {
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
        for (int i = 0; i < e_shnum; i++) {
            uint32_t tmp;
            getWord(&sectionHeaders[i].sh_name);
            getWord(&sectionHeaders[i].sh_type);
            getWord(&tmp); sectionHeaders[i].sh_flags = tmp;
            getWord(&tmp); sectionHeaders[i].sh_addr = tmp;
            getWord(&tmp); sectionHeaders[i].sh_offset = tmp;
            getWord(&tmp); sectionHeaders[i].sh_size = tmp;
            getWord(&sectionHeaders[i].sh_link);
            getWord(&sectionHeaders[i].sh_info);
            getWord(&tmp); sectionHeaders[i].sh_addralign = tmp;
            getWord(&tmp); sectionHeaders[i].sh_entsize = tmp;
        }
    }

    for (int i = 0; i < e_shnum; i++) {

        // SHT_NOBITS and SHT_NULL have no data to read, regardless of size
        if (sectionHeaders[i].sh_type == 8 ||
            sectionHeaders[i].sh_type == 0) {
            continue;
        }

        elfIfStream.seekg(sectionHeaders[i].sh_offset, std::ios::beg);

        sections[i].bytes.resize(sectionHeaders[i].sh_size);
        elfIfStream.read(sections[i].bytes.data(), sectionHeaders[i].sh_size);
        // TODO safety check, this is the only naked read
    }

    // TODO bounds check names against section size.
    for (int i = 0; i < e_shnum; i++) {
        char* name = &sections[e_shstrndx].bytes[sectionHeaders[i].sh_name];
        sections[i].name = name;
    }

    status = Status::Loaded;
    elfIfStream.close();
}

void ElfFile::getByte(uint8_t* buf) {
    elfIfStream.read((char*)buf, 1);
}

void ElfFile::getHalfWord(uint16_t* buf) {
    elfIfStream.read((char*)buf, 2);
    if (endiansMismatch) {
        *buf = ElfFile::swapEndianness<uint16_t>(*buf);
    }
}

void ElfFile::getWord(uint32_t* buf) {
    elfIfStream.read((char*)buf, 4);
    if (endiansMismatch) {
        *buf = ElfFile::swapEndianness<uint32_t>(*buf);
    }
}

void ElfFile::getDoubleWord(uint64_t* buf) {
    elfIfStream.read((char*)buf, 8);
    if (endiansMismatch) {
        *buf = ElfFile::swapEndianness<uint64_t>(*buf);
    }
}

bool ElfFile::isHostBigEndian() {
    union { uint32_t word; char bytes[4]; } test = { 0x01000000 };
    return test.bytes[0] == 0x01;
}

} // namespace HighELF
