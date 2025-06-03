#include "MySqlMgr.h"

MySqlMgr::~MySqlMgr() {}

int MySqlMgr::RegUser(const std::string &name, const std::string &email,
                      const std::string &pwd, const std::string &icon) {
  return _dao.RegUserTransaction(name, email, pwd, icon);
}

// bool MySqlMgr::CheckEmail(const std::string &name, const std::string &email) {
//   return _dao.CheckEmail(name, email);
// }

// bool MySqlMgr::UpdatePwd(const std::string &name, const std::string &pwd) {
//   return _dao.UpdatePwd(name, pwd);
// }

MySqlMgr::MySqlMgr() {}

// bool MySqlMgr::CheckPwd(const std::string &email, const std::string &pwd,
//                         UserInfo &userInfo) {
//   return _dao.CheckPwd(email, pwd, userInfo);
// }

// bool MySqlMgr::TestProcedure(const std::string &email, int &uid,const std::string &name) {
//   return _dao.TestProcedure(email, uid, name);
// }
