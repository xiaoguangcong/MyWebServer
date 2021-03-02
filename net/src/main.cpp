//
// Created by xgc on 2020/10/8.
//

#include <cstdio>
#include <getopt.h>
#include <iostream>
#include "../include/eventloop.h"
#include "../../base/include/logging.h"
#include "../include/server.h"


int main(int argc, char *argv[]) 
{ 
    int thread_num = 4;
    int port = 80;
    std::string logpath = "./WebServe.log";

    // parse args
    int opt;
    const char* str = "t:l:p:";
    while((opt = getopt(argc, argv, str)) != -1)
    {
        switch(opt)
        {
            case 't' : {
                thread_num = atoi(optarg);
                break;
            }
            case 'l' : {
                logpath = optarg;
                if (logpath.size() < 2 || optarg[0] != '/')
                {
                    printf("logPath should start with \"/\"\n");
                    abort();
                }
                break;
            }
            case 'p' : {
                port = atoi(optarg);
                break;
            }
            default:
                break;

        }
    }
    Logger::setLogFileName(logpath);
    #ifndef _PTHREADS
        LOG << "_PTHREADS is not defined! ";
    #endif

    EventLoop main_loop;
    Server http_server(&main_loop, thread_num, port);
    http_server.Start();
    main_loop.Loop();
    return 0; 
}