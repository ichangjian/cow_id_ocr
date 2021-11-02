#include "net.hpp"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#ifdef WIN32
// 兼容Linux
#include <Windows.h>
#define socklen_t int
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#define closesocket close
#endif

#include <thread>
#define uchar unsigned char

struct Heartbeat
{
    //1、50 44：牛只盘点协议包头                                          2字节
    uchar head[2] = {0x50, 0x44};
    //2、03：协议标识（唤醒上报协议）                                     1字节
    uchar command = 0x03;
    //3、0863781152874558：设备号                                           8字节
    uchar device[8] = {0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
    //4、0123345576543300：SIM卡号                                     8字节
    uchar sim[8] = {0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
    //5、19 12 10 14 14 23 ：设备上报数据时间，日期格式”yy MM dd HH mm ss”,十进制   6字节
    uchar date[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    //6、00 00 ：软件版本号                                                 2字节
    uchar version[2] = {0x00, 0x00};
    //7、21 99：信号强度和误码率                                             2字节
    uchar singal[2] = {0x21, 0x99};
    //8、03 12：上报无线信号强度RSRP为-78.6dbm,上位机添加’-’符号         2字节
    uchar PSRP[2] = {0x03, 0x12};
    //9、00 94：上报无线信号信噪比SNR为14.8,该值越大越好                  2字节
    uchar SNR[2] = {0x00, 0xff};
    //10、00 01:  上报无线信号覆盖等级 1,该值越小越好,00ff                   2字节
    uchar grade[2] = {0x00, 0xff};
    //11、01 02 00 00：小区号,十六进制整数                                      4字节
    uchar village[4] = {0x00, 0x00, 0x00, 0x00};
    //12、AA 01 00 01：网关ID                                                4字节
    uchar gateway[4] = {0x00, 0x00, 0x00, 0x00};
    //13、0000：复位状态字                                                2字节
    uchar reset[2] = {0x00, 0x00};
    //14、 00 00 00 00 00 00 00 00 00 00：预留                     10字节
    uchar reserve10[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    //15、05 78：CRC-16校验                                               2字节
    uchar CRC[2];
    //16、0d 0a：结束符                                                        2字节
    uchar end[2] = {0x0d, 0x0a};
};

struct CowID
{
    //1、50 44：牛只盘点协议包头                                          2字节
    uchar head[2] = {0x50, 0x44};
    //2、04：协议标识（数据上报协议）                                     1字节
    uchar command = 0x04;
    //3、0863781152874558：设备号                                           8字节
    uchar device[8] = {0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
    //4、00 00：备用                                                       2字节
    uchar reserve2[2] = {0x02, 0x00};
    //5、19 12 10 14 14 23 ：设备上报数据时间，日期格式”yy MM dd HH mm ss”,十进制                                                                       6字节
    uchar date[6]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    //6、00 00 00 00 00 00 00 00 00 00 00 00：牛号,16进制               12字节
    uchar id[12]{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    //7、00 00 00 00 ：备用                                              4字节
    uchar reserve4[4] = {0x00, 0x00, 0x00, 0x00};
    //8、21 ：信号强度                                                  1字节
    //9、99：误码率
    uchar singal[2] = {0x21, 0x99};
    //10、03 12：上报无线信号强度RSRP为-78.6dbm,上位机添加’-’符号     2字节
    uchar PSRP[2] = {0x03, 0x12};
    //11、00 94：上报无线信号信噪比SNR为14.8,该值越大越好                  2字节
    uchar SNR[2] = {0x00, 0xff};
    //12、00 01:  上报无线信号覆盖等级 1,该值越小越好,00ff                   2字节
    uchar grade[2] = {0x00, 0xff};
    //13、01 02 00 00：小区号,十六进制整数                                      4字节
    uchar village[4] = {0x00, 0x00, 0x00, 0x00};
    //14、00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00：预留             16字节
    uchar reserve16[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    //15、05 78：CRC-16校验                                               2字节
    uchar CRC[2];
    //16、0d 0a：结束符                                                        2字节
    uchar end[2] = {0x0d, 0x0a};
};

struct reply
{
    //1、50 44：牛只盘点协议包头                                          2字节
    uchar head[2] = {0x00, 0x00};
    //2、05：协议标识（云平台应答协议）                                     1字节
    uchar command = 0x00;
    //3、4F 4B：应答参数，详见参数列表                                    2字节
    uchar answer[2] = {0x00, 0x00};
    //4、19 12 10 14 14 23 ：服务器时间，日期格式”yy MM dd HH mm ss”,十进制                                                                       6字节
    uchar date[6] = {0x00, 0x00};
    //5、00：同一牛号上报频次，单位为小时（十进制）                       1字节
    uchar frequency = 0x00;
    //6、00 00 00 00 00 00 00 00 00 00 00 00 00 00 00：预留             15字节
    uchar reserve15[15] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    //7、05 78：CRC-16校验                                               2字节
    uchar CRC[2] = {0x00, 0x00};
    //8、0d 0a：结束符                                                        2字节
    uchar end[2] = {0x00, 0x00};
};

//crc校验
const unsigned char CRC_TABLE_H[256] = //High
    {
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40};

const unsigned char CRC_TABLE_L[256] = //Low
    {
        0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
        0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
        0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
        0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
        0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
        0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
        0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
        0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
        0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
        0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
        0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
        0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
        0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
        0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
        0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
        0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
        0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
        0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
        0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
        0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
        0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
        0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
        0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
        0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
        0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
        0x43, 0x83, 0x41, 0x81, 0x80, 0x40};

uint do_CRC(unsigned char *p, unsigned char n)
{
    uchar uchCRCHi = 0xFF; /* ¸ßCRC×Ö½Ú³õÊ¼»¯ */
    uchar uchCRCLo = 0xFF; /* µÍCRC ×Ö½Ú³õÊ¼»¯ */
    ulong uIndex;          /* CRCÑ­»·ÖÐµÄË÷Òý */
    while (n--)            /* ´«ÊäÏûÏ¢»º³åÇø */
    {
        uIndex = uchCRCHi ^ *p++; /* ¼ÆËãCRC */
        uchCRCHi = uchCRCLo ^ CRC_TABLE_H[uIndex];
        uchCRCLo = CRC_TABLE_L[uIndex];
    }
    return (uchCRCHi << 8 | uchCRCLo);
}
// 计算校验
void RamAdjust(uchar *add, uint adjlength)
{
    uint ii;
    ii = do_CRC(add, adjlength - 2);
    *(add + adjlength - 1) = (uchar)ii;
    *(add + adjlength - 2) = (uchar)(ii / 0x100);
}
// 检查校验
uchar TestAdjust(uchar *add, uchar adjlength)
{
    uint ii;
    ii = (unsigned int)*(add + adjlength - 1) + (unsigned int)*(add + adjlength - 2) * 0x100;
    if (ii == do_CRC(add, adjlength - 2))
        return 1;
    else
        return 0;
}

NetUDP::NetUDP()
{
// 初始化库，如果不初始化的话会直接导致后面的socket函数无法使用，但是在初始化前
// 要加载Windows的网络库，就是在项目属性那里加ws2_32.lib
#ifdef WIN32
    static bool first = true;
    if (first)
    {
        WSADATA ws;
        WSAStartup(MAKEWORD(2, 2), &ws);
        first = false;
    }
#endif
}
bool NetUDP::Bind(unsigned short port)
{
    if (sock <= 0)
    {
        CreateSocket();
    }
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);     // host to network，本地字节序转换成网络字节序
    saddr.sin_addr.s_addr = htons(0); // 绑定ip地址，0的话其实可以不转。这里是任意的ip发过来的数据都接受的意思。至于为什么0就是监听任意端口，建议看看计算机网络
                                      // 一个int是4个char，所以可以通过int来表示ip地址

    // bind端口，很容易失败，一定要有判断
    if (::bind(sock, (sockaddr *)&saddr, sizeof(saddr)) != 0)
    { // :: 表示用的是全局的函数
        printf("bind port %d failed!", port);
        return false;
    }
    printf("bind port %d succeeded.", port);
    listen(sock, 10); // 监听指定的端口，只用来创建链接
    return true;
}
int NetUDP::CreateSocket()
{
    // 使用TCP/IP协议，所以AF_INET，TCP，所以是SOCK_STREAM
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    // 创建socket失败，例如Linux中因为超出了每个进程分配的文件具体数量而被拒绝创建
    if (sock == -1)
    {
        printf("Create socket failed!\n");
    }
    return sock;
}
bool NetUDP::Connect(const char *ip, unsigned short port, unsigned int timeoutms)
{
    if (sock <= 0)
    {
        CreateSocket();
    }
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(ip);

    SetBlock(false);
    fd_set set; // 文件描述符的数组
    if (connect(sock, (sockaddr *)&saddr, sizeof(saddr)) != 0)
    {
        FD_ZERO(&set); // 每次判断前必须要清空
        FD_SET(sock, &set);
        timeval tm;
        tm.tv_sec = 0;
        tm.tv_usec = timeoutms * 1000;
        if (select(sock + 1, 0, &set, 0, &tm) <= 0)
        {
            // 只要有一个可写，就会返回文件描述符的值，否则返回-1，超时返回0
            printf("connect timeout or error!\n");
            printf("connect %s:%d failed!: %s\n", ip, port, strerror(errno));
            return false;
        }
    }
    SetBlock(true);
    printf("connect %s:%d succeded!\n", ip, port);
    return true;
}

bool NetUDP::SetBlock(bool isblock)
{
    if (sock <= 0)
    {
        return false;
    }
#ifdef WIN32
    unsigned long ul = 0;
    if (!isblock)
    {
        ul = 1;
    }
    ioctlsocket(sock, FIONBIO, &ul);
    // 下面是Linux中的设置阻塞方式的代码
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0)
    {
        return false;
    }
    if (isblock)
    {
        flags = flags & ~O_NONBLOCK;
    }
    else
    {
        flags = flags | O_NONBLOCK; // 非阻塞模式
    }
    if (fcntl(sock, F_SETFL, flags) != 0)
    {
        return false; // 如果不等于0，那么设定失败
    }
#endif
    return true;
}
NetUDP NetUDP::Accept()
{
    NetUDP tcp;
    sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    int client = accept(sock, (sockaddr *)&caddr, &len); // 读取用户连接信息，会创建新的socket，用来单独和这个客户端通信，后面两个
                                                         // 参数要传指针，用来返回端口号和地址
    if (client <= 0)
    {
        return tcp;
    }
    printf("accept client %d\n", client);
    char *ip = inet_ntoa(caddr.sin_addr);
    strcpy(tcp.ip, ip);
    tcp.port = ntohs(caddr.sin_port); // short，恰好最大65535
    tcp.sock = client;
    printf("client ip is %s, port is %d \n", tcp.ip, tcp.port);
    return tcp;
}
int NetUDP::Send(const char *buf, int size)
{
    int s = 0;
    while (s != size)
    {
        int len = send(sock, buf + s, size - s, 0);
        if (len <= 0)
        {
            break;
        }
        s += len;
    }
    return s;
}
int NetUDP::Recv(char *buf, int bufsize)
{
    return recv(sock, buf, bufsize, 0);
}
void NetUDP::Close()
{
    if (sock <= 0)
        return;
    closesocket(sock);
}

void NetUDP::getCurrentTime(uchar utc[6])
{

    time_t now = time(0);
    now -= 28800; // 8h*60min*60s
    tm *ltm = localtime(&now);
    int year = (ltm->tm_year % 100);
    ltm->tm_mon += 1;

    utc[0] = (year / 10) * 16 + year % 10;
    utc[1] = (ltm->tm_mon / 10) * 16 + ltm->tm_mon % 10;
    utc[2] = (ltm->tm_mday / 10) * 16 + ltm->tm_mday % 10;
    utc[3] = (ltm->tm_hour / 10) * 16 + ltm->tm_hour % 10;
    utc[4] = (ltm->tm_min / 10) * 16 + ltm->tm_min % 10;
    utc[5] = (ltm->tm_sec / 10) * 16 + ltm->tm_sec % 10;
}

bool NetUDP::sendHeartbeat()
{
    Heartbeat hb;

    getCurrentTime(hb.date);
    RamAdjust((uchar *)(&hb), sizeof(hb) - 2);
    std::cout << std::hex << int(hb.CRC[0]) << int(hb.CRC[1]) << "\n";
    for (size_t i = 0; i < sizeof(hb); i++)
    {
        std::cout << std::hex << int(((char *)(&hb))[i]) << " ";
    }
    std::cout << "\n";

    if (Send((char *)(&hb), sizeof(hb)) != sizeof(hb))
        return false;
    reply rp;
    Recv((char *)(&rp), sizeof(rp));
    for (size_t i = 0; i < sizeof(rp); i++)
    {
        std::cout << std::hex << int(((char *)(&rp))[i]) << " ";
    }
    return true;
}

bool NetUDP::sendCowID(std::string id)
{
    CowID cowid;
    getCurrentTime(cowid.date);
    for (size_t i = 0; i < sizeof(cowid.date); i++)
    {
        char* ind=(char *)(cowid.date);
        std::cout << std::hex << int(ind[i]) << " ";
    }
    std::cout << "\n";
    std::vector<int> id_hex;

    for (int i = id.length() - 1; i >= 0; i = i - 2)
    {
        int num = 0;
        if (i == 0)
        {
            num = id[i] - 48;
        }
        else
        {
            num = (id[i - 1] - 48) * 16;
            num += id[i] - 48;
        }
        id_hex.push_back(num);
    }
    for (size_t i = 0; i < id_hex.size(); i++)
    {
        cowid.id[sizeof(cowid.id) - i - 1] = id_hex[i];
    }

    for (size_t i = 0; i < sizeof(cowid.id); i++)
    {
        std::cout << std::hex << int(((char *)(cowid.id))[i]) << " ";
    }
    std::cout << "\n";

    RamAdjust((uchar *)(&cowid), sizeof(cowid) - 2);
    std::cout << std::hex << int(cowid.CRC[0]) << int(cowid.CRC[1]) << "\n";
    for (size_t i = 0; i < sizeof(cowid); i++)
    {
        std::cout << std::hex << int(((char *)(&cowid))[i]) << " ";
    }
    std::cout << "\n";

    if (Send((char *)(&cowid), sizeof(cowid)) != sizeof(cowid))
        return false;
    reply rp;
    Recv((char *)(&rp), sizeof(rp));
    for (size_t i = 0; i < sizeof(rp); i++)
    {
        std::cout << std::hex << int(((char *)(&rp))[i]) << " ";
    }
    std::cout << "\n";
    return true;
}