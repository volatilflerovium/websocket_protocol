/*********************************************************************
* class WebsocketProtocol::WSP_imp                           			*
*                                                                    *
* Date:    24-06-2026                                                *
* Author:  Dan Machado                                               *
*                                                                    *
* https://github.com/volatilflerovium                                *
**********************************************************************/
#include "WebsocketProtocol/websocket_protocol.h"
#include "sha1/sha1.hpp"

//----------------------------------------------------------------------

class WebsocketProtocol::WSP_imp
{
	public:
		WSP_imp()=default;
		
		~WSP_imp()
		{
			if(m_encode64){
				delete[] m_encode64;
			}
			
			if(m_pack){
				delete[] m_pack;
			}

			if(m_umkData){
				delete[] m_umkData;
			}
		}

		std::pair<char*, int> handShake(const char* data);
		std::pair<char*, int> unmask(char* data, const int dataLength);
		std::pair<unsigned char*, int> encodeData(const char* text, const int dataLength);

	private:	
		unsigned char* m_pack{nullptr};
		int m_packSize{0};
		char m_hashSha1[21];
		char* m_encode64{nullptr};
		
		char* m_umkData{nullptr};
		int m_umkDataSize{0};
		
		int m_buffer64Size{0};
		int m_dataLength{0};

		void encoder(unsigned char a, unsigned char b, unsigned char c, int j);		
		std::pair<char*, int> base64Encode();
		void rawHash(const char* part1);
};

//====================================================================

namespace
{
	constexpr int _length(const char* data)
	{
		int i=0;
		while(data[i++]!='\0');
		return i-1;
	}

	constexpr const char* PROTOCOL_KEY="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	constexpr const char* c_base64Index="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	constexpr const char* PROTOCOL_HEADER="HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";
	constexpr int headerLength=_length(PROTOCOL_HEADER)+4;
}
//----------------------------------------------------------------------

void WebsocketProtocol::WSP_imp::encoder(unsigned char a, unsigned char b, unsigned char c, int j)
{
	m_encode64[j++]=c_base64Index[a>>2];
	m_encode64[j++]=c_base64Index[((a&3)<<4) | (b>>4)];
	m_encode64[j++]=c_base64Index[((b&15)<<2) | (c>>6)];				
	m_encode64[j++]=c_base64Index[c & 63];		
}

//----------------------------------------------------------------------

void WebsocketProtocol::WSP_imp::rawHash(const char* part1)
{
	static SHA1 m_sha1;
	m_sha1.update(part1);
	m_sha1.update(PROTOCOL_KEY);
	std::string str=m_sha1.final();

	m_hashSha1[0]=0;
	unsigned char a;
	unsigned int j=0;
	for(unsigned int i=0; i<str.length(); i++){
		a=static_cast<unsigned char>(str[i]);
		if(a>87){
			a-=87;
		}
		else{
			a-=48;
		}

		j=i>>1;
		if((i&1)==0){
			m_hashSha1[j]=a<<4;
		}
		else{
			m_hashSha1[j]=m_hashSha1[j] | a;
		}
	}
	m_hashSha1[20]=0;
}

//----------------------------------------------------------------------

std::pair<char*, int> WebsocketProtocol::WSP_imp::base64Encode()
{	
	const int size=20;
	const int tripples=size/3;
	int bufferSize=1+(tripples+1)*4+headerLength;

	if(m_buffer64Size<bufferSize){
		delete[] m_encode64;
		m_buffer64Size=bufferSize;
		m_encode64=new char[m_buffer64Size];
	}

	std::memcpy(m_encode64, PROTOCOL_HEADER, headerLength-4);

	int i=0, j=headerLength-4;
	for(; i<tripples; i++){
		encoder(m_hashSha1[3*i], m_hashSha1[3*i+1], m_hashSha1[3*i+2], j);
		j+=4;
	}

	int k=size-3*tripples;
	if(k>0){
		if(k==1){
			encoder(m_hashSha1[3*i], 0x00, 0x00, j);
			j+=2;
			m_encode64[j++]='=';
			m_encode64[j++]='=';
		}
		else{
			encoder(m_hashSha1[3*i], m_hashSha1[3*i+1], 0x00, j);
			j+=3;
			m_encode64[j++]='=';
		}
	}

	m_encode64[j++]='\r';
	m_encode64[j++]='\n';
	m_encode64[j++]='\r';
	m_encode64[j++]='\n';
	m_encode64[j]=0;

	return std::pair<char*, int>(m_encode64, j);
}

//----------------------------------------------------------------------

std::pair<char*, int> WebsocketProtocol::WSP_imp::unmask(char* data, const int dataLength)
{
	if(dataLength<1){
		return std::pair<char*, int>(nullptr, 0);
	}

	if(m_umkDataSize<dataLength){
		if(m_umkData){
			delete m_umkData;
		}
		m_umkDataSize=dataLength;
		m_umkData=new char[m_umkDataSize];
	}

	unsigned int length=static_cast<int>(data[1]) & 127;

	int mask=2;
	int offset=6;
	if(length==126){
		mask=4;
		offset=8;
	}
	else if(length==127){
		mask=10;
		offset=14;
	}

	if(dataLength<offset){
		return std::pair<char*, int>(nullptr, 0);
	}

	for(int i=0; i<dataLength-offset; i++) {
		m_umkData[i]=data[offset+i]^data[mask+(i%4)];
	}
	return std::pair<char*, int>(m_umkData, dataLength-offset);
	//return result;
}

//----------------------------------------------------------------------

std::pair<unsigned char*, int> WebsocketProtocol::WSP_imp::encodeData(const char* data, const int dataLength)
{
	const unsigned char b1=0x80 | (0x1 & 0x0f);
	m_dataLength=0;
	
	if(m_packSize<dataLength+8){
		if(m_pack){
			delete[] m_pack;
		}
		m_packSize=dataLength+8;
		m_pack=new unsigned char[m_packSize];
	}
	
	m_pack[0]=static_cast<unsigned char>(b1);

	if(dataLength<=125){
		m_pack[1]=static_cast<unsigned char>(dataLength) & ~(1<<7);		
		m_dataLength=2;
	}
	else if(dataLength<65536){
		m_pack[1]=126;
		m_pack[2]=static_cast<unsigned char>(dataLength>>8);
		m_pack[3]=static_cast<unsigned char>(dataLength);

		m_dataLength=4;
	}
	else{// if(length>=65536)
		m_pack[1]=static_cast<unsigned char>(127);
		
		uint32_t d=static_cast<uint32_t>(dataLength);
		unsigned char data[4];
		std::memcpy(data, &d, sizeof(d));

		m_pack[2]=data[3];
		m_pack[3]=data[2];
		m_pack[4]=data[1];
		m_pack[5]=data[0];

		m_dataLength=6;
	}

	std::memcpy(&m_pack[m_dataLength], data, dataLength*sizeof(char));
	m_dataLength+=dataLength;
	m_pack[m_dataLength]=0;
	
	return std::pair<unsigned char*, int>(m_pack, m_dataLength);
}

//----------------------------------------------------------------------

std::pair<char*, int> WebsocketProtocol::WSP_imp::handShake(const char* data)
{
	static std::string headers;
	headers.clear();
	headers=data;

	std::size_t found=headers.find("Sec-WebSocket-Key:");
	if(found!=std::string::npos){
		found=headers.find(":", found);
		if(found!=std::string::npos){
			std::size_t found2=headers.find("\r\n", found);
			if(found2!=std::string::npos){
			
				/*std::string acceptKey=headers.substr(found+2, found2-found-2);
				rawHash(acceptKey.c_str());
				*/
				
				headers[found2]=0;
				const char* dd=&headers[found+2];

				rawHash(dd);

				return base64Encode();
			}
		}
	}

	return std::pair<char*, int>(nullptr, 0);
}


//======================================================================

WebsocketProtocol::WebsocketProtocol()
:m_protocol{new WebsocketProtocol::WSP_imp}
{	
}

WebsocketProtocol::~WebsocketProtocol()
{
	if(m_protocol){
		delete m_protocol;
		m_protocol=nullptr;
	}
}

std::pair<char*, int> WebsocketProtocol::handShake(const char* data)
{
	return m_protocol->handShake(data);
}

std::pair<char*, int> WebsocketProtocol::unmaskMessage(char* msg, const int msgLength)
{
	return m_protocol->unmask(msg, msgLength);
}

std::pair<unsigned char*, int> WebsocketProtocol::encodeMessage(const char* msg, const int msgLength)
{
	return m_protocol->encodeData(msg, msgLength);
}

//======================================================================
