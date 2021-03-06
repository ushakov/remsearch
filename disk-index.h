#ifndef _DISK_INDEX_H_
#define _DISK_INDEX_H_

#include <inttypes.h>
#include <string>
#include <vector>
#include <ostream>

#include <boost/iostreams/device/mapped_file.hpp>

namespace boost {
    namespace iostreams {
        class mapped_file_source;
    }
}

struct Query {
  std::string query;
  bool has_viewport;
  int minx, miny;
  int maxx, maxy;
};

class DiskIndex {
public:
    static const uint32_t kEOF;
    DiskIndex();

    void LoadFromFile(std::string basename);

    // Search for string query in the index. Append coordinates and
    // titles of items where it is found to the vectors x, y, and
    // titles. Store not more than n_results results. Return a
    // (opaque) integer that can be passed as start arg to continue
    // searching. Initial searches should use 0 as start. Will return
    // kEOF if end of data was reached (i.e., there will be no more
    // matches). If in_titles == true, search only in titles, not in
    // the full text.
    uint32_t Search(const Query& query,
                    std::vector<uint32_t>* id,
                    std::vector<uint32_t>* x,
                    std::vector<uint32_t>* y,
                    std::vector<std::string>* titles,
                    int n_results, uint32_t start, bool in_titles) const;
    

    void DumpData(int handle, std::ostream& out) const;

private:
  uint32_t GetNextMatchIdx(const std::string& query, uint32_t start_idx,
			   const boost::iostreams::mapped_file_source& data,
			   const std::vector<uint32_t>& offsets) const;
    
    boost::iostreams::mapped_file_source data;
    boost::iostreams::mapped_file_source titles;
    std::vector<uint32_t> obj_id;
    std::vector<uint32_t> x_coords;
    std::vector<uint32_t> y_coords;
    std::vector<uint32_t> name_offsets;
    std::vector<uint32_t> data_offsets;
};


#endif /* _DISK_INDEX_H_ */
