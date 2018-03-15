/******************************************************************************
Class: NetworkBase
Implements:
Author: Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description:
	A simple wrapper to encapsulate the core functionality provided by ENET library.

	In previous years, this would have been the class that handled all the sockets 
	directly. However, ENET can do this for us now.. so this wrapper class only really
	serves in keeping track of the network transfer rate in kilobits per second.

	ENET Overview
	----------------
	The ENET library is a nice low level library that sits directly on top of the sockets. 
	Automatically handling packet reliability, network throttling, packet splitting and
	other not so fun network tasks that previously made networking a pain to teach.

	The documentation is a little sparse, though in essecense each client will have it's own
	ENetHost object that manages all incoming and outgoing packets. An ENetHost can have one or
	more ENetPeer's connected at any given time which can communicated with via enet_peer_send(<packet with data>).

	The packets sent between clients can each optionally be sent via:
		- PACKET_TRANSPORT_UNRELABLE  (UDP)
		  These packets are a simple fire and forget. If they get lost over the network or dropped through throttling
		  we don't care. These are useful for time-sensitive data, which becomes obsolete after a given timeframe anyway.
		  In ENET this bypasses most of the library infrastructure and simply gets sent as a standard UDP datagram.

		- PACKET_TRANSPORT_RELIABLE  (~TCP)
		  These packets are tracked over the network with ACK's (acknowledgement identifier). Each packet to and from the peer's
		  will contain an additional ACK field informing the peer which packets it has previously recieved. If, after a given
		  timeframe, the peer has not recieved our packet (due to the ACK ID's not being contiguous) it will be re-sent.
		  This process continues until the peer acknoledges it has recieved our packet. This means we can make no assumptions
		  about when the packet will arrive, but we can assert that it /will/ arrive eventually. ENET accomplishes this by emulating
		  it's own variation of the TCP protocol on top of the existing UDP socket.

	For more information see: http://enet.bespin.org/Features.html

*//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <enet\enet.h>
#include <stdint.h>
#include <functional>

enum PacketTransportType
{
	//Packet will be sent, but may be lost in transit
	// - Best for regular updates that are time-sensitive e.g. position updates
	// - See UDP Protocol
	PACKET_TRANSPORT_UNRELABLE = 0,								
	
	//Packet will be sent and tracked, resending after timeout if peer has not recieved it yet
	// - Best for data that MUST reach the client, no matter how long (or how many attempts) it takes e.g. game start/end and player deaths
	// - See TCP Protocol
	PACKET_TRANSPORT_RELIABLE = ENET_PACKET_FLAG_RELIABLE
};

class NetworkBase
{
public:
	NetworkBase();
	~NetworkBase();

	//Initialze network
	// - external_port_number MUST be specified to allow peers to connect to this computer - default: use any available port number and disable incoming connections
	// - max_peers defines the maximum number of connected peers the host can support
	// - This MUST be called before sending/recieving any packets
	bool Initialize(uint16_t external_port_number = 0, size_t max_peers = 1);

	//Closes all connections/sockets and releases all associated memory
	// - Note: This is called automatically in destructor
	void Release();

	// Attempt to connect to a peer with a given IP4 Address:Port No
	// - Example usage: BeginConnect(127,0,0,1, 1234) to connect to localhost on port 1234
	// - Note: ENetPeer pointer is used to identify the peer and is needed to send/recieve packets to that computer
	ENetPeer* ConnectPeer(uint8_t ip_part1, uint8_t ip_part2, uint8_t ip_part3, uint8_t ip_part4, uint16_t port_number);


	// Enqueues data to be sent to the peer computer over the network.
	// - Note: All enqueued packets will automatically be sent the next time 'ServiceNetwork' is called
	void EnqueuePacket(ENetPeer* peer, PacketTransportType transport_type, void* packet_data, size_t data_length);


	// Processes all incoming packets and sends all enqueued outgoing packets
	// - All incoming packets and network events will be parsed through the given callback function in the form of an ENetEvent.
	//   Possible Event.type's include:
	//			ENET_EVENT_TYPE_CONNECT, returned when a peer is connected and has responded via 'ConnectPeer' or an external peer is attempting to connect to us and awaiting our confirmation
	//			ENET_EVENT_TYPE_RECEIVE, returned when a packet from a peer has been recieved and is awaiting processing
	//			ENET_EVENT_TYPE_DISCONNECT, returned when a peer has disconnected
	void ServiceNetwork(float dt, std::function<void(const ENetEvent&)> callback);


	// This manages our open sockets/channels used to send and recieve network packets
	ENetHost* m_pNetwork;

	//Profilling Information
	//	- Incoming and Outgoing Kilobytes transmitted over the last second
	float m_OutgoingKb;	///Usually refered to as Tx (Transmit rate)
	float m_IncomingKb;	///Usually refered to as Rx	(Recieve rate)

private:
	//Used to keep track of profilling transmit rate
	float m_SecondTimer;
};