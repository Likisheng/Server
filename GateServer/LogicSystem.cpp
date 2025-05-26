#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"


void LogicSystem::regGet(std::string url, httpHandler handler) {
  _getHandles.insert(std::make_pair(url, handler));
}

void LogicSystem::regPost(std::string url, httpHandler handler) {
  _postHandles.insert(std::make_pair(url, handler));
}

LogicSystem::LogicSystem() {
  regGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
    beast::ostream(connection->_response.body())
        << "receive get_test req " << std::endl;
    int i = 0;
    for (auto &elem : connection->_get_params) {
      i++;
      beast::ostream(connection->_response.body())
          << "param" << i << " key is " << elem.first;
      beast::ostream(connection->_response.body())
          << ", " << " value is " << elem.second << std::endl;
    }
  });

  regPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
    // 将body转成字符串
    auto body_str =
        boost::beast ::buffers_to_string(connection->_request.body().data());
    std::cout << "receive body is " << body_str << std::endl;
    // 设置返回类型，验证码设置为text/json
    connection->_response.set(http::field::content_type, "text/json");
    // 将要返回到客户端的json结构
    Json::Value root;
    Json::Reader reader;
    // 解析来源存储到该json结构
    Json::Value src_root;
    bool parse_success = reader.parse(body_str, src_root);

    // 判断解析是否成功
    if (!parse_success) {
      std::cout << "failed to parse json data!" << std::endl;
      root["error"] = ERRORCODE::ERROR_JSON;
      std::string jsonstr = root.toStyledString();
      beast::ostream(connection->_response.body()) << jsonstr;
      return true;
    }

    if (!src_root.isMember("email")) {
      std::cout << "failed to parse json data!" << std::endl;
      root["error"] = ERRORCODE::ERROR_JSON;
      std::string jsonstr = root.toStyledString();
      beast::ostream(connection->_response.body()) << jsonstr;
      return true;
    }

    auto email = src_root["email"].asString();
    GetVerifyRsp rsp = VerifyGrpcClient::GetInstance()->GetVerifyCode(email); // 获取单例

    std::cout << "email is " << email << std::endl;
    root["error"] = rsp.error();
    root["email"] = src_root["email"];
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->_response.body()) << jsonstr;
    return true;
  });
}

bool LogicSystem::handleGet(std::string url,
                            std::shared_ptr<HttpConnection> connection) {
  if (_getHandles.find(url) == _getHandles.end()) {
    return false;
  }

  _getHandles[url](connection);
  return true;
}

bool LogicSystem::handlePost(std::string url,
                             std::shared_ptr<HttpConnection> connection) {
  if (_postHandles.find(url) == _postHandles.end()) {
    return false;
  }

  _postHandles[url](connection);
  return true;
}