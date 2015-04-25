//
// NTPClient.h
//
// $Id: //poco/1.4/NetEx/include/Poco/Net/NTPClient.h#1 $
//
// Library: NetEx
// Package: NTP
// Module:  NTPClient
//
// Definition of the NTPClient class.
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#ifndef NetEx_NTPClient_INCLUDED
#define NetEx_NTPClient_INCLUDED


#include "Poco/Net/NetEx.h"
#include "Poco/Net/NTPEventArgs.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/BasicEvent.h"


namespace Poco {
namespace Net {


class NetEx_API NTPClient
	/// This class provides NTP (Network Time Protocol) client functionality.
{
public:
	mutable Poco::BasicEvent<NTPEventArgs> response;

	explicit NTPClient(IPAddress::Family family, int timeout = 3000000);
		/// Creates an NTP client.

	~NTPClient();
		/// Destroys the NTP client.

	int request(SocketAddress& address) const;
		/// Request the time from the server at address.
		/// Notifications are posted for events.
		/// 
		/// Returns the number of valid replies.

	int request(const std::string& address) const;
		/// Request the time from the server at address.
		/// Notifications are posted for events.
		/// 
		/// Returns the number of valid replies.

private:
	mutable IPAddress::Family _family;
	int _timeout;
};


} } // namespace Poco::Net


#endif // NetEx_NTPClient_INCLUDED