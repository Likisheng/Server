#include "CServer.h"
#include "ConfigMgr.h"
#include "HttpConnection.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include "AsioIOContextPool.h"
#include "hiredis.h"

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

void TestRedis() {
  // 连接redis 需要启动才可以进行连接
  // redis默认监听端口为6387 可以再配置文件中修改
  redisContext *c = redisConnect("127.0.0.1", 6379);
  if (c->err) {
    printf("Connect to redisServer faile:%s\n", c->errstr);
    redisFree(c);
    return;
  }
  printf("Connect to redisServer Success\n");
  const char* redis_password = "123456";
  redisReply *r = (redisReply *)redisCommand(c, "AUTH %s", redis_password);
  if (r->type == REDIS_REPLY_ERROR) {
    printf("Redis认证失败！\n");
  } else {
    printf("Redis认证成功！\n");
  }
  // 为redis设置key
  const char *command1 = "set stest1 value1";
  // 执行redis命令行
  r = (redisReply *)redisCommand(c, command1);
  // 如果返回NULL则说明执行失败
  if (NULL == r) {
    printf("Execut command1 failure\n");
    redisFree(c);
    return;
  }
  // 如果执行失败则释放连接
  if (!(r->type == REDIS_REPLY_STATUS &&
        (strcmp(r->str, "OK") == 0 || strcmp(r->str, "ok") == 0))) {
    printf("Failed to execute command[%s]\n", command1);
    freeReplyObject(r);
    redisFree(c);
    return;
  }
  // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
  freeReplyObject(r);
  printf("Succeed to execute command[%s]\n", command1);
  const char *command2 = "strlen stest1";
  r = (redisReply *)redisCommand(c, command2);
  // 如果返回类型不是整形 则释放连接
  if (r->type != REDIS_REPLY_INTEGER) {
    printf("Failed to execute command[%s]\n", command2);
    freeReplyObject(r);
    redisFree(c);
    return;
  }
  // 获取字符串长度
  int length = r->integer;
  freeReplyObject(r);
  printf("The length of 'stest1' is %d.\n", length);
  printf("Succeed to execute command[%s]\n", command2);
  // 获取redis键值对信息
  const char *command3 = "get stest1";
  r = (redisReply *)redisCommand(c, command3);
  if (r->type != REDIS_REPLY_STRING) {
    printf("Failed to execute command[%s]\n", command3);
    freeReplyObject(r);
    redisFree(c);
    return;
  }
  printf("The value of 'stest1' is %s\n", r->str);
  freeReplyObject(r);
  printf("Succeed to execute command[%s]\n", command3);
  const char *command4 = "get stest2";
  r = (redisReply *)redisCommand(c, command4);
  if (r->type != REDIS_REPLY_NIL) {
    printf("Failed to execute command[%s]\n", command4);
    freeReplyObject(r);
    redisFree(c);
    return;
  }
  freeReplyObject(r);
  printf("Succeed to execute command[%s]\n", command4);
  // 释放连接资源
  redisFree(c);
}

int main() {

  TestRedis();
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