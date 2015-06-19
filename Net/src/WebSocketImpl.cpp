//
// WebSocketImpl.cpp
//
// $Id: //poco/1.4/Net/src/WebSocketImpl.cpp#9 $
//
// Library: Net
// Package: WebSocket
// Module:  WebSocketImpl
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


#include "Poco/Net/WebSocketImpl.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Buffer.h"
#include "Poco/BinaryWriter.h"
#include "Poco/BinaryReader.h"
#include "Poco/MemoryStream.h"
#include "Poco/Format.h"
#include <cstring>


namespace Poco {
namespace Net {


WebSocketImpl::WebSocketImpl(StreamSocketImpl* pStreamSocketImpl, bool mustMaskPayload):
	StreamSocketImpl(pStreamSocketImpl->sockfd()),
	_pStreamSocketImpl(pStreamSocketImpl),
	_frameFlags(0),
	_mustMaskPayload(mustMaskPayload)
{
	poco_check_ptr(pStreamSocketImpl);
	_pStreamSocketImpl->duplicate();
}


WebSocketImpl::~WebSocketImpl()
{
	_pStreamSocketImpl->release();
	reset();
}

	
int WebSocketImpl::sendBytes(const void* buffer, int length, int flags)
{
	Poco::Buffer<char> frame(length + MAX_HEADER_LENGTH);
	Poco::MemoryOutputStream ostr(frame.begin(), frame.size());
	Poco::BinaryWriter writer(ostr, Poco::BinaryWriter::NETWORK_BYTE_ORDER);
	
	if (flags == 0) flags = WebSocket::FRAME_BINARY;
	writer << static_cast<Poco::UInt8>(flags);
	Poco::UInt8 lengthByte(0);
	if (_mustMaskPayload)
	{
		lengthByte |= FRAME_FLAG_MASK;
	}
	if (length < 126)
	{
		lengthByte |= static_cast<Poco::UInt8>(length);
		writer << lengthByte;
	}
	else if (length < 65536)
	{
		lengthByte |= 126;
		writer << lengthByte << static_cast<Poco::UInt16>(length);
	}
	else
	{
		lengthByte |= 127;
		writer << lengthByte << static_cast<Poco::UInt64>(length);
	}
	if (_mustMaskPayload)
	{
		const Poco::UInt32 mask = _rnd.next();
		const char* m = reinterpret_cast<const char*>(&mask);
		const char* b = reinterpret_cast<const char*>(buffer);
		writer.writeRaw(m, 4);
		char* p = frame.begin() + ostr.charsWritten();
		for (int i = 0; i < length; i++)
		{
			p[i] = b[i] ^ m[i % 4];
		}
	}
	else
	{
		std::memcpy(frame.begin() + ostr.charsWritten(), buffer, length);
	}
	_pStreamSocketImpl->sendBytes(frame.begin(), length + static_cast<int>(ostr.charsWritten()));
	return length;
}


int WebSocketImpl::receiveBytes(std::string &buffer, int )
{
    char header[MAX_HEADER_LENGTH];
    int n = receiveNBytes(header, 2);
    if (n <= 0)
    {
        _frameFlags = 0;
        return n;
    }
    poco_assert (n == 2);
    Poco::UInt8 lengthByte = static_cast<Poco::UInt8>(header[1]);
    int maskOffset = 0;
    if (lengthByte & FRAME_FLAG_MASK) maskOffset += 4;
    lengthByte &= 0x7f;
	if (lengthByte > 0 || maskOffset > 0)
	{
		if (lengthByte + 2 + maskOffset < MAX_HEADER_LENGTH)
		{
			n = receiveNBytes(header + 2, lengthByte + maskOffset);
		}
		else
		{
			n = receiveNBytes(header + 2, MAX_HEADER_LENGTH - 2);
		}
		if (n <= 0) throw WebSocketException("Incomplete header received", WebSocket::WS_ERR_INCOMPLETE_FRAME);
		n += 2;
	}
    Poco::MemoryInputStream istr(header, n);
    Poco::BinaryReader reader(istr, Poco::BinaryReader::NETWORK_BYTE_ORDER);
    Poco::UInt8 flags;
    char mask[4];
    reader >> flags >> lengthByte;
    _frameFlags = flags;
    int payloadLength = 0;
    int payloadOffset = 2;
    if ((lengthByte & 0x7f) == 127)
    {
        Poco::UInt64 l;
        reader >> l;
        payloadLength = static_cast<int>(l);
        payloadOffset += 8;
    }
    else if ((lengthByte & 0x7f) == 126)
    {
        Poco::UInt16 l;
        reader >> l;
        payloadLength = static_cast<int>(l);
        payloadOffset += 2;
    }
    else
    {
        Poco::UInt8 l = lengthByte & 0x7f;
        payloadLength = static_cast<int>(l);
    }
    if (lengthByte & FRAME_FLAG_MASK)
    {
        reader.readRaw(mask, 4);
        payloadOffset += 4;
    }
    int received = 0;

    buffer.resize(payloadLength);
    if (payloadOffset < n)
    {

        received = n - payloadOffset;
        std::memcpy( &buffer[0], header + payloadOffset, received );

    }
    if (received < payloadLength)
    {
        n = receiveNBytes(reinterpret_cast<char*>(&buffer[0]) + received, payloadLength - received);
        if (n <= 0) throw WebSocketException("Incomplete frame received", WebSocket::WS_ERR_INCOMPLETE_FRAME);
        received += n;
    }
    if (received>0  && (lengthByte & FRAME_FLAG_MASK))
    {
        char* p = &buffer[0];
        for (int i = 0; i < received; i++)
        {
            p[i] ^= mask[i % 4];
        }
    }
    return received;

}

	
int WebSocketImpl::receiveBytes(void* buffer, int length, int)
{
	char header[MAX_HEADER_LENGTH];
	int n = receiveNBytes(header, 2);
	if (n <= 0)
	{
		_frameFlags = 0;
		return n;
	}
	poco_assert (n == 2);
	Poco::UInt8 lengthByte = static_cast<Poco::UInt8>(header[1]);
	int maskOffset = 0;
	if (lengthByte & FRAME_FLAG_MASK) maskOffset += 4;
	lengthByte &= 0x7f;

	if (lengthByte > 0 || maskOffset > 0)
	{
		if (lengthByte + 2 + maskOffset < MAX_HEADER_LENGTH)
		{
			n = receiveNBytes(header + 2, lengthByte + maskOffset);
		}
		else
		{
			n = receiveNBytes(header + 2, MAX_HEADER_LENGTH - 2);
		}
		if (n <= 0) throw WebSocketException("Incomplete header received", WebSocket::WS_ERR_INCOMPLETE_FRAME);
		n += 2;
	}
	Poco::MemoryInputStream istr(header, n);
	Poco::BinaryReader reader(istr, Poco::BinaryReader::NETWORK_BYTE_ORDER);
	Poco::UInt8 flags;
	char mask[4];
	reader >> flags >> lengthByte;
	_frameFlags = flags;
	int payloadLength = 0;
	int payloadOffset = 2;
	if ((lengthByte & 0x7f) == 127)
	{
		Poco::UInt64 l;
		reader >> l;
		if (l > length) throw WebSocketException(Poco::format("Insufficient buffer for payload size %Lu", l), WebSocket::WS_ERR_PAYLOAD_TOO_BIG);
		payloadLength = static_cast<int>(l);
		payloadOffset += 8;
	}
	else if ((lengthByte & 0x7f) == 126)
	{
		Poco::UInt16 l;
		reader >> l;
		if (l > length) throw WebSocketException(Poco::format("Insufficient buffer for payload size %hu", l), WebSocket::WS_ERR_PAYLOAD_TOO_BIG);
		payloadLength = static_cast<int>(l);
		payloadOffset += 2;
	}
	else
	{
		Poco::UInt8 l = lengthByte & 0x7f;
		if (l > length) throw WebSocketException(Poco::format("Insufficient buffer for payload size %u", unsigned(l)), WebSocket::WS_ERR_PAYLOAD_TOO_BIG);
		payloadLength = static_cast<int>(l);
	}
	if (lengthByte & FRAME_FLAG_MASK)
	{
		reader.readRaw(mask, 4);
		payloadOffset += 4;
	}
	int received = 0;
	if (payloadOffset < n)
	{
		std::memcpy(buffer, header + payloadOffset, n - payloadOffset);
		received = n - payloadOffset;
	}
	if (received < payloadLength)
	{
		n = receiveNBytes(reinterpret_cast<char*>(buffer) + received, payloadLength - received);
		if (n <= 0) throw WebSocketException("Incomplete frame received", WebSocket::WS_ERR_INCOMPLETE_FRAME);
		received += n;
	}
	if (lengthByte & FRAME_FLAG_MASK)
	{
		char* p = reinterpret_cast<char*>(buffer);
		for (int i = 0; i < received; i++)
		{
			p[i] ^= mask[i % 4];
		}
	}
	return received;
}


int WebSocketImpl::receiveNBytes(void* buffer, int bytes)
{
	int received = _pStreamSocketImpl->receiveBytes(reinterpret_cast<char*>(buffer), bytes);
	if (received > 0)
	{
		while (received < bytes)
		{
			int n = _pStreamSocketImpl->receiveBytes(reinterpret_cast<char*>(buffer) + received, bytes - received);
			if (n > 0)
				received += n;
			else
				throw WebSocketException("Incomplete frame received", WebSocket::WS_ERR_INCOMPLETE_FRAME);
		}
	}
	return received;
}


SocketImpl* WebSocketImpl::acceptConnection(SocketAddress& clientAddr)
{
	throw Poco::InvalidAccessException("Cannot acceptConnection() on a WebSocketImpl");
}


void WebSocketImpl::connect(const SocketAddress& address)
{
	throw Poco::InvalidAccessException("Cannot connect() a WebSocketImpl");
}


void WebSocketImpl::connect(const SocketAddress& address, const Poco::Timespan& timeout)
{
	throw Poco::InvalidAccessException("Cannot connect() a WebSocketImpl");
}


void WebSocketImpl::connectNB(const SocketAddress& address)
{
	throw Poco::InvalidAccessException("Cannot connectNB() a WebSocketImpl");
}


void WebSocketImpl::bind(const SocketAddress& address, bool reuseAddress)
{
	throw Poco::InvalidAccessException("Cannot bind() a WebSocketImpl");
}


void WebSocketImpl::bind6(const SocketAddress& address, bool reuseAddress, bool ipV6Only)
{
	throw Poco::InvalidAccessException("Cannot bind6() a WebSocketImpl");
}


void WebSocketImpl::listen(int backlog)
{
	throw Poco::InvalidAccessException("Cannot listen() on a WebSocketImpl");
}


void WebSocketImpl::close()
{
	_pStreamSocketImpl->close();
	reset();
}


void WebSocketImpl::shutdownReceive()
{
	_pStreamSocketImpl->shutdownReceive();
}


void WebSocketImpl::shutdownSend()
{
	_pStreamSocketImpl->shutdownSend();
}

	
void WebSocketImpl::shutdown()
{
	_pStreamSocketImpl->shutdown();
}


int WebSocketImpl::sendTo(const void* buffer, int length, const SocketAddress& address, int flags)
{
	throw Poco::InvalidAccessException("Cannot sendTo() on a WebSocketImpl");
}


int WebSocketImpl::receiveFrom(void* buffer, int length, SocketAddress& address, int flags)
{
	throw Poco::InvalidAccessException("Cannot receiveFrom() on a WebSocketImpl");
}


void WebSocketImpl::sendUrgent(unsigned char data)
{
	throw Poco::InvalidAccessException("Cannot sendUrgent() on a WebSocketImpl");
}


bool WebSocketImpl::secure() const
{
	return _pStreamSocketImpl->secure();
}


void WebSocketImpl::setSendTimeout(const Poco::Timespan& timeout)
{
	_pStreamSocketImpl->setSendTimeout(timeout);
}


Poco::Timespan WebSocketImpl::getSendTimeout()
{
	return _pStreamSocketImpl->getSendTimeout();
}


void WebSocketImpl::setReceiveTimeout(const Poco::Timespan& timeout)
{
	_pStreamSocketImpl->setReceiveTimeout(timeout);
}


Poco::Timespan WebSocketImpl::getReceiveTimeout()
{
	return _pStreamSocketImpl->getReceiveTimeout();
}

	
} } // namespace Poco::Net
