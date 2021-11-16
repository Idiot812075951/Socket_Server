
#include <windows.h>
#include <string>
#include <fstream>
#include <iostream>
#include <array>
#include <vector>

#define LOG(logStr)\
{\
char LogAry[512] = {};\
sprintf_s(LogAry, "%s %d %s \n", __FUNCTION__, __LINE__, logStr);\
OutputDebugStringA(LogAry);\
}

struct FileName {//存储形式："文件名.扩展名"
	char Fname[64];
	int len;
};

class FileSender
{
public:

	FileSender();

	~FileSender();

	std::string FolderPath;
	std::string UnrealPath;

	int Accept(
		int port,
		SOCKET ListenSock,
		const sockaddr_in &Addr
	);

	int SendFile(const std::string& path);

	void Close();

	int Say(const std::string & message);

	int NewSay(const std::string & message);


	int SayMd5filesName(const std::vector<std::string>& Md5files);
	int SayMd5filesValue(const std::vector<std::string>& Md5files);

	std::string Hear();

	std::string NewHear();

private:

	static const int CacheSize = 64;//48
	//当传输容器最后一个索引的时候，会乱码

	std::array<char, CacheSize>CacheAry;

	SOCKET Sock;

};

