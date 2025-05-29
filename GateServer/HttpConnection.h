#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include <unordered_map>
namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

class LogicSystem;
class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
public:
  friend class LogicSystem;
  HttpConnection(boost::asio::io_context &ioc);
  void start();
  tcp::socket& GetSocket(boost::asio::io_context &ioc) {
    if (!_socket.is_open()) {
      _socket = tcp::socket(ioc);
    }
    return _socket;
  }
private:
  void checkDeadline();
  void writeResponse();
  void handleReq();
  void PreParseGetParam();
  tcp::socket _socket;
  beast::flat_buffer _buffer{8192};
  http::request<http::dynamic_body> _request;
  std::string _get_url;
  std::unordered_map<std::string, std::string> _get_params;
  http::response<http::dynamic_body> _response;
  net::steady_timer _deadline{_socket.get_executor(), std::chrono::seconds(60)};
};