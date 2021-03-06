#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>

struct PackInfo
{
	long long id;
	long long length;
	bool isLast;
};

struct SendPack
{
	PackInfo head;
	char data[1024];
};

typedef SendPack RecvPack;
typedef std::vector<RecvPack> RP_vector;

class UDP
{
public:
	static int Create()
	{
		int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd == -1)
		{
			std::cout << "Create error!\n";
			exit(1);
		}
		return sockfd;
	}

	static void Bind(int sockfd, const int port, const char* ip = nullptr)
	{
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		if (nullptr != ip)
		{
			addr.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			addr.sin_addr.s_addr = INADDR_ANY;
		}

		if (bind(sockfd, (sockaddr*)& addr, sizeof(addr)) == -1)
		{
			//throw "Bind error!";
			std::cout << "Bind error!\n";
			exit(2);
		}
	}

    static bool IsRepeat(int id, RP_vector &rp_v)
    {
        for(const auto &e: rp_v)
        {
            if(e.head.id == id)return true;
        }
        return false;
    }

	static void RecvVideo(int socket, const char *FileName, struct sockaddr_in& PeerAddr)
	{
		std::ofstream video(FileName, std::ios::out | std::ios::binary);
		if (!video.is_open())
		{
			exit(-1);
		}

		RP_vector rp_vec;
		RecvPack rp;
	    socklen_t  addr_len = sizeof(PeerAddr);
		int sum_size = 0;

		while (recvfrom(socket, &rp, sizeof(rp), 0, (struct sockaddr*)&PeerAddr, &addr_len))
		{
            if(IsRepeat(rp.head.id, rp_vec))continue;
			rp_vec.push_back(rp);
			std::cout << "id:" << rp.head.id <<
				" length:" << rp.head.length <<
				" isLast: " << rp.head.isLast << std::endl;
			sum_size += rp.head.length;

			sendto(socket, (char*)&rp.head.id, sizeof(rp.head.id), 0, (struct sockaddr*)&PeerAddr, addr_len);
		}

		sort(rp_vec.begin(), rp_vec.end(),
			[](const RecvPack &rp1, const RecvPack &rp2) {return rp1.head.id < rp2.head.id; });

		for (const auto e : rp_vec)
		{
			std::cout << e.head.id << " ";
			video.write(e.data, e.head.length);
		}
		std::cout << std::endl;

		std::cout << "file size: " << sum_size << std::endl;
		video.close();
	}
};
