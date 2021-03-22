//#include "../winsock_help.h"
//#define _WIN32_WINDOWS

#include <iostream>
#include <thread>
#include <functional>
#include <atomic>
#include <mutex>

#include <boost/asio.hpp>

const short PORT_NUM = 7707;
const short BLOG_SIZE = 5;
const short MAX_MSG_LEN = 256;

std::vector<std::thread> workers;
std::mutex mutexG;

using boost::asio::ip::tcp;

void WorkerThread(tcp::socket& socket)
{
	SOCKET dosock;
	DWORD receiveBytes;
	DWORD bytesSend;

	while (1) {
		/*if (GetQueuedCompletionStatus(hIOCP, &receiveBytes, (PULONG_PTR)&keyInfo, (LPOVERLAPPED*)&info, INFINITE)) {
			dosock = keyInfo->socket;
			{
				std::lock_guard<std::mutex> lock_guard(mutexG);
				printf("%s: %d recv from queued [%d] \n",
					keyInfo->ipADDR.c_str(),
					keyInfo->port,
					(int)keyInfo->socket);
				if (receiveBytes == SOCKET_ERROR) {
					closesocket(dosock);
				}
				else if (receiveBytes != 0) {
					printf("recv: %s  [ num : %d ]\n", info->msg, info->myNum);

					auto len = WSASend(dosock, (LPWSABUF)&info->dataBuffer, 1, &bytesSend, 0, NULL, NULL);

					if (len == SOCKET_ERROR) {
						if (WSAGetLastError() != WSA_IO_PENDING) {
							closesocket(dosock);
							break;
						}
					}
					WSARecv(dosock, &info->dataBuffer, 1, &receiveBytes, &flags, &info->overlapped, NULL);

					if (len == SOCKET_ERROR) {
						if (WSAGetLastError() != WSA_IO_PENDING) {
							closesocket(dosock);
							break;
						}
					}
				}
			}
		}
		else {
			closesocket(dosock);
			break;
		}*/
	}
}
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
						printf("recv: %s \n", data_);
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
					std::make_shared<session>(std::move(socket_))->start();
				}
				do_accept();
			});
	}
	tcp::acceptor acceptor_;
	tcp::socket socket_;
};
int main()
{
	boost::asio::io_service io_service_S;
	server s(io_service_S, PORT_NUM);
	io_service_S.run();

	return 0;
}