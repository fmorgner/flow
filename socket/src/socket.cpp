/***************************************************************************
 socket.cpp
 -----------------------
 begin                 : Fri Oct 29 2010
 copyright             : Copyright (C) 2010 by Manfred Morgner
 email                 : manfred@morgner.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *                                                                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place Suite 330,                                            *
 *   Boston, MA  02111-1307, USA.                                          *
 *                                                                         *
 ***************************************************************************/


#include "socket.h"
#include "socketexception.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <netdb.h> // getaddrinfo, addrinfo
// #include <netinet/tcp.h> // TCP_NODELAY

#include <iostream>


const int  CSocket::INVALID_SOCKET      = -1;
const int  CSocket::CLIENT_BACKLOG      =  5;
const int  CSocket::RECEIVE_BUFFER_SIZE =  512;


CSocket::CSocket( const int  nSocket,
                  const bool bVerbose )
  : m_nSocket( nSocket ),
    m_bVerbose( bVerbose )
  {
  } // CSocket::CSocket( const int nSocket )

CSocket::CSocket( const CSocket& src )
  {
  if ( !src.isValid() )
    {
    throw CSocketException( "Source Socket is invalid, so new socket can't be better" );
    }

  socklen_t addr_length = m_otAddr.SizeGet();
  m_nSocket      = ::accept( src.m_nSocket,
                             src.m_otAddr,
                             &addr_length );
  if ( m_nSocket <= 0 )
    {
    throw CSocketException( "Could not 'accept', new socket is invalid" );
    }

  timeval timeout;
  timeout.tv_sec  = 0;
  timeout.tv_usec = 1000; // looks like this value is ignored
  int nResult = ::setsockopt( m_nSocket,
                              SOL_SOCKET,
                              SO_RCVTIMEO,
                              (const char*)&timeout,
                              sizeof(timeout) );
  if ( nResult == -1 )
    {
    throw CSocketException( "Could not set SO_RCVTIMEO, new socket is invalid" );
    } // if ( result == -1 )

  } // CSocket::CSocket( const CSocket& src )

CSocket::~CSocket()
  {
  if ( isValid() )
    {
    ::close(m_nSocket);
    m_nSocket = INVALID_SOCKET;
    }
  } // CSocket::~CSocket()


void CSocket::Create()
  {
  m_nSocket = ::socket(AF_INET,
                     SOCK_STREAM,
                     0);
  if ( !isValid() )
    {
    std::cout << "errno == " << errno << "  in CSocket::Receive\n";
    throw CSocketException( "Could not create socket in CSocket::Create()." );
    }

  int on = 1;
  int nResult = ::setsockopt( m_nSocket,
                              SOL_SOCKET,
                              SO_REUSEADDR,
                              (void*)&on,
                              sizeof(on));
  if ( nResult == -1 )
    {
    throw CSocketException( "Could not 'setsockopt' SO_REUSEADDR in CSocket::Create()." );
    }
/*
  int off = 0;
  nResult = ::setsockopt( m_nSocket,
                          IPPROTO_TCP,
                          TCP_NODELAY,
                          (void*) &off,
                          sizeof(on) );
  if ( nResult == -1 )
    {
    throw CSocketException( "Could not 'setsockopt' TCP_NODELAY in CSocket::Create()." );
    }
*/
  } // void CSocket::Create()


void CSocket::Bind ( const int nPort )
  {
  if ( !isValid() )
    {
    throw CSocketException( "Socket is invalid in CSocket::Bind()." );
    }
  
  m_otAddr.sin_family      = AF_INET;
  m_otAddr.sin_addr.s_addr = INADDR_ANY; // inet_addr("127.0.0.1"); // INADDR_ANY;
  m_otAddr.sin_port        = htons ( nPort );
  
  int result = ::bind( m_nSocket,
                       m_otAddr,
                       m_otAddr.SizeGet() );
  if ( result == -1 )
    {
    throw CSocketException( "Could not bind to port in CSocket::Bind." );
    }
  } // void CSocket::Bind ( const int nPort )


void CSocket::Listen() const
  {
  if ( !isValid() )
    {
    throw CSocketException( "Socket is invalid in CSocket::Listen()." );
    }

  int result = ::listen( m_nSocket,
                         CLIENT_BACKLOG );
  if ( result == -1 )
    {
    throw CSocketException( "Could not listen to socket in CSocket::Listen()." );
    }
  } // void CSocket::Listen() const


CSocket* CSocket::Accept() const
  {
  return new CSocket( *this );
  } // CSocket* CSocket::Accept () const


void CSocket::Close( )
  {
  if ( !isValid() ) return;

  if ( ::close( m_nSocket ) == -1 )
    {
    throw CSocketException( "Could not 'close' in CSocket::Close()." );
    }

  m_nSocket = INVALID_SOCKET;
  } // void CSocket::Close( )


const CSocket& CSocket::operator << ( const std::string& s ) const
  {
  Send(s);
  return *this;
  }


const CSocket& CSocket::operator << ( const long n ) const
  {
  Send( to_string(n) );
  return *this;
  }


const CSocket& CSocket::operator >> ( std::string& s )
  {
  Receive( s );
  return *this;
  }


size_t CSocket::Send(const std::string& s) const
  {
  if ( !isValid() )
    {
    throw CSocketException( "Socket is invalid in CSocket::Send()." );
    }

  int status = ::send( m_nSocket,
                       s.c_str(),
                       s.length(),
                       0 );
  if ( status == -1 )
    {
    throw CSocketException( "Could not write to socket." );
    }

  return s.length();
  } // size_t CSocket::Send(const std::string& s) const


// Receive data from socket. To be used from >> operator
size_t CSocket::Receive( std::string& s )
  {
  if ( !isValid() )
    {
    throw CSocketException( "Socket is invalid in CSocket::Receive()." );
    }
  s.clear();

  ssize_t nResult = ::recv( m_nSocket,
                            m_ovBuffer,
                            m_ovBuffer.capacity() -1,
                            0 );
  if ( (nResult == -1) and ( errno != EAGAIN ) )
    {
    std::cout << "errno == " << errno << "  in CSocket::Receive\n";
    throw CSocketException( "Could not read from socket in CSocket::Receive()." );
    }
  else
    if ( nResult > 0 )
      {
      m_ovBuffer[(size_t)nResult] = 0;
      s = m_ovBuffer;
      }
  return nResult;
  } // const std::string& CSocket::Receive( std::string& s ) const


// Connect to the given server on the given port
// if successful, the connection is represented by an socket
void CSocket::Connect( const std::string& rsHost,
                       const std::string& rsPort )
  {
  Close();

  addrinfo  hints;
  addrinfo* servinfo;

  memset( &hints, 0, sizeof(hints) );
  hints.ai_family   = AF_UNSPEC;   // use AF_INET6 to force IPv6
  hints.ai_socktype = SOCK_STREAM;

  int nResult = ::getaddrinfo( rsHost.c_str(),
                               rsPort.c_str(),
                               &hints,
                               &servinfo );
  if ( nResult != 0 )
    {
    if ( m_bVerbose ) std::cerr << "getaddrinfo: " << gai_strerror( nResult ) << std::endl;
    throw CSocketException( "Could not find out host IP in CSocket::Connect()." );
    }

  // loop through all the results and connect to the first we can
  for( addrinfo* p = servinfo; p != NULL; p = p->ai_next)
    {
    if ( (m_nSocket = ::socket(p->ai_family,
                             p->ai_socktype,
                             p->ai_protocol)) == INVALID_SOCKET )
      {
      if ( m_bVerbose ) std::cout << "socket fail" << std::endl;
      continue;
      }
    if ( m_bVerbose ) std::cout << "socket ok" << std::endl;

    if ( ::connect(m_nSocket,
                   p->ai_addr,
                   p->ai_addrlen) == -1 )
      {
      // We are to close and invalidate a low level socket
      // so we must not call any derived Close() methode
      ::close( m_nSocket );
      m_nSocket = INVALID_SOCKET;
      if ( m_bVerbose ) std::cout << "connect fail" << std::endl;
      continue;
      }
    if ( m_bVerbose ) std::cout << "connect ok" << std::endl;

    break; // if we get here, we must have connected successfully
    }

  ::freeaddrinfo( servinfo );

  if ( !isValid() )
    {
    throw CSocketException( "Could not connect to host in CSocket::Connect()." );
    }

  } // void CSocket::Connect(const std::string& rsHost, ...


bool CSocket::isValid() const
  {
  return m_nSocket != INVALID_SOCKET;
  }  // bool CSocket::isValid() const
