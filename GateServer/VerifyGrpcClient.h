#pragma once
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/status.h>
#include <memory>
#include "message.grpc.pb.h"
#include "Singleton.h"
#include "message.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;
class VerifyGrpcClient : public Singleton<VerifyGrpcClient> {
  friend class Singleton<VerifyGrpcClient>;

 public:
  ~VerifyGrpcClient() {}

  // 获取验证码
  GetVerifyRsp GetVerifyCode(const std::string &email) {
    GetVerifyReq request;
    request.set_email(email);
    GetVerifyRsp response;

    ClientContext context;
    Status status = _stub->GetVerifyCode(&context, request, &response);

    if (!status.ok()) {
      std::cerr << "GetVerifyCode RPC failed: " << status.error_message() << std::endl;
      response.set_error(0);
    }

    return response;
  }

 private:
   VerifyGrpcClient() {
     std::shared_ptr<Channel> channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
     _stub = VerifyService::NewStub(channel);
     if (!_stub) {
       std::cerr << "Failed to create stub for VerifyService" << std::endl;
     }
     std::cout << "VerifyGrpcClient initialized successfully." << std::endl;
   }
  VerifyGrpcClient(const VerifyGrpcClient&) = delete;
  VerifyGrpcClient& operator=(const VerifyGrpcClient&) = delete;
  VerifyGrpcClient(VerifyGrpcClient&&) = delete;
  VerifyGrpcClient& operator=(VerifyGrpcClient&&) = delete;
  std::unique_ptr<VerifyService::Stub> _stub;
};