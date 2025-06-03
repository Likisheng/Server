#include "MySqlPool.h"

MySqlPool::MySqlPool(const std::string &db, const std::string &server,
                     const std::string &user, const std::string &pass,
                     int poolSize)
    : db_(db), url_(server), user_(user), pass_(pass), poolSize_(poolSize),
      b_stop_(false) {
  try {
    for (int i = 0; i < poolSize_; ++i) {
      mysqlpp::Connection *conn = new mysqlpp::Connection(
          db_.c_str(), url_.c_str(), user_.c_str(), pass_.c_str());
      // 获取当前时间戳
      auto currentTime = std::chrono::system_clock::now().time_since_epoch();
      // 将时间戳转换为秒
      long long timestamp =
          std::chrono::duration_cast<std::chrono::seconds>(currentTime)
              .count();

      pool_.push(std::make_unique<SqlConnection>(conn, timestamp));
    }

    _check_thread = std::thread([this]() {
      while (!b_stop_) {
        checkConnection();
        std::this_thread::sleep_for(std::chrono::seconds(60));
      }
    });

    _check_thread.detach();

  } catch (const mysqlpp::Exception &e) {
    std::cout << "mysql pool init failed: " << e.what() << std::endl;
  }
}

void MySqlPool::checkConnection() {
  std::unique_lock<std::mutex> lock(mutex_);
  int poolSize = pool_.size();
  // 获取当前时间戳
  auto currentTime = std::chrono::system_clock::now().time_since_epoch();
  // 将时间戳转换为秒
  long long timestamp =
      std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();

  for (int i = 0; i < poolSize; i++) {
    if (pool_.empty()) {
      break;
    }
    auto conn = std::move(pool_.front());
    pool_.pop();

    // 使用Defer确保连接在检查后被归还到池中
    Defer defer([this, &conn]() { pool_.push(std::move(conn)); });
    if (timestamp - conn->_last_oper_time < 5) {
      continue; // 跳过
    }

    try {
      mysqlpp::Query query = conn->_con->query("SELECT 1");
      query.execute();
      conn->_last_oper_time = timestamp; // 更新最后操作时间
      std::cout << "MySQL connection is valid." << std::endl;
    } catch (const mysqlpp::Exception &e) {

      std::cout << "MySQL connection is invalid, closing it: " << e.what()
                << std::endl;
      mysqlpp::Connection *new_conn = new mysqlpp::Connection(
          db_.c_str(), url_.c_str(), user_.c_str(), pass_.c_str());
      conn->_con.reset(new_conn); // 关闭连接并释放资源
    }
  }
}

std::unique_ptr<SqlConnection> MySqlPool::getConnection() {
  std::unique_lock<std::mutex> lock(mutex_);
  cond_.wait(lock, [this] { return b_stop_ || !pool_.empty(); });
  if (b_stop_ || pool_.empty()) {
    return nullptr;
  }
  std::unique_ptr<SqlConnection> conn = std::move(pool_.front());
  pool_.pop();
  return conn;
}

void MySqlPool::returnConnection(std::unique_ptr<SqlConnection> con) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (b_stop_) {
    return;
  }
  pool_.push(std::move(con));
  cond_.notify_one();
}
void MySqlPool::Close() {
  b_stop_ = true;
  cond_.notify_all();
}
MySqlPool::~MySqlPool() {
  Close();
  std::unique_lock<std::mutex> lock(mutex_);
  while (!pool_.empty()) {
    pool_.pop();
  }
}