cmake_minimum_required(VERSION 3.20)
project(GateServer CXX)

# 设置 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 避免 CMP0167 警告（兼容旧 Boost）
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

# 启用调试输出
set(CMAKE_VERBOSE_MAKEFILE ON)

# ================= 查找依赖 =================

# ---------- hiredis ----------
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)

# ---------- Boost ----------
find_package(Boost REQUIRED COMPONENTS filesystem system)

# ---------- gRPC ----------
find_package(gRPC REQUIRED COMPONENTS grpc++ grpc)
find_package(Protobuf REQUIRED)

# ---------- JsonCpp ----------
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# ---------- MySQL++ ----------
pkg_check_modules(MYSQLPP mysql++ QUIET)

if(NOT MYSQLPP_FOUND)
    set(MYSQLPP_INCLUDE_DIR /usr/include/mysql++)
    set(MYSQLPP_LIBRARY /usr/lib/libmysqlpp.so)

    if(EXISTS ${MYSQLPP_INCLUDE_DIR} AND EXISTS ${MYSQLPP_LIBRARY})
        message(STATUS "MySQL++ found manually")
        set(MYSQLPP_LIBRARIES ${MYSQLPP_LIBRARY})
        set(MYSQLPP_FOUND TRUE)
    else()
        message(FATAL_ERROR "MySQL++ not found")
    endif()
endif()

# 自动获取 mariadb 配置
find_package(PkgConfig REQUIRED)
pkg_check_modules(MARIADB REQUIRED libmariadb)





# ================= 包含路径 =================
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${JSONCPP_INCLUDE_DIRS}
    ${HIREDIS_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${MYSQLPP_INCLUDE_DIR}
    ${MARIADB_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)


# ================= 源文件 =================
set(SOURCES 
    CServer.cpp
    HttpConnection.cpp
    LogicSystem.cpp
    ConfigMgr.cpp
    VerifyGrpcClient.cpp
    message.grpc.pb.cc
    message.pb.cc
    AsioIOContextPool.cpp
    RedisMgr.cpp
    MySqlMgr.cpp
    MySqlDao.cpp
    MySqlPool.cpp
)

# ================= 可执行目标 =================
add_executable(GateServer ${SOURCES})

# ================= 链接库 =================
target_link_libraries(GateServer 
    ${Boost_LIBRARIES} 
    ${JSONCPP_LIBRARIES} 
    ${HIREDIS_LIBRARIES}
    ${Protobuf_LIBRARIES} 
    gRPC::grpc++ 
    gRPC::grpc 
    Boost::filesystem 
    Boost::system
    ${MYSQLPP_LIBRARIES}
    ${MARIADB_LIBRARIES}
)


