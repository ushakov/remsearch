#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

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
            filenames.push_back(name);
        }
    }
    
    vector<string> filenames;
};

int dump_one_file(string filename, ostream& out) {
    ifstream ifs(filename.c_str());
    const int buflen = 2048;
    char buf[buflen];
    int bytes_written = 0;
    while (!ifs.eof()) {
        ifs.read(buf, buflen);
        int r = ifs.gcount();
        out.write(buf, r);
        bytes_written += r;
    }
    return bytes_written;
}

static boost::regex wikimapia_id_re("wmdescr_([0-9]*)\\.html");

void dump_files(const vector<string>& filenames, string basename) {
    string dataname(basename + ".dat");
    string idxname(basename + ".idx");
    ofstream data(dataname.c_str());
    ofstream index(idxname.c_str());
    uint32_t offset = 0;
    boost::match_results<std::string::const_iterator> match;
    for (int i = 0; i < filenames.size(); ++i) {
        if (!boost::regex_search(filenames[i], match, wikimapia_id_re)) {
            continue;
        }
        uint32_t id = boost::lexical_cast<int>(string(match[1].first, match[1].second));
        int length = dump_one_file(filenames[i], data);
        index.write((char*)&id, sizeof(id));
        index.write((char*)&offset, sizeof(offset));
        offset += length;
        if (i % 1000 == 999) {
            cout << "Dumped " << (i+1) << " out of " << filenames.size() << "; size=" << offset << endl;
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
    dump_files(files.filenames, "search");
}
