#include <fstream>
#include <algorithm>

#include <boost/iostreams/device/mapped_file.hpp>

#include "disk-index.h"

using namespace std;

const uint32_t DiskIndex::kEOF = (uint32_t)(-1);

DiskIndex::DiskIndex() {}

void DiskIndex::LoadFromFile(std::string basename) {
    string dataname(basename + ".dat");
    string idxname(basename + ".idx");
    ifstream index(idxname.c_str());
    while (!index.eof()) {
        uint32_t id, offset;
        index.read((char*)&id, sizeof(id));
        if (index.gcount() < sizeof(id)) continue;
        index.read((char*)&offset, sizeof(offset));
        if (index.gcount() < sizeof(offset)) continue;
        ids.push_back(id);
        offsets.push_back(offset);
    }
    data.open(dataname);
}

uint32_t DiskIndex::Search(
    const std::string& query, std::vector<uint32_t>* results,
        int n_results, uint32_t start) const {
    // We are interpreting start as the index in our arrays of the
    // first non-searched data chunk.
    uint32_t idx = start;
    int matches_found = 0;
    while (true) {
        uint32_t next_idx = GetNextMatchIdx(query, idx);
        if (next_idx == kEOF) {
            return kEOF;
        }
        results->push_back(ids[next_idx]);
        matches_found ++;
        if (matches_found == n_results) {
            if (next_idx + 1 < ids.size()) {
                return next_idx + 1;
            } else {
                return kEOF;
            }
        }
        idx = next_idx + 1;
    }
}
    
uint32_t DiskIndex::GetNextMatchIdx(
    const std::string& query, uint32_t start_idx) const {
    if (start_idx >= offsets.size()) {
        return kEOF;
    }
    uint32_t start = offsets[start_idx];
    const char *found = search(data.data() + start,
                               data.data() + data.size(),
                               query.begin(), query.end());
    if (found == data.data() + data.size()) {
        return kEOF;
    }
    int offset = found - data.data();
    vector<uint32_t>::const_iterator next_idx = upper_bound(offsets.begin(), offsets.end(), offset);
    if (next_idx == offsets.end()) {
        return offsets.size() - 1;
    }
    int idx = next_idx - offsets.begin();
    return idx - 1;
}

int DiskIndex::GetHandle(uint32_t id) const {
    vector<uint32_t>::const_iterator id_it = find(ids.begin(), ids.end(), id);
    if (id_it == ids.end()) {
        return -1;
    }
    return id_it - ids.begin();
}

void DiskIndex::DumpData(int handle, ostream& out) const {
    uint32_t offset = offsets[handle];
    uint32_t len = offsets[handle+1] - offset;
    
    out.write(data.data() + offset, len);
}
