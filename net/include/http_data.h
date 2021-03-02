#pragma once

#include <memory>
#include <pthread.h>
#include <unordered_map>
#include <map>
#include <unistd.h>
#include "timer.h"

class EventLoop;
class TimerNode;
class Channel;

enum ProcessState
{
    STATE_PARSE_URI = 1,
    STATE_PARSE_HEADERS,
    STATE_RECV_BODY,
    STATE_ANALYSIS,
    STATE_FINISH
};

enum URIState
{
    PARSE_URI_AGAIN = 1,
    PARSE_URI_ERROR,
    PARSE_URI_SUCCESS
};

enum HeaderState
{
    PARSE_HEADER_SUCCESS = 1,
    PARSE_HEADER_AGAIN,
    PARSE_HEADER_ERROR
};

enum AnalysisState
{
    ANALYSIS_SUCCESS = 1,
    ANALYSIS_ERROR,
};

enum ParseState
{
    H_START = 0,
    H_KEY,
    H_COLON,
    H_SPACES_AFTER_COLON,
    H_VALUE,
    H_CR,
    H_LF,
    H_END_CR,
    H_END_LF
};

enum ConnectionState
{
    H_CONNECTED = 0,
    H_DISCONNECTING,
    H_DISCONNECTED
};

enum HttpMethod
{
    METHOD_POST = 1,
    METHOD_GET,
    METHOD_HEAD
};

enum HttpVersion
{
    HTTP_10 = 1,
    HTTP_11
};

class MimeType
{
private:
    static void Init();
    static std::unordered_map<std::string, std::string> mime_;
    MimeType();
    MimeType(const MimeType&);

private:
    static pthread_once_t once_control_;

public:
    static std::string GetMime(const std::string &suffix);
};

class HttpData : public std::enable_shared_from_this<HttpData>
{
public:
    HttpData(EventLoop* loop, int conn_fd);
    ~HttpData() { close(fd_); }
    void Reset();
    void SeperateTimer();
    void LinkTimer(std::shared_ptr<TimerNode> timer) { timer_ = timer; }
    std::shared_ptr<Channel> GetChannel() { return channel_; }
    EventLoop* GetLoop() { return loop_; }
    void HandleClose();
    void NewEvent();

private:
    EventLoop *loop_;
    std::shared_ptr<Channel> channel_;
    int fd_;
    std::string inbuffer_;
    std::string outbuffer_;
    bool error_;

    ConnectionState connection_state_;
    HttpMethod method_;
    HttpVersion version_;

    std::string filename_;
    std::string path_;

    int now_read_pos_;
    ProcessState state_;
    ParseState h_state_;
    bool keep_alive_;
    std::map<std::string, std::string> headers_;
    std::weak_ptr<TimerNode> timer_;

    void HandleRead();
    void HandleWrite();
    void HandleConn();
    void HandleError(int fd, int err_num, std::string short_msg);
    URIState ParseURI();
    HeaderState ParseHeaders();
    AnalysisState AnalysisRequest();
};