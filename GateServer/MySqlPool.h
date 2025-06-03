#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <mysql++/mysql++.h>
#include <queue>
#include <thread>
class SqlConnection {
public:
  SqlConnection(mysqlpp::Connection *con, int64_t lasttime): _con(con), _last_oper_time(lasttime) {}
  std::unique_ptr<mysqlpp::Connection> _con;
  int64_t _last_oper_time;
};

// Defer类
class Defer {
public:
  // 接受一个lambda表达式或者函数指针
  Defer(std::function<void()> func) : func_(func) {}

  // 析构函数中执行传入的函数
  ~Defer() { func_(); }

private:
  std::function<void()> func_;
};

class MySqlPool {
public:
  MySqlPool(const std::string &db, const std::string &server,
            const std::string &user, const std::string &pass, int poolSize);

  void checkConnection();

  std::unique_ptr<SqlConnection> getConnection();

  void returnConnection(std::unique_ptr<SqlConnection> con);

  void Close();

  ~MySqlPool();

private:
  std::string db_;
  std::string url_;
  std::string user_;
  std::string pass_;
  int poolSize_;

  std::queue<std::unique_ptr<SqlConnection>> pool_;
  std::mutex mutex_;
  std::condition_variable cond_;
  std::atomic<bool> b_stop_;

  std::thread _check_thread;
};