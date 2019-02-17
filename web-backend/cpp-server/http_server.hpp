#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>

#include <boost/algorithm/string_regex.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <iostream>
#include <queue>

class Request : public std::enable_shared_from_this<Request> {
   public:
    std::map<std::string, std::string> header;
    std::string url;
    std::string method;
    std::string content;
    Request() : header(), url(), method(), content(){};
};
class Response : public std::enable_shared_from_this<Response> {
   public:
    std::map<std::string, std::string> header;
    std::ostringstream data;
    Response() : header(), data(""){};
};

typedef std::map<
    std::string,
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
                std::cout << "ec.value " << ec.value() << "\tec.message"
                          << ec.message() << std::endl;
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
                //     std::cout << kv.first << "-+-" << kv.second << std::endl;
                // }
                streambuf.consume(req_header_length);
                typename std::map<std::string, std::string>::const_iterator it =
                    _header.find("Content-Length");
                int content_length = 0;
                if (it != _header.end())
                    content_length = std::atoi(it->second.data());
                // std::cout << ec.value() << std::endl;
                // std::cout << "REST LENGTH" << content_length << std::endl;
                read_req_content(req, content_length);
            });
    }
    void read_req_content(std::shared_ptr<Request> req,
                          int content_length = 0) {
        if (content_length > 0) {
            auto self(shared_from_this());
            boost::asio::async_read_until(
                *_socket, streambuf, "\r\n\r\n",
                [this, self, req](const boost::system::error_code& ec,
                                  std::size_t bytes_transferred) {
                    parse_req_content(req, ec, bytes_transferred);
                });
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
                    *_socket, boost::asio::buffer(response->data.str()),
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
        // std::ostringstream response("");
        // response << "HTTP/1.1 200 OK\r\n";
        // response << "Content-Type: text/html\r\n";
        // // std::string response_data = "<!DOCTYPE
        // html><html><head></head><body><h1>hello world!</h1></body></html>";
        // std::string response_data = "Hello world!";
        // response << "Content-Length: " << response_data.length() << "\r\n";
        // response << "\r\n";
        // response << response_data;
        // std::cout << response.str() << std::endl;
        // boost::asio::async_write(*_socket,
        // boost::asio::buffer(response.str()), [this, self](const
        // boost::system::error_code& ec, std::size_t bytes_transferred) {
        //     if (!ec) {
        //         //
        //         _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
        //         // _socket.shutdown_send();
        //     }
        // });
        for (auto& url : *_router) {
            auto it = url.second.find(req->method);
            if (it != url.second.end()) {
                std::regex r(url.first);
                if (std::regex_match(req->url, r)) {
                    it->second(req, response);
                    break;
                }
            }
        }
    }

    // void parse_header(const boost::system::error_code& ec, std::size_t
    // bytes_transferred) {
    //     auto _header = request->header;
    //     std::cout << "parse Header" << std::endl;
    //     // std::ostringstream ss;
    //     // ss << buf;
    //     // std::string header_str = ss.str();
    //     std::vector<std::string> lines;
    //     boost::asio::streambuf::const_buffers_type bufs = streambuf.data();
    //     std::string header_str(boost::asio::buffers_begin(bufs),
    //     boost::asio::buffers_begin(bufs) + bytes_transferred); std::cout <<
    //     header_str << std::endl; boost::split(lines, header_str,
    //     boost::algorithm::is_any_of("\r\n"));

    //     // PARSE
    //     std::vector<std::string> method_url_version(3);
    //     boost::split(method_url_version, lines.front(),
    //     boost::algorithm::is_any_of(" ")); lines.front() =
    //     std::move(lines.back()); lines.pop_back();
    //     assert(method_url_version.size() == 3);
    //     std::string method = method_url_version[0],
    //                 url = method_url_version[1],
    //                 version = method_url_version[2];
    //     std::cout << url << std::endl;
    //     for (auto l : lines) {
    //         if (l.length() == 0) {
    //             continue;
    //         }
    //         // std::cout << l.length() << " " << l << std::endl;
    //         std::size_t split_pos = l.find_first_of(':');

    //         if (split_pos == std::string::npos) {
    //             continue;
    //         }

    //         std::string k = std::string(std::move(l.substr(0, split_pos)));
    //         std::string v = std::string(std::move(l.substr(split_pos + 1,
    //         l.length()))); std::cout << split_pos << "[" << k << "]" << v <<
    //         std::endl; _header.insert(std::pair<std::string, std::string>(k,
    //         v));
    //     }
    //     for (auto kv : _header) {
    //         std::cout << kv.first << "-+-" << kv.second << std::endl;
    //     }
    //     streambuf.consume(bytes_transferred);
    //     typename std::map<std::string, std::string>::const_iterator it =
    //     _header.find("Content-Length"); if (it == _header.end())
    //         content_length = 0;
    //     else
    //         content_length = std::atoi(it->second.data());
    //     std::cout << ec.value() << std::endl;
    //     std::cout << "REST LENGTH" << content_length << std::endl;
    // };
    // void read_header() {
    //     auto self(shared_from_this());
    //     boost::asio::async_read_until(*_socket, streambuf, "\r\n\r\n", [this,
    //     self](const boost::system::error_code& ec, std::size_t
    //     bytes_transferred) {
    //         parse_header(ec, bytes_transferred);
    //         read_content();
    //     });
    // }
};