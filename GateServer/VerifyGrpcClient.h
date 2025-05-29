#pragma once
#include "Singleton.h"
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/status.h>
#include <memory>
#include <queue>
#include <condition_variable>
#include <atomic>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;

class RPConlPool {
public:
  RPConlPool(size_t pool_size, std::string host, std::string port);
  ~RPConlPool();
  void close();
  std::unique_ptr<VerifyService::Stub> getStub();
  // 返回一个stub到连接池
  void returnStub(std::unique_ptr<VerifyService::Stub> stub);
private:
  std::atomic<bool> _stop;
  size_t _pool_size;
  std::string _host;
  std::string _port;
  std::queue<std::unique_ptr<VerifyService::Stub>> _stubs;
  std::mutex _mutex;
  std::condition_variable _cond_var;
};

class VerifyGrpcClient : public Singleton<VerifyGrpcClient> {
  friend class Singleton<VerifyGrpcClient>;

public:
  ~VerifyGrpcClient() {}

  // 获取验证码
  GetVerifyRsp GetVerifyCode(const std::string &email);

private:
  VerifyGrpcClient();
  VerifyGrpcClient(const VerifyGrpcClient &) = delete;
  VerifyGrpcClient &operator=(const VerifyGrpcClient &) = delete;
  VerifyGrpcClient(VerifyGrpcClient &&) = delete;
  VerifyGrpcClient &operator=(VerifyGrpcClient &&) = delete;
  std::unique_ptr<VerifyService::Stub> _stub;
  std::unique_ptr<RPConlPool> _pool;
};