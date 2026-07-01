/*********************************************************************
* class WebsocketProtocol                                     			*
*                                                                    *
* Date:    24-06-2026                                                *
* Author:  Dan Machado                                               *
*                                                                    *
* https://github.com/volatilflerovium                                *
**********************************************************************/
#ifndef _WEBSOCKET_PROTOCOL_H
#define _WEBSOCKET_PROTOCOL_H


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  #if defined(WSPDLL)
    #define EXPORT __declspec(dllexport)
  #elif defined(WSPLIB)
    #define EXPORT
  #else
    #define EXPORT __declspec(dllimport)
  #endif
#else
  #define EXPORT
#endif

#include <utility>

class WebsocketProtocol
{
	public:
		EXPORT WebsocketProtocol();
		EXPORT ~WebsocketProtocol();

		EXPORT std::pair<char*, int> handShake(const char* data);
		EXPORT std::pair<char*, int> unmaskMessage(char* msg, const int msgLength);
		EXPORT std::pair<unsigned char*, int> encodeMessage(const char* msg, const int msgLength);

	private:
		class WSP_imp;
		WSP_imp* m_protocol{nullptr};
};

#endif
