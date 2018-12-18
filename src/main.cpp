#include <iostream>

#include <autobahn/autobahn.hpp>
#include <autobahn/wamp_websocketpp_websocket_transport.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

int main() {
    std::cout << "Running publisher!\n";

    boost::asio::io_service io;

    websocketpp::client<websocketpp::config::asio_client> ws_client;
    ws_client.set_access_channels(websocketpp::log::alevel::none);
    ws_client.init_asio(&io);

    std::string url("ws://127.0.0.1:43000");

    auto transport = std::make_shared<autobahn::wamp_websocketpp_websocket_transport<websocketpp::config::asio_client>>(ws_client, url);
    auto session = std::make_shared<autobahn::wamp_session>(io);

    transport->attach(std::static_pointer_cast<autobahn::wamp_transport_handler>(session));

    boost::future<void> connect_future;
    boost::future<void> start_future;
    boost::future<void> join_future;
    boost::future<void> leave_future;
    boost::future<void> stop_future;

    connect_future = transport->connect().then([&](boost::future<void> connected) {
        try {
            connected.get();
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
            io.stop();
            return;
        }

        std::cout << "Transport connected\n";

        start_future = session->start().then([&](boost::future<void> started) {
            try {
                started.get();
            }
            catch (const std::exception& e) {
                std::cerr << e.what() << "\n";
                io.stop();
                return;
            }

            std::cout << "Session started\n";

            std::vector<std::string> auth_methods;

            join_future = session->join("my_realm").then([&](boost::future<uint64_t> joined) {
                try {
                    std::cout << "Joined realm: " << joined.get() << "\n";
                }
                catch (const std::exception& e) {
                    std::cerr << e.what() << "\n";
                    io.stop();
                    return;
                }

                std::tuple<std::string> args(std::string("hello"));
                session->publish("com.examples.subscriptions.topic1", args);

                leave_future = session->leave().then([&](boost::future<std::string> reason) {
                    try {
                        std::cout << "Left session: " << reason.get() << "\n";
                    }
                    catch (const std::exception& e) {
                        std::cerr << e.what() << "\n";
                        io.stop();
                        return;
                    }

                    stop_future = session->stop().then([&](boost::future<void> stopped) {
                        std::cout << "Stopped session\n";
                        io.stop();
                    });
                });
            });
        });
    });

    std::cout << "Starting io service\n";
    io.run();
    std::cerr << "Stopped io service\n";

    transport->detach();

    return 0;
}