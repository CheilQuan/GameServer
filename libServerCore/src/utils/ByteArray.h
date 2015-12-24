#ifndef __WS_UTILS_BYTEARRAY_H__
#define __WS_UTILS_BYTEARRAY_H__

#include <string>
#include <mutex>
namespace ws
{
	namespace utils
	{
#pragma pack(push)
#pragma pack(1)
		class ByteArray
		{
		public:
			ByteArray(void);
			ByteArray(unsigned int length);
			ByteArray(void* bytes, unsigned int length);
			virtual ~ByteArray(void);

			size_t				position;
			inline size_t		getSize() const { return contentSize; }
			inline size_t		available() const { return contentSize - position; };
			void				truncate();
			void				cutHead(size_t length, char* pOut = nullptr);
			void				cutHead(size_t length, ByteArray* ba);
			void				cutTail(size_t length, char* pOut = nullptr);
			void				cutTail(size_t length, ByteArray* ba);
			inline void			lock() { mtx.lock(); };
			inline void			unlock(){ mtx.unlock(); };
			inline const void*	getBytes(){ return pBytes; };

			char				readByte();
			unsigned char		readUnsignedByte();
			short				readShort();
			unsigned short		readUnsignedShort();
			int					readInt();
			unsigned int		readUnsignedInt();
			unsigned long long	readUnsignedInt64();
			float				readFloat();
			double				readDouble();
			/************************************************************************/
			/* �����ݶ�����һ��ByteArray��                                           */
			/* ba		Ҫ�����Ŀ��ByteArray                                             */
			/* offset	��ba��offsetλ�ÿ�ʼд��                                      */
			/* length	Ҫ��ȡ�����ݴ�С                                              */
			/************************************************************************/
			size_t				readBytes(ByteArray& outBytes, unsigned int offset = 0, size_t length = 0);
			/************************************************************************/
			/* �����ݶ���һ���ڴ����                                                 */
			/* outBuff	Ҫ�������ڴ��                                               */
			/* size		Ҫ��ȡ�����ݴ�С                                             */
			/************************************************************************/
			size_t				readObject(void* outBuff, size_t size = 0);
			std::string			readString(size_t length);
			template <typename T> ByteArray& operator>>(T& val);
			template <typename T> ByteArray& readType(T& val);

			void				writeBoolean(const bool& b);
			void				writeByte(const char& b);
			void				writeUnsignedByte(const unsigned char& b);
			void				writeShort(const short& s);
			void				writeUnsignedShort(const unsigned short& s);
			void				writeInt(const int& i);
			void				writeUnsignedInt(const unsigned int& i);
			void				writeUnsignedInt64(const unsigned long long& ll);
			void				writeFloat(const float& f);
			void				writeDouble(const double& d);
			void				writeBytes(const ByteArray& inBytes, size_t offset = 0, size_t length = 0);
			void				writeObject(const void* inBuff, size_t length);
			template <typename T> ByteArray& operator<<(const T& val);
			template <typename T> ByteArray& writeType(const T& val);

		private:
			static const unsigned int DEFAULT_SIZE = 100;
			static const unsigned int STEP_SIZE = 100;
			void*				pBytes;			//�����ڴ��
			size_t				capacity;		//pBytes�ڴ���С
			size_t				contentSize;	//ʵ�����ݴ�С
			void				resize(size_t typeSize);	//����typeSize���·���pBytes���ڴ�
			std::mutex			mtx;
		};
#pragma pack(pop)
	}
}

#endif	//__BYTEARRAY_H__