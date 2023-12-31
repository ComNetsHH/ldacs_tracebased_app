// The L-Band Digital Aeronautical Communications System (LDACS) Trace-Based App implements a trace-based application in OMNeT++ for the LDACS Air-Air Medium Access Control simulator.
// Copyright (C) 2023  Sebastian Lindner, Konrad Fuger, Musab Ahmed Eltayeb Ahmed, Andreas Timm-Giel, Institute of Communication Networks, Hamburg University of Technology, Hamburg, Germany
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

import inet.applications.udpapp.UdpBasicApp;

package data_traffic_applications;

// Sends UDP packets to the given IP address/es. 
// It is compatible with both ~Ipv4 and ~Ipv6.
// It Inherits its main functionality from ~UdpBasicApp with the addition feature 
// of readng traces to trigger packet transmissions.
// ~UdpTraceBasedApp excluded the usage of startTime, stopTime and sendInterval used in ~UdpBasicApp.
// @see ~UdpBasicApp.
//
// ~UdpTraceBasedApp Uses a trace file to trigger the packet transmission.
// The file is a plain text file, where every line describes the trigger
// time of one packet, e.g., 
//
// - t1      
// - t2
// - ...
// - tLast
//
// The application send the first packet at t1 and the last packet at tLast.
// The peer can be a ~UdpSink, another ~UdpBasicApp (it handles received packets
// like ~UdpSink), or a ~UdpEchoApp. When used with ~UdpEchoApp, the rcvdPkLifetime
// statistic will contain the round-trip times.
// Received packets are discarded.
//
//
simple UdpTraceBasedApp extends UdpBasicApp
{
    parameters:
        string traceFile; // The path to the trace file written as "path"
        packetName = default("UdpTraceBasedApp"); 
        sendInterval = default(-1s); // the parameter sendInterval is not used in this application 
        //////////////////////////////////////////////////////////////////////////
        // Allow multuple groundstations (Musab) 
        //////////////////////////////////////////////////////////////////////////
        bool multiGroundStationUsed = default(false);
        string groundstationsTraceFile = default(""); //Name of the Trace file where ground_stations_coordinates along with corresponding ethernet are given
        // double GSx @unit(m) = default(4734222.285m);
        // double GSy @unit(m) = default(1381949.583m);
        // double GSz @unit(m) = default(662813.2938m);
        // double GS2x @unit(m) = default(1588538.41038296m);
        // double GS2y @unit(m) = default(1246029.47871449m);
        // double GS2z @unit(m) = default(665146.321414376m);
        @class(UdpTraceBasedApp);
        //////////////////////////////////////////////////////////////////////////
        // Emit application packet sent Signal (Musab)
        //////////////////////////////////////////////////////////////////////////
	    @signal[appPacketSent](type=simtime_t);
        @statistic[appPacketSent](source=appPacketSent; record=vector);
}
