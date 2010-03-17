#include <iostream>
#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>
#include <glib.h>

#include "disk-index.h"

DiskIndex g_index;

class Serve: public Fastcgipp::Request<char>
{
public:
    void out_int_be(int n) {
        char buf[4];
        buf[0] = (n >> 24) & 0xff;
        buf[1] = (n >> 16) & 0xff;
        buf[2] = (n >> 8) & 0xff;
        buf[3] = n & 0xff;
        out.write(buf, 4);
    }

    bool response()
    {
        int id;
        if (environment.requestVarGet("id", id)) {
            // Document dump requested
            out << "Content-Type: text/html; charset=utf-8\r\n\r\n";
            g_index.DumpData(id, out);
            return true;
        } else {
            std::string query;
            int start;
            int num;
            if (!environment.requestVarGet("q", query)) {
                out << "Status: 400\r\n\r\n";
                return true;
            }
            // For some reason, requestVarGet does not convert + back to space
            for (int i = 0; i < query.size(); ++i) {
                if (query[i] == '+') {
                    query[i] = ' ';
                }
            }
            // convert to lowercase
            char *q_lower = g_utf8_strdown(query.c_str(), -1);
            query.assign(q_lower);
            free(q_lower);

            if (!environment.requestVarGet("s", start)) {
                start = 0;
            }

            if (!environment.requestVarGet("n", num)) {
                num = 10;
            }

            std::vector<uint32_t> x;
            std::vector<uint32_t> y;
            std::vector<std::string> titles;
            uint32_t cont = g_index.Search(query, &x, &y, &titles, num, start);

            int text_size = 0;
            for (int i = 0; i < titles.size(); ++i) {
                text_size += titles[i].size();
            }

            int content_length = 4 + 4 + x.size()*12 + text_size;
            out << "Content-Type: application/octet-stream\r\n";
            out << "Content-Length: " << content_length << "\r\n";
            out << "\r\n";

            if (cont == DiskIndex::kEOF) {
                out_int_be(-1);
            } else {
                out_int_be(cont);
            }
            out_int_be(x.size());
            for (int i = 0; i < x.size(); ++i) {
                out_int_be(x[i]);
                out_int_be(y[i]);
                out_int_be(titles[i].size());
                out.write(titles[i].c_str(), titles[i].size());
            }
            return true;
        }
        return true;
    }
};

int main()
{
    g_index.LoadFromFile("/home/ushakov/maps/mobile/wikimapia-data/search");
    try
    {
        Fastcgipp::Manager<Serve> fcgi;
        fcgi.handler();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
