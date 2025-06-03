#pragma once
#include "Singleton.h"
#include <hiredis/hiredis.h>
#include <memory>
#include <queue>
#include <condition_variable>
#include <atomic>

class RedisConPool {
public:
  RedisConPool(size_t poolSize, const char *host, int port, const char *pwd)
      : _poolSize(poolSize), _host(host), port_(port), _stop(false) {
    for (size_t i = 0; i < _poolSize; ++i) {
      auto *context = redisConnect(host, port);
      if (context == nullptr || context->err != 0) {
        if (context != nullptr) {
          redisFree(context);
        }
        continue;
      }
      auto reply = (redisReply *)redisCommand(context, "AUTH %s", pwd);
      if (reply->type == REDIS_REPLY_ERROR) {
        std::cout << "认证失败" << std::endl;
        // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
        freeReplyObject(reply);
        continue;
      }
      // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
      freeReplyObject(reply);
      std::cout << "认证成功" << std::endl;
      _connections.push(context);
    }
  }
  ~RedisConPool() {
    std::lock_guard<std::mutex> lock(_mutex);
    while (!_connections.empty()) {
      auto *context = _connections.front();
      redisFree(context);
      _connections.pop();
    }
  }
  redisContext *getConnection() {
    std::unique_lock<std::mutex> lock(_mutex);
    _cond.wait(lock, [this] {
      if (_stop) {
        return true;
      }
      return !_connections.empty();
    });
    // 如果停止则直接返回空指针
    if (_stop) {
      return nullptr;
    }
    auto *context = _connections.front();
    _connections.pop();
    return context;
  }
  void returnConnection(redisContext *context) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_stop) {
      return;
    }
    _connections.push(context);
    _cond.notify_one();
  }
  void Close() {
    _stop = true;
    _cond.notify_all();
  }

private:
  std::atomic<bool> _stop;
  size_t _poolSize;
  const char *_host;
  int port_;

  // 这里使用裸指针是因为redisContext的释放需要调用redisFree
  // 而redisFree是一个C函数，不支持智能指针的自动释放
  // 如果使用智能指针需要自定义deleter
  // 但为了简化代码，这里使用裸指针
  // 如果需要更好的内存管理，可以考虑使用std::shared_ptr或std::unique_ptr
  std::queue<redisContext *> _connections;
  std::mutex _mutex;
  std::condition_variable _cond;
};

class RedisMgr : public Singleton<RedisMgr>,
                 public std::enable_shared_from_this<RedisMgr> {
  friend class Singleton<RedisMgr>;

public:
  ~RedisMgr();

  bool Get(const std::string &key, std::string &value);
  bool Set(const std::string &key, const std::string &value);
  bool Auth(const std::string &password);
  bool LPush(const std::string &key, const std::string &value);
  bool LPop(const std::string &key, std::string &value);
  bool RPush(const std::string &key, const std::string &value);
  bool RPop(const std::string &key, std::string &value);
  bool HSet(const std::string &key, const std::string &hkey,
            const std::string &value);
  bool HSet(const char *key, const char *hkey, const char *hvalue,
            size_t hvaluelen);
  std::string HGet(const std::string &key, const std::string &hkey);
  bool Del(const std::string &key);
  bool ExistsKey(const std::string &key);
  void Close();

private:
  RedisMgr();
  std::unique_ptr<RedisConPool> _pool;
};