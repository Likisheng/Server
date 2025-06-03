#include "MySqlDao.h"
#include "ConfigMgr.h"

MySqlDao::MySqlDao() {
  auto &cfg = ConfigMgr::Inst();
  auto db = cfg["MySQL"]["db"];
  auto host = cfg["MySQL"]["host"];
  auto port = cfg["MySQL"]["port"];
  auto user = cfg["MySQL"]["user"];
  auto passwd = cfg["MySQL"]["passwd"];

  // 初始化连接池
  pool_ = std::make_unique<MySqlPool>(db, host + ":" + port, user, passwd, 5);
  // 涉及两次动态内存操作：1) 构造临时对象 2) 重置智能指针
  // 可能存在潜在的内存泄漏风险：如果new成功但reset前发生异常
  // pool_.reset(new MySqlPool(db, host + ":" + port, user, passwd, 5));
}

MySqlDao::~MySqlDao() {
  // 自动调用 MySqlPool 的析构函数，关闭连接池
  pool_->Close();
}

int MySqlDao::RegUser(const std::string &name, const std::string &email,
                      const std::string &pwd) {
  auto con = pool_->getConnection();
  try {
    if (con == nullptr) {
      return -1;
    }
    // 使用 MySQL 存储过程注册用户
    // 注意：%0q, %1q, %2q 是 MySQL++ 的格式化字符串，用于安全地插入参数
    // 确保输入参数是安全的，避免 SQL 注入
    mysqlpp::Query query =
        con->_con->query("CALL reg_user(%0q, %1q, %2q, @result)");
    query.parse();
    query.execute(name, email, pwd);

    // 查询输出参数
    mysqlpp::Query result_query = con->_con->query("SELECT @result AS result");
    mysqlpp::StoreQueryResult res = result_query.store();
    if (!res.empty()) {
      int result = res[0]["result"];
      pool_->returnConnection(std::move(con));
      return result;
    }
    pool_->returnConnection(std::move(con));
    return -1;
  } catch (const mysqlpp::Exception &e) {
    pool_->returnConnection(std::move(con));
    std::cerr << "MySQL++ Exception: " << e.what() << std::endl;
    return -1;
  }
}

int MySqlDao::RegUserTransaction(const std::string &name,
                 const std::string &email,
                 const std::string &pwd,
                 const std::string &icon) {
  auto con = pool_->getConnection();
  if (con == nullptr) {
  return -1;
  }

  Defer defer([this, &con] { pool_->returnConnection(std::move(con)); });

  try {
  // 开始事务
  con->_con->disable_exceptions();
  con->_con->query("SET AUTOCOMMIT=0").execute();

  // 检查email是否存在
  mysqlpp::Query query_email = con->_con->query("SELECT 1 FROM user WHERE email = %0q");
  mysqlpp::StoreQueryResult res_email = query_email.store(email);
  if (!res_email.empty()) {
    con->_con->query("ROLLBACK").execute();
    std::cout << "email " << email << " exist";
    return 0;
  }

  // 检查name是否存在
  mysqlpp::Query query_name = con->_con->query("SELECT 1 FROM user WHERE name = %0q");
  mysqlpp::StoreQueryResult res_name = query_name.store(name);
  if (!res_name.empty()) {
    con->_con->query("ROLLBACK").execute();
    std::cout << "name " << name << " exist";
    return 0;
  }

  // 更新user_id
  mysqlpp::Query query_upid = con->_con->query("UPDATE user_id SET id = id + 1");
  query_upid.execute();

  // 获取新的id
  mysqlpp::Query query_uid = con->_con->query("SELECT id FROM user_id");
  mysqlpp::StoreQueryResult res_uid = query_uid.store();
  int newId = 0;
  if (!res_uid.empty()) {
    newId = int(res_uid[0]["id"]);
  } else {
    std::cout << "select id from user_id failed" << std::endl;
    con->_con->query("ROLLBACK").execute();
    return -1;
  }

  // 插入user
  mysqlpp::Query query_insert = con->_con->query(
    "INSERT INTO user (uid, name, email, pwd, nick, icon) "
    "VALUES (%0q, %1q, %2q, %3q, %4q, %5q)");
  query_insert.parse();
  query_insert.execute(newId, name, email, pwd, name, icon);

  // 提交事务
  con->_con->query("COMMIT").execute();
  std::cout << "newuser insert into user success" << std::endl;
  return newId;
  } catch (const mysqlpp::Exception &e) {
  if (con) {
    con->_con->query("ROLLBACK").execute();
  }
  std::cerr << "MySQL++ Exception: " << e.what() << std::endl;
  return -1;
  }
}
