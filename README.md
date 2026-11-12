# websocket_protocol
C++ class implementing server side websocket protocol 

## Build and install

cmake .. -DCMAKE_INSTALL_PREFIX=/some/path [-DENABLE_SHARED=ON]

make

make install


## How to use it

Assuming you can provide with a tcp server class that implement:
```
class CustonTcpServer
{
	public:

		bool waitForConnection();

		int sendData(const char* data, const size_t dataSize);

		int recvData(char* buffer, const unsigned int bufferSize);
}
```
Then, you can wrap your TCP server in a class as following

```
class WebsocketServer
{
	public:
		WebsocketServer(const char* ip, uint16_t port)
		: m_custonTcpServer{ip, port}
		{}
		
		void stop()
		{
			m_tcpServer.stop();
		}
		
		bool waitForConnection()
		{
			if(!m_custonTcpServer.waitForConnection()){
				return false;
			}

			constexpr int bufferSize{1024};

			char buffer[bufferSize]{};

			int recvBytes=m_custonTcpServer.recvData(buffer, bufferSize-1);
			if(recvBytes>0){
				buffer[recvBytes]=0;

				auto [data, length]=m_websocketProtocol.handShake(buffer);
				if(length>0){
					if(m_custonTcpServer.sendData(data, length)>0){
						return true;
					}
				}
			}

			return false;
		}
		
		std::string receiveData(char* buffer, const unsigned int bufferSize)
		{
			int dataSize=m_custonTcpServer.recvData(buffer, bufferSize);

			if(dataSize<1){
				return "";
			}
			auto [data, length]=m_websocketProtocol.unmaskMessage(buffer, dataSize);
			return std::string(data, length);	
		}

		int sendData(const std::string& msg)
		{
			return sendData(msg.c_str(), msg.length());
		}
		
		int sendData(const char* msg)
		{
			return sendData(msg, std::strlen(msg));
		}

	private:
		TCPServer m_custonTcpServer;
		WebsocketProtocol m_websocketProtocol;

		int sendData(const char* msg, size_t msgLength)
		{
			auto [data, length]=m_websocketProtocol.encodeMessage(msg, msgLength);
		
			return m_custonTcpServer.sendData(data, length);
		}
};

```
HTTP client can be like:
```
<html lang="en">
<head>
	<meta charset="UTF-8" />
	<title>Websocket Client</title>
	<style>
		div{
			sborder: 1px solid;
		}
	</style>
</head>
<body>
	<div id="message" style="border-style: solid;  margin:20px;">
	</div>
</body>
<script>
	var ws=new WebSocket("ws://ip:port");
	
	ws.onerror=function(event){
		// report error
	};
	
	ws.onopen=function(){
		//do something
	};
	
	ws.onmessage=function(event){
		document.getElementById('message').innerHTML=event.data;
	};
	
	//	ws.send(event.data);

</script>
</html>
```
