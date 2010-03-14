#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <glib.h>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include "placemark-storage.h"

using namespace std;

template<typename Callable>
void walk(string root, Callable& visitor) {
    using namespace boost::filesystem;
    basic_recursive_directory_iterator<path> cur(root);
    basic_recursive_directory_iterator<path> end;
    for (; cur != end; ++cur) {
        visitor(cur->path().string());
    }
}

class GatheringVisitor {
public:
    void operator() (string name) {
        if (name.find("wmdescr_") != string::npos) {
            descr_names.push_back(name);
        }
        if (name.find("wmapi_") != string::npos) {
            data_names.push_back(name);
        }
    }
    
    vector<string> descr_names;
    vector<string> data_names;
};

int dump_one_file(string filename, ostream& out) {
    ifstream ifs(filename.c_str());
    const int buflen = 2048;
    string content;
    char buf[buflen];
    while (!ifs.eof()) {
        ifs.read(buf, buflen);
        int r = ifs.gcount();
        content.append(buf, buf + r);
    }
    char *lower = g_utf8_strdown(content.c_str(), -1);
    size_t size = strlen(lower);
    out.write(lower, size);
    free(lower);
    return size;
}

static boost::regex wikimapia_id_re("wmdescr_([0-9]*)\\.html");

void dump_files(const vector<string>& filenames, PlacemarkStorage& ps, string basename) {
    string dataname(basename + ".dat");
    string idxname(basename + ".idx");
    string titlename(basename + ".ttl");
    ofstream data(dataname.c_str());
    ofstream index(idxname.c_str());
    ofstream titles(titlename.c_str());
    uint32_t offset = 0;
    uint32_t names_offset = 0;
    int dumped = 0;
    boost::match_results<std::string::const_iterator> match;
    for (int i = 0; i < filenames.size(); ++i) {
        if (!boost::regex_search(filenames[i], match, wikimapia_id_re)) {
            continue;
        }
        uint32_t id = boost::lexical_cast<int>(string(match[1].first, match[1].second));
        if (ps.names.find(id) != ps.names.end()) {
            int length = dump_one_file(filenames[i], data);

            int x = ps.lng[id];
            int y = ps.lat[id];
            string name = ps.names[id];

            index.write((char*)&x, sizeof(id));
            index.write((char*)&y, sizeof(id));

            index.write((char*)&names_offset, sizeof(names_offset));
            titles.write(name.c_str(), name.size());
            names_offset += name.size();

            index.write((char*)&offset, sizeof(offset));
            offset += length;
            dumped++;
            if (dumped % 1000 == 0) {
                cout << "Dumped " << dumped << " out of " << filenames.size() << "; size=" << offset << endl;
            }
        }
    }
    uint32_t dummy = 0;
    index.write((char*)&dummy, sizeof(dummy));
    index.write((char*)&offset, sizeof(offset));
    data.close();
    index.close();
    cout << "Total size: " << offset << endl;
}

int main(int argc, char **argv) {
    GatheringVisitor files;
    walk(argv[1], files);
    PlacemarkStorage ps;
    for (int i = 0; i < files.data_names.size(); ++i) {
        ps.AddFromFile(files.data_names[i]);
    }
    dump_files(files.descr_names, ps, "search");
}
