#ifndef XTCP_H
#define XTCP_H

#include <string>

struct Heartbeat
{
    //1、50 44：牛只盘点协议包头                                          2字节
    unsigned char head[2] = {0x50, 0x44};
    //2、03：协议标识（唤醒上报协议）                                     1字节
    unsigned char command = 0x03;
    //3、0863781152874558：设备号                                           8字节
    unsigned char device[8] = {0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
    //4、0123345576543300：SIM卡号                                     8字节
    unsigned char sim[8] = {0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
    //5、19 12 10 14 14 23 ：设备上报数据时间，日期格式”yy MM dd HH mm ss”,十进制   6字节
    unsigned char date[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    //6、00 00 ：软件版本号                                                 2字节
    unsigned char version[2] = {0x00, 0x00};
    //7、21 99：信号强度和误码率                                             2字节
    unsigned char singal[2] = {0x21, 0x99};
    //8、03 12：上报无线信号强度RSRP为-78.6dbm,上位机添加’-’符号         2字节
    unsigned char PSRP[2] = {0x03, 0x12};
    //9、00 94：上报无线信号信噪比SNR为14.8,该值越大越好                  2字节
    unsigned char SNR[2] = {0x00, 0xff};
    //10、00 01:  上报无线信号覆盖等级 1,该值越小越好,00ff                   2字节
    unsigned char grade[2] = {0x00, 0xff};
    //11、01 02 00 00：小区号,十六进制整数                                      4字节
    unsigned char village[4] = {0x00, 0x00, 0x00, 0x00};
    //12、AA 01 00 01：网关ID                                                4字节
    unsigned char gateway[4] = {0x00, 0x00, 0x00, 0x00};
    //13、0000：复位状态字                                                2字节
    unsigned char reset[2] = {0x00, 0x00};
    //14、 00 00 00 00 00 00 00 00 00 00：预留                     10字节
    unsigned char reserve10[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    //15、05 78：CRC-16校验                                               2字节
    unsigned char CRC[2];
    //16、0d 0a：结束符                                                        2字节
    unsigned char end[2] = {0x0d, 0x0a};
};

struct CowID
{
    //1、50 44：牛只盘点协议包头                                          2字节
    unsigned char head[2] = {0x50, 0x44};
    //2、04：协议标识（数据上报协议）                                     1字节
    unsigned char command = 0x04;
    //3、0863781152874558：设备号                                           8字节
    unsigned char device[8] = {0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
    //4、00 00：备用                                                       2字节
    unsigned char reserve2[2] = {0x02, 0x00};
    //5、19 12 10 14 14 23 ：设备上报数据时间，日期格式”yy MM dd HH mm ss”,十进制                                                                       6字节
    unsigned char date[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    //6、00 00 00 00 00 00 00 00 00 00 00 00：牛号,16进制               12字节
    unsigned char id[12]{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    //7、00 00 00 00 ：备用                                              4字节
    unsigned char reserve4[4] = {0x00, 0x00, 0x00, 0x00};
    //8、21 ：信号强度                                                  1字节
    //9、99：误码率
    unsigned char singal[2] = {0x21, 0x99};
    //10、03 12：上报无线信号强度RSRP为-78.6dbm,上位机添加’-’符号     2字节
    unsigned char PSRP[2] = {0x03, 0x12};
    //11、00 94：上报无线信号信噪比SNR为14.8,该值越大越好                  2字节
    unsigned char SNR[2] = {0x00, 0xff};
    //12、00 01:  上报无线信号覆盖等级 1,该值越小越好,00ff                   2字节
    unsigned char grade[2] = {0x00, 0xff};
    //13、01 02 00 00：小区号,十六进制整数                                      4字节
    unsigned char village[4] = {0x00, 0x00, 0x00, 0x00};
    //14、00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00：预留             16字节
    unsigned char reserve16[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    //15、05 78：CRC-16校验                                               2字节
    unsigned char CRC[2];
    //16、0d 0a：结束符                                                        2字节
    unsigned char end[2] = {0x0d, 0x0a};
};

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
    bool sendHeartbeat(Heartbeat hb);
    bool sendCowID(CowID _cowid);
};

class SendData
{
private:
    std::string m_ip;
    std::string m_url;
    int m_port;
    std::string m_device_id;
    std::string m_gateway_id;
    void string2hex(std::string _s_num, unsigned char *_hex, int _hex_length);
    void time2utc2hex(time_t _time, unsigned char _hex[6]);

public:
    SendData();
    bool setIP(std::string _ip, int _port);
    bool setDeviceID(std::string _id);
    bool setGatewayID(std::string _id);
    ~SendData();
    bool setURL(std::string _url, int _port);
    bool sendHeartbeat();
    bool sendCowIDPen(time_t _time, std::string _id, std::string _pen);
};

#endif