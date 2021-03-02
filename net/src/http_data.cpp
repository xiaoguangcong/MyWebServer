#include "../include/http_data.h"
#include "../include/channel.h"
#include "../include/util.h"
#include "../../base/include/logging.h"
#include "../include/eventloop.h"
#include <pthread.h>
#include <memory>
#include <sys/types.h>
#include <unordered_map>
#include <sys/epoll.h>
#include<stdio.h>
#include<pthread.h>

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


