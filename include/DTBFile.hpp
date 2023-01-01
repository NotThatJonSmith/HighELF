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

    struct fdt_prop_data {
        uint32_t len;
        uint32_t nameoff;
    };

    enum class Status {
        Unloaded, BadFile, BadHeaderMagic, Loaded
    };

    struct Prop {
        std::string name = "";
        std::vector<char> bytes;
    };

    struct Node {
        std::string name = "";
        std::vector<Prop> props;
        std::vector<Node> child_nodes;
    };

    static constexpr uint32_t TOK_FDT_BEGIN_NODE = 0x00000001;
    static constexpr uint32_t TOK_FDT_END_NODE = 0x00000002;
    static constexpr uint32_t TOK_FDT_PROP = 0x00000003;
    static constexpr uint32_t TOK_FDT_NOP = 0x00000004;
    static constexpr uint32_t TOK_FDT_END = 0x00000009;

public:

    DTBFile();
    DTBFile(std::string filename);
    virtual void Load(std::string filename) override;

    Status status = Status::Unloaded;
    fdt_header header;

    Node root;
    std::vector<fdt_reserve_entry> memoryReservationBlocks;

private:

    Node getNode();
    Prop getProp();
};

} // namespace HighELF