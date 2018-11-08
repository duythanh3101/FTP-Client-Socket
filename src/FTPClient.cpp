#include "stdafx.h"
#include "FTPClient.h"


FTPClient::FTPClient()
{
	mode = ACTIVE;
	isConnected = false;
	int retCode = 0;
	if (retCode = WSAStartup(MAKEWORD(2, 2), &wsaData))
		cout << "Startup failed: " << retCode << endl;
}


FTPClient::~FTPClient()
{
	delete sendBuffer;
	delete recvBuffer;
	DisconnectServer();
}

bool FTPClient::ConnectToServer(int portServer, string ipAddress)
{
	controlSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (controlSocket == INVALID_SOCKET)
	{
		cout << "Socket creation failed: " << WSAGetLastError() << endl;
		return false;
	}

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portServer);
	hostIP = ipAddress;
	if (ipAddress == "localhost")
		hostIP = "127.0.0.1";

	serverAddress.sin_addr.S_un.S_addr = inet_addr(hostIP.c_str());

	if (connect(controlSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		cout << "Connect failed: " << WSAGetLastError() << endl;
		return false;
	}

	cout << "Connection established, waiting for welcome message...\n";
	recvData();
	isConnected = true;
	return true;
}

void FTPClient::DisconnectServer()
{
	closesocket(controlSocket);
	WSACleanup();
}

//227 Entering Passive Mode(127, 0, 0, 1, 212, 85)
int FTPClient::getPort()
{
	char *copiedBuffer = recvBuffer;
	char *tokenWord = strtok(copiedBuffer, ",)");
	int number[6];

	for (int i = 0; i < 6; i++)
	{
		number[i] = atoi(tokenWord);
		tokenWord = strtok(NULL, ",)");
	}

	return number[4] * 256 + number[5];
}

bool FTPClient::Login()
{
	cout << "Username ";
	getline(cin, userName);	
	return user(userName);
}

bool FTPClient::user(vector<string> cmd)
{
	if (cmd.size() > 2)
	{
		cout << "Invalid command" << endl;
		return false;
	}
	if (cmd.size() == 1)
	{
		Login();
	}
	else if (cmd.size() == 2)
	{
		user(cmd[1]);
	}
	return true;
}

bool FTPClient::user(string userName)
{
	sendCommand("USER " + userName);
	int ret = recvData();

	if (ret > 0)
	{
		//331 Password required for username
		if (strncmp(recvBuffer, "331", 3) == 0)
		{
			cout << "Password ";
			//getline(cin, password);
			password = "";
			char ch;
			ch = _getch();
			while (ch != 13) //Ki tu Enter
			{
				if (ch != 8) //Ki tu BackSpace
				{
					password.push_back(ch);
				}
				else
				{
					if (password.size() > 0)
					{
						password.pop_back();
					}
				}
				ch = _getch();
			}
			cout << endl;

			sendCommand("PASS " + password);
			ret = recvData();

			if (ret > 0)
			{
				//230 Logged on
				if (strncmp(recvBuffer, "230", 3) == 0)
				{
					return true;
				}
				else
				{
					//cout << "530 Login or password incorrect!" << endl;
					return false;

				}
			}
		}
	}
}

int FTPClient::sendCommand(string cmd)
{
	cmd += "\r\n";

	send(controlSocket, cmd.c_str(), cmd.length(), 0);
	return 0;
}

int FTPClient::recvData()
{
	memset(recvBuffer, 0, LENGTH);
	int retCode = recv(controlSocket, recvBuffer, LENGTH, 0);
	if (retCode == -1)
	{
		cout << "Not connected" << endl;
	}
	cout << recvBuffer;
	return retCode;
}

int FTPClient::getCode()
{
	int x = 0;
	sscanf_s(recvBuffer, "%d", &x);

	return x;
}

bool FTPClient::dir()
{
	//Chuyen sang pasv
	this->pasv();
	//---------//
	sendCommand("LIST -al");
	int ret = recvData();
	memset(recvBuffer, 0, LENGTH);
	ret = recvData();
	memset(recvBuffer, 0, LENGTH);
	ret = recv(dataSocket, recvBuffer, LENGTH, 0);
	if (ret == -1)
	{
		cout << "Recv() failed " << WSAGetLastError() << endl;
		return false;
	}
	//---------//
	cout << recvBuffer;
	memset(recvBuffer, 0, LENGTH);
	return true;
}

bool FTPClient::ls()
{
	//Chuyen sang pasv
	this->pasv();
	//---------//
	sendCommand("NLST ");
	int ret = recvData();
	memset(recvBuffer, 0, LENGTH);
	ret = recvData();
	memset(recvBuffer, 0, LENGTH);
	memset(recvBuffer, 0, LENGTH);
	ret = recv(dataSocket, recvBuffer, LENGTH, 0);
	if (ret == -1)
	{
		cout << "Recv() failed " << WSAGetLastError() << endl;
		return false;
	}
	//----------//
	cout << recvBuffer;
	memset(recvBuffer, 0, LENGTH);
	return true;
}

bool FTPClient::cd(vector<string> cmd)
{
	if (cmd.size() == 1)
	{
		string remote;
		cout << "Remote directory ";
		getline(cin, remote);
		return cd(remote);
	}
	else if (cmd.size() >= 2)
	{
		return cd(cmd[1]);
	}
}

bool FTPClient::cd(string remote)
{
	int retCode = sendCommand("CWD " + remote);
	if (retCode == -1)
	{
		cout << "send() failed" << endl;
		return false;
	}
	retCode = recvData();
	if (retCode == -1)
	{
		cout << "recv() failed" << endl;
		return false;
	}
	return true;
}

bool FTPClient::pasv()
{
	int retCode = sendCommand("PASV");
	if (retCode == SOCKET_ERROR)
	{
		cout << "Send error " << WSAGetLastError() << endl;
		return false;
	}
	

	retCode = recvData();
	if (retCode < 0)
	{
		cout << "Receive error " << WSAGetLastError() << endl;
		return false;	
	}
	int serverPort = getPort();
	dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (dataSocket == SOCKET_ERROR)
	{
		cout << "Connect failed" << endl;
		return false;
	}
	serverAddress.sin_port = htons(serverPort);
	retCode = connect(dataSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
	if (retCode == SOCKET_ERROR)
	{
		cout << "Connect failed" << endl;
		return false;
	}
	mode = PASSIVE;
	return true;
}

int FTPClient::actv()
{
	SOCKADDR_IN dataClient;
	unsigned int port;
	short port0, port1;
	int length = sizeof(dataClient);
	getsockname(controlSocket, (sockaddr*)&dataClient, &length);
	port = ntohs(dataClient.sin_port);

	port1 = port / 256;
	port0 = port % 256;

	string fullHostIP = hostIP + "." + to_string(port1) + "." + to_string(port0);
	for (int i = 0; i < fullHostIP.length(); i++)
	{
		if (fullHostIP[i] == '.')
			fullHostIP[i] = ',';
	}

	SOCKET listeningSocket = ListeningSocket();
	int retCode = sendCommand("PORT " + fullHostIP);
	if (retCode == SOCKET_ERROR)
	{
		cout << "Send() failed " << WSAGetLastError()<< endl;
		return false;
	}
	
	retCode = recvData();
	if (retCode == -1 || getCode() != 200)
	{
		cout << "Connect active mode failed !!!" << endl;
		return false;
	}
	
	// Chap nhan ket noi moi.
	dataSocket = accept(listeningSocket, NULL, NULL);

	mode = ACTIVE;
	return true;
}

SOCKET FTPClient::ListeningSocket()
{
	SOCKET ListeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN clientAdress;
	memset((void *)&clientAdress, 0, sizeof(clientAdress));
	clientAdress.sin_family = AF_INET;					// Socket kieu IPv4
	clientAdress.sin_addr.s_addr = htonl(INADDR_ANY);	//Su dung bat ky giao dien nao
	clientAdress.sin_port = htons(0);					// Port mac dinh la 21

	// Bind socket cua server.
	int status = bind(ListeningSocket, (SOCKADDR *)&serverAddress, sizeof(serverAddress));
	if (status == SOCKET_ERROR)
		closesocket(ListeningSocket);

	// Chuyen sang trang thai doi ket noi
	// Server cho phep lang nghe toi da 5 ket noi, cac client se duoc dua vao hang doi (queue)
	status = listen(ListeningSocket, 5);
	if (status == SOCKET_ERROR)
		closesocket(ListeningSocket);
	// Sau khi chap nhan ket noi, server co the tiep tuc chap nhan them cac ket noi	khac,
	// hoac gui nhan du lieu voi cac client thong qua cac socket duoc accept voi client
	// Dong socket
	//closesocket(ListeningSocket);

	return ListeningSocket;
}

bool FTPClient::put(vector<string> cmd)
{
	string local, remote;
	if (cmd.size() == 1)
	{
		cout << "Local file: ";
		getline(cin, local);
		cout << "Remote file: ";
		getline(cin, remote);
	
		return put(local, remote);
	}
	else if (cmd.size() == 2)
	{
		cout << "Remote file: ";
		getline(cin, remote);

		return get(cmd[1], remote);
	}
	else if (cmd.size() > 1)
	{
		return put(cmd[1], cmd[2]);
	}

}

bool FTPClient::put(string local, string remote)
{
	//if (mode == PASSIVE)
		this->pasv();

	FILE* fileIn = fopen(local.c_str(), "rb");
	if (!fileIn)
	{
		cout << "File not found" << endl;
		return false;
	}
	sendCommand("STOR " + remote);
	int retCode = recvData();
	if (retCode == -1)
	{
		return false;
	}

	int count;
	while (!feof(fileIn))
	{
		count = fread(sendBuffer, 1, LENGTH, fileIn);
		if (send(dataSocket, sendBuffer, count, 0) == -1) {
			fclose(fileIn);
		}
	}
	send(dataSocket, sendBuffer, 1, 0);
	fclose(fileIn);
	return true;
}

bool FTPClient::mput(vector<string> cmd)
{
	vector<string> files(cmd.size() - 1);
	if (cmd.size() == 1)
	{
		string localFiles;
		cout << "Local files: ";
		getline(cin, localFiles);

		// Tach ten cac file ra thanh mot vector<string>
		files = getCommand(localFiles);
	}
	else if (cmd.size() > 1)
	{
		copy(cmd.begin() + 1, cmd.end(), files.begin());
	}
	for (int i = 0; i < files.size(); i++)
	{
		cout << "Confirm to download \"" + files.at(i) + "\" ?\n";
		char ch = _getch();
		if (ch == 13)
		{
			put(files.at(i), files.at(i));
		}

	}
	return true;
}

bool FTPClient::get(vector<string> cmd)
{
	string remoteFile, localFile;
	if (cmd.size() == 1)
	{
		cout << "Remote file: ";
		getline(cin, remoteFile);
		cout << "Local file: ";
		getline(cin, localFile);

		return get(remoteFile, localFile);
	}
	else if (cmd.size() == 2)
	{
		cout << "Local file: ";
		getline(cin, localFile);

		return get(cmd[1], localFile);
	}
	else if (cmd.size() > 1)
	{
		return get(cmd[1], cmd[2]);
	}
}

bool FTPClient::get(string remote, string local)
{
	//if (mode == PASSIVE)
		this->pasv();

	int retCode = sendCommand("RETR " + remote);
	if (retCode == -1)
	{
		cout << "Send() failed " << WSAGetLastError() << endl;
		return false;
	}

	memset((void *)recvBuffer, 0, LENGTH);
	retCode = recv(dataSocket, recvBuffer, LENGTH, 0);
	if (retCode == -1)
	{
		cout << "Get() failed" << endl;
		return false;
	}

	ofstream openFile;
	openFile.open(local, ios::binary);
	while (retCode > 0)
	{
		openFile.write(recvBuffer, retCode);
		retCode = recv(dataSocket, recvBuffer, LENGTH, 0);
	}
	openFile.close();
	recvData();


	return true;
}

bool FTPClient::mget(vector<string> cmd)
{
	vector<string> files(cmd.size() - 1);
	if (cmd.size() == 1)
	{
		string remoteFiles;
		cout << "Remote files: ";
		getline(cin, remoteFiles);

		// Tach ten cac file ra thanh mot vector<string>
		files = getCommand(remoteFiles);
	}
	else if (cmd.size() > 1)
	{
		copy(cmd.begin() + 1, cmd.end(), files.begin());
	}
	for (int i = 0; i < files.size(); i++)
	{
		cout << "Confirm to download \"" + files.at(i) + "\" ?\n";
		char ch = _getch();
		if (ch == 13)
		{
			get(files.at(i), files.at(i));
		}

	}
	return true;
}

void FTPClient::dele(vector<string> cmd)
{
	if (cmd.size() >= 2)
	{
		dele(cmd[1]);
	}
	else if (cmd.size() == 1)
	{
		string remoteFile;
		cout << "Remote flie ";
		getline(cin, remoteFile);
		dele(remoteFile);
	}
}

void FTPClient::dele(string remoteFile)
{
	sendCommand("DELE " + remoteFile);
	recvData();
}

void FTPClient::mdele(vector<string> cmd)
{
	vector<string> remote(cmd.size() - 1);
	if (cmd.size() == 1)
	{
		string remoteFiles;
		cout << "Remote files: ";
		getline(cin, remoteFiles);

		// Tach ten cac file ra thanh mot vector<string>
		remote = getCommand(remoteFiles);
	}
	else if (cmd.size() > 1)
	{
		copy(cmd.begin() + 1, cmd.end(), remote.begin());
	}
	for (int i = 0; i < remote.size(); i++)
	{
		cout << "Confirm to delete \"" + remote.at(i) + "\" ?\n";
		char ch = _getch();
		if (ch == 13)
		{
			dele(remote.at(i));
		}

	}
}


void FTPClient::rmdir(vector<string> cmd)
{
	if (cmd.size() >= 2)
	{
		rmdir(cmd[1]);
	}
	else if (cmd.size() == 1)
	{
		string folderName;
		cout << "Directory name ";
		getline(cin, folderName);
		rmdir(folderName);
	}
}


void FTPClient::rmdir(string folderName)
{
	sendCommand("XRMD " + folderName);
	recvData();
}

void FTPClient::mkdir(vector<string> cmd)
{
	if (cmd.size() >= 2)
	{
		mkdir(cmd[1]);
	}
	else if (cmd.size() == 1)
	{
		string folderName;
		cout << "Directory name ";
		getline(cin, folderName);
		mkdir(folderName);
	}
}

void FTPClient::mkdir(string folderName)
{
	sendCommand("XMKD " + folderName);
	recvData();
}

void FTPClient::pwd()
{
	sendCommand("pwd");
	recvData();
}


bool FTPClient::lcd(vector<string> cmd)
{
	if (cmd.size() == 1)
	{
		string remote;
		cout << "Remote directory ";
		getline(cin, remote);
		return lcd(remote);
	}
	else if (cmd.size() == 2)
	{
		return lcd(cmd[1]);
	}
}

string FTPClient::getCurrentDir()
{
	string path = "";
	TCHAR buffer[MAX_PATH];
	DWORD dWord;
	dWord = GetCurrentDirectoryW(MAX_PATH, buffer);

	for (int i = 0; i < dWord; i++)
		path += (char)*(buffer + i);
	return path;
}

bool FTPClient::lcd(string remote)
{
	wstring stemp = wstring(remote.begin(), remote.end());
	LPCWSTR sw = stemp.c_str();
	SetCurrentDirectoryW(sw);
	remote = CW2A(sw);

	if (remote == getCurrentDir())
		cout << "Local directory now " << getCurrentDir() << endl;
	else cout << "Local directory has not been changed !" << endl;

	return true;
}


bool FTPClient::quit()
{
	sendCommand("QUIT");
	recvData();
	DisconnectServer();
	return 1;
}


void FTPClient::ExecuteFTPCommand(vector<string> lstCommand)
{
	if (lstCommand[0] == "user")
		user(lstCommand);				// done
	else if (lstCommand[0] == "dir")
		dir();							// done
	else if (lstCommand[0] == "ls")		
		ls();							// done
	else if (lstCommand[0] == "put")
		put(lstCommand);				// done
	else if (lstCommand[0] == "mput")
		mput(lstCommand);				// done
	else if (lstCommand[0] == "get")
		get(lstCommand);				// done
	else if (lstCommand[0] == "mget")
		mget(lstCommand);				// done
	else if (lstCommand[0] == "cd")
		cd(lstCommand);					// done
	else if (lstCommand[0] == "lcd")
		lcd(lstCommand);				// done
	else if (lstCommand[0] == "delete" or lstCommand[0] == "dele")
		dele(lstCommand);				// done
	else if (lstCommand[0] == "mdelete" or lstCommand[0] == "mdele")
		mdele(lstCommand);				// done
	else if (lstCommand[0] == "mkdir")
		mkdir(lstCommand);				// done			
	else if (lstCommand[0] == "rmdir")
		rmdir(lstCommand);				// done
	else if (lstCommand[0] == "pwd")
		pwd();							// done
	else if (lstCommand[0] == "bye" || lstCommand[0] == "quit")
		quit();							// done
	else if (lstCommand[0] == "quote" && lstCommand[1] == "pasv")
		pasv();							// done
	else if (lstCommand[0] == "quote" && lstCommand[1] == "actv")
		actv();
	else
		cout << "Invalid command." << endl;
}