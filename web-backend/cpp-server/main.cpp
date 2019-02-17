#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/write.hpp>

#include <boost/array.hpp>
#include <boost/filesystem.hpp>
#include <chrono>
#include <iostream>
#include <regex>

#include "http_server.hpp"

namespace ip = boost::asio::ip;

void Print(const boost::system::error_code &ec,
           boost::asio::steady_timer *timer, int *count) {
    if (*count < 3) {
        std::cout << "[" << *count << "] "
                  << "Hello world!" << std::endl;
        ++(*count);
        timer->expires_at(timer->expires_at() + std::chrono::seconds(1));
        timer->async_wait(
            std::bind(&Print, std::placeholders::_1, timer, count));
    }
}

#define BUF_SIZE 1024
void Datalink(boost::asio::ip::tcp::socket sock) {
    while (true) {
        boost::array<char, BUF_SIZE> data;

        boost::system::error_code ec;
        std::size_t length = sock.read_some(boost::asio::buffer(data), ec);

        if (ec == boost::asio::error::eof)
            break;
        else if (ec) {
            throw boost::system::system_error(ec);
        }
        // boost::asio::write(sock, boost::asio::buffer(data, length));
        sock.write_some(boost::asio::buffer(data, length));
    }
}

int main(int argc, char *argv[]) {
    auto web_root_path = boost::filesystem::canonical("../static");
    auto path = boost::filesystem::canonical(web_root_path / "index.html");
    auto ifs = std::make_shared<std::ifstream>();
    ifs->open(path.string(),
              std::ifstream::in | std::ios::binary | std::ios::ate);

    if (*ifs) {
        auto length = ifs->tellg();
        std::cout << "Len " << length << std::endl;
        ifs->seekg(0, std::ios::beg);
        boost::asio::streambuf sb;
        ifs->get(sb, '\0');

        boost::asio::streambuf::const_buffers_type bufs = sb.data();
        std::string file_content(boost::asio::buffers_begin(bufs),
                                 boost::asio::buffers_end(bufs));
        std::cout << file_content << std::endl;
    }
    // std::regex r("^/user/([0-9]+)/article/([a-zA-Z0-9]+)$");
    std::regex r("^/(.*)$");
    std::smatch match_result;
    std::string source_str("/user/19977/article/ebacff07");
    std::cout << std::regex_match(source_str, r) << std::endl;
    std::regex_match(source_str, match_result, r);
    for (auto s : match_result) {
        std::cout << s << std::endl;
    }
    // return 0;
    int port = 8000;
    if (argc < 2)
        std::cout << "Using default port 8000" << std::endl;
    else
        port = std::atoi(argv[1]);

    boost::asio::io_service io_ctx;

    // boost::asio::steady_timer timer(io_ctx, std::chrono::seconds(3));
    // int count = 0;
    // timer.async_wait( std::bind(&Print, std::placeholders::_1, &timer,
    // &count));

    ip::tcp::endpoint tcp_endpoint(ip::tcp::v4(), port);
    ip::tcp::acceptor tcp_acceptor(io_ctx, tcp_endpoint);
    tcp_acceptor.listen();

    // auto write_handler = [&tcp_socket](const boost::system::error_code& ec,
    //                                    std::size_t bytes_transferred) {
    //     if (!ec) {
    //         tcp_socket.shutdown(ip::tcp::socket::shutdown_send);
    //     }
    // };

    std::vector<std::shared_ptr<HttpConnection>> connections;
    std::cout << "declare accept_handler" << std::endl;
    std::string data;
    std::function<void(const boost::system::error_code &,
                       std::shared_ptr<HttpConnection>)>
        accept_handler;
    auto router = std::shared_ptr<router_type>(new router_type());
    (*router)["/"]["GET"] = [](std::shared_ptr<Request> req,
                               std::shared_ptr<Response> res) {
        auto &response = res->data;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/html\r\n";
        // std::string response_data = "<!DOCTYPE
        // html><html><head></head><body><h1>hello world!</h1></body></html>";
        std::string response_data = "Hello world!";
        response << "Content-Length: " << response_data.length() << "\r\n";
        response << "\r\n";
        response << response_data;
        std::cout << response.str() << std::endl;
    };
    // (*router)["/index.html"]["GET"] = [](std::shared_ptr<Request> req,
    //                                      std::shared_ptr<Response> res) {
    //     auto &response = res->data;
    //     response << "HTTP/1.1 200 OK\r\n";
    //     response << "Content-Type: text/html\r\n";
    //     std::string response_data =
    //         "<!DOCTYPE html><html><head></head><body><h1>hello "
    //         "world!</h1></body></html>";
    //     response << "Content-Length: " << response_data.length() << "\r\n";
    //     response << "\r\n";
    //     response << response_data;
    //     // std::cout << response.str() << std::endl;
    // };
    (*router)["^/(.*)$"]["GET"] = [](std::shared_ptr<Request> req,
                                     std::shared_ptr<Response> res) {
        try {
            auto web_root_path = boost::filesystem::canonical("../static");
            auto path = boost::filesystem::canonical(web_root_path / req->url);
            // Check if path is within web_root_path

            if (boost::filesystem::is_directory(path)) path /= "index.html";

            auto ifs = std::make_shared<std::ifstream>();
            std::cout << path.string() << std::endl;
            ifs->open(path.string(),
                      std::ifstream::in | std::ios::binary | std::ios::ate);

            if (*ifs) {
                auto length = ifs->tellg();
                ifs->seekg(0, std::ios::beg);
                res->header["Content-Length"] = std::to_string(length);
                auto &response = res->data;
                response << "HTTP/1.1 200 OK\r\n";
                response << "Content-Type: text/html\r\n";
                response << "Content-Length: " << res->header["Content-Length"]
                         << "\r\n";
                response << "\r\n";

                boost::asio::streambuf sb;
                ifs->get(sb, '\0');

                boost::asio::streambuf::const_buffers_type bufs = sb.data();
                std::string file_content(boost::asio::buffers_begin(bufs),
                                         boost::asio::buffers_end(bufs));
                response << file_content;
            }
        } catch (const std::exception &ex) {
            std::cout << ex.what() << std::endl;
        }
    };
    accept_handler = [&io_ctx, &connections, &tcp_acceptor, &accept_handler,
                      &router](const boost::system::error_code &ec,
                               std::shared_ptr<HttpConnection> conn) {
        // std::cout << ec.value() << std::endl;
        if (ec == boost::asio::error::operation_aborted) {
            std::cout << "fuck up" << std::endl;
            return;
        }
        if (!ec) {
            // while(true){
            //     std::string buf_data;
            //     tcp_socket.read_some(boost::asio::buffer(buf_data));
            // }
            // boost::asio::async_write(tcp_socket,
            // boost::asio::buffer(data), write_handler);
            // std::shared_ptr<HttpConnection> conn =
            // std::make_shared<HttpConnection>(sock_ptr);
            // connections.push_back(conn);
            conn->deal();
        }
        // std::unique_ptr<ip::tcp::socket> new_sock_ptr =
        // std::make_unique<ip::tcp::socket>(io_ctx);
        std::unique_ptr<ip::tcp::socket> new_sock_ptr(
            new ip::tcp::socket(io_ctx));
        std::shared_ptr<HttpConnection> new_conn =
            std::make_shared<HttpConnection>(std::move(new_sock_ptr), router);
        tcp_acceptor.async_accept(
            *new_conn->_socket,
            std::bind(accept_handler, std::placeholders::_1, new_conn));
    };
    // std::unique_ptr<ip::tcp::socket> tcp_socket =
    // std::make_unique<ip::tcp::socket>(io_ctx);
    std::unique_ptr<ip::tcp::socket> tcp_socket(new ip::tcp::socket(io_ctx));
    std::shared_ptr<HttpConnection> new_conn =
        std::make_shared<HttpConnection>(std::move(tcp_socket), router);
    std::cout << "async.accept" << std::endl;
    tcp_acceptor.async_accept(
        *new_conn->_socket,
        std::bind(accept_handler, std::placeholders::_1, new_conn));
    std::cout << "io_ctx.run()" << std::endl;
    io_ctx.run();

    return 0;
}