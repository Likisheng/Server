#include "HttpConnection.h"
#include "LogicSystem.h"
#include <boost/asio/io_context.hpp>

HttpConnection::HttpConnection(boost::asio::io_context &ioc)
    : _socket(ioc) {}

void HttpConnection::start() {
  auto self = shared_from_this();
  http::async_read(_socket, _buffer, _request,
                   [self](beast::error_code ec, std::size_t bytes_transferred) {
                     try {
                       if (ec) {
                         std::cout << "http read is error:" << ec.what()
                                   << std::endl;
                         return;
                       }

                       boost::ignore_unused(bytes_transferred);
                       self->handleReq();
                       self->checkDeadline();
                     } catch (std::exception e) {
                       std::cout << "exception is " << e.what() << std::endl;
                     }
                   });
}

// url从10进制转16进制
unsigned char toHex(unsigned char x) { return x > 9 ? x + 55 : x + 48; }

// url从16进制转10进制
unsigned char fromHex(unsigned char x) {
  unsigned char y;
  if (x >= 'A' && x <= 'Z')
    y = x - 'A' + 10;
  else if (x >= 'a' && x <= 'z')
    y = x - 'a' + 10;
  else if (x >= '0' && x <= '9')
    y = x - '0';
  else
    assert(0);
  return y;
}

std::string urlEncode(const std::string &str) {
  std::string strTemp = "";
  size_t length = str.length();
  for (size_t i = 0; i < length; i++) {
    // 判断是否仅有数字和字母构成
    if (isalnum((unsigned char)str[i]) || str[i] == '-' || str[i] == '_' ||
        str[i] == '.' || str[i] == '~')
      strTemp += str[i];
    else if (str[i] == ' ') // 为空字符
      strTemp += "+";
    else {
      // 其他字符需要加%并且高四位和低四位分别转为16进制
      strTemp += '%';
      strTemp += toHex((unsigned char)str[i] >> 4);
      strTemp += toHex((unsigned char)str[i] & 0x0f);
    }
  }
  return strTemp;
}

std::string UrlDecode(const std::string &str) {
  std::string strTemp = "";
  size_t length = str.length();
  for (size_t i = 0; i < length; i++) {
    // 还原+为空
    if (str[i] == '+')
      strTemp += ' ';
    // 遇到%将后面的两个字符从16进制转为char再拼接
    else if (str[i] == '%') {
      assert(i + 2 < length);
      unsigned char high = fromHex((unsigned char)str[++i]);
      unsigned char low = fromHex((unsigned char)str[++i]);
      strTemp += high * 16 + low;
    } else
      strTemp += str[i];
  }
  return strTemp;
}

void HttpConnection::PreParseGetParam() {
  // 提取 URI get_test?key1=value1&key2=value2
  auto uri = _request.target();
  // 查找查询字符串的开始位置（即 '?' 的位置）
  auto query_pos = uri.find('?');
  // 如果没有参数，即没有查找到？
  if (query_pos == std::string::npos) {
    _get_url = uri;
    return;
  }

  // 如果有参数，执行如下操作
  _get_url = uri.substr(0, query_pos);
  std::string query_string = uri.substr(query_pos + 1);
  std::string key;
  std::string value;
  size_t pos = 0;
  while ((pos = query_string.find('&')) != std::string::npos) {
    auto pair = query_string.substr(0, pos);
    size_t eq_pos = pair.find('=');
    if (eq_pos != std::string::npos) {
      key = UrlDecode(
          pair.substr(0, eq_pos)); // 假设有 url_decode 函数来处理URL解码
      value = UrlDecode(pair.substr(eq_pos + 1));
      _get_params[key] = value;
    }
    query_string.erase(0, pos + 1);
  }
  // 处理最后一个参数对（如果没有 & 分隔符）
  if (!query_string.empty()) {
    size_t eq_pos = query_string.find('=');
    if (eq_pos != std::string::npos) {
      key = UrlDecode(query_string.substr(0, eq_pos));
      value = UrlDecode(query_string.substr(eq_pos + 1));
      _get_params[key] = value;
    }
  }
}

void HttpConnection::handleReq() {
  // 设置版本
  _response.version(_request.version());
  // http短连接，不需要保活
  _response.keep_alive(false);

  // 处理http的get请求
  if (_request.method() == http::verb::get) {
    PreParseGetParam();
    bool success =
        LogicSystem::GetInstance()->handleGet(_get_url, shared_from_this());
    // 如果处理出错
    if (!success) {
      _response.result(http::status::not_found);
      _response.set(http::field::content_type, "text/plain");
      beast::ostream(_response.body()) << "url not found\r\n";

      // 给对方回包
      writeResponse();
    }

    // 处理成功
    _response.result(http::status::ok);
    _response.set(http::field::server, "GateServer");
    // 给对方回包
    writeResponse();
  }

  // 处理http的post请求
  if (_request.method() == http::verb::post) {
    bool success = LogicSystem::GetInstance()->handlePost(_request.target(),
                                                          shared_from_this());
    // 如果处理出错
    if (!success) {
      std::cout << _request.target() << std::endl;
      _response.result(http::status::not_found);
      _response.set(http::field::content_type, "text/plain");
      beast::ostream(_response.body()) << "url not found\r\n";

      // 给对方回包
      writeResponse();
    }

    // 处理成功
    _response.result(http::status::ok);
    _response.set(http::field::server, "GateServer");
    // 给对方回包
    writeResponse();
  }
}

void HttpConnection::writeResponse() {
  auto self = shared_from_this();
  _response.content_length(_response.body().size());
  http::async_write(
      _socket, _response,
      [self](beast::error_code ec, std::size_t bytes_transferred) {
        beast::error_code shutdown_ec;
        auto result =
            self->_socket.shutdown(tcp::socket::shutdown_send, shutdown_ec);
        if (shutdown_ec) {
          std::cerr << "Socket shutdown error: " << shutdown_ec.message()
                    << std::endl;
        }
        self->_deadline.cancel();
      });
}

void HttpConnection::checkDeadline() {
  auto self = shared_from_this();
  _deadline.async_wait([self](beast::error_code ec) {
    if (!ec) {
      // 服务器直接断开客户端连接，有隐患，会陷入timewait状态
      auto close_result = self->_socket.close(ec);
      // Optionally, handle close_result or log if needed
    }
  });
}