#define _CSREALISE_
#include "ServerClient.h"
#include <algorithm>
using namespace std;

#define poll(type) \
type BoundArray::poll##type(int& pos) {\
	int last = pos + sizeof(type) - 1;\
	inBound(last);\
	type ans = *(type*)(mBuffer + pos);\
	pos = last + 1;\
	return ans;\
}

poll(char)
poll(short)
poll(int)

BoundArray::BoundArray()
{
	mBufferSize = 256;
	mSize = 0;
	mBuffer = new char[mBufferSize];
}

BoundArray::~BoundArray()
{
	delete[] mBuffer;
}

void BoundArray::append(int len, const char* buf)
{
	int newsize = len + mSize;
	if (newsize > mBufferSize)
	{
		char* newbuf = new char[newsize];
		copy(mBuffer, mBuffer + mSize, newbuf);
		delete[] mBuffer;
		mBuffer = newbuf;
		mBufferSize = newsize;
	}

	copy(buf, buf + len, mBuffer + mSize);
	mSize = newsize;
}

void BoundArray::removeHead(int size)
{
	copy(mBuffer + size, mBuffer + mSize, mBuffer);
	mSize -= size;
}

void BoundArray::inBound(int pos)
{
	if (pos >= mSize)
		throw 1;
}