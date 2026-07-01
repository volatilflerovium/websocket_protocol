# websocket_protocol
C++ class implementing websocket protocol

## Build and install

cmake .. -DCMAKE_INSTALL_PREFIX=/some/path [-DENABLE_SHARED=ON]

make

make install


## How to use it

The workflow should be as follows
```
TCP_server.wait_for_connections

// do handshake

recived_bytes=TCP_server.recv(buffer, buffersize);
auto [data, length]=m_webSocketProtocolPtr->handShake(buf.data());
if(length<1)
{
		error;
		return 1;
}

if(TCP_server.send(data, length, 0)<1)
{
	error;
	return 1;
}

// Then we need to unmaks every message received from the client:

{
   recived_bytes=TCP_server.recv(buffer, buffersize);
	auto [data, length]=m_webSocketProtocolPtr->unmaskMessage(buffer, recived_bytes);
}

// And we need to encode every message we want to send to the client:

{
	auto [pack, length]=m_webSocketProtocolPtr->encodeData(message, messageLength);
	TCP_server.send(pack, length);
}	

```