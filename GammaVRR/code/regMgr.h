#ifndef _REGMGR_H_
#define _REGMGR_H_
#include <fstream>
#include <sstream>
#include <iostream>
#include "stdint.h"
using namespace std;

class CRegList
{
public:
  CRegList(string regName, unsigned short elementCount);
  virtual ~CRegList(); // 新增：虚析构，保证子类析构顺序
  static bool   FillAllRegisters(fstream &fpConfiguration, bool show_cfg_en);
  unsigned short        mCount;
  void RemoveFromList(); // 新增：从全局链表移除当前对象并置空mNext
protected:
	static string gmDelimiter;  // separator between key and value
	static string gmComment;    // separator between value and comments
  static CRegList *gmpRegList;
  string        mName;
  CRegList      *mNext;
  static  void  TrimContent(string& str);
  virtual bool  FillRegister(string &regName, string &regContent) = 0;
};

template <class T>
class CRegMgr : public CRegList
{
public:
	CRegMgr(string regName);
	CRegMgr(string regName, unsigned short elementCount);
	~CRegMgr();

	// 拷贝构造：深拷贝mArray
	CRegMgr(const CRegMgr& other)
		: CRegList(other),
		mValue(other.mValue)
	{
		if (other.mArray && other.mCount > 0)
		{
			mArray = new T[other.mCount];
			for (unsigned short i = 0; i < other.mCount; ++i)
				mArray[i] = other.mArray[i];
		}
		else
		{
			mArray = nullptr;
		}
	}

	CRegMgr& operator=(const CRegMgr& other)
	{
		if (this != &other)
		{
			if (mArray)
			{
				delete[] mArray;
				mArray = nullptr;
			}
			CRegList::operator=(other);
			mValue = other.mValue;
			if (other.mArray && other.mCount > 0)
			{
				mArray = new T[other.mCount];
				for (unsigned short i = 0; i < other.mCount; ++i)
					mArray[i] = other.mArray[i];
			}
		}
		return *this;
	}

	// 移动构造/赋值保持不变
	CRegMgr(CRegMgr&& other) noexcept
		: CRegList(std::move(other)),
		mValue(other.mValue),
		mArray(other.mArray)
	{
		other.mArray = nullptr;
	}

	CRegMgr& operator=(CRegMgr&& other) noexcept
	{
		if (this != &other)
		{
			if (mArray)
			{
				delete[] mArray;
				mArray = nullptr;
			}
			CRegList::operator=(std::move(other));
			mValue = other.mValue;
			mArray = other.mArray;
			other.mArray = nullptr;
		}
		return *this;
	}

	T             mValue;
	T             *mArray;
protected:
	bool          FillRegister(string &regName, string &regContent);
};

template <class T>
CRegMgr<T>::CRegMgr(string regName)
  : CRegList(regName, 1)
{
  this->mArray = NULL;
}

template <class T>
CRegMgr<T>::CRegMgr(string regName, unsigned short elementCount)
  : CRegList(regName, elementCount)
{
  this->mArray = new T [elementCount];
}

template <class T>
CRegMgr<T>::~CRegMgr()
{
  // 优化：仅非空时释放，释放后置空，避免野指针/重复释放
  if (this->mArray != NULL)
  {
    delete [] this->mArray;
    this->mArray = NULL;
  }
}

template <class T>
bool            CRegMgr<T>::FillRegister(string &regName, string &regContent)
{
unsigned short       i;
stringstream streamContent;
   TrimContent(this->mName);
  if (this->mName.compare(regName) != 0)
  {
    return false;
  }
  streamContent.str(regContent);
  if (this->mCount == 1)
  {
    streamContent >> this->mValue;
  }
  else
  {
    for (i = 0; (i < this->mCount) && (streamContent.eof() == false); i ++)
    {
      streamContent >> this->mArray[i];
    }
  }
  return true;
}

#endif