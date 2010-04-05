#include <iostream>
#include <fstream>
#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>
#include <glib.h>

#include "disk-index.h"

DiskIndex g_index;

static const char* index_filename;
static const char* html_path;

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

    void output_binary(const std::vector<uint32_t>& x,
                       const std::vector<uint32_t>& y,
                       const std::vector<std::string>& titles,
                       int cont) {
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
    }

    std::string quote(std::string q) {
        std::string o;
        for (int i = 0; i < q.size(); ++i) {
            if (q[i] == '\\') {
                o += "\\";
            } else if (q[i] == '\'') {
                o += "\'";
            } else {
                o += q[i];
            }
        }
        return o;
    }

    void output_json(std::string query,
                     const std::vector<uint32_t>& x,
                     const std::vector<uint32_t>& y,
                     const std::vector<std::string>& titles,
                     int cont) {
        out << "Content-Type: application/json\r\n";
        out << "\r\n";
        out << "{'cont':" << cont << ", ";
        out << "'q': '" << quote(query) << "',";
        out << "'data': [";
        for (int i = 0; i < x.size(); ++i) {
            out << "{ 'x': " << x[i] << ", ";
            out << "'y': " << y[i] << ", ";
            out << "'title': '" << quote(titles[i]) << "'}, ";
        }
        out << "]}";
    }

    bool response()
    {
        int id;
        std::string query;
        if (environment.requestVarGet("id", id)) {
            // Document dump requested
            out << "Content-Type: text/html; charset=utf-8\r\n\r\n";
            g_index.DumpData(id, out);
            return true;
        } else if (environment.requestVarGet("q", query)) {
            int start;
            int num;
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

            std::string output;
            if (environment.requestVarGet("output", output) &&
                output == "json") {
                output_json(query, x, y, titles, cont);
            } else {
                output_binary(x, y, titles, cont);
            }
            
            return true;
        } else {
            // Serve static files
            std::string name;
            if (!environment.requestVarGet("name", name)) {
                name = "index.html";
            }
            if (name.find("..") != std::string::npos) {
                out << "Status: 400 Bad request\r\n\r\n";
                return true;
            }
            name = std::string(html_path) + "/" + name;
            std::ifstream ifs(name.c_str());
            if (!ifs.is_open()) {
                out << "Status: 404 Not found\r\n\r\n";
                return true;
            }
            out << "Content-Type: text/html; charset=utf-8\r\n";
            out << "\r\n";
            out.dump(ifs);
            return true;
        }
        return true;
    }
};

static const char* socket_name = "/tmp/remsearch.sock";

int main(int argc, char **argv)
{
  if (argc < 3) {
    std::cerr << "Two arguments required: ./serve index_path html_path\n";
    return 1;
  }
  index_filename = argv[1];
  html_path = argv[2];
  
  struct sockaddr_un sock_addr;
  int error;

  unlink(socket_name);
  int fd = socket(PF_UNIX, SOCK_STREAM, 0);
  memset(&sock_addr, 0, sizeof(struct sockaddr_un));
  sock_addr.sun_family = AF_UNIX;
  strcpy(sock_addr.sun_path, socket_name);
  error = bind(fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
  if (error) {
    std::ostringstream err;
    err << "Cannot bind socket: " << strerror(errno);
    throw std::runtime_error(err.str());
  }
  chmod(socket_name, 0666);

  ::listen(fd, 10);

  g_index.LoadFromFile(argv[1]);
    try
    {
        Fastcgipp::Manager<Serve> fcgi(fd);
        fcgi.handler();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
