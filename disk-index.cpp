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
    string titlename(basename + ".ttl");
    ifstream index(idxname.c_str());
    while (!index.eof()) {
        uint32_t id, x, y, name_offset, data_offset;
        index.read((char*)&id, sizeof(id));
        index.read((char*)&x, sizeof(x));
        if (index.gcount() < sizeof(x)) continue;
        index.read((char*)&y, sizeof(y));
        if (index.gcount() < sizeof(y)) continue;
        index.read((char*)&name_offset, sizeof(name_offset));
        if (index.gcount() < sizeof(name_offset)) continue;
        index.read((char*)&data_offset, sizeof(data_offset));
        if (index.gcount() < sizeof(data_offset)) continue;
        obj_id.push_back(id);
        x_coords.push_back(x);
        y_coords.push_back(y);
        name_offsets.push_back(name_offset);
        data_offsets.push_back(data_offset);
    }
    data.open(dataname);
    titles.open(titlename);
    name_offsets.push_back(titles.size());
}

uint32_t DiskIndex::Search(
	const Query& query,
        std::vector<uint32_t>* id,
        std::vector<uint32_t>* x,
        std::vector<uint32_t>* y,
        std::vector<std::string>* t,
        int n_results, uint32_t start, bool in_titles) const {
    // We are interpreting start as the index in our arrays of the
    // first non-searched data chunk.
    uint32_t idx = start;
    int matches_found = 0;
    while (true) {
        uint32_t next_idx;
        if (in_titles) {
	    next_idx = GetNextMatchIdx(query.query, idx, titles, name_offsets);
        } else {
	    next_idx = GetNextMatchIdx(query.query, idx, data, data_offsets);
        }
        if (next_idx == kEOF) {
            return kEOF;
        }
        idx = next_idx + 1;
	if (query.has_viewport) {
	  if (x_coords[next_idx] < query.minx || x_coords[next_idx] > query.maxx) {
	    continue;
	  }
	  if (y_coords[next_idx] < query.miny || y_coords[next_idx] > query.maxy) {
	    continue;
	  }
	}
        id->push_back(obj_id[next_idx]);
        x->push_back(x_coords[next_idx]);
        y->push_back(y_coords[next_idx]);
        std::string title(titles.data() + name_offsets[next_idx],
                          titles.data() + name_offsets[next_idx+1]);
        t->push_back(title);
        matches_found ++;
        if (matches_found == n_results) {
            if (next_idx + 1 < data_offsets.size()) {
                return next_idx + 1;
            } else {
                return kEOF;
            }
        }
    }
}
    
uint32_t DiskIndex::GetNextMatchIdx(
    const std::string& query, uint32_t start_idx,
    const boost::iostreams::mapped_file_source& data,
    const std::vector<uint32_t>& offsets) const {
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
    vector<uint32_t>::const_iterator next_idx =
        upper_bound(offsets.begin(), offsets.end(), offset);
    if (next_idx == offsets.end()) {
        return offsets.size() - 1;
    }
    int idx = next_idx - offsets.begin();
    return idx - 1;
}

void DiskIndex::DumpData(int handle, ostream& out) const {
    uint32_t offset = data_offsets[handle];
    uint32_t len = data_offsets[handle+1] - offset;
    
    out.write(data.data() + offset, len);
}
