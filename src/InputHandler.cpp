#include "stdafx.h"
#include "InputHandler.h"


string getInputHandleCommand(string cmd)
{
	string ftpCommand;
	stringstream ss(cmd);
	ss >> ftpCommand;

	return ftpCommand;
}

vector<string> getCommand(string cmd) {
	vector<string> cmdList;
	stringstream ss(cmd);
	do {
		string word;
		ss >> word;
		cmdList.push_back(toLower(word));
	} while (ss);

	while (cmdList.size() > 0 && cmdList.back() == "")
		cmdList.pop_back();
	return cmdList;
}

string toLower(string str)
{
	int pos = 0;
	while (pos < str.length())
	{
		if (isupper((int)str[pos]))
		{
			str[pos] = str[pos] + 32;
		}
		pos++;
	}
	return str;
}

