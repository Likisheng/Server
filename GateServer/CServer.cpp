#include "CServer.h"
#include "ConfigMgr.h"
#include "HttpConnection.h"
#include <iostream>
#include <memory>
#include "AsioIOContextPool.h"
#include "RedisMgr.h"
CServer::CServer(net::io_context &ioc, unsigned short &port)
    : _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {}

void CServer::start() {

  auto self = shared_from_this();
  auto &ioc = AsioIOContextPool::GetInstance()->GetIOContext();
  std::shared_ptr<HttpConnection> connection =
      std::make_shared<HttpConnection>(ioc);
  _acceptor.async_accept(connection->GetSocket(ioc), [self,connection](beast::error_code ec) {
    try {
      // 出错放弃socket连接，监听其他连接
      if (ec) {
        self->start();
        return;
      }

      // 没有出错，使用连接管理类去处理事件,管理连接
      connection->start();

      // 继续监听
      self->start();
    } catch (std::exception &e) {
    }
  });
}

void TestRedisMgr() {


  assert(RedisMgr::GetInstance()->Set("blogwebsite", "llfc.club"));
  std::string value = "";
  assert(RedisMgr::GetInstance()->Get("blogwebsite", value));
  assert(RedisMgr::GetInstance()->Get("nonekey", value) == false);
  assert(RedisMgr::GetInstance()->HSet("bloginfo", "blogwebsite", "llfc.club"));
  assert(RedisMgr::GetInstance()->HGet("bloginfo", "blogwebsite") != "");
  assert(RedisMgr::GetInstance()->ExistsKey("bloginfo"));
  assert(RedisMgr::GetInstance()->Del("bloginfo"));
  assert(RedisMgr::GetInstance()->Del("bloginfo"));
  assert(RedisMgr::GetInstance()->ExistsKey("bloginfo") == false);
  assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue1"));
  assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue2"));
  assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue3"));
  assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
  assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
  assert(RedisMgr::GetInstance()->LPop("lpushkey1", value));
  assert(RedisMgr::GetInstance()->LPop("lpushkey2", value) == false);
  RedisMgr::GetInstance()->Close();
}

int main() {


  ConfigMgr& config_mgr=ConfigMgr::Inst();
  std::string gate_port_str = config_mgr["GateServer"]["port"]; // 设置默认端口
  unsigned short port = atoi(gate_port_str.c_str());
 
  try {
    
    net::io_context ioc{1};
    net::signal_set signals(ioc, SIGINT, SIGTERM);

    signals.async_wait(
        [&ioc](const boost::system::error_code &error, int signal_number) {
          if (error) {

            return;
          }

          ioc.stop();
        });

    std::make_shared<CServer>(ioc, port)->start();
    std::cout << "GateServer listen on port:" << port << std::endl;
    // 开启底层事件轮询
    ioc.run();

  } catch (std::exception e) {
    std::cerr << "Error:" << e.what() << std::endl;
  }
  return 0;
}