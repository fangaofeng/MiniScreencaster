#if 0

#include "Sender.hpp"
#include "ThreadBase.hpp"
#include <iostream>
using namespace std;

class mythread :public ThreadBase
{
public:
	void start()override
	{
		quit_ = false;
		ThreadBase::start();
	}

	void quite()
	{
		quit_ = true;
		ThreadBase::quit();
	}

	void threadMain()override
	{
		while(!quit_)
		{
			cout << "threadMain()" << this_thread::get_id() << endl;
			quite();
		}

	}
private:
	int data_;
	bool quit_;
};

int main()
{
	mythread t;
	t.start();
	t.getpThread()->join();
	return 0;
}

#else

#include "WinSocket.hpp"
#include "Screen.hpp"
#include <iostream>
#include <string.h>
#include <vector>
#pragma comment (lib, "ws2_32.lib")  //���� ws2_32.dll
#pragma warning(disable : 4996)

using namespace cv;
using namespace std;

struct Addr
{
	struct in_addr ip;
	int port;
};


bool sendVector(const vector<uchar>& vec, Socket socket)
{
	char c;
	int check = 0;
	int sentBytes = 0;
	int toSendByte = vec.size();
	int ret = socket.Recv(&c, sizeof(char));
	if (ret <= 0)
		return false;

	while (sentBytes < toSendByte)
	{
		ret = socket.Send((char *)&vec[0] + sentBytes, toSendByte - sentBytes);
		if (ret <= 0)
			return false;
		sentBytes += ret;
		//cout << "Send " << sentByte << " Byte\n";
	}
	//cout << "Check: " << check << endl;
	cout << "sentBytes: " << sentBytes << endl;
	return true;
}

int main()
{
	//��ʼ�� DLL
	WSADATA data;
	WORD w = MAKEWORD(2, 0);
	::WSAStartup(w, &data);

	Addr myaddr;

	char c = '1';
	Socket Sender;
	Sender.Create(AF_INET, SOCK_STREAM, 0);
	int contain;
	setsockopt(Sender.getSocket(), SOL_SOCKET, SO_REUSEADDR, (char *)&contain, sizeof(int));

	Sender.Connect(8888, "39.108.227.206");

	Sender.Recv((char *)&myaddr, sizeof(Addr));
	std::cout << "local ip:" << inet_ntoa(myaddr.ip) << " port:" << myaddr.port << std::endl;

	int width, height;
	GetWidthAndHeight(width, height);
	HBITMAP hBmp;

	while (1)
	{
		Screen(hBmp, width, height);
		Mat mat;
		Mat mat2;

		HBitmapToMat(hBmp, mat);

		vector<uchar> buff;
		vector<int> param = vector<int>(2);
		param[0] = IMWRITE_JPEG_CHROMA_QUALITY;
		param[1] = 0;

		imencode(".jpeg", mat, buff, param);
		int len = buff.size();
		cout << "code file size(jpeg):" << buff.size() << endl;
		cout << "==============================================\n";
		Sender.Send((char *)&len, sizeof(int));

		if (!sendVector(buff, Sender))
		{
			cout << "�������\n";
			break;
		}
		mat2 = imdecode(Mat(buff), IMREAD_COLOR);

		resize(mat, mat2, Size(width * 2 / 3, height * 2 / 3), 0, 0);
		imshow("Sender", mat2);
		if (waitKey(10) == 27)
			break;
	}

	//writer.release();
	WSACleanup();
}

#endif