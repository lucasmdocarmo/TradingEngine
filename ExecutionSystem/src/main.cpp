#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <nlohmann/json.hpp>
#include <openssl/ssl.h>

namespace net = boost::asio;
namespace websocket = boost::beast::websocket;
using json = nlohmann::json;

int main() {
    std::cout << "Starting basic execution system environment check..." << std::endl;
    
    // Test JSON.
    json j = {{"system", "execution_engine"}, {"status", "initializing"}};
    std::cout << "JSON Check: " << j.dump(4) << std::endl;

    // Test OpenSSL linking
    std::cout << "OpenSSL Check: " << OpenSSL_version(OPENSSL_VERSION) << std::endl;

    // Test Boost linking
    net::io_context ioc;
    std::cout << "Boost ASIO Check: IoContext created." << std::endl;

    return 0;
}
