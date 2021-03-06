//
// Created by wonder on 2020/10/22.
//

#ifndef HttpWEBSERVER_HttpCONN_H
#define HttpWEBSERVER_HttpCONN_H


#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cstdio>


class HttpConn {
public:
    //文件名的最大长度
    static const int FILENAME_LEN = 1024;
    //读缓冲区的大小
    static const int READ_BUFFER_SIZE = 2048;
    //写缓冲区的大小
    static const int WRITE_BUFFER_SIZE = 1024;
    //Http的请求方法
    enum METHOD {
            GET,POST,HEAD,PUT,DELETE,TRACE,OPTIONS,CONNECT,PATCH
    };
    //主状态机所处的状态
    enum CHECK_STATE{
        CHECK_STATE_REQUESTLINE,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    //服务器处理Http请求的可能结果
    enum HTTP_CODE {
        NO_REQUEST,GET_REQUEST,BAD_REQUEST,
        NO_RESOURCE,FORBIDDEN_REQUEST,FILE_REQUEST,
        INTERNAL_ERROR,CLOSED_CONNECTION
    };
    //行读取状态
    enum LINE_STATE{
        LINE_OK,LINE_BAD,LINE_OPEN
    };

public:
    HttpConn() = default;
    ~HttpConn() = default;
    //初始化新接受的连接
    void init(int sockfd,const sockaddr_in & addr);
    //关闭连接
    void close_conn(bool real_close = true);
    //处理客户请求
    void process();
    //非阻塞读操作
    bool read();
    //非阻塞写操作
    bool write();

private:
    //初始化连接
    void init();
    //解析Http请求
    HTTP_CODE process_read();
    //填充Http应答
    bool process_write(HTTP_CODE ret);

    /*被process_read调用以解析Http请求*/
    HTTP_CODE parse_request_line(char * text);
    HTTP_CODE parse_headers(char * text);
    HTTP_CODE parse_content(char * text);
    HTTP_CODE do_request();
    char * get_line(){
        return m_read_buf + m_start_line;
    }
    LINE_STATE parse_line();

    /*被process_write调用以填充Http请求*/
    void unmap();
    bool add_response(const char * format,...);
    bool add_content(const char * content);
    bool add_status_line(int status,const char * title);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();
public:
    //将所有的socket上的事件都注册到同一个epoll内核事件表上，所以设置为静态
    static int m_epollfd;
    //统计用户数量
    static int m_user_count;
private:
    //Http连接的socket
    int m_sockfd;
    //对方的socket地址
    sockaddr_in m_address;
    //读缓冲区
    char m_read_buf[READ_BUFFER_SIZE];
    //读缓冲区中已经读入的客户数据的最后一个字节的下一个位置
    int m_read_idx;
    //当前正在解析的字符在读缓冲区中的位置
    int m_checked_idx;
    //当前正在解析的行的起始位置
    int m_start_line;
    //写缓冲区
    char m_write_buf[WRITE_BUFFER_SIZE];
    //写缓冲区中待发送的字节数
    int m_write_idx;
    //保存post请求最后的输入
    char * m_string;
    //主状态机当前所处的状态
    CHECK_STATE m_check_state;
    //请求方法
    METHOD m_method;

    //客户端请求的目标文件的完整路径
    char m_real_file[FILENAME_LEN];
    //客户端请求的目标文件的文件名
    char * m_url;
    //Http版本协议号
    char * m_version;
    //主机名
    char * m_host;
    //Http请求的消息体长度
    int m_content_length;
    //Http请求是否要求保持连接
    bool m_linger;

    //客户端请求的目标文件被mmap到内存的起始位置
    char * m_file_address;
    //目标文件的状态
    struct stat m_file_stat;
    //使用writev执行写操作所需的结构
    struct iovec m_iv[2];
    int m_iv_count;

};


#endif //HttpWEBSERVER_HttpCONN_H
