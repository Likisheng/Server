#pragma once
#include "MySqlDao.h"
#include "Singleton.h"

class MySqlMgr : public Singleton<MySqlMgr> {
  friend class Singleton<MySqlMgr>;

public:
  ~MySqlMgr();

  int RegUser(const std::string &name, const std::string &email,
              const std::string &pwd, const std::string &icon);
  bool CheckEmail(const std::string &name, const std::string &email);
  bool UpdatePwd(const std::string &name, const std::string &email);
  bool CheckPwd(const std::string &email, const std::string &pwd,
                UserInfo &userInfo);
  bool TestProcedure(const std::string &email, int &uid,const std::string &name);

private:
  MySqlMgr();
  MySqlDao _dao;
};
