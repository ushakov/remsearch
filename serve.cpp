#include <iostream>
#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>

#include "disk-index.h"

DiskIndex g_index;

class Serve: public Fastcgipp::Request<char>
{
public:
    bool response()
    {
        int id;
        if (environment.requestVarGet("id", id)) {
            // Concrete id requested
            int handle = g_index.GetHandle(id);
            if (handle < 0) {
                out << "Status: 404\r\n\r\n";
                return true;
            }
            out << "Content-Type: text/html; charset=utf-8\r\n\r\n";
            g_index.DumpData(handle, out);
            return true;
        } else {
            std::string query;
            int start;
            int num;
            if (!environment.requestVarGet("q", query)) {
                out << "Status: 400r\n\r\n";
                return true;
            }
            
            if (!environment.requestVarGet("s", start)) {
                start = 0;
            }

            if (!environment.requestVarGet("n", num)) {
                num = 10;
            }

            std::vector<uint32_t> results;
            uint32_t cont = g_index.Search(query, &results, num, start);

            out << "Content-Type: text; charset=utf-8\r\n\r\n";

            if (cont == DiskIndex::kEOF) {
                out << "N: EOF\r\n";
            } else {
                out << "N: " << cont << "\r\n";
            }
            for (int i = 0; i < results.size(); ++i) {
                out << results[i] << "\r\n";
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
