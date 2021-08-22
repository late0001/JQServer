#pragma once
#include <sys/time.h>
#define CMD_SENDUI 			0xfffe
#define CMD_GETREMOTE		0xfffd
#define CMD_GETREMOTE_ACK 	0xfffc
#define CMD_HANDSHAKE   	0xfffb
#define CMD_HANDSHAKE_ACK   0xfffa
#define CMD_GETALLUSER      0xfff9
#define CMD_GETALLUSER_ACK  0xfff8
#define CMD_HEARTBEAT   	0xfff1 
// 客户端之间发送消息格式
struct stJQMessage
{
	int iMessageType;
	int iMessageLen;
	//unsigned short Port;
};

struct stJQAckMessage
{
	int iMessageType;
	int iMessageLen;
	char info[512];
};

struct HandshakeMessage
{
	char greetings[256];
};

struct HandshakeAckMessage
{
	char info[256];
};

struct SendUIMessage
{
	char userId[10];
	char passwd[7];
};

struct GetRemoteMessage
{
	char userId[10];
	char passwd[7];
};
struct GetRemoteAckMessage
{
	int flag; // if success return true, otherwise return false
	int ip;
	int port;
	char info[256];
};
struct HeartBeatMessage
{
	int iMessageType;
};

struct JQUserNode
{
	time_t now;
	/*********************/
	/*The Client IP address and PORT : TCP*/
	char client_ip_addr[20];
	int client_port;
	char UsrhashId[10];
	char passwd[7];
};
struct GetAllUserAckMessage
{
	int cnt;
	struct JQUserNode node[1];
};