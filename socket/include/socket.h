/***************************************************************************
 socket.h
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


#ifndef _SOCKET_H
#define _SOCKET_H

#include <string>
#include <vector>

#include <arpa/inet.h>
#include <memory.h>


#include <sstream>
template <typename T>
  std::string to_string(const T& val)
    {
    std::ostringstream sout;
    sout << val;
    return sout.str();
    }


class CSocket
  {
  protected:
    class CAddrInet : public sockaddr_in
      {
      /*
       *  __uint8_t      sin_len;
       *  sa_family_t    sin_family;
       *  in_port_t      sin_port;
       *  struct in_addr sin_addr;
       *  char           sin_zero[8];
       */
      public:
        CAddrInet()
          {
          memset( (sockaddr_in*)this, 0, sizeof(sockaddr_in) );
          }
        operator sockaddr* () const
          {
          return (struct sockaddr*) this;
          }
        int SizeGet() const
          {
          return sizeof(struct sockaddr_in);
          }
      }; // class CAddrInet

    class CBuffer : public std::vector<char>
      {
      public:
        CBuffer()
          {
          resize( RECEIVE_BUFFER_SIZE );
          }
        operator char*()
          {
          return &front();
          }
      }; // class CBuffer

  protected:
    static const int       INVALID_SOCKET;
    static const int       CLIENT_BACKLOG;
    static const int       RECEIVE_BUFFER_SIZE;

                 int       m_nSocket;
                 CAddrInet m_otAddr;
                 CBuffer   m_ovBuffer;

                 bool      m_bVerbose;

             CSocket( const CSocket& src );
  public:
             CSocket( const int  nSocket  = INVALID_SOCKET,
                      const bool bVerbose = false );
    virtual ~CSocket();

    virtual void     Create  ();
    virtual void     Bind    ( const int           nPort );
    virtual void     Listen  ()                            const;
    virtual CSocket* Accept  ()                            const;
    virtual void     Connect ( const std::string& rsHost,
                               const std::string& rsPort );
    virtual void     Close   ();

    virtual size_t   Send    ( const std::string& s )     const;
    virtual size_t   Receive (       std::string& s );

  public:
    const CSocket& operator << ( const std::string& s ) const;
    const CSocket& operator << ( const long         n ) const;
    const CSocket& operator >> (       std::string& s );

    bool isValid() const;

  }; // class CSocket

#endif // _SOCKET_H
