//
// WebSocketImpl.h
//
// $Id: //poco/1.4/Net/include/Poco/Net/WebSocketImpl.h#6 $
//
// Library: Net
// Package: WebSocket
// Module:  WebSocketImpl
//
// Definition of the StreamSocketImpl class.
//
// Copyright (c) 2012, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#ifndef Net_WebSocketImpl_INCLUDED
#define Net_WebSocketImpl_INCLUDED


#include "Poco/Net/StreamSocketImpl.h"
#include "Poco/Random.h"


namespace Poco {
namespace Net {


class Net_API WebSocketImpl: public StreamSocketImpl
	/// This class implements a WebSocket, according
	/// to the WebSocket protocol described in RFC 6455.
{
public:
	WebSocketImpl(StreamSocketImpl* pStreamSocketImpl, bool mustMaskPayload);
		/// Creates a StreamSocketImpl using the given native socket.
	
	// StreamSocketImpl
	virtual int sendBytes(const void* buffer, int length, int flags);
		/// Sends a WebSocket protocol frame.
		

	virtual int receiveBytes(std::string &buffer, int flags);
	virtual int receiveBytes(void* buffer, int length, int flags);
		/// Receives a WebSocket protocol frame.
		
	virtual SocketImpl* acceptConnection(SocketAddress& clientAddr);
	virtual void connect(const SocketAddress& address);
	virtual void connect(const SocketAddress& address, const Poco::Timespan& timeout);
	virtual void connectNB(const SocketAddress& address);
	virtual void bind(const SocketAddress& address, bool reuseAddress = false);
	virtual void bind6(const SocketAddress& address, bool reuseAddress = false, bool ipV6Only = false);
	virtual void listen(int backlog = 64);
	virtual void close();
	virtual void shutdownReceive();
	virtual void shutdownSend();
	virtual void shutdown();
	virtual int sendTo(const void* buffer, int length, const SocketAddress& address, int flags = 0);
	virtual int receiveFrom(void* buffer, int length, SocketAddress& address, int flags = 0);
	virtual void sendUrgent(unsigned char data);
	virtual bool secure() const;
	virtual void setSendTimeout(const Poco::Timespan& timeout);	
	virtual Poco::Timespan getSendTimeout();
	virtual void setReceiveTimeout(const Poco::Timespan& timeout);
	virtual Poco::Timespan getReceiveTimeout();

	// Internal
	int frameFlags() const;
		/// Returns the frame flags of the most recently received frame.
		
	bool mustMaskPayload() const;
		/// Returns true if the payload must be masked.

protected:
	enum
	{
		FRAME_FLAG_MASK   = 0x80,
		MAX_HEADER_LENGTH = 14
	};
	
	int receiveNBytes(void* buffer, int bytes);
	virtual ~WebSocketImpl();

private:
	WebSocketImpl();
	
	StreamSocketImpl* _pStreamSocketImpl;
	int _frameFlags;
	bool _mustMaskPayload;
	Poco::Random _rnd;
};


//
// inlines
//
inline int WebSocketImpl::frameFlags() const
{
	return _frameFlags;
}


inline bool WebSocketImpl::mustMaskPayload() const
{
	return _mustMaskPayload;
}


} } // namespace Poco::Net


#endif // Net_WebSocketImpl_INCLUDED
