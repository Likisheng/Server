#include "RedisMgr.h"
#include "hiredis.h"
#include <iostream>
#include <cstring>
#include "ConfigMgr.h"
RedisMgr::RedisMgr() {
  auto &config = ConfigMgr::Inst();
  auto host = config["Redis"]["host"];
  auto port = config["Redis"]["port"];
  auto pwd = config["Redis"]["password"];
  _pool.reset(new RedisConPool(5, host.c_str(), atoi(port.c_str()), pwd.c_str()));
}
RedisMgr::~RedisMgr() {
  Close();
}


bool RedisMgr::Get(const std::string &key, std::string &value) {
  auto connection = _pool->getConnection();
  if(connection == nullptr) { 
    std::cout << "Failed to get connection from pool" << std::endl;
    return false;
  }
  auto reply =
      (redisReply *)redisCommand(connection, "GET %s", key.c_str());
  if (reply == nullptr) {
    std::cout << "[ GET  " << key << " ] failed" << std::endl;
    
    return false;
  }
  if (reply->type != REDIS_REPLY_STRING) {
    std::cout << "[ GET  " << key << " ] failed" << std::endl;
    freeReplyObject(reply);
    return false;
  }
  value = reply->str;
  freeReplyObject(reply);
  std::cout << "Succeed to execute command [ GET " << key << "  ]" << std::endl;
  return true;
}

bool RedisMgr::Set(const std::string &key, const std::string &value) {
  auto connection = _pool->getConnection();
  if (connection == nullptr) {
    std::cout << "Failed to get connection from pool" << std::endl;
    return false;
  }
  auto reply = (redisReply *)redisCommand(connection, "SET %s %s", key.c_str(), value.c_str());

  if (reply == nullptr) {
    std::cerr << "Execut command [ SET " << key << "  " << value
              << " ] failure ! " << std::endl;
    
    return false;
  }

  if (!(reply->type == REDIS_REPLY_STATUS &&
        std::string(reply->str) == "OK")) {
    std::cerr << "Execut command [ SET " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }
  freeReplyObject(reply);
  std::cout << "Execut command [ SET " << key << "  " << value
            << " ] success ! " << std::endl;
  return true;
}

// 认证功能
// 认证功能在RedisConPool中实现
// bool RedisMgr::Auth(const std::string &password) {
//   auto connection = _pool->getConnection();
//   if (connection == nullptr) {
//     std::cout << "Failed to get connection from pool" << std::endl;
//     return false;
//   }
//   auto reply =
//       (redisReply *)redisCommand(connection, "AUTH %s", password.c_str());
 
//   if (reply->type == REDIS_REPLY_ERROR) {
//     std::cout << "认证失败" << std::endl;
//     // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
//     freeReplyObject(reply);
//     return false;
//   } else {
//     // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
//     freeReplyObject(reply);
//     std::cout << "认证成功" << std::endl;
//     return true;
//   }
// }

bool RedisMgr::LPush(const std::string &key, const std::string &value) {
  auto connection = _pool->getConnection();
  if (connection == nullptr) {
    std::cout << "Failed to get connection from pool" << std::endl;
    return false;
  }
  auto reply = (redisReply *)redisCommand(connection, "LPUSH %s %s", key.c_str(), value.c_str());

  if (reply == nullptr) {
    std::cout << "Execut command [ LPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;
    
    return false;
  }
  if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
    std::cout << "Execut command [ LPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }
  std::cout << "Execut command [ LPUSH " << key << "  " << value
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  return true;
}

bool RedisMgr::LPop(const std::string &key, std::string &value) {
  auto connection = _pool->getConnection();
  if (connection == nullptr) {
    std::cout << "Failed to get connection from pool" << std::endl;
    return false;
  }
  auto reply = (redisReply *)redisCommand(connection, "LPOP %s ", key.c_str());

  if (reply == nullptr) {
    std::cout << "Execut command [ LPOP " << key << " ] failure ! "
              << std::endl;
    return false;
  }
  if (reply->type == REDIS_REPLY_NIL) {
    std::cout << "Execut command [ LPOP " << key << " ] failure ! "
              << std::endl;
    freeReplyObject(reply);
    return false;
  }
  value = reply->str;
  std::cout << "Execut command [ LPOP " << key << " ] success ! " << std::endl;
  freeReplyObject(reply);
  return true;
}

bool RedisMgr::RPush(const std::string &key, const std::string &value) {
  auto connection = _pool->getConnection();
  if (connection == nullptr) {
    std::cout << "Failed to get connection from pool" << std::endl;
    return false;
  }
  auto reply = (redisReply *)redisCommand(connection, "RPUSH %s %s", key.c_str(), value.c_str());
  if (reply == nullptr) {
    std::cout << "Execut command [ RPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;

    return false;
  }
  if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
    std::cout << "Execut command [ RPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }
  std::cout << "Execut command [ RPUSH " << key << "  " << value
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  return true;
}

bool RedisMgr::RPop(const std::string &key, std::string &value) {
  auto connection = _pool->getConnection();
  if (connection == nullptr) {
    std::cout << "Failed to get connection from pool" << std::endl;
    return false;
  }
  auto reply = (redisReply *)redisCommand(connection, "RPOP %s ", key.c_str());
  
  if (reply == nullptr) {
    std::cout << "Execut command [ RPOP " << key << " ] failure ! "
              << std::endl;

    return false;
  }
  if (reply->type == REDIS_REPLY_NIL) {
    std::cout << "Execut command [ RPOP " << key << " ] failure ! "
              << std::endl;
    freeReplyObject(reply);
    return false;
  }
  value = reply->str;
  std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
  freeReplyObject(reply);
  return true;
}

bool RedisMgr::HSet(const std::string &key, const std::string &hkey,
                    const std::string &value) {
  auto connection = _pool->getConnection();
  if (connection == nullptr) {
    std::cout << "Failed to get connection from pool" << std::endl;
    return false;
  }
  auto reply = (redisReply *)redisCommand(
      connection, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
  
  if (reply == nullptr) {
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << value << " ] failure ! " << std::endl;
    return false;
  }
  if (reply->type != REDIS_REPLY_INTEGER) {
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << value << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }
  std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  return true;
}
bool RedisMgr::HSet(const char *key, const char *hkey, const char *hvalue,
                    size_t hvaluelen) {

  auto connection = _pool->getConnection();
  if (connection == nullptr) {
    std::cout << "Failed to get connection from pool" << std::endl;
    return false;
  }

  const char *argv[4];
  size_t argvlen[4];
  argv[0] = "HSET";
  argvlen[0] = 4;
  argv[1] = key;
  argvlen[1] = strlen(key);
  argv[2] = hkey;
  argvlen[2] = strlen(hkey);
  argv[3] = hvalue;
  argvlen[3] = hvaluelen;

  
  auto reply = (redisReply *)redisCommandArgv(connection, 4, argv, argvlen);

  if (reply == nullptr) {
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << hvalue << " ] failure ! " << std::endl;
  
    return false;
  }
  if (reply->type != REDIS_REPLY_INTEGER) {
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << hvalue << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }
  std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  return true;
}

//TODO: 需要优化 返回值的方式
std::string RedisMgr::HGet(const std::string &key, const std::string &hkey) {

  auto connection = _pool->getConnection();
  if (connection == nullptr) {
    std::cout << "Failed to get connection from pool" << std::endl;
    return "";
  }

  const char *argv[3];
  size_t argvlen[3];
  argv[0] = "HGET";
  argvlen[0] = 4;
  argv[1] = key.c_str();
  argvlen[1] = key.length();
  argv[2] = hkey.c_str();
  argvlen[2] = hkey.length();

  
  auto reply = (redisReply *)redisCommandArgv(connection, 3, argv, argvlen);

 
  if (reply == nullptr) {

    std::cout << "Execut command [ HGet " << key << " " << hkey
              << "  ] failure ! " << std::endl;
    return "";
  }
  if (reply->type == REDIS_REPLY_NIL) {
    freeReplyObject(reply);
    std::cout << "Execut command [ HGet " << key << " " << hkey
              << "  ] failure ! " << std::endl;
    return "";
  }
  std::string value = reply->str;
  freeReplyObject(reply);
  std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! "
            << std::endl;
  return value;
}

bool RedisMgr::Del(const std::string &key) {
  auto connection = _pool->getConnection();
  if (connection == nullptr) {
    std::cout << "Failed to get connection from pool" << std::endl;
    return false;
  }
  auto reply = (redisReply *)redisCommand(connection, "DEL %s", key.c_str());
  
  if (reply == nullptr) {
    std::cout << "Execut command [ Del " << key << " ] failure ! " << std::endl;
    
    return false;
  }
  if (reply->type != REDIS_REPLY_INTEGER) {
    std::cout << "Execut command [ Del " << key << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }
  std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
  freeReplyObject(reply);
  return true;
}

bool RedisMgr::ExistsKey(const std::string &key) {
  auto connection = _pool->getConnection();
  if (connection == nullptr) {
    std::cout << "Failed to get connection from pool" << std::endl;
    return false;
  }
  auto reply = (redisReply *)redisCommand(connection, "exists %s", key.c_str());
  
  if (reply == nullptr) {
    std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;

    return false;
  }
  if (reply->type != REDIS_REPLY_INTEGER ||
      reply->integer == 0) {
    std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }
  std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
  freeReplyObject(reply);
  return true;
}

void RedisMgr::Close() { _pool->Close(); }
