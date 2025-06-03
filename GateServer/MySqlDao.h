#pragma once
#include "MySqlPool.h"


//TODO :暂时没用
struct UserInfo {
  int uid;
  std::string name;
  std::string email;
  std::string pwd;
};

class MySqlDao {
public:
  MySqlDao();
  ~MySqlDao();
  int RegUser(const std::string &name, const std::string &email,
              const std::string &pwd);
  int RegUserTransaction(const std::string &name, const std::string &email,
                         const std::string &pwd, const std::string &icon);
  // bool CheckEmail(const std::string &name, const std::string &email);
  // bool UpdatePwd(const std::string &name, const std::string &newpwd);
  // bool CheckPwd(const std::string &name, const std::string &pwd,
  //               UserInfo &userInfo);
  // bool TestProcedure(const std::string &email, int &uid,const std::string &name);

private:
  std::unique_ptr<MySqlPool> pool_;
};