#pragma once
#include <iostream>
#include <memory>
#include <mutex>

template <typename T> class Singleton {
protected:
  Singleton() = default;
  Singleton(const Singleton<T> &) = delete;
  Singleton &operator=(const Singleton<T> &) = delete;
  static std::shared_ptr<T> _instance;

public:
  static std::shared_ptr<T> GetInstance() {
    static std::once_flag flag;
    std::call_once(flag, [&]() { _instance = std::shared_ptr<T>(new T); });
    return _instance;
  }

  void printAddress() { std::cout << _instance.get() << std::endl; }

  ~Singleton() { std::cout << "destruct the singleton" << std::endl; }
};

template <typename T> std::shared_ptr<T> Singleton<T>::_instance = nullptr;