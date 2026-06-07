#define ASIO_STANDALONE  // 不需要Boost
#define ASIO_HEADER_ONLY // 使用头文件模式

#ifndef ASIO_IP_HPP
#define ASIO_IP_HPP

#include <asio/asio.hpp>  // 使用-I参数指定的路径

std::string getLocalIP() {
    try {
        asio::io_context io_context;
        
        // 创建一个UDP socket来获取本地IP
        asio::ip::udp::socket socket(io_context);
        socket.open(asio::ip::udp::v4());
        
        // 连接到外部地址
        asio::ip::udp::endpoint remote_endpoint(
            asio::ip::make_address("8.8.8.8"), 80);
        
        socket.connect(remote_endpoint);
        
        // 获取本地端点信息
        asio::ip::udp::endpoint local_endpoint = socket.local_endpoint();
        return local_endpoint.address().to_string();
        
    } catch (std::exception& e) {
        std::cerr << "Error getting local IP: " << e.what() << std::endl;
        return "127.0.0.1";
    }
}

#endif
