
#include <iostream>
#include <thread>
#include <functional>
#include <atomic>
#include <mutex>
#include <functional>

#include <boost/asio.hpp>

const short PORT_NUM = 7707;
const short BLOG_SIZE = 5;
const short MAX_MSG_LEN = 256;

std::vector<std::thread> workers;
std::mutex mutexG;

using boost::asio::ip::tcp;

class session : public std::enable_shared_from_this<session>
{
public:
	session(tcp::socket socket) : socket_(std::move(socket))
	{

	}
	void start()
	{
		do_read();
	}
private:
	void do_read() {
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(data_, MAX_MSG_LEN),
			[this, self](boost::system::error_code ec, std::size_t length)
			{
				if (!ec) {
					{
						std::lock_guard<std::mutex> lock_guard(mutexG);
						std::stringstream ss;
						ss << std::this_thread::get_id();
						printf("[%s] recv: %s \n", ss.str().c_str(), data_);
					}
					do_write(length);
				}
			}
		);
	}
	void do_write(std::size_t length) {
		auto self(shared_from_this());
		boost::asio::async_write(socket_, boost::asio::buffer(data_, MAX_MSG_LEN),
			[this, self](boost::system::error_code ec, std::size_t /*length*/)
			{
				if (!ec) {
					do_read();
				}
			}
		);
	}
	tcp::socket socket_;
	char data_[MAX_MSG_LEN];
};
class server
{
public:
	server(boost::asio::io_service& io_service, short port) : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), socket_(io_service)
	{
		do_accept();
	}
private:
	void do_accept()
	{
		acceptor_.async_accept(socket_,
			[this](boost::system::error_code ec) {
				if (!ec) {
					{
						std::lock_guard<std::mutex> lock_guard(mutexG);
						std::stringstream ss;
						ss << std::this_thread::get_id();
						printf("[%s] accept: \n", ss.str().c_str());
					}
					std::make_shared<session>(std::move(socket_))->start();
				}
				do_accept();
			});
	}
	tcp::acceptor acceptor_;
	tcp::socket socket_;
};

void worker(boost::asio::io_service& io_service)
{
	io_service.run();
}
int main()
{
	std::stringstream ss;
	ss << std::this_thread::get_id();
	printf("[%s] main : \n", ss.str().c_str());

	boost::asio::io_service io_service_S;
	std::vector<std::thread> workers;
	server s(io_service_S, PORT_NUM);

	for (DWORD i = 0; i < 12 * 2; ++i) {
		/*workers.emplace_back([&io_service_S] {
			io_service_S.run();
			});*/
		workers.emplace_back(std::thread(&worker, std::ref(io_service_S)));
	}

	while (1) {

	}


	return 0;
}