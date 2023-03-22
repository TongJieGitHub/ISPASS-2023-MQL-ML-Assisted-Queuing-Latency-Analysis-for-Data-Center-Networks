/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//
// Copyright (c) 2006 Georgia Tech Research Corporation
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: George F. Riley<riley@ece.gatech.edu>
//

// ns3 - On/Off Data Source Application class
// George F. Riley, Georgia Tech, Spring 2007
// Adapted from ApplicationOnOff in GTNetS.
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <chrono>
#include <ctime>

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "load-trace.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/double.h"

using namespace std;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoadTrace");

NS_OBJECT_ENSURE_REGISTERED (LoadTrace);

TypeId
LoadTrace::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoadTrace")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<LoadTrace> ()
    .AddAttribute ("DataRate", "The data rate in on state.",
                   DataRateValue (DataRate ("500kb/s")),
                   MakeDataRateAccessor (&LoadTrace::m_cbrRate),
                   MakeDataRateChecker ())
    .AddAttribute ("PacketSize", "The size of packets sent in on state",
                   UintegerValue (512),
                   MakeUintegerAccessor (&LoadTrace::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&LoadTrace::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("OnTime", "A RandomVariableStream used to pick the duration of the 'On' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&LoadTrace::m_onTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("OffTime", "A RandomVariableStream used to pick the duration of the 'Off' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&LoadTrace::m_offTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("MaxBytes", 
                   "The total number of bytes to send. Once these bytes are sent, "
                   "no packet is sent again, even in on state. The value zero means "
                   "that there is no limit.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&LoadTrace::m_maxBytes),
                   MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("Protocol", "The type of protocol to use. This should be "
                   "a subclass of ns3::SocketFactory",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&LoadTrace::m_tid),
                   // This should check for SocketFactory as a parent
                   MakeTypeIdChecker ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&LoadTrace::m_txTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("TxWithAddresses", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&LoadTrace::m_txTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
    
    // Adding min-max packet size attributes
    .AddAttribute ("MinPacketSize", "The min size of packets sent in on state",
                   UintegerValue (512),
                   MakeUintegerAccessor (&LoadTrace::m_minPktSize),
                   MakeUintegerChecker<uint32_t> (1))

    .AddAttribute ("MaxPacketSize", "The max size of packets sent in on state",
                   UintegerValue (1536),
                   MakeUintegerAccessor (&LoadTrace::m_maxPktSize),
                   MakeUintegerChecker<uint32_t> (1))
    
    .AddAttribute ("packetSizeDistrubution", "Packet size distrubution type",
                   StringValue ("deterministic"),
                   MakeStringAccessor (&LoadTrace::m_pktSizeDistrubution),
                   MakeStringChecker())

    .AddAttribute ("p_burst", "The probility of burst",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&LoadTrace::p_burst),
                   MakeDoubleChecker<double> (0.0))
    
    .AddAttribute ("trace_name", "trace name",
                StringValue ("wireshark/trace_c0_s1.csv"),
                MakeStringAccessor (&LoadTrace::m_trace_name),
                MakeStringChecker())

  ;
  return tid;
}


LoadTrace::LoadTrace ()
  : m_socket (0),
    m_connected (false),
    m_residualBits (0),
    m_lastStartTime (Seconds (0)),
    m_totBytes (0)
{
  NS_LOG_FUNCTION (this);
  expRanVar = CreateObject<ExponentialRandomVariable> ();
  uniRanVar = CreateObject<UniformRandomVariable> ();
  packetSizeUni = CreateObject<UniformRandomVariable> ();
  packetSizeExp = CreateObject<ExponentialRandomVariable> ();
  uniRanVar->SetAttribute ("Min", DoubleValue (0.0));
  uniRanVar->SetAttribute ("Max", DoubleValue (1.0));
  state_burst = 0;
  sum_burst = 0;
  count_burst = 0;
  sum_packetsize = 0;
  sum_packetnum = 0;
  sum_time = 0;
}

LoadTrace::~LoadTrace()
{
  NS_LOG_FUNCTION (this);
}

void 
LoadTrace::SetMaxBytes (uint64_t maxBytes)
{
  NS_LOG_FUNCTION (this << maxBytes);
  m_maxBytes = maxBytes;
}

Ptr<Socket>
LoadTrace::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

int64_t 
LoadTrace::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_onTime->SetStream (stream);
  m_offTime->SetStream (stream + 1);
  return 2;
}

void
LoadTrace::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

// Application Methods
void LoadTrace::StartApplication () // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);
  m_trace_ifstream.open(m_trace_name, std::ifstream::in);
  if (!m_trace_ifstream.good()) {
    return;
  }
  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      if (Inet6SocketAddress::IsMatchingType (m_peer))
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
        }
      else if (InetSocketAddress::IsMatchingType (m_peer) ||
               PacketSocketAddress::IsMatchingType (m_peer))
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
        }
      m_socket->Connect (m_peer);
      m_socket->SetAllowBroadcast (true);
      m_socket->ShutdownRecv ();

      m_socket->SetConnectCallback (
        MakeCallback (&LoadTrace::ConnectionSucceeded, this),
        MakeCallback (&LoadTrace::ConnectionFailed, this));
    }
  m_cbrRateFailSafe = m_cbrRate;

  // Insure no pending event
  CancelEvents ();
  // If we are not yet connected, there is nothing to do here
  // The ConnectionComplete upcall will start timers at that time
  //if (!m_connected) return;
  ScheduleStartEvent ();
}

void LoadTrace::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  CancelEvents ();
  if(m_socket != 0)
    {
      m_socket->Close ();
    }
  else
    {
      NS_LOG_WARN ("LoadTrace found null socket to close in StopApplication");
    }
  }

void LoadTrace::CancelEvents ()
{
  NS_LOG_FUNCTION (this);

  if (m_sendEvent.IsRunning () && m_cbrRateFailSafe == m_cbrRate )
    { // Cancel the pending send packet event
      // Calculate residual bits since last packet sent
      Time delta (Simulator::Now () - m_lastStartTime);
      int64x64_t bits = delta.To (Time::S) * m_cbrRate.GetBitRate ();
      m_residualBits += bits.GetHigh ();
    }
  m_cbrRateFailSafe = m_cbrRate;
  Simulator::Cancel (m_sendEvent);
  Simulator::Cancel (m_startStopEvent);
}

// Event handlers
void LoadTrace::StartSending ()
{
  NS_LOG_FUNCTION (this);
  m_lastStartTime = Simulator::Now ();
  ScheduleNextTx ();  // Schedule the send packet event
  ScheduleStopEvent ();
}

void LoadTrace::StopSending ()
{
  NS_LOG_FUNCTION (this);
  CancelEvents ();

  ScheduleStartEvent ();
}

// Private helpers
void LoadTrace::ScheduleNextTx ()
{
  NS_LOG_FUNCTION (this);

  if (m_maxBytes == 0 || m_totBytes < m_maxBytes)
    {
      std::string line;
      std::vector<std::string> arr;
      if (!getline(m_trace_ifstream, line)) {
        StopApplication ();
        return;
      }

      arr = split(line, ",");

      // std::cout 
      //   << m_trace_ifstream.is_open() << ", "
      //   << m_trace_ifstream.good() << ", " 
      //   << m_trace_ifstream.eof() << ", " 
      //   << m_trace_ifstream.fail() << ", " 
      //   << m_trace_ifstream.bad() << ", " 
      //   << m_trace_ifstream.rdstate() << ", " 
      //   << line << endl;
      // std::cout << "str: "<< arr[0] << ", " << arr[1] << endl;
      stringstream converter_time(arr[0]);
      stringstream converter_length(arr[1]);

      double current_time;
      uint32_t current_length;
      converter_time >> current_time;
      converter_length >> current_length;
      // std::cout << "[" << m_trace_name << "] " << "value: "<< current_time << ", " << current_length << endl;
      
      m_pktSize = current_length; 
      uint32_t bits = m_pktSize * 8 - m_residualBits;
      NS_LOG_LOGIC ("bits = " << bits);

      Time nextTime;
      nextTime = Time(Seconds((current_time)));

      NS_LOG_LOGIC ("nextTime = " << nextTime);
      m_sendEvent = Simulator::Schedule (nextTime,
                                         &LoadTrace::SendPacket, this);
    }
  else
    { // All done, cancel any pending events
      StopApplication ();
    }
}

void LoadTrace::ScheduleStartEvent ()
{  // Schedules the event to start sending data (switch to the "On" state)
  NS_LOG_FUNCTION (this);

  Time offInterval = Seconds (m_offTime->GetValue ());
  NS_LOG_LOGIC ("start at " << offInterval);
  m_startStopEvent = Simulator::Schedule (offInterval, &LoadTrace::StartSending, this);
}

void LoadTrace::ScheduleStopEvent ()
{  // Schedules the event to stop sending data (switch to "Off" state)
  NS_LOG_FUNCTION (this);

  Time onInterval = Seconds (m_onTime->GetValue ());
  NS_LOG_LOGIC ("stop at " << onInterval);
  m_startStopEvent = Simulator::Schedule (onInterval, &LoadTrace::StopSending, this);
}


void LoadTrace::SendPacket ()
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());
  Ptr<Packet> packet = Create<Packet> (m_pktSize);
  m_txTrace (packet);
  m_socket->Send (packet);
  m_totBytes += m_pktSize;
  Address localAddress;
  m_socket->GetSockName (localAddress);
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s on-off application sent "
                   <<  packet->GetSize () << " bytes to "
                   << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
                   << " port " << InetSocketAddress::ConvertFrom (m_peer).GetPort ()
                   << " total Tx " << m_totBytes << " bytes");
      m_txTraceWithAddresses (packet, localAddress, InetSocketAddress::ConvertFrom (m_peer));
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s on-off application sent "
                   <<  packet->GetSize () << " bytes to "
                   << Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6 ()
                   << " port " << Inet6SocketAddress::ConvertFrom (m_peer).GetPort ()
                   << " total Tx " << m_totBytes << " bytes");
      m_txTraceWithAddresses (packet, localAddress, Inet6SocketAddress::ConvertFrom(m_peer));
    }
  m_lastStartTime = Simulator::Now ();
  m_residualBits = 0;
  ScheduleNextTx ();
}


void LoadTrace::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_connected = true;
}

void LoadTrace::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

std::vector<std::string> LoadTrace::split(std::string str, std::string sep)
{
	char *cstr = const_cast<char *>(str.c_str());
	char *current;
	std::vector<std::string> arr;
	current = strtok(cstr, sep.c_str());
	while (current != NULL)
	{
		arr.push_back(current);
		current = strtok(NULL, sep.c_str());
	}
	return arr;
}

} // Namespace ns3
