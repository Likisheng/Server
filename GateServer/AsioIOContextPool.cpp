#include "AsioIOContextPool.h"
#include <boost/asio/io_context.hpp>
#include <iostream>
using namespace std;

AsioIOContextPool::AsioIOContextPool(std::size_t size)
    : _ioContexts(size), _works(size), _nextIOContext(0) {
  for (std::size_t i = 0; i < size; i++) {
    _works[i] = std::unique_ptr<Work>(new Work(_ioContexts[i].get_executor()));
  }

  for (std::size_t i = 0; i < _ioContexts.size(); i++) {
    _threads.emplace_back([this, i]() {
      try {
        _ioContexts[i].run();
      } catch (const std::exception &e) {
        std::cerr << "Exception in thread " << i << ": " << e.what()
                  << std::endl;
      }
    });
  }
}

AsioIOContextPool::~AsioIOContextPool() {
  Stop();
  cout << "destruct the AsioIOContextPool" << endl;
}

boost::asio::io_context& AsioIOContextPool::GetIOContext() {
  if (_ioContexts.empty()) {
    throw std::runtime_error("No IOContext available");
  }
  
  auto& ioContext = _ioContexts[_nextIOContext];
  _nextIOContext = (_nextIOContext+ 1) % _ioContexts.size();
  return ioContext;
}

void AsioIOContextPool::Stop() {

  for(auto &ioContext : _ioContexts) {
    ioContext.stop(); // Stop each IOContext
  }
  // Reset the work guards to allow IOContext to stop
  for (auto &work : _works) {
    work.reset(); // Reset the work guard to allow IOContext to stop
  }
  for (auto& thread : _threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
  
  _threads.clear();
  _works.clear();
  _ioContexts.clear();
}