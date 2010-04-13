#include <iconv.h>
#include <errno.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "placemark-storage.h"

using namespace std;

namespace {
    int ReadInt(istream& in) {
        int res = 0;
        unsigned char c;
        for (int i = 0; i < 4; ++i) {
            res <<= 8;
            in.read((char*)&c, 1);
            res += c;
        }
        return res;
    }

    void Skip(istream& in, int len) {
        char t;
        for (int i = 0; i < len; ++i) {
            in.read(&t, 1);
        }
    }

    void SkipGeometry(istream& in) {
        int len = ReadInt(in);
        Skip(in, (len+1)*4);
        int cntlen = ReadInt(in);
        Skip(in, cntlen*4);
    }

    void ReadUtf8String(istream& in, int num, string* str) {
        const int BUFLEN = 8192;
        char inbuf[BUFLEN];
        int converted = 0;
        while (converted < num) {
            int len = min(num - converted, BUFLEN);
            in.read(inbuf, len);
            str->append(inbuf, inbuf + len);
            converted += len;
        }
    }
    
    void ReadUtf16String(istream& in, int num, string* str) {
        static iconv_t conv = NULL;
        if (!conv) {
            conv = iconv_open("utf8", "utf16be");
        }
        const int BUFLEN = 512;
        char inbuf[BUFLEN];
        char outbuf[BUFLEN*2];
        int converted = 0;
        while (converted < num) {
            int len = min((num - converted)*2, BUFLEN);
            in.read(inbuf, len);
            char *inptr = inbuf;
            char *outptr = outbuf;
            size_t inbytesleft = len;
            size_t outbytesleft = BUFLEN*2;
            size_t cnv = iconv(conv, &inptr, &inbytesleft, &outptr, &outbytesleft);
            if (cnv == (size_t)-1) {
                cerr << "Error: " << errno << endl;
                cerr << "converted " << converted + (inptr - inbuf)/2 << " out of " << num << endl;
                exit(1);
            }
            if (inbytesleft != 0) {
                cerr << "Incomplete conversion" << endl;
                exit(2);
            }
            str->append(outbuf, outptr);
            converted += len/2;
        }
    }

    void ReadVector(istream& in, int size, vector<int>* v) {
        for (int i = 0; i < size; ++i) {
            v->push_back(ReadInt(in));
        }
    }
}

void PlacemarkStorage::AddFromFile(string name) {
    ifstream ifs(name.c_str());
    while (!ifs.eof()) {
        // First should be 1 or 2 (version number)
        int version = ReadInt(ifs);
        if (version != 1 && version != 2) {
            if (ifs.eof()) {
                continue;
            }
            cerr << "Wrong format (" << version << "): " << name << endl;
            exit(1);
        }
        // Names
        int num_entries = ReadInt(ifs);
        //cerr << "num_entries=" << num_entries << endl;
        vector<int32_t> start;
        ReadVector(ifs, num_entries+1, &start);
        unsigned int cntlen = ReadInt(ifs);
        //cerr << "cntlen=" << cntlen << endl;
        // Now read name contents and place into vector
        vector<string> names_in_array(num_entries);
        for (int i = 0; i < num_entries; ++i) {
            //cerr << "start=" << start[i] << " next=" << start[i+1] << endl;
            int len = start[i + 1] - start[i];
            //cerr << "i=" << i << " len=" << len << endl;
            if (version == 1) {
                ReadUtf16String(ifs, len, &names_in_array[i]);
            } else {
                ReadUtf8String(ifs, len, &names_in_array[i]);
            }
        }
        // Geometry
        SkipGeometry(ifs);
        int num_entries_coords = ReadInt(ifs);
        if (num_entries_coords != num_entries) {
            cerr << "num_entries mismatch: " << name << endl;
            cerr << "num_entries=" << num_entries << " num_entries_coords=" << num_entries_coords << endl;
            exit(1);
        }
        vector<int32_t> ids;
        ReadVector(ifs, num_entries, &ids);
        vector<int32_t> lowx;
        ReadVector(ifs, num_entries, &lowx);
        vector<int32_t> highx;
        ReadVector(ifs, num_entries, &highx);
        vector<int32_t> lowy;
        ReadVector(ifs, num_entries, &lowy);
        vector<int32_t> highy;
        ReadVector(ifs, num_entries, &highy);
        for (int i = 0; i < num_entries; ++i) {
            int x = lowx[i] + (highx[i] - lowx[i])/2;
            int y = lowy[i] + (highy[i] - lowy[i])/2;
            names[ids[i]] = names_in_array[i];
            xcoord[ids[i]] = y;
            ycoord[ids[i]] = x;
            //cerr << names_in_array[i] << endl;
        }
    }
}
