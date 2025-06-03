cmake_minimum_required(VERSION 3.20)
project(GateServer CXX)

# 指定C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 设置策略，避免 CMP0167 警告（兼容旧的 FindBoost）
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

# 查找hiredis库 - 添加在这里
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)

# 查找Boost库
find_package(Boost REQUIRED COMPONENTS filesystem system)

# 查找 gRPC 并指定完整组件
find_package(gRPC REQUIRED COMPONENTS grpc++ grpc)

# 查找JsonCpp库
pkg_check_modules(JSONCPP jsoncpp)

# 添加源文件和头文件路径
include_directories(${CMAKE_SOURCE_DIR})
include_directories(
    ${JSONCPP_INCLUDE_DIRS}
    ${HIREDIS_INCLUDE_DIRS}  # 添加hiredis头文件路径
)


set(SOURCES 
    CServer.cpp
    HttpConnection.cpp
    LogicSystem.cpp
    ConfigMgr.cpp
    VerifyGrpcClient.cpp
    message.grpc.pb.cc
    message.pb.cc
    AsioIOContextPool.cpp
    RedisMgr.cpp)


# 添加可执行文件
add_executable(GateServer ${SOURCES})

# 链接库 - 添加hiredis
target_link_libraries(GateServer 
    ${Boost_LIBRARIES} 
    ${JSONCPP_LIBRARIES} 
    ${HIREDIS_LIBRARIES}  # 添加hiredis链接
    ${Protobuf_LIBRARIES} 
    gRPC::grpc++ 
    gRPC::grpc 
    Boost::filesystem 
    Boost::system)


