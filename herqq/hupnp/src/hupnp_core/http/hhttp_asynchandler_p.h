/*
 *  Copyright (C) 2010 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
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

#ifndef HTTP_ASYNCHANDLER_P_H_
#define HTTP_ASYNCHANDLER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "./../general/hdefs_p.h"

#include "hhttp_p.h"
#include "hhttp_messaginginfo_p.h"
#include "./../devicehosting/messages/hevent_messages_p.h"

#include <QHash>
#include <QUuid>
#include <QObject>
#include <QByteArray>
#include <QAbstractSocket>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>

class QtSoapMessage;

namespace Herqq
{

namespace Upnp
{

class HHttpAsyncHandler;

//
//
//
class HHttpAsyncOperation :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HHttpAsyncOperation)
friend class HHttpAsyncHandler;

private:

    enum InternalState
    {
        Internal_Failed,
        Internal_NotStarted,
        Internal_WritingBlob,
        Internal_WritingChunkedSizeLine,
        Internal_WritingChunk,
        Internal_ReadingHeader,
        Internal_ReadingData,
        Internal_ReadingChunkSizeLine,
        Internal_ReadingChunk,
        Internal_FinishedSuccessfully
    };

    MessagingInfo* m_mi;

    QByteArray m_dataToSend;
    // the data which will be sent to the target socket

    qint64 m_dataSend;
    // used only with chunked encoding when a chunk cannot be sent in full and
    // the operation needs to be continued later

    qint64 m_dataSent;
    // the amount of data that has been successfully sent

    InternalState m_state;
    // the current state of this "state machine"

    QHttpResponseHeader m_headerRead;
    // the http reader read as response from the target socket

    QByteArray m_dataRead;
    // the response data that is currently read from the target socket

    qint64 m_dataToRead;
    // the amount of data that should be available (once the operation is
    // successfully completed)

    QUuid m_uuid;
    // id for the operation

    const QByteArray m_loggingIdentifier;

private:

    void sendChunked();

    void readBlob();
    bool readChunkedSizeLine();
    bool readChunk();
    void readHeader();
    void readData();

    bool run();
    void done_(InternalState state);

private Q_SLOTS:

    void bytesWritten(qint64);
    void readyRead();
    void error(QAbstractSocket::SocketError);

public:

    enum State
    {
        Failed,
        NotStarted,
        Writing,
        Reading,
        Succeeded
    };

    HHttpAsyncOperation(
        const QByteArray& loggingIdentifier, MessagingInfo* mi,
        const QByteArray& data, QObject* parent);

    ~HHttpAsyncOperation();

    inline QUuid uuid() const { return m_uuid; }

    State state() const;

    // the data of the response
    inline QByteArray dataRead() const { return m_dataRead; }

    // the header of the response
    inline QHttpResponseHeader headerRead() const { return m_headerRead; }

Q_SIGNALS:

    void done(const QUuid&);
};

//
// Performs async messaging utilizing the event loop.
// This class is not thread-safe.
//
class HHttpAsyncHandler :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HHttpAsyncHandler)
friend class HHttpAsyncOperation;

private:

    const QByteArray m_loggingIdentifier;

    QHash<QUuid, HHttpAsyncOperation*> m_operations;

private Q_SLOTS:

    void done(const QUuid&);

Q_SIGNALS:

    // user is expected to delete the transferred object
    void msgIoComplete(HHttpAsyncOperation*);

public:

    HHttpAsyncHandler(const QByteArray& loggingIdentifier, QObject* parent);
    virtual ~HHttpAsyncHandler();

    //
    // \param mi
    // \param data contains an entire HTTP message, including headers.
    //
    // \return an object that contains state data for the operation.
    // once the operation is done, user is expected to delete the object, but
    // NOT any sooner!
    HHttpAsyncOperation* msgIo(MessagingInfo* mi, const QByteArray& data);

    //
    // Helper overload
    //
    HHttpAsyncOperation* msgIo(
        MessagingInfo*, QHttpRequestHeader&, const QtSoapMessage&);
};

}
}

#endif /* HTTP_ASYNCHANDLER_P_H_ */
