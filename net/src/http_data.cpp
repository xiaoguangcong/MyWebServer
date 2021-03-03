
#include <pthread.h>
#include <cstddef>
#include <memory>
#include <sys/types.h>
#include <unordered_map>
#include <sys/epoll.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "http_data.h"
#include "channel.h"
#include "util.h"
#include "logging.h"
#include "eventloop.h"

pthread_once_t MimeType::once_control_ = PTHREAD_ONCE_INIT;
std::unordered_map<std::string, std::string> MimeType::mime_;

const __uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;
const int DEFAULT_EXPIRED_TIME = 2000;             // ms
const int DEFAULT_KEEP_ALIVE_TIME = 5 * 60 * 1000; // ms

char favicon[555] = {
    '\x89', 'P',    'N',    'G',    '\xD',  '\xA',  '\x1A', '\xA',  '\x0',
    '\x0',  '\x0',  '\xD',  'I',    'H',    'D',    'R',    '\x0',  '\x0',
    '\x0',  '\x10', '\x0',  '\x0',  '\x0',  '\x10', '\x8',  '\x6',  '\x0',
    '\x0',  '\x0',  '\x1F', '\xF3', '\xFF', 'a',    '\x0',  '\x0',  '\x0',
    '\x19', 't',    'E',    'X',    't',    'S',    'o',    'f',    't',
    'w',    'a',    'r',    'e',    '\x0',  'A',    'd',    'o',    'b',
    'e',    '\x20', 'I',    'm',    'a',    'g',    'e',    'R',    'e',
    'a',    'd',    'y',    'q',    '\xC9', 'e',    '\x3C', '\x0',  '\x0',
    '\x1',  '\xCD', 'I',    'D',    'A',    'T',    'x',    '\xDA', '\x94',
    '\x93', '9',    'H',    '\x3',  'A',    '\x14', '\x86', '\xFF', '\x5D',
    'b',    '\xA7', '\x4',  'R',    '\xC4', 'm',    '\x22', '\x1E', '\xA0',
    'F',    '\x24', '\x8',  '\x16', '\x16', 'v',    '\xA',  '6',    '\xBA',
    'J',    '\x9A', '\x80', '\x8',  'A',    '\xB4', 'q',    '\x85', 'X',
    '\x89', 'G',    '\xB0', 'I',    '\xA9', 'Q',    '\x24', '\xCD', '\xA6',
    '\x8',  '\xA4', 'H',    'c',    '\x91', 'B',    '\xB',  '\xAF', 'V',
    '\xC1', 'F',    '\xB4', '\x15', '\xCF', '\x22', 'X',    '\x98', '\xB',
    'T',    'H',    '\x8A', 'd',    '\x93', '\x8D', '\xFB', 'F',    'g',
    '\xC9', '\x1A', '\x14', '\x7D', '\xF0', 'f',    'v',    'f',    '\xDF',
    '\x7C', '\xEF', '\xE7', 'g',    'F',    '\xA8', '\xD5', 'j',    'H',
    '\x24', '\x12', '\x2A', '\x0',  '\x5',  '\xBF', 'G',    '\xD4', '\xEF',
    '\xF7', '\x2F', '6',    '\xEC', '\x12', '\x20', '\x1E', '\x8F', '\xD7',
    '\xAA', '\xD5', '\xEA', '\xAF', 'I',    '5',    'F',    '\xAA', 'T',
    '\x5F', '\x9F', '\x22', 'A',    '\x2A', '\x95', '\xA',  '\x83', '\xE5',
    'r',    '9',    'd',    '\xB3', 'Y',    '\x96', '\x99', 'L',    '\x6',
    '\xE9', 't',    '\x9A', '\x25', '\x85', '\x2C', '\xCB', 'T',    '\xA7',
    '\xC4', 'b',    '1',    '\xB5', '\x5E', '\x0',  '\x3',  'h',    '\x9A',
    '\xC6', '\x16', '\x82', '\x20', 'X',    'R',    '\x14', 'E',    '6',
    'S',    '\x94', '\xCB', 'e',    'x',    '\xBD', '\x5E', '\xAA', 'U',
    'T',    '\x23', 'L',    '\xC0', '\xE0', '\xE2', '\xC1', '\x8F', '\x0',
    '\x9E', '\xBC', '\x9',  'A',    '\x7C', '\x3E', '\x1F', '\x83', 'D',
    '\x22', '\x11', '\xD5', 'T',    '\x40', '\x3F', '8',    '\x80', 'w',
    '\xE5', '3',    '\x7',  '\xB8', '\x5C', '\x2E', 'H',    '\x92', '\x4',
    '\x87', '\xC3', '\x81', '\x40', '\x20', '\x40', 'g',    '\x98', '\xE9',
    '6',    '\x1A', '\xA6', 'g',    '\x15', '\x4',  '\xE3', '\xD7', '\xC8',
    '\xBD', '\x15', '\xE1', 'i',    '\xB7', 'C',    '\xAB', '\xEA', 'x',
    '\x2F', 'j',    'X',    '\x92', '\xBB', '\x18', '\x20', '\x9F', '\xCF',
    '3',    '\xC3', '\xB8', '\xE9', 'N',    '\xA7', '\xD3', 'l',    'J',
    '\x0',  'i',    '6',    '\x7C', '\x8E', '\xE1', '\xFE', 'V',    '\x84',
    '\xE7', '\x3C', '\x9F', 'r',    '\x2B', '\x3A', 'B',    '\x7B', '7',
    'f',    'w',    '\xAE', '\x8E', '\xE',  '\xF3', '\xBD', 'R',    '\xA9',
    'd',    '\x2',  'B',    '\xAF', '\x85', '2',    'f',    'F',    '\xBA',
    '\xC',  '\xD9', '\x9F', '\x1D', '\x9A', 'l',    '\x22', '\xE6', '\xC7',
    '\x3A', '\x2C', '\x80', '\xEF', '\xC1', '\x15', '\x90', '\x7',  '\x93',
    '\xA2', '\x28', '\xA0', 'S',    'j',    '\xB1', '\xB8', '\xDF', '\x29',
    '5',    'C',    '\xE',  '\x3F', 'X',    '\xFC', '\x98', '\xDA', 'y',
    'j',    'P',    '\x40', '\x0',  '\x87', '\xAE', '\x1B', '\x17', 'B',
    '\xB4', '\x3A', '\x3F', '\xBE', 'y',    '\xC7', '\xA',  '\x26', '\xB6',
    '\xEE', '\xD9', '\x9A', '\x60', '\x14', '\x93', '\xDB', '\x8F', '\xD',
    '\xA',  '\x2E', '\xE9', '\x23', '\x95', '\x29', 'X',    '\x0',  '\x27',
    '\xEB', 'n',    'V',    'p',    '\xBC', '\xD6', '\xCB', '\xD6', 'G',
    '\xAB', '\x3D', 'l',    '\x7D', '\xB8', '\xD2', '\xDD', '\xA0', '\x60',
    '\x83', '\xBA', '\xEF', '\x5F', '\xA4', '\xEA', '\xCC', '\x2',  'N',
    '\xAE', '\x5E', 'p',    '\x1A', '\xEC', '\xB3', '\x40', '9',    '\xAC',
    '\xFE', '\xF2', '\x91', '\x89', 'g',    '\x91', '\x85', '\x21', '\xA8',
    '\x87', '\xB7', 'X',    '\x7E', '\x7E', '\x85', '\xBB', '\xCD', 'N',
    'N',    'b',    't',    '\x40', '\xFA', '\x93', '\x89', '\xEC', '\x1E',
    '\xEC', '\x86', '\x2',  'H',    '\x26', '\x93', '\xD0', 'u',    '\x1D',
    '\x7F', '\x9',  '2',    '\x95', '\xBF', '\x1F', '\xDB', '\xD7', 'c',
    '\x8A', '\x1A', '\xF7', '\x5C', '\xC1', '\xFF', '\x22', 'J',    '\xC3',
    '\x87', '\x0',  '\x3',  '\x0',  'K',    '\xBB', '\xF8', '\xD6', '\x2A',
    'v',    '\x98', 'I',    '\x0',  '\x0',  '\x0',  '\x0',  'I',    'E',
    'N',    'D',    '\xAE', 'B',    '\x60', '\x82',
};

void MimeType::Init() {
  mime_[".html"] = "text/html";
  mime_[".avi"] = "video/x-msvideo";
  mime_[".bmp"] = "image/bmp";
  mime_[".c"] = "text/plain";
  mime_[".doc"] = "application/msword";
  mime_[".gif"] = "image/gif";
  mime_[".gz"] = "application/x-gzip";
  mime_[".htm"] = "text/html";
  mime_[".ico"] = "image/x-icon";
  mime_[".jpg"] = "image/jpeg";
  mime_[".png"] = "image/png";
  mime_[".txt"] = "text/plain";
  mime_[".mp3"] = "audio/mp3";
  mime_["default"] = "text/html";
}

std::string MimeType::GetMime(const std::string &suffix)
{
    pthread_once(&once_control_, MimeType::Init);
    if (mime_.find(suffix) == mime_.end())
    {
        return mime_["default"];
    }
    else {
        return mime_[suffix];
    }
}

HttpData::HttpData(EventLoop* loop, int conn_fd)
    : loop_(loop)
    , channel_(new Channel(loop, conn_fd))
    , fd_(conn_fd)
    , error_(false)
    , connection_state_(H_CONNECTED)
    , method_(METHOD_GET)
    , version_(HTTP_11)
    , now_read_pos_(0)
    , state_(STATE_PARSE_URI)
    , h_state_(H_START)
    , keep_alive_(false)
{
    channel_->SetReadHandler(bind(&HttpData::HandleRead, this));
    channel_->SetWriteHandler(bind(&HttpData::HandleWrite, this));
    channel_->SetConnHandler(bind(&HttpData::HandleConn, this));
}

void HttpData::Reset()
{
    filename_.clear();
    path_.clear();
    now_read_pos_ = 0;
    state_ = STATE_PARSE_URI;
    h_state_ = H_START;
    headers_.clear();
    if (timer_.lock())
    {
        std::shared_ptr<TimerNode> my_timer(timer_.lock());
        my_timer->ClearRequest();
        timer_.reset();
    }
}

void HttpData::SeperateTimer()
{
    if (timer_.lock())
    {
        std::shared_ptr<TimerNode> my_timer(timer_.lock());
        my_timer->ClearRequest();
        timer_.reset();
    }
}

void HttpData::HandleRead()
{
    __uint32_t &events = channel_->GetEvents();
    do {
        bool zero = false;
        int read_num = Readn(fd_, inbuffer_, zero);
        LOG << "Request: " << inbuffer_;
        if (connection_state_ == H_DISCONNECTING)
        {
            inbuffer_.clear();
            break;
        }
        if (read_num < 0)
        {
            perror("1");
            error_ = true;
            HandleError(fd_, 400, "Bad Request");
            break;
        }
        else if (zero)
        {
            // 有请求出现但是读不到数据，可能是Request
            // Aborted，或者来自网络的数据没有达到等原因
            // 最可能是对端已经关闭了，统一按照对端已经关闭处理
            connection_state_ = H_DISCONNECTING;
            if (read_num == 0)
            {
                break;
            }
        }
        if (state_ == STATE_PARSE_URI)
        {
            URIState flag = this->ParseURI();
            if (flag == PARSE_URI_AGAIN)
            {
                break;
            }
            else if (flag == PARSE_URI_ERROR)
            {
                perror("2");
                LOG << "FD = " << fd_ << "," << inbuffer_ << "******";
                inbuffer_.clear();
                error_ = true;
                HandleError(fd_, 400, "Bad Request");
                break;
            }
            else {
                state_ = STATE_PARSE_HEADERS;
            }
        }

        if (state_ == STATE_PARSE_HEADERS)
        {
            HeaderState flag = this->ParseHeaders();
            if (flag == PARSE_HEADER_AGAIN)
            {
                break;
            }
            else if (flag == PARSE_HEADER_ERROR) 
            {
                perror("3");
                error_ = true;
                HandleError(fd_, 400, "Bad Request");
                break;
            }
            if (method_ == METHOD_POST) {
                // POST方法准备
                state_ = STATE_RECV_BODY;
            } 
            else 
            {
                state_ = STATE_ANALYSIS;
            }
        }

        if (state_ == STATE_RECV_BODY) {
            int content_length = -1;
            if (headers_.find("Content-length") != headers_.end()) 
            {
                content_length = stoi(headers_["Content-length"]);
            } 
            else 
            {
                error_ = true;
                HandleError(fd_, 400, "Bad Request: Lack of argument (Content-length)");
                break;
            }
            if (static_cast<int>(inbuffer_.size()) < content_length) break;
            state_ = STATE_ANALYSIS;
        }

        if (state_ == STATE_ANALYSIS) 
        {
            AnalysisState flag = this->AnalysisRequest();
            if (flag == ANALYSIS_SUCCESS) 
            {
                state_ = STATE_FINISH;
                break;
            } 
            else 
            {
                // cout << "state_ == STATE_ANALYSIS" << endl;
                error_ = true;
                break;
            }
        }
    }while (false);

    if (!error_)
    {
        if (outbuffer_.size() > 0)
        {
            HandleWrite();
        }

        if (!error_ && state_ ==STATE_FINISH)
        {
            this->Reset();
            if (inbuffer_.size() > 0)
            {
                if (connection_state_ != H_DISCONNECTING)
                {
                    HandleRead();
                }
            }
        } 
        else if (!error_ && connection_state_ != H_DISCONNECTED)
        {
            events |= EPOLLIN;
        }
    }
}

void HttpData::HandleWrite()
{
    if (!error_ && connection_state_ != H_DISCONNECTED)
    {
        __uint32_t& events = channel_->GetEvents();
        if (Writen(fd_, outbuffer_) < 0)
        {
            perror("writen");
            events = 0;
            error_ = true;
        }
        if (outbuffer_.size() > 0)
        {
            events = EPOLLOUT;
        }
    }
}

void HttpData::HandleConn()
{
    SeperateTimer();
    __uint32_t& events = channel_->GetEvents();
    if (!error_ && connection_state_ == H_CONNECTED) 
    {
        if (events != 0) 
        {
            int timeout = DEFAULT_EXPIRED_TIME;
            if (keep_alive_) timeout = DEFAULT_KEEP_ALIVE_TIME;
            if ((events & EPOLLIN) && (events & EPOLLOUT)) 
            {
                events = __uint32_t(0);
                events |= EPOLLOUT;
            }
            events |= EPOLLET;
            loop_->UpdateEpoller(channel_, timeout);
        } 
        else if (keep_alive_) 
        {
            events |= (EPOLLIN | EPOLLET);
            int timeout = DEFAULT_KEEP_ALIVE_TIME;
            loop_->UpdateEpoller(channel_, timeout);
        } else {
            events |= (EPOLLIN | EPOLLET);
            int timeout = (DEFAULT_KEEP_ALIVE_TIME >> 1);
            loop_->UpdateEpoller(channel_, timeout);
        }
    } 
    else if (!error_ && connection_state_ == H_DISCONNECTING &&
             (events & EPOLLOUT)) 
    {
        events = (EPOLLOUT | EPOLLET);
    } 
    else 
    {
        loop_->RunInLoop(bind(&HttpData::HandleClose, shared_from_this()));
    }
}

URIState HttpData::ParseURI()
{
    std::string &str = inbuffer_;
    std::string cop = str;
    // 读到完整的请求行再开始解析请求
    size_t pos = str.find('\r', now_read_pos_);
    if (pos < 0)
    {
        return PARSE_URI_AGAIN;
    }
    // 去掉请求行所占的空间，节省空间
    std::string request_line = str.substr(0, pos);
    if (str.size() > pos + 1)
    {
        str = str.substr(pos + 1);
    }
    else {
        str.clear();
    }

    // Method
    int pos_get = request_line.find("GET");
    int pos_post = request_line.find("POST");
    int pos_head = request_line.find("HEAD");

    if (pos_get >= 0) {
        pos = pos_get;
        method_ = METHOD_GET;
    }
    else if (pos_post >= 0)
    {
        pos = pos_post;
        method_ = METHOD_POST;
    }
    else if (pos_head >= 0)
    {
        pos = pos_head;
        method_ = METHOD_HEAD;
    }
    else
    {
        return PARSE_URI_ERROR;
    }

    // filename
    pos = request_line.find("/", pos);
    if (pos < 0)
    {
        filename_ = "index.html";
        version_ = HTTP_11;
        return PARSE_URI_SUCCESS;
    }
    else
    {
        size_t pos_ = request_line.find(' ', pos);
        if (pos_ < 0)
        {
            return PARSE_URI_ERROR;
        }
        else
        {
            if (pos_ - pos > 1)
            {
                filename_ = request_line.substr(pos+1, pos_-pos-1);
                size_t pos__ = filename_.find('?');
                if (pos__ >= 0)
                {
                    filename_ = filename_.substr(0, pos__);
                }
            }
            else {
                filename_ = "index.html";
            }
        }
        pos = pos_;
    }

    // HTTP 版本号
    pos = request_line.find("/", pos);
    if (pos < 0)
    {
        return PARSE_URI_ERROR;
    }
    else {
        if (request_line.size() - pos <= 3)
        {
            return PARSE_URI_ERROR;
        }
        else {
            std::string ver = request_line.substr(pos + 1, 3);
            if (ver == "1.0")
                version_ = HTTP_10;
            else if (ver == "1.1")
                version_ = HTTP_11;
            else
                return PARSE_URI_ERROR;
        }
    }
    return PARSE_URI_SUCCESS;
}

HeaderState HttpData::ParseHeaders() {
  std::string &str = inbuffer_;
  int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
  int now_read_line_begin = 0;
  bool not_finish = true;
  size_t i = 0;
  for (; i < str.size() && not_finish; ++i) {
    switch (h_state_) {
      case H_START: {
        if (str[i] == '\n' || str[i] == '\r') break;
        h_state_ = H_KEY;
        key_start = i;
        now_read_line_begin = i;
        break;
      }
      case H_KEY: {
        if (str[i] == ':') {
          key_end = i;
          if (key_end - key_start <= 0) return PARSE_HEADER_ERROR;
          h_state_ = H_COLON;
        } else if (str[i] == '\n' || str[i] == '\r')
          return PARSE_HEADER_ERROR;
        break;
      }
      case H_COLON: {
        if (str[i] == ' ') {
          h_state_ = H_SPACES_AFTER_COLON;
        } else
          return PARSE_HEADER_ERROR;
        break;
      }
      case H_SPACES_AFTER_COLON: {
        h_state_ = H_VALUE;
        value_start = i;
        break;
      }
      case H_VALUE: {
        if (str[i] == '\r') {
          h_state_ = H_CR;
          value_end = i;
          if (value_end - value_start <= 0) return PARSE_HEADER_ERROR;
        } else if (i - value_start > 255)
          return PARSE_HEADER_ERROR;
        break;
      }
      case H_CR: {
        if (str[i] == '\n') {
          h_state_ = H_LF;
          std::string key(str.begin() + key_start, str.begin() + key_end);
          std::string value(str.begin() + value_start, str.begin() + value_end);
          headers_[key] = value;
          now_read_line_begin = i;
        } else
          return PARSE_HEADER_ERROR;
        break;
      }
      case H_LF: {
        if (str[i] == '\r') {
          h_state_ = H_END_CR;
        } else {
          key_start = i;
          h_state_ = H_KEY;
        }
        break;
      }
      case H_END_CR: {
        if (str[i] == '\n') {
          h_state_ = H_END_LF;
        } else
          return PARSE_HEADER_ERROR;
        break;
      }
      case H_END_LF: {
        not_finish = false;
        key_start = i;
        now_read_line_begin = i;
        break;
      }
    }
  }
  if (h_state_ == H_END_LF) {
    str = str.substr(i);
    return PARSE_HEADER_SUCCESS;
  }
  str = str.substr(now_read_line_begin);
  return PARSE_HEADER_AGAIN;
}

AnalysisState HttpData::AnalysisRequest() {
  if (method_ == METHOD_POST) {
    // ------------------------------------------------------
    // My CV stitching handler which requires OpenCV library
    // ------------------------------------------------------
    // string header;
    // header += string("HTTP/1.1 200 OK\r\n");
    // if(headers_.find("Connection") != headers_.end() &&
    // headers_["Connection"] == "Keep-Alive")
    // {
    //     keepAlive_ = true;
    //     header += string("Connection: Keep-Alive\r\n") + "Keep-Alive:
    //     timeout=" + to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
    // }
    // int length = stoi(headers_["Content-length"]);
    // vector<char> data(inBuffer_.begin(), inBuffer_.begin() + length);
    // Mat src = imdecode(data, CV_LOAD_IMAGE_ANYDEPTH|CV_LOAD_IMAGE_ANYCOLOR);
    // //imwrite("receive.bmp", src);
    // Mat res = stitch(src);
    // vector<uchar> data_encode;
    // imencode(".png", res, data_encode);
    // header += string("Content-length: ") + to_string(data_encode.size()) +
    // "\r\n\r\n";
    // outBuffer_ += header + string(data_encode.begin(), data_encode.end());
    // inBuffer_ = inBuffer_.substr(length);
    // return ANALYSIS_SUCCESS;
  } else if (method_ == METHOD_GET || method_ == METHOD_HEAD) {
    std::string header;
    header += "HTTP/1.1 200 OK\r\n";
    if (headers_.find("Connection") != headers_.end() &&
        (headers_["Connection"] == "Keep-Alive" ||
         headers_["Connection"] == "keep-alive")) {
      keep_alive_ = true;
      header += std::string("Connection: Keep-Alive\r\n") + "Keep-Alive: timeout=" +
                std::to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
    }
    int dot_pos = filename_.find('.');
    std::string filetype;
    if (dot_pos < 0)
      filetype = MimeType::GetMime("default");
    else
      filetype = MimeType::GetMime(filename_.substr(dot_pos));

    // echo test
    if (filename_ == "hello") {
      outbuffer_ =
          "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\nHello World";
      return ANALYSIS_SUCCESS;
    }
    if (filename_ == "favicon.ico") {
      header += "Content-Type: image/png\r\n";
      header += "Content-Length: " + std::to_string(sizeof favicon) + "\r\n";
      header += "Server: LinYa's Web Server\r\n";

      header += "\r\n";
      outbuffer_ += header;
      outbuffer_ += std::string(favicon, favicon + sizeof favicon);
      ;
      return ANALYSIS_SUCCESS;
    }

    struct stat sbuf;
    if (stat(filename_.c_str(), &sbuf) < 0) {
      header.clear();
      HandleError(fd_, 404, "Not Found!");
      return ANALYSIS_ERROR;
    }
    header += "Content-Type: " + filetype + "\r\n";
    header += "Content-Length: " + std::to_string(sbuf.st_size) + "\r\n";
    header += "Server: LinYa's Web Server\r\n";
    // 头部结束
    header += "\r\n";
    outbuffer_ += header;

    if (method_ == METHOD_HEAD) return ANALYSIS_SUCCESS;

    int src_fd = open(filename_.c_str(), O_RDONLY, 0);
    if (src_fd < 0) {
      outbuffer_.clear();
      HandleError(fd_, 404, "Not Found!");
      return ANALYSIS_ERROR;
    }
    void *mmapRet = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    close(src_fd);
    if (mmapRet == (void *)-1) {
      munmap(mmapRet, sbuf.st_size);
      outbuffer_.clear();
      HandleError(fd_, 404, "Not Found!");
      return ANALYSIS_ERROR;
    }
    char *src_addr = static_cast<char *>(mmapRet);
    outbuffer_ += std::string(src_addr, src_addr + sbuf.st_size);
    ;
    munmap(mmapRet, sbuf.st_size);
    return ANALYSIS_SUCCESS;
  }
  return ANALYSIS_ERROR;
}

void HttpData::HandleError(int fd, int err_num, std::string short_msg) {
  short_msg = " " + short_msg;
  char send_buff[4096];
  std::string body_buff, header_buff;
  body_buff += "<html><title>哎~出错了</title>";
  body_buff += "<body bgcolor=\"ffffff\">";
  body_buff += std::to_string(err_num) + short_msg;
  body_buff += "<hr><em> LinYa's Web Server</em>\n</body></html>";

  header_buff += "HTTP/1.1 " + std::to_string(err_num) + short_msg + "\r\n";
  header_buff += "Content-Type: text/html\r\n";
  header_buff += "Connection: Close\r\n";
  header_buff += "Content-Length: " + std::to_string(body_buff.size()) + "\r\n";
  header_buff += "Server: LinYa's Web Server\r\n";
  ;
  header_buff += "\r\n";
  // 错误处理不考虑writen不完的情况
  sprintf(send_buff, "%s", header_buff.c_str());
  Writen(fd, send_buff, strlen(send_buff));
  sprintf(send_buff, "%s", body_buff.c_str());
  Writen(fd, send_buff, strlen(send_buff));
}

void HttpData::HandleClose() {
  connection_state_ = H_DISCONNECTED;
  std::shared_ptr<HttpData> guard(shared_from_this());
  loop_->RemoveFromEpoller(channel_);
}

void HttpData::NewEvent() {
  channel_->SetEvents(DEFAULT_EVENT);
  loop_->AddToEpoller(channel_, DEFAULT_EXPIRED_TIME);
}
