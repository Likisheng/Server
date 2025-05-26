cmake_minimum_required(VERSION 3.20)
project(GateServer CXX)

# 指定C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 设置策略，避免 CMP0167 警告（兼容旧的 FindBoost）
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

# 查找Boost库
find_package(Boost REQUIRED COMPONENTS filesystem system)

# 查找 gRPC 并指定完整组件
find_package(gRPC REQUIRED COMPONENTS grpc++ grpc)

# 查找JsonCpp库
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
if(JSONCPP_FOUND)
    include_directories(${JSONCPP_INCLUDE_DIRS})
    link_directories(${JSONCPP_LIBRARY_DIRS})
else()
    message(FATAL_ERROR "JsonCpp not found")
endif()

# 添加源文件和头文件路径
include_directories(${CMAKE_SOURCE_DIR})
set(SOURCES 
    CServer.cpp
    HttpConnection.cpp
    LogicSystem.cpp
    ConfigMgr.cpp
    VerifyGrpcClient.cpp
    message.grpc.pb.cc
    message.pb.cc)


# 添加可执行文件
add_executable(GateServer ${SOURCES})

# 链接库
target_link_libraries(GateServer ${Boost_LIBRARIES} ${JSONCPP_LIBRARIES} ${Protobuf_LIBRARIES} gRPC::grpc++ gRPC::grpc Boost::filesystem Boost::system)


