#pragma once

#include <memory>
#include <pthread.h>
#include <unordered_map>
#include <map>


class EventLoop;
class TimerNode;
class Channel;

enum ProcessState
{
    STATE_PRASE_URI = 1,
    STATE_PRASE_HEADERS,
    STATE_RECV_BODY,
    STATE_ANALYSIS,
    STATE_FINSH
};

enum URIState
{
    PRASE_URI_AGAIN = 1,
    PRASE_URI_ERROR,
    PRASE_URI_SUCCESS
};

enum HeaderState
{
    PARSE_HEADER_SUCCESS = 1,
    PARSE_HEADER_AGAIN,
    PARSE_HEADER_ERROR
};

enum AnalysisState
{
    AYALYSIS_SUCCESS = 1,
    ASALYSIS_ERROE,
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
    H_CONNNECTED = 0,
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
    static void init();
    static std::unordered_map<std::string, std::string> mime;
    MimeType();
    MimeType(const MimeType&);

private:
    static pthread_once_t once_control;

public:
    static std::string getMime(const std::string &suffix);
};

class HttpData : public std::enable_shared_from_this<HttpData>
{

private:
    EventLoop *loop_;
    std::shared_ptr<Channel> channel_;
    int fd_;
    std::string inBuffer_;
    std::string outBuffer_;
    bool error_;

    ConnectionState connection_state_;
    HttpMethod method_;
    HttpVersion version_;

    std::string filename_;
    std::string path_;

    int nowReadPos_;
    ProcessState state_;
    ParseState hState_;
    bool keep_avail_;
    std::map<std::string, std::string> headers_;
    std::weak_ptr<TimerNode> timer_;

    void handleRead();
    void handleWrite();
    void handleConn();
    void handleError(int fd, int err_num, std::string short_msg);
    URIState parseURI();
    HeaderState parseHeaders();
    AnalysisState analysisRequest();
};