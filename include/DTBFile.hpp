#pragma once

#include <stdint.h>

#include <string>
#include <vector>

#include <BinaryFile.hpp>

namespace HighELF {

class DTBFile : public BinaryFile {

public:

    struct fdt_header {
        uint32_t magic;
        uint32_t totalsize;
        uint32_t off_dt_struct;
        uint32_t off_dt_strings;
        uint32_t off_mem_rsvmap;
        uint32_t version;
        uint32_t last_comp_version;
        uint32_t boot_cpuid_phys;
        uint32_t size_dt_strings;
        uint32_t size_dt_struct;
    };

    struct fdt_reserve_entry {
        uint64_t address;
        uint64_t size;
    };

    struct Node {
        
    };

    std::vector<fdt_reserve_entry> memoryReservationBlocks;

public:

    DTBFile();
    DTBFile(std::string filename);
    virtual void Load(std::string filename) override;

};

} // namespace HighELF