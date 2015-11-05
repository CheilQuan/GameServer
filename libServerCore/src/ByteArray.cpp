#include "ByteArray.h"
#include <assert.h>
#include <memory.h>

namespace ws
{
	/************************************************************************/
	/* ��ȡ�ֽ�������                                                        */
	/************************************************************************/

#define READ_TYPE(TypeName, pos, totalSize, ptr)\
{ \
	size_t typeSize = sizeof(TypeName); \
	if (pos + typeSize <= totalSize)\
	{\
		TypeName* pType = (TypeName*)((intptr_t)ptr + pos); \
		pos += typeSize; \
		return *pType; \
	}\
}

	ByteArray::ByteArray(void) :capacity(DEFAULT_SIZE), contentSize(0), position(0)
	{
		pBytes = malloc(DEFAULT_SIZE);
		memset(pBytes, 0, DEFAULT_SIZE);
	}

	ByteArray::ByteArray(unsigned int length) :capacity(length), contentSize(0), position(0)
	{
		if (length > 0)
		{
			pBytes = malloc(length);
			memset(pBytes, 0, length);
		}
	}

	ByteArray::ByteArray(void* bytes, unsigned int length) :capacity(length), contentSize(length), position(0)
	{
		pBytes = malloc(length);
		memcpy(pBytes, bytes, length);
	}

	ByteArray::~ByteArray(void)
	{
		free(pBytes);
		pBytes = NULL;
	}

	bool ByteArray::readBoolean()
	{
		READ_TYPE(bool, position, contentSize, pBytes)
		return false;
	}

	char ByteArray::readByte()
	{
		READ_TYPE(char, position, contentSize, pBytes)
		return 0;
	}

	unsigned char ByteArray::readUnsignedByte()
	{
		READ_TYPE(unsigned char, position, contentSize, pBytes)
		return 0;
	}

	short ByteArray::readShort()
	{
		READ_TYPE(short, position, contentSize, pBytes)
		return 0;
	}

	unsigned short ByteArray::readUnsignedShort()
	{
		READ_TYPE(unsigned short, position, contentSize, pBytes)
		return 0;
	}

	int ByteArray::readInt()
	{
		READ_TYPE(int, position, contentSize, pBytes)
		return 0;
	}

	unsigned int ByteArray::readUnsignedInt()
	{
		READ_TYPE(unsigned int, position, contentSize, pBytes)
		return 0;
	}

	float ByteArray::readFloat()
	{
		READ_TYPE(float, position, contentSize, pBytes)
		return 0.0f;
	}

	double ByteArray::readDouble()
	{
		READ_TYPE(double, position, contentSize, pBytes)
		return 0.0;
	}

	size_t ByteArray::readBytes(ByteArray& outBytes, unsigned int offset /*= 0*/, size_t length /*= 0*/)
	{
		if (length == 0 || length > contentSize - position)	//��ȡ����������
		{
			length = contentSize - position;
		}
		if (length > 0)
		{
			outBytes.resize(length);
			void* pDst = (void*)((intptr_t)outBytes.pBytes + offset);
			void* pSrc = (void*)((intptr_t)pBytes + position);
			memcpy(pDst, pSrc, length);
			position += length;
		}
		return length;
	}

	size_t ByteArray::readObject(void* outBuff, size_t length)
	{
		if (outBuff == NULL)
		{
			return 0;
		}
		if (length == 0 || length > contentSize - position)
		{
			length = contentSize - position;
		}
		if (length > 0)
		{
			memcpy(outBuff, (void*)((intptr_t)pBytes + position), length);
			position += length;
		}
		return length;
	}

	std::string ByteArray::readString(size_t length)
	{
		if (length == 0 || length > contentSize - position)
		{
			length = contentSize - position;
		}
		if (length > 0)
		{
			char* pStr = (char*)((intptr_t)pBytes + position);
			return std::string(pStr, length);
		}
		return std::string();
	}

	/************************************************************************/
	/* д���ֽ�������                                                        */
	/************************************************************************/

	void ByteArray::resize(size_t size)
	{
		while (position + size > capacity)
		{
			pBytes = realloc(pBytes, capacity + STEP_SIZE);
			assert(pBytes != NULL);
			capacity += STEP_SIZE;
		}
	}

	template <typename T>
	void ByteArray::writeType(const T& val)
	{
		resize(sizeof(T));
		memcpy((void*)((intptr_t)pBytes + position), &val, sizeof(T));
		position += sizeof(T);
		if (contentSize < position)
		{
			contentSize = position;
		}
	}

	void ByteArray::writeBoolean(const bool& b)
	{
		writeType(b);
	}

	void ByteArray::writeByte(const char& b)
	{
		writeType(b);
	}

	void ByteArray::writeUnsignedByte(const unsigned char& b)
	{
		writeType(b);
	}

	void ByteArray::writeShort(const short& s)
	{
		writeType(s);
	}

	void ByteArray::writeUnsignedShort(const unsigned short& s)
	{
		writeType(s);
	}

	void ByteArray::writeInt(const int& i)
	{
		writeType(i);
	}

	void ByteArray::writeUnsignedInt(const unsigned int& i)
	{
		writeType(i);
	}

	void ByteArray::writeFloat(const float& f)
	{
		writeType(f);
	}

	void ByteArray::writeDouble(const double& d)
	{
		writeType(d);
	}

	void ByteArray::writeBytes(const ByteArray& inBytes, size_t offset /*= 0*/, size_t length /*= 0*/)
	{
		if (offset >= inBytes.contentSize)
		{
			return;
		}
		if (length == 0 || offset + length > inBytes.contentSize)
		{
			length = inBytes.contentSize - offset;
		}
		if (length > 0)
		{
			resize(length);
			void* pDst = (void*)((intptr_t)pBytes + position);
			void* pSrc = (void*)((intptr_t)inBytes.pBytes + offset);
			memcpy(pDst, pSrc, length);
			position += length;
			if (contentSize < position)
			{
				contentSize = position;
			}
		}
	}

	void ByteArray::writeObject(const void* obj, size_t length)
	{
		if (length > 0)
		{
			resize(length);
			memcpy((void*)((intptr_t)pBytes + position), obj, length);
			position += length;
			if (contentSize < position)
			{
				contentSize = position;
			}
		}
	}

	void ByteArray::truncate()
	{
		free(pBytes);
		pBytes = malloc(DEFAULT_SIZE);
		capacity = DEFAULT_SIZE;
		position = 0;
		contentSize = 0;
	}

	void ByteArray::cutHead(size_t length, char* pOut /*= nullptr*/)
	{
		if (length > contentSize)
		{
			length = contentSize;
		}
		if (length > 0)
		{
			if (pOut)
			{
				memcpy(pOut, pBytes, length);
			}
			if (length < contentSize)
			{
				contentSize -= length;
				memmove(pBytes, (void*)((intptr_t)pBytes + length), contentSize);
			}
			else
			{
				contentSize = 0;
			}
			if (length < position)
			{
				position -= length;
			}
			else
			{
				position = 0;
			}
		}
	}

	void ByteArray::cutHead(size_t length, ByteArray* ba)
	{
		if (length > contentSize)
		{
			length = contentSize;
		}
		if (length > 0)
		{
			if (ba)
			{
				ba->writeBytes(*this, 0, length);
			}
			if (length < contentSize)
			{
				contentSize -= length;
				memmove(pBytes, (void*)((intptr_t)pBytes + length), contentSize);
			}
			else
			{
				contentSize = 0;
			}
			if (length < position)
			{
				position -= length;
			}
			else
			{
				position = 0;
			}
		}
	}

	void ByteArray::cutTail(size_t length, char* pOut /*= nullptr*/)
	{
		if (length > contentSize)
		{
			length = contentSize;
		}
		if (length > 0)
		{
			if (pOut)
			{
				memcpy(pOut, (void*)((intptr_t)pBytes + contentSize - length), length);
			}
			if (length < contentSize)
			{
				contentSize -= length;
			}
			else
			{
				contentSize = 0;
			}
			if (length < position)
			{
				position -= length;
			}
			else
			{
				position = 0;
			}
		}
	}

	void ByteArray::cutTail(size_t length, ByteArray* ba /*= nullptr*/)
	{
		if (length > contentSize)
		{
			length = contentSize;
		}
		if (length > 0)
		{
			if (ba)
			{
				ba->writeBytes(*this, contentSize - length, length);
			}
			if (length < contentSize)
			{
				contentSize -= length;
			}
			else
			{
				contentSize = 0;
			}
			if (length < position)
			{
				position -= length;
			}
			else
			{
				position = 0;
			}
		}
	}
}