#ifndef _LOG_H_
#define _LOG_H_

#include <time.h>
#include <sstream>
#include <iostream>

#include "singleton.h"

namespace EA_COMMON
{
    class CLogger
    {
    public:
        CLogger();
        ~CLogger();
    public:
        bool Init(const char *apFile);
    public:
        void DebugR(char *apFile,int aiLine,char *apInfo,...);

        void InfoR(char *apInfo,...);

        void WarnR(char *apInfo,...);

        void ErrorR(char *apInfo,...);

        void FatalR(char *apInfo,...);
        inline bool isInit() {
          return m_bInit;
        }
    private:
        void *m_pLogger;
        bool    m_bInit;
    };

    typedef CSingleton<CLogger> CSingletonLogger;

    #define Init_Log        CSingletonLogger::Instance().Init
    #define Debug(x1,...)   CSingletonLogger::Instance().DebugR(__FILE__,__LINE__,x1,##__VA_ARGS__)
    #define Info            CSingletonLogger::Instance().InfoR
    #define Warn            CSingletonLogger::Instance().WarnR
    #define Error           CSingletonLogger::Instance().ErrorR
    #define Fatal           CSingletonLogger::Instance().FatalR

    enum LOG_LEVEL {
        E_DEBUG,
        E_INFO,
        E_WARN,
        E_ERROR,
        E_FATAL
    };
    class StreamLogger{
    public:
        StreamLogger( LOG_LEVEL level):stream(m_linebuf),m_level(level) {};
        ~StreamLogger(){
         if  ( CSingletonLogger::Instance().isInit() ) {
        switch (m_level){
        case E_DEBUG:
        case E_INFO:
          Info((char*)"%s",(m_linebuf.str().c_str()));
          break;
        case E_WARN:
          Warn((char*)"%s",(m_linebuf.str().c_str()));
          break;
        case E_ERROR:
          Error((char*)"%s",(m_linebuf.str().c_str()));
          break;
        case E_FATAL:
          Fatal((char*)"%s",(m_linebuf.str().c_str()));
          break;
        default:
          Warn((char*)"%s",(m_linebuf.str().c_str()));
          break;
        }
         } else {
           static const char *  level_str[] =
              {"debug","info","warn","error","fatal"};
           time_t t = time(0);
           char ts[32];
           //产生"YYYY-MM-DD hh:mm:ss"格式的字符串。
           strftime(ts, sizeof(ts), "%F %T", localtime(&t));
           std::cout <<"["<<ts<<"] "<< level_str[m_level] << " " <<m_linebuf.str()<<"\n";
         }
        }
        std::ostream &stream;
    private:
        std::stringstream m_linebuf;
        LOG_LEVEL m_level;
    };
};

#define Debug_S    EA_COMMON::StreamLogger(EA_COMMON::E_DEBUG).stream<<__LINE__<<" "
#define Info_S     EA_COMMON::StreamLogger(EA_COMMON::E_INFO).stream
#define Warn_S     EA_COMMON::StreamLogger(EA_COMMON::E_WARN).stream
#define Error_S    EA_COMMON::StreamLogger(EA_COMMON::E_ERROR).stream <<__FILE__<<":"<<__LINE__<<" "
#define Fatal_S    EA_COMMON::StreamLogger(EA_COMMON::E_FATAL).stream <<__FILE__<<":"<<__LINE__<<" "

#endif //_LOG_H_

