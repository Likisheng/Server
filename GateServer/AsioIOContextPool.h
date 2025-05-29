#pragma once
#include <boost/asio/io_context.hpp>
#include<vector>
#include <boost/asio.hpp>
#include "Singleton.h"

// 新版本Work用net::executor_work_guard<net::io_context::executor_type>
using IOContext = boost::asio::io_context;
using Work = boost::asio::executor_work_guard<IOContext::executor_type>;
using WorkPtr = std::shared_ptr<Work>;

class AsioIOContextPool : public Singleton<AsioIOContextPool> {
  friend class Singleton<AsioIOContextPool>;

public:

  ~AsioIOContextPool();
  AsioIOContextPool(const AsioIOContextPool &) = delete;
  AsioIOContextPool &operator=(const AsioIOContextPool &) = delete;
  boost::asio::io_context &GetIOContext();
  void Stop();

private:
  //默认线程大小
  AsioIOContextPool(std::size_t size = 2);
  //有多少context就有多少线程以及workptr
  std::vector<boost::asio::io_context> _ioContexts;
  std::vector<WorkPtr> _works;
  std::vector<std::thread> _threads;
  std::size_t _nextIOContext;
};