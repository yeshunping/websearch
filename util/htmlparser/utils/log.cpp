#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cstdarg>
#include <string>

#include "log.h"

#include "thirdparty/log4cpp/include/log4cpp/Category.hh"
#include "thirdparty/log4cpp/include/log4cpp/PropertyConfigurator.hh"

template<typename T>
inline void ignore_result(const T& ignored) {
}

EA_COMMON::CLogger::CLogger() {
  m_pLogger = 0;
  m_bInit = false;
}

EA_COMMON::CLogger::~CLogger() {
  m_bInit = false;
  m_pLogger = 0;
}

bool EA_COMMON::CLogger::Init(const char *apFile) {
  m_bInit = true;
  try {
    log4cpp::PropertyConfigurator::configure(apFile);
  } catch (log4cpp::ConfigureFailure& f) {
    m_bInit = false;
  }

  if (m_bInit) {
    m_pLogger = (void*) (&log4cpp::Category::getRoot());
  }

  return m_bInit;
}

void EA_COMMON::CLogger::DebugR(char *apFile, int aiLine, char *apInfo, ...) {
  if (m_bInit) {
    char *lpDebug = 0, *lpContent = 0;
    int liLenDebug = 0, liLenContent = 0;

    liLenDebug = asprintf(&lpDebug, "%s %d", apFile, aiLine);

    if (!lpDebug) {
      return;
    }

    va_list loVaLst;
    va_start(loVaLst, apInfo);
    liLenContent = vasprintf(&lpContent, apInfo, loVaLst);
    va_end(loVaLst);

    if (!lpContent) {
      free(lpDebug);
      return;
    }

    int liLen = liLenDebug + liLenContent + 5;
    char *lpBuf = new char[liLen];
    snprintf(lpBuf, liLen, "%s %s", lpDebug, lpContent);

    ((log4cpp::Category *) (m_pLogger))->debug(std::string(lpBuf));

    delete lpBuf;

    free(lpDebug);
    free(lpContent);
  }
}

void EA_COMMON::CLogger::InfoR(char *apInfo, ...) {
  if (m_bInit) {
    char *lstrContent = 0;

    va_list loVaLst;
    va_start(loVaLst, apInfo);
    ignore_result(vasprintf(&lstrContent, apInfo, loVaLst));
    va_end(loVaLst);

    if (!lstrContent) {
      return;
    }

    ((log4cpp::Category *) (m_pLogger))->info(std::string(lstrContent));

    free(lstrContent);
  }
}

void EA_COMMON::CLogger::WarnR(char *apInfo, ...) {
  if (m_bInit) {
    char *lstrContent = 0;

    va_list loVaLst;
    va_start(loVaLst, apInfo);
    ignore_result(vasprintf(&lstrContent, apInfo, loVaLst));
    va_end(loVaLst);

    if (!lstrContent) {
      return;
    }

    ((log4cpp::Category *) (m_pLogger))->warn(std::string(lstrContent));

    free(lstrContent);
  }
}

void EA_COMMON::CLogger::FatalR(char *apInfo, ...) {
  if (m_bInit) {
    char *lstrContent = 0;

    va_list loVaLst;
    va_start(loVaLst, apInfo);
    ignore_result(vasprintf(&lstrContent, apInfo, loVaLst));
    va_end(loVaLst);

    if (!lstrContent) {
      return;
    }

    ((log4cpp::Category *) (m_pLogger))->fatal(std::string(lstrContent));

    free(lstrContent);
  }
}
void EA_COMMON::CLogger::ErrorR(char *apInfo, ...) {
  if (m_bInit) {
    char *lstrContent = 0;

    va_list loVaLst;
    va_start(loVaLst, apInfo);
    ignore_result(vasprintf(&lstrContent, apInfo, loVaLst));
    
    va_end(loVaLst);

    if (!lstrContent) {
      return;
    }

    ((log4cpp::Category *) (m_pLogger))->error(std::string(lstrContent));

    free(lstrContent);
  }
}
