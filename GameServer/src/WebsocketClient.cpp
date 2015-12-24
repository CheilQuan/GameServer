#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <openssl/evp.h>
#include "WebsocketClient.h"
#include "utils/Log.h"
#include "utils/String.h"
#include "gameLogic/ActivationCode.h"
#include "gameLogic/UIControl.h"
#include "gameLogic/SDKControl.h"

const char* WEBSOCKET_REPLY = "HTTP/1.1 101 Switching Protocols\r\n"
"Upgrade: websocket\r\n"
"Connection: Upgrade\r\n"
"Sec-WebSocket-Accept: %s\r\n\r\n";

WebsocketClient::WebsocketClient() : pHead(nullptr), isWebsocketConnected(false)
{

}

WebsocketClient::~WebsocketClient()
{
	if (pHead)
	{
		delete pHead;
		pHead = nullptr;
	}
}

void WebsocketClient::onRecv()
{
	if (!isWebsocketConnected && !parseWebsocketHandshake())
	{
		return;
	}

	
}

bool WebsocketClient::parseWebsocketHandshake()
{
	char buffer[1024] = {0};
	readBuffer->readObject(buffer, 1024);
	Log::d(buffer);

	std::vector<char*> httpRequest;
	utils::String::split(buffer, "\r\n\r\n", httpRequest);
	// httpRequest[0] is http headers, httpRequest[1] is http body
	std::vector<char*> allLines;
	utils::String::split(httpRequest[0], "\r\n", allLines);

	std::map<std::string, std::string> httpHeaders;
	for (auto line : allLines)
	{
		std::vector<char*> headerTexts;
		utils::String::split(line, ": ", headerTexts);
		if (headerTexts.size() == 2)
		{
			auto key = std::string(headerTexts[0]);
			auto value = std::string(headerTexts[1]);
			httpHeaders.insert(std::make_pair(key, value));
		}
		else
		{
			if (!strstr(line, "HTTP/1.1"))
			{
				isClosing = true;
				return false;
			}
		}
	}
	if (httpHeaders["Upgrade"] != "Websocket" || httpHeaders["Connection"] != "Upgrade")
	{
		isClosing = true;
		return false;
	}
	std::string serverKey = httpHeaders["Sec-WebSocket-Key"];
	if (serverKey.length() == 0)
	{
		isClosing = true;
		return false;
	}
	//RFC6544_MAGIC_KEY
	serverKey += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	unsigned int md_len(0);
	unsigned char md_value[EVP_MAX_MD_SIZE] = {0};

	EVP_MD_CTX* mdctx = EVP_MD_CTX_create();
	EVP_DigestInit_ex(mdctx, EVP_sha1(), NULL);
	EVP_DigestUpdate(mdctx, serverKey.c_str(), serverKey.length());
	EVP_DigestFinal_ex(mdctx, md_value, &md_len);
	EVP_MD_CTX_destroy(mdctx);

	BIO* b64, *bmem;
	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	BIO_push(b64, bmem);
	BIO_write(b64, md_value, md_len);
	BIO_flush(b64);
	char b64result[128] = {0};
	int b64resultLength(0);
	b64resultLength = BIO_read(bmem, b64result, 128);
	BIO_free_all(b64);

	char reply[1024] = {0};
	sprintf(reply, WEBSOCKET_REPLY, b64result);
	send(reply, strlen(reply));
	flush();
	isWebsocketConnected = true;
	return true;
}

WebsocketFrame::WebsocketFrame() :isFinalFragment(true), type(TEXT_FRAME), isMasked(false), maskKey(0), payloadLength(0)
{

}

WebsocketFrame* WebsocketFrame::unpack(ByteArray& input)
{
	WebsocketFrame* frame = new WebsocketFrame;
	/*
	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-------+-+-------------+-------------------------------+
	|F|R|R|R| opcode|M| Payload len |    Extended payload length    |
	|I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
	|N|V|V|V|       |S|             |   (if payload len==126/127)   |
	| |1|2|3|       |K|             |                               |
	+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
	|     Extended payload length continued, if payload len == 127  |
	+ - - - - - - - - - - - - - - - +-------------------------------+
	|                               |Masking-key, if MASK set to 1  |
	+-------------------------------+-------------------------------+
	| Masking-key (continued)       |          Payload Data         |
	+-------------------------------- - - - - - - - - - - - - - - - +
	:                     Payload Data continued ...                :
	+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
	|                     Payload Data continued ...                |
	+---------------------------------------------------------------+
	*/
	// ����3�ֽڣ���Ҫ�����ȴ�
	if (input.available() < 3)
	{
		frame->type = INCOMPLETE_FRAME;
		return frame;
	}

	// ��һ���ֽ�, ���λ����������Ϣ�Ƿ����,
	unsigned char bytedata = input.readUnsignedByte();
	frame->isFinalFragment = (bytedata >> 7) & 0x01;

	//���4λ����������Ϣ����
	unsigned char msg_opcode = bytedata & 0x0F;
	if (msg_opcode == 0x0) frame->type = INCOMPLETE_FRAME;
	else if (msg_opcode == 0x1) frame->type = TEXT_FRAME;
	else if (msg_opcode == 0x2) frame->type = BINARY_FRAME;
	else if (msg_opcode == 0x8) frame->type = CLOSE_FRAME;
	else if (msg_opcode == 0x9) frame->type = PING_FRAME;
	else if (msg_opcode == 0xA) frame->type = PONG_FRAME;
	else frame->type = ERROR_FRAME;

	// ��Ϣ�ĵڶ����ֽڣ����λ��0��1�������Ƿ������봦��
	bytedata = input.readUnsignedByte();
	frame->isMasked = (bytedata >> 7) & 0x01;

	// ��7λ����������Ϣ����
	unsigned char msgLength = bytedata & (~0x80);

	// ʣ�µĺ���7λ����������Ϣ����, ����7λ���ֻ������127�������ֵ������������
	// һ������Ϣ��������126�洢��Ϣ����, �����Ϣ��������UINT16�������ֵΪ126
	// ����Ϣ���ȴ���UINT16������´�ֵΪ127;
	// �������������Ϣ���ȴ洢����������byte[], �ֱ���UINT16(2λbyte)��UINT64(4λbyte)
	if (msgLength <= 125)
	{
		frame->payloadLength = msgLength;
	}
	else if (msgLength == 126)
	{
		frame->payloadLength = input.readUnsignedShort();
	}
	else if (msgLength == 127)
	{
		frame->payloadLength = input.readUnsignedInt64();
	}

	// ����������������»�ȡ4�ֽ�����ֵ
	unsigned int msg_mask(0);
	if (frame->isMasked)
	{
		frame->maskKey = input.readUnsignedInt();
	}
	return frame;
}

WebsocketFrame* WebsocketFrame::pack(ByteArray& output)
{
	WebsocketFrame* frame = new WebsocketFrame;
	return frame;
}
