#pragma once
#include <boost/asio.hpp>
#include <memory>

namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

class HttpConnection;
class ConfigMgr;
class CServer : public std::enable_shared_from_this<CServer> {

public:
  CServer(net::io_context &ioc, unsigned short &port);
  void start();

private:
  tcp::acceptor _acceptor;
  net::io_context &_ioc;
  tcp::socket _socket;
};