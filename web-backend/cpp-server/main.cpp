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
#include "nlohmann/json.hpp"

namespace ip = boost::asio::ip;
using json = nlohmann::json;

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

    auto mime = std::map<std::string, std::string>();
    mime[std::string(".html")] = std::string("text/html");
    mime[std::string(".js")] = std::string("application/json");
    std::cout << "MIME done" << std::endl;
    std::cout << mime.find(std::string(".html"))->second << std::endl;

    std::vector<std::shared_ptr<HttpConnection>> connections;
    std::cout << "declare accept_handler" << std::endl;
    std::string data;
    std::function<void(const boost::system::error_code &,
                       std::shared_ptr<HttpConnection>)>
        accept_handler;
    auto router = std::shared_ptr<router_type>(new router_type());
    (*router)["^/(.*)$"]["GET"] = [&mime](std::shared_ptr<Request> req,
                                          std::shared_ptr<Response> res) {
        try {
            auto web_root_path = boost::filesystem::canonical("../static");
            auto path = boost::filesystem::canonical(web_root_path / req->url);
            // Check if path is within web_root_path

            if (boost::filesystem::is_directory(path)) path /= "index.html";

            if (boost::filesystem::exists(path)) res->send_file(path);

        } catch (const std::exception &ex) {
            std::cout << ex.what() << std::endl;
        }
    };
    (*router)["^/json$"]["POST"] = [](std::shared_ptr<Request> req,
                                      std::shared_ptr<Response> res) {
        std::cout << "/json POST" << std::endl;
        std::cout << req->header["Content-Type"] << std::endl;
        std::cout << req->content << std::endl;
        auto json_data = json::parse(req->content);

        std::string json_str = json_data.dump();
        res->status(200).header("Content-Type", "application/json");
        res->header("Content-Length", std::to_string(json_str.length()));

        res->send_header();
        res->content << json_str << "\r\n";
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
            conn->deal();
        }
        std::unique_ptr<ip::tcp::socket> new_sock_ptr(
            new ip::tcp::socket(io_ctx));
        std::shared_ptr<HttpConnection> new_conn =
            std::make_shared<HttpConnection>(std::move(new_sock_ptr), router);
        tcp_acceptor.async_accept(
            *new_conn->_socket,
            std::bind(accept_handler, std::placeholders::_1, new_conn));
    };
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