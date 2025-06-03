#pragma once
#include "Singleton.h"
#include <functional>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <map>
#define CODEPRFIX "code_"
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

enum ErrorCodes {
  Success = 0,
  Error_Json = 1001,     // Json解析错误
  RPCFailed = 1002,      // RPC请求错误
  VarifyExpired = 1003,  // 验证码过期
  VarifyCodeErr = 1004,  // 验证码错误
  UserExist = 1005,      // 用户已经存在
  PasswdErr = 1006,      // 密码错误
  EmailNotMatch = 1007,  // 邮箱不匹配
  PasswdUpFailed = 1008, // 更新密码失败
  PasswdInvalid = 1009,  // 密码更新失败
  TokenInvalid = 1010,   // Token失效
  UidInvalid = 1011,     // uid无效

};