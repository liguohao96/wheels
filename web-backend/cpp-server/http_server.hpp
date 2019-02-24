#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>

#include <boost/algorithm/string_regex.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

#include <iostream>
#include <queue>
#include <regex>

class Request : public std::enable_shared_from_this<Request> {
   public:
    std::map<std::string, std::string> header;
    std::string url;
    std::string method;
    std::string content;
    Request() : header(), url(), method(), content(){};
};
typedef std::map<std::string, std::string> mime_type;
class Response : public std::enable_shared_from_this<Response> {
   private:
    std::map<std::string, std::string> _header;

   public:
    std::ostringstream content;
    static std::shared_ptr<mime_type> MIME;
    Response() : _header(), content(""){};
    Response& status(const unsigned short& status_code) {
        content << "HTTP/1.1 " << status_code << " OK\r\n";
        return *this;
    };
    Response& header(const std::string& key, const std::string& value) {
        _header[key] = value;
        return *this;
    };
    Response& send_header() {
        for (auto& itr : _header) {
            content << itr.first << ": " << itr.second << "\r\n";
        }
        content << "\r\n";
        return *this;
    }
    void send_file(boost::filesystem::path abs_file_path) {
        auto ifs = std::make_shared<std::ifstream>();
        std::cout << abs_file_path << std::endl;
        // (*MIME)[std::string(".html")] = std::string("text/html");
        // MIME->insert(std::pair<std::string, std::string>(".html", "1"));
        std::cout << ".html" << std::endl;
        auto mime_itr = MIME->find(abs_file_path.extension().string());
        std::string mime_str;
        if (mime_itr != MIME->end()) {
            std::cout << mime_itr->second << std::endl;
            mime_str = std::move(mime_itr->second);
        }
        ifs->open(abs_file_path.string(),
                  std::ifstream::in | std::ios::binary | std::ios::ate);

        if (*ifs) {
            auto length = ifs->tellg();
            ifs->seekg(0, std::ios::beg);
            _header["Content-Length"] = std::to_string(length);
            _header["Content-Type"] = mime_str;

            status(200);
            for (auto& itr : _header) {
                content << itr.first << ": " << itr.second << "\r\n";
            }
            // response << "Content-Type: " << mime_str << "\r\n";
            // response << "Content-Length: " << header["Content-Length"]
            //          << "\r\n";
            content << "\r\n";

            boost::asio::streambuf sb;
            ifs->get(sb, '\0');

            boost::asio::streambuf::const_buffers_type bufs = sb.data();
            std::string file_content(boost::asio::buffers_begin(bufs),
                                     boost::asio::buffers_end(bufs));
            content << file_content;
        }
    };
};
mime_type* mime_ptr = new mime_type{
    std::pair<std::string, std::string>(".html", "text/html"),
    std::pair<std::string, std::string>(".css", "text/css"),
    std::pair<std::string, std::string>(".js", "application/javascript"),
    std::pair<std::string, std::string>(".svg", "image/svg+xml"),
    };
std::shared_ptr<mime_type> Response::MIME(mime_ptr);
class OrderedRegex : public std::regex {
   public:
    std::string regex_str;

   public:
    OrderedRegex(const char* c_str) : std::regex(c_str), regex_str(c_str){};
    OrderedRegex(std::string str)
        : std::regex(str), regex_str(std::move(str)){};
    bool operator<(const OrderedRegex& rhs) const {
        return regex_str < rhs.regex_str;
    };
};
typedef std::map<
    OrderedRegex,
    std::map<std::string, std::function<void(std::shared_ptr<Request>,
                                             std::shared_ptr<Response>)>>>
    router_type;

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
   private:
    boost::asio::streambuf streambuf;
    std::shared_ptr<router_type> _router;

   public:
    std::unique_ptr<boost::asio::ip::tcp::socket> _socket;
    // std::shared_ptr<Request> request;
    // std::shared_ptr<Response> response;

   public:
    HttpConnection(std::unique_ptr<boost::asio::ip::tcp::socket> sock,
                   std::shared_ptr<router_type> router)
        : _socket(std::move(sock)), _router(router){};

    void deal() {
        // read_header();
        auto self(shared_from_this());
        boost::asio::async_read_until(
            *_socket, streambuf, "\r\n\r\n",
            [this, self](const boost::system::error_code& ec,
                         std::size_t req_header_length) {
                // parse_header(ec, bytes_transferred);
                if (ec) {
                    return;
                }
                auto req = std::shared_ptr<Request>(new Request);
                auto _header = req->header;
                // std::cout << "parse Header" << std::endl;
                // std::ostringstream ss;
                // ss << buf;
                // std::string header_str = ss.str();
                std::vector<std::string> lines;
                boost::asio::streambuf::const_buffers_type bufs =
                    streambuf.data();
                std::string header_str(
                    boost::asio::buffers_begin(bufs),
                    boost::asio::buffers_begin(bufs) + req_header_length);
                // std::cout << header_str << std::endl;
                boost::split(lines, header_str,
                             boost::algorithm::is_any_of("\r\n"));

                // PARSE
                std::vector<std::string> method_url_version(3);
                std::string first_line(lines.front());
                lines.front() = std::move(lines.back());
                lines.pop_back();
                boost::split(method_url_version, first_line,
                             boost::algorithm::is_any_of(" "));
                std::cout << "ec.value=" << ec.value()
                          << " ec.message=" << ec.message() << std::endl;
                if (method_url_version.size() != 3) {
                    std::cout << method_url_version.size() << " " << first_line
                              << std::endl;
                    for (auto s : method_url_version) {
                        std::cout << s << "\t";
                    }
                    std::cout << std::endl;
                }
                assert(method_url_version.size() == 3);
                std::string method = method_url_version[0],
                            url = method_url_version[1],
                            version = method_url_version[2];
                req->url = method_url_version[1];
                req->method = method_url_version[0];
                // std::cout << url << std::endl;
                for (auto l : lines) {
                    if (l.length() == 0) {
                        continue;
                    }
                    // std::cout << l.length() << " " << l << std::endl;
                    std::size_t split_pos = l.find_first_of(':');

                    if (split_pos == std::string::npos) {
                        continue;
                    }

                    std::string k =
                        std::string(std::move(l.substr(0, split_pos)));
                    std::string v = std::string(
                        std::move(l.substr(split_pos + 1, l.length())));
                    // std::cout << split_pos << "[" << k << "]" << v <<
                    // std::endl;
                    _header.insert(std::pair<std::string, std::string>(k, v));
                }
                // for (auto kv : _header) {
                //     std::cout << kv.first << "-+-" << kv.second <<
                //     std::endl;
                // }
                streambuf.consume(req_header_length);
                typename std::map<std::string, std::string>::const_iterator it =
                    _header.find("Content-Length");
                int content_length = 0;
                if (it != _header.end())
                    content_length = std::atoi(it->second.data());
                // std::cout << ec.value() << std::endl;
                // std::cout << "REST LENGTH" << content_length <<
                // std::endl;
                read_req_content(req, content_length - streambuf.size());
            });
    }
    void read_req_content(std::shared_ptr<Request> req, int unread_length = 0) {
        if (unread_length > 0) {
            std::cout << "read the rest " << unread_length << std::endl;
            auto self(shared_from_this());
            boost::asio::async_read(
                *_socket, streambuf,
                boost::asio::transfer_exactly(unread_length),
                [this, self, req](const boost::system::error_code& ec,
                                  std::size_t bytes_transferred) {
                    std::cout << bytes_transferred << std::endl;
                    parse_req_content(req, ec, bytes_transferred);
                });
            // boost::asio::async_read_until(
            //     *_socket, streambuf, "\r\n\r\n",
            //     [this, self, req](const boost::system::error_code& ec,
            //                       std::size_t bytes_transferred) {
            //         std::cout << bytes_transferred << std::endl;
            //         parse_req_content(req, ec, bytes_transferred);
            //     });
        } else {
            boost::system::error_code ec = boost::system::error_code();
            parse_req_content(req, ec, std::size_t(0));
        }
    }
    void parse_req_content(std::shared_ptr<Request> req,
                           const boost::system::error_code& ec,
                           std::size_t bytes_transferred) {
        boost::asio::streambuf::const_buffers_type bufs = streambuf.data();
        // std::cout << "parse content" << std::endl;
        std::string content_data;
        if (bytes_transferred > 0) {
            content_data = std::move(std::string(
                boost::asio::buffers_begin(bufs),
                boost::asio::buffers_begin(bufs) + bytes_transferred));
        } else if (bufs.size() > 0) {
            content_data =
                std::move(std::string(boost::asio::buffers_begin(bufs),
                                      boost::asio::buffers_end(bufs)));
        }

        // std::cout << content_data << std::endl;
        req->content = std::move(content_data);

        auto self(shared_from_this());
        auto response = std::shared_ptr<Response>(
            new Response(), [this, self](Response* res_ptr) {
                auto response = std::shared_ptr<Response>(res_ptr);
                // boost::asio::buffer send_raw_data();
                // send_raw_data <<

                boost::asio::async_write(
                    *_socket, boost::asio::buffer(response->content.str()),
                    [this, self](const boost::system::error_code& ec,
                                 std::size_t write_res_length) {
                        std::cout << "async write in deletetor, ec.value = "
                                  << ec.value() << std::endl;
                        if (!ec) {
                            // _socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                            _socket->close();
                        }
                    });
            });
        std::cout << req->method << std::endl;
        for (auto& url_method_itr : *_router) {
            auto it = url_method_itr.second.find(req->method);
            if (it != url_method_itr.second.end()) {
                if (std::regex_match(req->url, url_method_itr.first)) {
                    it->second(req, response);
                    break;
                }
            }
        }
    }
};