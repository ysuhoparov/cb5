#include <iostream>
#include <fstream>
#include <istream>
#include <ostream>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;


std::mutex readyMutex;

// Measure time elapsed for URL input 
void inputWait() {
    int count = 0;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        count++;
        if (readyMutex.try_lock()) {
            std::cout << "Elapsed time: " << (float)count/10 << " sec. \n";
            readyMutex.unlock();
            break;
        }
    }
}



int main()
{
    std::locale::global(std::locale(""));
    try
    {
        std::string URL;
        std::thread mezureTimeWait(inputWait);

        std::cout << "Enter URL(example: cyberforum.ru):";
        readyMutex.lock();

        std::getline(std::cin, URL, '\n');

        readyMutex.unlock();

        mezureTimeWait.join();

        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(URL, "80");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::resolver::iterator end;

        tcp::endpoint ep = *endpoint_iterator;
        std::cout << ep.address().to_string() << std::endl;



        tcp::socket socket(io_service);
        boost::system::error_code error = boost::asio::error::host_not_found;
        while (error && endpoint_iterator != end)
        {
            socket.close();
            socket.connect(*endpoint_iterator++, error);
        }
        if (error)
            throw boost::system::system_error(error);

        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "GET / HTTP/1.0\r\n"
            << "Host: " + URL + "\r\n"
            << "Accept: text/html\r\n"
            << "Connection: close\r\n\r\n";

        boost::asio::write(socket, request);
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");

        std::istream response_stream(&response);
        std::string header;
        while (std::getline(response_stream, header) && header != "\r");

        std::ofstream file("web_source.txt");
        if (!file)
            throw std::runtime_error("Cannot create file");

        while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
        {
            file << &response;
        }
        if (error != boost::asio::error::eof)
            throw boost::system::system_error(error);

        std::cout << "Done!\n";

    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what();
    }
    return 0;
}





/*


#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <thread>
#include <iostream>



using namespace boost::asio;
io_service service;
ip::udp::socket sock(service);
boost::asio::ip::udp::endpoint sender_ep;
char buff[512];
void on_read(const boost::system::error_code& err, std::size_t
    read_bytes)
{
    std::cout << "read " << read_bytes << std::endl;
    sock.async_receive_from(buffer(buff), sender_ep, on_read);
}
int main(int argc, char* argv[])
{

    std::cout << "begin \n";
    ip::udp::endpoint ep(ip::address::from_string("127.0.0.1"), 8001);
    sock.open(ep.protocol());
    std::cout << "open\n";
    sock.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    sock.bind(ep);
    std::cout << "bind\n";
    sock.async_receive_from(buffer(buff, 512), sender_ep, on_read);

    std::cout << "async_receive\n";

    service.run();

    std::cout << "run\n";
}


*/

//
//
//using namespace boost::asio;
//
//int main()
//{
//
//
//    io_service ioservice;
//
//    steady_timer timer1{ ioservice, std::chrono::seconds{2} };
//    timer1.async_wait([](const boost::system::error_code& ec)
//        { std::cout << "timer1 2 sec\n"; });
//
//    steady_timer timer2{ ioservice, std::chrono::seconds{1} };
//    timer2.async_wait([](const boost::system::error_code& ec)
//        { std::cout << "timer2 1 sec\n"; });
//
//    std::thread thread1{[&ioservice]() { ioservice.run(); }};
//    std::thread thread2{[&ioservice]() { ioservice.run(); }};
//    thread1.join();
//    thread2.join();
//}