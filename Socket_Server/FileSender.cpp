
#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "FileSender.h"

#include <filesystem>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

FileSender::FileSender() 
{
	Sock = socket(AF_INET, SOCK_STREAM, 0);
}

FileSender::~FileSender()
{
	Close();
}

int FileSender::Accept(
	int port,
	SOCKET ListenSock,
	const sockaddr_in &Addr
)
{
	int len = sizeof(Addr);
	Sock = accept(ListenSock, (sockaddr*)&Addr, &len);
	auto error = GetLastError();
	if (Sock == INVALID_SOCKET)
	{
		cout << "没有连接上" << endl;
	}
	else
	{
		cout << "连上了文件服务器" << endl;
	}
	return 1;
}

int FileSender::SendFile(const std::string& path)
{
	ifstream Fstream(path, ios::binary | ios::beg);
	if (Fstream.is_open())
	{
		//Say(filesystem::path(path).filename().string());
		string s = filesystem::path(path).string();
		s= s.replace(s.find(FolderPath), FolderPath.length(),UnrealPath);

		//Say(filesystem::path(path).string());
		Say(s);
		LOG(s.data());

		long long FileSize = filesystem::file_size(filesystem::path(path));
		send(Sock, (CHAR*)&FileSize, sizeof(FileSize), 0);
		LOG(std::to_string(FileSize).data());

		for (; ;)
		{
			memset(CacheAry.data(), 0, CacheSize);
			Fstream.read(CacheAry.data(), CacheSize);

			if (Fstream.eof())
			{
				auto Gcount = Fstream.gcount();
				send(Sock, (CHAR*)CacheAry.data(), Gcount, 0);
				break;
			}
			else 
			{
				send(Sock, (CHAR*)CacheAry.data(), CacheSize, 0);
			}
		}
	}
	return 0;
}

int FileSender::Say(const std::string & message)
{
	memset(CacheAry.data(), 0, CacheSize);
	strcpy(CacheAry.data(), message.data());
	int send_len = send(Sock, CacheAry.data(), CacheSize, 0);
	return send_len;
}


int FileSender::NewSay(const std::string & message)
{
	string s = message;
	Say(std::to_string(s.size()));
	int SendSize = 0;
	for (; ;)
	{
		if ((s.size() - SendSize) < CacheSize)
		{
			auto TempStr = s.substr(SendSize, s.size() - SendSize);
			send(Sock, (CHAR*)TempStr.data(), TempStr.size(), 0);
			break;
		}
		else
		{
			auto TempStr = s.substr(SendSize, CacheSize);
			SendSize += send(Sock, (CHAR*)TempStr.data(), TempStr.size(), 0);
		}
	}

	return s.size();
}


//发送名称已经好了
//先发送名称，渲染服务器再把md5发送回来
//通过校验，再次把修改过的文件，加入下载列表
int FileSender::SayMd5filesName(const vector<string>& Md5files)
{
	string s = "";
	for(auto i:Md5files)
	{
		s +=i+";";
	}
	
	string FilesLen = std::to_string(s.size());
	//send(Sock, (CHAR*)&FilesLen, sizeof(FilesLen), 0);
	Say(FilesLen);

	int SendSize = 0;
	for (; ;)
	{
		if ((s.size() - SendSize) < CacheSize)
		{
			auto TempStr = s.substr(SendSize, s.size() - SendSize);
			send(Sock, (CHAR*)TempStr.data(), TempStr.size(), 0);
			break;
		}
		else
		{
			auto TempStr = s.substr(SendSize, CacheSize);
			SendSize += send(Sock, (CHAR*)TempStr.data(), TempStr.size(), 0);
		}
	}

	return s.size();
}

int FileSender::SayMd5filesValue(const std::vector<std::string>& Md5files)
{
	return 0;
}

std::string FileSender::Hear()
{
	int recv_len = recv(Sock, CacheAry.data(), CacheSize, 0);
	if (recv_len < 0)
	{
		//cout << "接受失败！" << endl;
		return "";
	}
	else
	{
		cout << "recieve:" << CacheAry.data() << endl;
		return CacheAry.data();
	}

	
}


std::string FileSender::NewHear()
{
	std::string Md5filesNum;
	Md5filesNum = FileSender::Hear();
	int Ssize = atoi(Md5filesNum.c_str());
	int RecSize = 0;
	vector<char>StrContent;
	for (; ;)
	{
		memset(CacheAry.data(), 0, CacheSize);
		if ((Ssize - RecSize) < CacheSize)
		{
			recv(Sock, CacheAry.data(), Ssize - RecSize, 0);
			for (int Index = 0; Index < (Ssize - RecSize); Index++)
			{
				StrContent.emplace_back(CacheAry[Index]);
			}
			break;
		}
		else
		{
			RecSize += recv(Sock, CacheAry.data(), CacheSize, 0);
			for (int Index = 0; Index < CacheSize; Index++)
			{
				StrContent.emplace_back(CacheAry[Index]);
			}
			//sprintf(StrContent + RecSize - CacheSize, "%s", CacheAry.data());
		}
	}

	string s = "";
	std::vector<char>::iterator it= StrContent.begin();
	while (it!=StrContent.end())
	{
		s += *it;
		it++;
	}

	//std::string Result(StrContent);
	return s;

}


void FileSender::Close()
{
	closesocket(Sock);
}
