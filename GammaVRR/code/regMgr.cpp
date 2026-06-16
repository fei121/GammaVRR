#include <iostream>
#include <string>
#include "regMgr.h"

// initial globle member variables
CRegList        *CRegList::gmpRegList = NULL;
string          CRegList::gmDelimiter = "=";
string          CRegList::gmComment = "//";

// 原构造函数，保留无需修改
CRegList::CRegList(string regName, unsigned short elementCount)
{
	this->mName = regName;
	this->mCount = elementCount;
	this->mNext = gmpRegList;
	this->gmpRegList = this;
}

// 新增：CRegList虚析构函数实现，析构时自动从链表移除
CRegList::~CRegList()
{
	this->RemoveFromList();
}

// 新增：从全局链表移除当前对象，置空mNext，解决野指针
void CRegList::RemoveFromList()
{
	if (gmpRegList == NULL || this == NULL)
		return;

	CRegList *pRegPrevious = NULL;
	CRegList *pRegCurrent = gmpRegList;

	// 遍历链表找到当前对象
	while (pRegCurrent != NULL && pRegCurrent != this)
	{
		pRegPrevious = pRegCurrent;
		pRegCurrent = pRegCurrent->mNext;
	}

	// 找到则从链表中移除，并置空当前对象的mNext
	if (pRegCurrent == this)
	{
		if (pRegPrevious != NULL)
		{
			pRegPrevious->mNext = this->mNext;
		}
		else
		{
			gmpRegList = this->mNext; // 当前为链表头，更新头指针
		}
		this->mNext = NULL; // 关键：置空mNext，彻底解决野指针
	}
}

// 原FillAllRegisters函数，仅修改**链表删除逻辑**，其余保留
bool CRegList::FillAllRegisters(fstream &fpConfiguration, bool show_cfg_en)
{
	if (show_cfg_en) {
		cout << "\n===============show cfg start===============\n";
	}
	CRegList *pRegPrevious, *pRegCurrent, *pRegNext;
	unsigned short   regCount, delimiterCount, remainCount;
	regCount = 0;
	delimiterCount = 0;
	for (pRegCurrent = (CRegList *)gmpRegList;
		pRegCurrent != NULL;
		pRegCurrent = pRegCurrent->mNext, regCount++);
	string   lineBufferCurrent, lineBufferNext;
	size_t   posDelimiter;
	string   keyWord, contentWord;
	unsigned char    lengthDelimiter = CRegList::gmDelimiter.length();
	lineBufferNext = "";
	while (!fpConfiguration.eof())
	{
		if (lineBufferNext.length() > 0)
		{
			lineBufferCurrent = lineBufferNext;
			lineBufferNext = "";
		}
		else
		{
			getline(fpConfiguration, lineBufferCurrent);
		}
		TrimContent(lineBufferCurrent);
		posDelimiter = lineBufferCurrent.find(CRegList::gmDelimiter);
		if (posDelimiter != string::npos)
		{
			keyWord = lineBufferCurrent.substr(0, posDelimiter);
			contentWord = lineBufferCurrent.substr(posDelimiter + lengthDelimiter, lineBufferCurrent.find("\n"));
			delimiterCount++;
			while (!fpConfiguration.eof())
			{
				getline(fpConfiguration, lineBufferNext);
				if (lineBufferNext.find(CRegList::gmDelimiter) != string::npos)
					break;
				TrimContent(lineBufferNext);
				if (lineBufferNext != "")
				{
					contentWord += "\n";
				}
				contentWord += lineBufferNext;
			}
			TrimContent(keyWord);
			TrimContent(contentWord);
			pRegPrevious = NULL;
			pRegCurrent = gmpRegList;
			while (pRegCurrent != NULL)
			{
				pRegNext = pRegCurrent->mNext;
				if (pRegCurrent->FillRegister(keyWord, contentWord))
				{
					if (show_cfg_en) {
						cout << keyWord << " " << contentWord << std::endl;
					}
					// ------------------- 核心修改 -------------------
					// 替换原手动修改链表指针的逻辑，调用RemoveFromList自动移除+置空
					pRegCurrent->RemoveFromList();
					// 原代码注释掉，无需再使用
					/*
					if (pRegPrevious != NULL)
					{
						pRegPrevious->mNext = pRegNext;
					}
					else
					{
						gmpRegList = pRegNext;
					}
					*/
					// ------------------------------------------------
					break;
				}
				pRegPrevious = pRegCurrent;
				pRegCurrent = pRegNext;
			}
			if (NULL == pRegCurrent)
			{
				cout << "define but no use: " << keyWord << endl;
			}
		}
	}
	if (show_cfg_en) {
		cout << "===============show cfg end===============\n\n";
	}
	remainCount = 0;
	for (pRegCurrent = (CRegList *)gmpRegList;
		pRegCurrent != NULL;
		pRegCurrent = pRegCurrent->mNext, remainCount++)
	{
		cout << "not configured register: " << pRegCurrent->mName << endl;
	}
	cout << "*********** Register Manager Report ***********\n**\n";
	cout << "**    Declared in simulation program : " << regCount << "\n";
	cout << "**    Configured in setting file     : " << delimiterCount << "\n";
	cout << "**    Declared but not configured    : " << remainCount << "\n";
	cout << "**    Redundant configurations       : " << (delimiterCount - regCount + remainCount) << "\n";
	cout << "**\n***********************************************\n\n";
	return true;
}

// 原TrimContent函数，保留无需修改
void CRegList::TrimContent(string& str)
{
	static const char whitespace[] = " \n\t\v\r\f";
	str = str.substr(0, str.find(CRegList::gmComment));
	str.erase(0, str.find_first_not_of(whitespace));
	str.erase(str.find_last_not_of(whitespace) + 1);
}