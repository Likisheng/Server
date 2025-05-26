#pragma once
#include "Singleton.h"
#include <functional>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <map>

class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> httpHandler;
class LogicSystem : public Singleton<LogicSystem> {
  friend class Singleton<LogicSystem>;

public:
  ~LogicSystem() {};
  // 处理get请求
  bool handleGet(std::string url, std::shared_ptr<HttpConnection>);
  // 处理post请求
  bool handlePost(std::string url, std::shared_ptr<HttpConnection>);
  // 注册函数,类似Controller层
  void regGet(std::string url, httpHandler handler);
  void regPost(std::string url, httpHandler handler);

private:
  LogicSystem();
  // httpHandler是重命名过后的，原类型为std::function<void(std::shared_ptr<HttpConnection>)>
  std::map<std::string, httpHandler> _postHandles;
  std::map<std::string, httpHandler> _getHandles;
};

enum ERRORCODE { SUCCESS = 0, ERROR_JSON = 1001, RPC_FAILED = 1002 };