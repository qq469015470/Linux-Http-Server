# 项目描述
  C++开发的HttpServer。
  使用openssl库，可加密成https通讯。
  有基本的HttpServer功能，可以使用websocket通讯。
	使用Google的gtest库，写了部分模块的单元测试。

# CMake版本需求
  
   该项目使用CMake管理，最低版本需要3.18
  
# 引用的库
  ## OpenSSL
  https://github.com/openssl/openssl 用于加密Https通讯
  ## gtest
  https://github.com/google/googletest 谷歌的单元测试库，用于单元测试

# 使用方法
  该库只需要引用web.h即可，路径位于src/web.h
  使用方法可参考src/main.cpp文件
