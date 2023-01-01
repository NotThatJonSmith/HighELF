#include <DTBFile.hpp>

namespace HighELF {
DTBFile::Prop DTBFile::getProp() {
    fdt_prop_data prop_data;
    getWord(&prop_data.len);
    getWord(&prop_data.nameoff);
    Prop prop;
    prop.name = getNullTerminatedString(prop_data.nameoff + header.off_dt_strings);
    for (unsigned int i = 0; i < prop_data.len; i++) {
        char propDataByte;
        getByte((uint8_t*)&propDataByte);
        prop.bytes.push_back(propDataByte);
    }

    alignFilePointer(4);
    return prop;
}

DTBFile::Node DTBFile::getNode() {
    DTBFile::Node node;
    node.name = getNullTerminatedString();
    alignFilePointer(4);
    while (true) {
        uint32_t token;
        getWord(&token);
        switch (token) {
        case TOK_FDT_BEGIN_NODE:
            node.child_nodes.push_back(getNode());
            break;
        case TOK_FDT_END_NODE:
            return node;
        case TOK_FDT_PROP:    
            node.props.push_back(getProp());
            break;
        case TOK_FDT_NOP:
            // Fine, noop
            break;
        case TOK_FDT_END:
            // TODO is this even legal? Should have seen end-node before this
            break;
        default:
            // TODO something bad happened
            break;
        }
    }
}

DTBFile::DTBFile() { }

DTBFile::DTBFile(std::string filename) {
    Load(filename);
}

void DTBFile::Load(std::string filename) {

    // Every integer in DTB is big-endian
    fileEndianness = Endianness::Big;

    inputStream.open(filename, std::ios::in | std::ios::binary);

    if (!inputStream.is_open()) {
        status = Status::BadFile;
        return;
    }

    inputStream.seekg(0, std::ios::beg);

    getWord(&header.magic);
    if (header.magic != 0xd00dfeed) {
        status = Status::BadHeaderMagic;
        inputStream.close();
        return;
    }

    getWord(&header.totalsize);
    getWord(&header.off_dt_struct);
    getWord(&header.off_dt_strings);
    getWord(&header.off_mem_rsvmap);
    getWord(&header.version);
    getWord(&header.last_comp_version);
    getWord(&header.boot_cpuid_phys);
    getWord(&header.size_dt_strings);
    getWord(&header.size_dt_struct);
    // TODO header size value sense checks

    inputStream.seekg(header.off_mem_rsvmap, std::ios::beg);

    while (true) {
        fdt_reserve_entry entry;
        getDoubleWord(&entry.address);
        getDoubleWord(&entry.size);
        if (entry.address == 0 && entry.size == 0) {
            break;
        }
        memoryReservationBlocks.push_back(entry);
    }

    inputStream.seekg(header.off_dt_struct, std::ios::beg);
    bool done = false;
    while (!done) {
        uint32_t token;
        getWord(&token);
        switch (token) {
        case TOK_FDT_BEGIN_NODE:
            root = getNode();
            break;
        case TOK_FDT_END_NODE:
            // TODO error
            break;
        case TOK_FDT_PROP:
            // TODO error
            break;
        case TOK_FDT_NOP:
            break;
        case TOK_FDT_END:
            done = true;
            break;
        default:
            break;
        }
    }
}

} // namespace HighELF
