#include <iostream>
#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>
#include <glib.h>

#include "disk-index.h"

DiskIndex g_index;

class Serve: public Fastcgipp::Request<char>
{
public:
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

            out << "Content-Type: text/plain; charset=utf-8\r\n\r\n";

            if (cont == DiskIndex::kEOF) {
                out << "N: EOF\r\n";
            } else {
                out << "N: " << cont << "\r\n";
            }
            for (int i = 0; i < x.size(); ++i) {
                out << x[i] << " " << y[i] << " " << titles[i] <<"\r\n";
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
