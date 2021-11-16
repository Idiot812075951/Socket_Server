#pragma once

#include <windows.h>

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <iterator>
#include <algorithm>
#include <thread>
#include <mutex>
#include <filesystem>

#include "Md5file.h"
#include "FileSender.h"

#pragma comment(lib,"ws2_32.lib")


#define PATH "D:\\1.txt"



using namespace std;

set<string>files;
static set<string>truefolders;

static vector<string> sendfiles;
static vector<string> deletefiles;
static vector<string> intersectfiles;
static bool isTransromFile = false;
static bool isScan = true;

static set<string>files1;
static set<string>files2;

static vector<string>txtfiles1;
static vector<string>txtfiles2;
static vector<string>MD5s1;
static vector<string>MD5s2;





std::mutex mtx;
void initialization();

int main();

void listenfolder(string folderpath);

void scanfolder1(string folderpath);

void scanfolder2(string folderpath);

void CompareFolder(string StdFolder, string TargetFolder);


void SplitStringToSet(const string& s, set<string>& S, const string& c)
{
	string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (string::npos != pos2)
	{
		S.insert(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		S.insert(s.substr(pos1));
}

void SplitStringToVector(const string& s, vector<string>& V, const string& c)
{
	string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (string::npos != pos2)
	{
		V.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		V.push_back(s.substr(pos1));
}


set<string> extractFiles(const string &folder)
{
	set<string> Result;

	if (!exists(filesystem::path(folder)))
	{         //目录不存在直接返回
		return Result;
	}
	auto begin = filesystem::recursive_directory_iterator(folder); //获取文件系统迭代器
	auto end = filesystem::recursive_directory_iterator();    //end迭代器 
	for (auto it = begin; it != end; it++)
	{
		const string spacer(it.depth() * 2, ' ');  //这个是用来排版的空格
		auto& entry = *it;
		if (filesystem::is_regular_file(entry))
		{
			Result.insert(entry.path().string());
		}
		else if (filesystem::is_directory(entry))
		{
			auto Temp = extractFiles(entry.path().string());
			for (auto Iter : Temp)
			{
				Result.insert(Iter);
			}
		}
	}
	return Result;
}

void AsynCompareFiles(shared_ptr<FileSender>SPFileSenderPtr)
{
	//string std_folder = "C://WindowsNoEditor//test";
	string std_folder;
	string unreal = "Unreal:";
	string FileSubname = ".txt";

	std_folder = SPFileSenderPtr->Hear();



	SPFileSenderPtr->FolderPath = std_folder;
	SPFileSenderPtr->UnrealPath = unreal;

	set<string>client_files;
	set<string>server_files;
	set<string>delfiles;
	set<string>addfiles;




	string s = SPFileSenderPtr->NewHear();
	if (s.size()>1)
	SplitStringToSet(s, client_files, ";");

	set<string>::iterator it = client_files.begin();
	while (it != client_files.end())
	{
		cout << *it << endl;
		it++;
	}


	set<string> tmp11 = extractFiles(std_folder);
	set<string>::iterator itt = tmp11.begin();

	while (itt != tmp11.end())
	{
		string ss = *itt;
		ss = ss.replace(ss.find(std_folder), std_folder.length(), SPFileSenderPtr->UnrealPath);
		server_files.insert(ss);
		itt++;
	}


	std::set_difference(client_files.begin(), client_files.end(), server_files.begin(), server_files.end(), inserter(delfiles, delfiles.begin()));
	std::set_difference(server_files.begin(), server_files.end(), client_files.begin(), client_files.end(), inserter(addfiles, addfiles.begin()));
	std::set_intersection(client_files.begin(), client_files.end(), server_files.begin(), server_files.end(), inserter(intersectfiles, intersectfiles.begin()));
	string lastname = ".txt";
	vector<string> Md5SharedFiles;

	for (auto i: intersectfiles)
	{
		auto idx = i.find(lastname);//在a中查找b.
		if (idx == string::npos)//不存在。
		{
			continue;
		}
		else//存在。
		{
			Md5SharedFiles.push_back(i);
		}
	}


	//这里开始告诉对方，需要计算md5的文件名有哪些
	if (Md5SharedFiles.size()>0)
	{
		SPFileSenderPtr->SayMd5filesName(Md5SharedFiles);
	}
	else
	{
		SPFileSenderPtr->NewSay("0");
	}
	
	//开始收听对方发过来的md5
	string client_files_md5s = SPFileSenderPtr->NewHear();
	//   格式     D://Recieve0222221\432432.docx::e04e6d7e4e4134aa5b45a65689fe4a82;

	vector<string> ClientMd5;
	vector<string> ServerMd5;

	for (auto i : Md5SharedFiles)
	{
		string filename = i.replace(i.find(unreal), unreal.length(), std_folder);
		string md5 = getFileMD5(filename);
		//这是为了把 ServerMd5 填充成发过来的md5值一样的格式
		string newpath= filename.replace(filename.find(std_folder), std_folder.length(), unreal);
		//这里不加“;”，因为;只是为了用那个分割函数分割，这里是直接添加
		string md5sValue = newpath + "::" + md5;
		ServerMd5.push_back(md5sValue);
	}

	//SplitStringToSet(client_files_md5s, ClientMd5, ";");
	if (client_files_md5s.size()>1)
	{
		SplitStringToVector(client_files_md5s, ClientMd5, ";");
	}

	//已经完成了 ClientMd5 和 ServerMd5的填充，剩下来就是比较

	//set<string> ChangedMd5Files;
	vector<string> ChangedMd5Files;

	std::set_difference(ClientMd5.begin(), ClientMd5.end(), ServerMd5.begin(), ServerMd5.end(), inserter(ChangedMd5Files, ChangedMd5Files.begin()));

	for (auto i: ChangedMd5Files)
	{
		int name_index=i.find_last_of(":");
		string name = i.substr(0, name_index - 1);
		name = name.replace(name.find(unreal), unreal.length(), std_folder);
		sendfiles.push_back(name);
	}




	if (delfiles.size() > 0)
	{
		for (auto i : delfiles)
		{
			if (find(deletefiles.begin(), deletefiles.end(), i) == deletefiles.end())
			{
				deletefiles.push_back(i);
			}
			else
			{

			}
		}
	}

	if (addfiles.size() > 0)
	{
		for (auto i : addfiles)
		{
			auto f = find(addfiles.begin(), addfiles.end(), i);

			if (find(sendfiles.begin(), sendfiles.end(), i) == sendfiles.end())
			{
				string s = i.replace(i.find(SPFileSenderPtr->UnrealPath), SPFileSenderPtr->UnrealPath.length(), std_folder);
				sendfiles.push_back(i);
			}
			else
			{
				//sendfiles.push_back(i);
			}
		}
	}

	if (sendfiles.size() > 0)
	{
		SPFileSenderPtr->Say("IsTransformFiles");
		string TransformFilesNum = std::to_string(sendfiles.size());
		SPFileSenderPtr->Say(TransformFilesNum);
		for (auto i : sendfiles)
		{
			char *f = i.data();
			//		char p[] = i.data();
			SPFileSenderPtr->SendFile(i);
			std::cout << i << std::endl;
		}

		SPFileSenderPtr->Say("EndRecvFile");
	}
	else
	{
		SPFileSenderPtr->Say("NoTransformFiles");
	}



	//已经发送了下载文件过去，还要通知客户端删除文件；
	string s_delname;
	for (auto i : deletefiles)
	{
		s_delname += i+";";
	}
	SPFileSenderPtr->NewSay(s_delname);


}

int main()
{
	int port = 3300;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ListenSock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in Addr;

	Addr.sin_family = AF_INET;
	Addr.sin_port = htons(port);
	Addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	int r = ::bind(ListenSock, (SOCKADDR*)&Addr, sizeof(Addr));
	listen(ListenSock, 5);
	int len = sizeof(Addr);
	printf("waiting for connect\n");

	for (;;)
	{
		shared_ptr<FileSender>SPFileSenderPtr(new FileSender);
		SPFileSenderPtr->Accept(port, ListenSock, Addr);
		std::thread Thread(AsynCompareFiles, SPFileSenderPtr);
		Thread.detach();

		cout << "新的连接" << endl;
	}

	WSACleanup();
	return 0;
}

void listenfolder(string folderpath)
{
	set<string>files1 = extractFiles(folderpath);
	while (true)
	{
		set<string> delfiles;
		set<string> addfiles;


		Sleep(1000);

		mtx.lock();
		set<string>files2 = extractFiles(folderpath);
		mtx.unlock();


		set_difference(files1.begin(), files1.end(), files2.begin(), files2.end(), inserter(delfiles, delfiles.begin()));
		set_difference(files2.begin(), files2.end(), files1.begin(), files1.end(), inserter(addfiles, addfiles.begin()));


		if (delfiles.size() > 0)
		{
			for (auto i : delfiles)
			{
				cout << "删除了" << i << "\n";
				mtx.lock();
				if (find(deletefiles.begin(), deletefiles.end(), i) == deletefiles.end())
				{
					deletefiles.push_back(i);
				}
				else
				{

				}
				mtx.unlock();
			}
		}

		if (addfiles.size() > 0)
		{
			for (auto i : addfiles)
			{
				cout << "增加了" << i << "\n";
				mtx.lock();
				auto f = find(addfiles.begin(), addfiles.end(), i);

				if (find(sendfiles.begin(), sendfiles.end(), i) == sendfiles.end())
				{
					sendfiles.push_back(i);
				}
				else
				{
					//sendfiles.push_back(i);
				}
				mtx.unlock();
			}
		}
		vector<string>txtfiles;
		vector<string>MD5s1;
		vector<string>MD5s2;

		for (auto i : files2)
		{
			string s = ".txt";
			auto idx = i.find(s);
			if (idx == string::npos)//不存在。
			{
			}
			else//存在。
			{
				mtx.lock();
				//cout << i << "含有txt \n";
				string FileMD5 = getFileMD5(i);
				MD5s1.push_back(FileMD5);
				txtfiles.push_back(i);
				mtx.unlock();
			}
		}

		Sleep(2000);


		for (auto i : files2)
		{
			string s = ".txt";
			auto idx = i.find(s);
			if (idx == string::npos)//不存在。
			{
			}
			else//存在。
			{
				mtx.lock();
				string FileMD5 = getFileMD5(i);
				MD5s2.push_back(FileMD5);
				mtx.unlock();
			}
		}
		if (MD5s2.size() == MD5s1.size())
		{
			for (int i = 0; i != MD5s1.size(); i++)
			{
				if (MD5s1[i] != MD5s2[i])
				{
					cout << "修改了" << txtfiles[i] << endl;
					mtx.lock();
					sendfiles.push_back(txtfiles[i]);
					mtx.unlock();
				}
			}
		}

	}
}



void scanfolder1(string folderpath)
{
	files1 = extractFiles(folderpath);
	for (auto i : files1)
	{
		string s = ".txt";
		auto idx = i.find(s);
		if (idx == string::npos)//不存在。
		{
		}
		else//存在。
		{
			mtx.lock();
			//cout << i << "含有txt \n";
			string FileMD5 = getFileMD5(i);
			MD5s1.push_back(FileMD5);
			txtfiles1.push_back(i);
			mtx.unlock();
		}
	}

}



void scanfolder2(string folderpath)
{

	set<string> delfiles;
	set<string> addfiles;
	mtx.lock();
	set<string>files2 = extractFiles(folderpath);
	mtx.unlock();

	set_difference(files1.begin(), files1.end(), files2.begin(), files2.end(), inserter(delfiles, delfiles.begin()));
	set_difference(files2.begin(), files2.end(), files1.begin(), files1.end(), inserter(addfiles, addfiles.begin()));


	if (delfiles.size() > 0)
	{
		for (auto i : delfiles)
		{
			cout << "删除了" << i << "\n";
			mtx.lock();
			if (find(deletefiles.begin(), deletefiles.end(), i) == deletefiles.end())
			{
				deletefiles.push_back(i);
			}
			else
			{

			}
			mtx.unlock();
		}
	}

	if (addfiles.size() > 0)
	{
		for (auto i : addfiles)
		{
			cout << "增加了" << i << "\n";
			mtx.lock();
			auto f = find(addfiles.begin(), addfiles.end(), i);

			if (find(sendfiles.begin(), sendfiles.end(), i) == sendfiles.end())
			{
				sendfiles.push_back(i);
			}
			else
			{
			}
			mtx.unlock();
		}
	}

	for (auto i : files2)
	{
		string s = ".txt";
		auto idx = i.find(s);
		if (idx == string::npos)//不存在。
		{
		}
		else//存在。
		{
			mtx.lock();
			string FileMD5 = getFileMD5(i);
			MD5s2.push_back(FileMD5);
			mtx.unlock();
		}
	}


	if (MD5s2.size() == MD5s1.size())
	{
		for (int i = 0; i != MD5s1.size(); i++)
		{
			if (MD5s1[i] != MD5s2[i])
			{
				cout << "修改了" << txtfiles2[i] << endl;
				mtx.lock();
				sendfiles.push_back(txtfiles2[i]);
				mtx.unlock();
			}
		}
	}
	//这里还要考虑，校对md5文件，数量不一致的逻辑
	else
	{

	}





}



void initialization() {
	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		cout << "初始化套接字库失败！" << endl;
	}
	else {
		cout << "初始化套接字库成功！" << endl;
	}
	//检测版本号
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		cout << "套接字库版本号不符！" << endl;
		WSACleanup();
	}
	else {
		cout << "套接字库版本正确！" << endl;
	}
	//填充服务端地址信息

}


void CompareFolder(string StdFolder, string TargetFolder)
{

	set<string> tmp = extractFiles(StdFolder);
	set<string>::iterator it = tmp.begin();

	set<string> StdFolderFiles;

	string unreal = "Unreal:";
	while (it != tmp.end())
	{
		string s = *it;
		s = s.replace(s.find(StdFolder), StdFolder.length(), unreal);
		StdFolderFiles.insert(s);
		it++;
	}


	set<string> tmp2 = extractFiles(TargetFolder);
	set<string>::iterator it1 = tmp2.begin();

	set<string> TargetFolderFiles;
	while (it1 != tmp2.end())
	{
		string s2 = *it1;
		s2 = s2.replace(s2.find(TargetFolder), TargetFolder.length(), unreal);
		TargetFolderFiles.insert(s2);
		it1++;
	}


	set<string> extraFiles;  //这是多的文件
	set<string> lackFiles;  //这是少的文件


	set_difference(StdFolderFiles.begin(), StdFolderFiles.end(), TargetFolderFiles.begin(), TargetFolderFiles.end(),
		inserter(lackFiles, lackFiles.begin()));
	set_difference(TargetFolderFiles.begin(), TargetFolderFiles.end(), StdFolderFiles.begin(), StdFolderFiles.end(),
		inserter(extraFiles, extraFiles.begin()));


	set<string>::iterator it2 = extraFiles.begin();
	while (it2 != extraFiles.end())
	{
		string s = *it2;
		//下面这句话是，替换掉  共有路径  到各自的绝对路径
		s = s.replace(s.find(unreal), unreal.length(), TargetFolder);
		cout << "应该删除：" << s << endl;


		//把多余的文件房间下载列表里面
		deletefiles.push_back(s);
		it2++;
	}

	set<string>::iterator it3 = lackFiles.begin();
	while (it3 != lackFiles.end())
	{
		string s1 = *it3;
		s1 = s1.replace(s1.find(unreal), unreal.length(), StdFolder);
		cout << "应该在目标文件夹下载：" << s1 << endl;

		//把缺失的文件房间下载列表里面
		sendfiles.push_back(s1);

		it3++;
	}



}