#ifndef XTCP_H
#define XTCP_H

#include <string>
class NetUDP
{
public:
    int CreateSocket();
    bool Bind(unsigned short port);
    NetUDP Accept();
    void Close();
    int Recv(char *buf, int bufsize);
    int Send(const char *buf, int sendsize);
    bool Connect(const char *ip, unsigned short port, unsigned int timeoutms = 1000);
    bool SetBlock(bool isblock);
    NetUDP();
    // virtual ~NetUDP();

    unsigned short port = 0; // 用来建立连接的端口
    int sock = 0;            // 用来通信的socket
    char ip[16];

    void getCurrentTime(unsigned char utc[6]);
    bool sendHeartbeat();
    bool sendCowID(std::string id);
    bool sendCowIDPen(std::string id, std::string pen);
};

#endif