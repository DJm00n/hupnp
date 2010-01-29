/*
 *  Copyright (C) 2010 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq Utils library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOGGER_H_
#define LOGGER_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../../core/include/HGlobal"
class QString;

namespace Herqq
{

//
//
//
class HLogger
{
H_DISABLE_COPY(HLogger)

private:

    const char* m_methodName;
    const char* m_logPrefix;

    static volatile int s_logLevel;

public:

    enum LogLevel
    {
        None = 0,
        Fatal = 1,
        Critical = 2,
        Warning = 3,
        Information = 4,
        Debug = 5,
        All = 6
    };

public:

    HLogger ();
    HLogger (const char* at, const char* methodName, const char* logPrefix = 0);
    ~HLogger();

    // the instance methods log the method name if it was specified. static
    // equivalents do not.
    void logDebug      (const QString& text);
    void logWarning    (const QString& text);
    void logInformation(const QString& text);
    void logCritical   (const QString& text);
    void logFatal      (const QString& text);

    inline static LogLevel traceLevel()
    {
        return static_cast<LogLevel>(s_logLevel);
    }

    inline static void setTraceLevel(LogLevel level)
    {
        s_logLevel = static_cast<qint32>(level);
    }

    static void logDebug_      (const QString& text);
    static void logWarning_    (const QString& text);
    static void logInformation_(const QString& text);
    static void logCritical_   (const QString& text);
    static void logFatal_      (const QString& text);
};

#define STR(X) #X
#define STRX(X) STR(X)

#ifndef H_FUN
#define H_FUN __FUNCTION__
#endif

#define H_AT __FILE__ ":" STRX(__LINE__)

#define HLOG(at, fun) \
    Herqq::HLogger herqqLog__(at, fun);

#define HLOG2(at, fun, logPrefix) \
    Herqq::HLogger herqqLog__(at, fun, logPrefix);

#define CHECK_LEVEL(level) \
    if (Herqq::HLogger::traceLevel() < Herqq::HLogger::level) ; \
    else

#define HLOG_WARN(text) \
    CHECK_LEVEL(Warning) herqqLog__.logWarning(text);

#define HLOG_WARN_AT(text, at) \
    CHECK_LEVEL(Warning) herqqLog__.logWarning(text, at);

#define HLOG_DBG(text) \
    CHECK_LEVEL(Debug) herqqLog__.logDebug(text);

#define HLOG_DBG_AT(text, at) \
    CHECK_LEVEL(Debug) herqqLog__.logDebug(text, at);

#define HLOG_INFO(text) \
    CHECK_LEVEL(Information) herqqLog__.logInformation(text);

#define HLOG_INFO_AT(text, at) \
    CHECK_LEVEL(Information) herqqLog__.logInformation(text, at);

#define HLOG_FATAL(text) \
    CHECK_LEVEL(Fatal) herqqLog__.logFatal(text);

#define HLOG_FATAL_AT(text, at) \
    CHECK_LEVEL(Fatal) herqqLog__.logFatal(text, at);

#define HLOG_CRIT(text) \
    CHECK_LEVEL(Critical) herqqLog__.logCritical(text);

#define HLOG_CRIT_AT(text, at) \
    CHECK_LEVEL(Critical) herqqLog__.logCritical(text, at);
}

#endif /* LOGGER_H_ */
