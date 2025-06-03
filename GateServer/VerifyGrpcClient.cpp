#include "VerifyGrpcClient.h"
#include <mutex>
#include <string>
#include "ConfigMgr.h"
RPConlPool::RPConlPool(size_t pool_size, std::string host, std::string port)
    : _pool_size(pool_size), _host(host), _port(port), _stop(false) {
  for (size_t i = 0; i < pool_size; ++i) {
    std::shared_ptr<Channel> channel = grpc::CreateChannel(
        host + ":" + port, grpc::InsecureChannelCredentials());
    _stubs.push(std::make_unique<VerifyService::Stub>(channel));
  }
  std::cout << "RPConlPool initialized with " << pool_size << " stubs."
            << std::endl;
}

RPConlPool::~RPConlPool() {
  std::lock_guard<std::mutex> lock(_mutex);
  close();
  while (!_stubs.empty()) {
    _stubs.pop();
  }
}

std::unique_ptr<VerifyService::Stub> RPConlPool::getStub() {
  std::unique_lock<std::mutex> lock(_mutex);

  if (_stop) {
    return nullptr; // If pool is stopped, return nullptr
  }

  _cond_var.wait(lock, [this]() {
    return !_stubs.empty(); // Wait until a stub is available
  });


  auto stub = std::move(_stubs.front());
  _stubs.pop();
  return stub;
  
}

void RPConlPool::close() {
  _stop = true;
  _cond_var.notify_all();
  std::cout << "RPConlPool closed." << std::endl;
}


void RPConlPool::returnStub(std::unique_ptr<VerifyService::Stub> stub) {
  std::lock_guard<std::mutex> lock(_mutex);
  if (_stop) {
    return; // If pool is stopped, do not return the stub
  }
  _stubs.push(std::move(stub));
  _cond_var.notify_one(); // Notify one waiting thread that a stub is available
}



VerifyGrpcClient::VerifyGrpcClient() {
  auto &config_mgr = ConfigMgr::Inst();
  std::string host = config_mgr["VerifyServer"]["host"];  // Default host
  std::string port = config_mgr["VerifyServer"]["port"];  // Default port
  _pool.reset(new RPConlPool(5, host, port));

}

GetVerifyRsp VerifyGrpcClient::GetVerifyCode(const std::string &email) {
  GetVerifyReq request;
  request.set_email(email);
  GetVerifyRsp response;

  ClientContext context;
  auto stub = _pool->getStub();
  Status status = stub->GetVerifyCode(&context, request, &response);

  if (!status.ok()) {
    std::cerr << "GetVerifyCode RPC failed: " << status.error_message()
              << std::endl;
    _pool->returnStub(std::move(stub));
    response.set_error(0);
  }
  _pool->returnStub(std::move(stub));
  return response;
}