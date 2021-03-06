#pragma once
#include "WinSocket.hpp"
#include "Screen.hpp"
#include "protocol.hpp"
#include <queue>
#include <string>
#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <thread>
#include <time.h>
#include <sstream>
#pragma comment (lib, "ws2_32.lib")  //���� ws2_32.dll
#pragma warning(disable : 4996)

using namespace cv;

typedef std::queue<std::string> SendQueue;

std::string int2str(int n)
{
	std::stringstream ss;
	ss << n;
	return ss.str();
}

void getFileName(std::string &filename)
{
	filename = "cache\\videocache";
	time_t nowtime;
	time_t timestamp = time(&nowtime);
	tm localtm;
	localtime_s(&localtm, &timestamp);

	filename += int2str(localtm.tm_year + 1900) + "_" + 
		int2str(localtm.tm_mon + 1) + "_" +
		int2str(localtm.tm_mday) + "_" +
		int2str(localtm.tm_hour) + "_" +
		int2str(localtm.tm_min) + "_" +
		int2str(localtm.tm_sec) + ".mp4";
}

void PrintFourCC(VideoCapture &capture)
{
	unsigned int f = (unsigned)capture.get(cv::CAP_PROP_FOURCC);
	char fourcc[] = {
		(char)f, // First character is lowest bits
		(char)(f >> 8), // Next character is bits 8-15
		(char)(f >> 16), // Next character is bits 16-23
		(char)(f >> 24), // Last character is bits 24-31
		'\0' // and don't forget to terminate
	};
	std::cout << "FourCC for this video was: " << fourcc << std::endl;
}

int Producer(SendQueue &sq)
{
	std::cout << &sq << std::endl;
	VideoCapture capture(0);
	if (!capture.isOpened())
		return -1;

	double dWidth = capture.get(CAP_PROP_FRAME_WIDTH); //get the width of frames of the video  
	double dHeight = capture.get(CAP_PROP_FRAME_HEIGHT);

	std::string filename;

	bool quit_flag = false;
	while (!quit_flag)
	{
		getFileName(filename);
		std::cout << "Screening:" << filename << "..." << std::endl;
		VideoWriter writer(filename.c_str(), MAKEFOURCC('D', 'I', 'V', 'X'), 15.0, Size(static_cast<int>(dWidth), static_cast<int>(dHeight)), true);

		if (!writer.isOpened())
		{
			std::cout << "writer open failed!\n";
			return -1;
		}
		Mat frame;
		int frameNum = 90;
		while (frameNum--) {
			capture >> frame;
			imshow("¼����Ļ...", frame);
			writer << frame;
			if (waitKey(33) == 27)
			{
				quit_flag = true;
				break;
			}
		}

		sq.push(filename);
		std::cout << "Screening:" << filename << " done" << std::endl;
		writer.release();
		//remove(filename.c_str());
	}

	capture.release();
	destroyAllWindows();

	return 0;
}

int Consumer(SendQueue &sq)
{
	std::cout << &sq << std::endl;
	int sockfd = TCP::Create();
	TCP::Bind(sockfd, 8888);

	if (TCP::Connect(sockfd, 8777, "127.0.0.1"))
	{
		while (1)
		{
			if (sq.empty())
			{
				std::cout << "queue empty...\n";
				Sleep(1000);
			}
			else
			{
				std::string filename(sq.front());
				sq.pop();
				std::cout << "send file : " << filename << std::endl;
				TCP::SendVideo(sockfd, filename.c_str());
			}

		}
	}
	else
	{
		std::cout << "Connect failed!\n";
	}

	return 0;
}

void Test()
{
	SendQueue sq;
	std::thread pth(Producer, ref(sq));
	std::thread cth(Consumer, ref(sq));

	cth.join();
	pth.join();
}

