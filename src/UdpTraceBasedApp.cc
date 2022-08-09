#include "UdpTraceBasedApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/lifecycle/ModuleOperations.h"

Define_Module(UdpTraceBasedApp);

void UdpTraceBasedApp::initialize(int stage)
{
    // initialize with UdpBasicApp
    UdpBasicApp::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *traceFile = par("traceFile");
        // read the timeStamp vector and in case it it empty or the file does not exist the module will not be started,
        // as specified in handleStartOperation method.
        parseTraceFile2Vector(traceFile,timeStamp);
        //////////////////////////////////////////////////////////////////////////
        // Allow multuple groundstations (Musab) 
        //////////////////////////////////////////////////////////////////////////
        multiGroundStationUsed = par("multiGroundStationUsed").boolValue();
        GSx = par("GSx");
        GSy = par("GSy");
        GSz = par("GSz");
        GS2x = par("GS2x");
        GS2y = par("GS2y");
        GS2z = par("GS2z");
        std::vector<std::vector<double>> locations
        {
            { GSx, GSy, GSz },
            { GS2x, GS2y, GS2z }
        };
        GSLocations = locations;
        if(multiGroundStationUsed){
            host = getContainingNode(this);
            mobility = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
        }
        //////////////////////////////////////////////////////////////////////////
        // Emit application packet sent Signal (Musab)
        //////////////////////////////////////////////////////////////////////////
        appPacketSentSignal = registerSignal("appPacketSent");
    }
}

void UdpTraceBasedApp::parseTraceFile2Vector(const char* filename, std::vector<double>& vecOfDoubles)
{
    std::vector<std::string> vecOfStrs;
    std::ifstream in(filename, std::ios::in);
    // Check if the file is opened (we modified the error message here to just  return in order to enable scripting the application)
    if (in.fail())
            return;
    std::string lineStr;
    // Read the file line by line until the end.
    while (std::getline(in, lineStr))
    {
        // Append the contents of a non-empty line to the vector of strings
        if (lineStr.size() > 0){
            double lineDouble = stod(lineStr);
            simtime_t lineStart = lineDouble;
            // only add line if it is within boundaries [startTime,stopTime]
            if ((lineStart >= startTime && (lineStart <= stopTime || stopTime == -1))){
                vecOfDoubles.push_back(lineDouble);
            // vecOfStrs.push_back(lineStr);
            }
        }
    }
    // Close The File
    in.close();
    // Assign the elements of vecOfStrs to vecOfDoubles after converting it from string to double
    // vecOfDoubles.resize(vecOfStrs.size());
    // std::transform(vecOfStrs.begin(), vecOfStrs.end(), vecOfDoubles.begin(),
    //     [](std::string const& val) {return stod(val); });
}

void UdpTraceBasedApp::handleStartOperation(LifecycleOperation *operation)
{
    if (timeStamp.empty()){
        return;
    }
    startTime = timeStamp.at(0);
    simtime_t start = std::max(startTime, simTime());
    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
}

void UdpTraceBasedApp::processStart(){
    // a counter for the index of the timeStamp vector
    timeIndex = 0;
    UdpBasicApp::processStart();
}

void UdpTraceBasedApp::processSend()
{
    sendPacket();
    //////////////////////////////////////////////////////////////////////////
    // Emit application packet sent Signal (Musab) (Musab)
    //////////////////////////////////////////////////////////////////////////
    emit(appPacketSentSignal, simTime());
    // Calculate the next packet transmission time from the timeStamp vector
    timeIndex = timeIndex + 1;
    // to ensure that the if statement will be checked if and only if the timeStamp has at least 2 elements.
    if (timeIndex < timeStamp.size()){
        simtime_t nextSendTime = simTime() + (timeStamp.at(timeIndex)  -timeStamp.at(timeIndex-1));
        selfMsg->setKind(SEND);
        scheduleAt(nextSendTime, selfMsg);
    } else {
        selfMsg->setKind(STOP);
        scheduleAt(simTime(), selfMsg);
    }

}

//////////////////////////////////////////////////////////////////////////
// Allow multuple groundstations (Musab) 
//////////////////////////////////////////////////////////////////////////
L3Address UdpTraceBasedApp::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    if(multiGroundStationUsed) {
        const Coord GroundStationLocation = Coord(GSx, GSy, GSz);
        const Coord GroundStationLocation2 = Coord(GS2x, GS2y, GS2z);
        m distanceToGroundStation = m(mobility->getCurrentPosition().distance(GroundStationLocation));
        m distanceToGroundStation2 = m(mobility->getCurrentPosition().distance(GroundStationLocation2));
        if (distanceToGroundStation < distanceToGroundStation2)
            return destAddresses[0];
        else
            return destAddresses[1];
        // for (int i = 0; i < GSLocations.size(); i++)
        // {
        //     EV_INFO << "Ground station (Destination) position x = " << GSLocations[i][0] << " y = " << GSLocations[i][1] << " z = " << GSLocations[i][2] << endl;
        // }
        // return;
    }
    if (destAddresses[k].isUnspecified() || destAddresses[k].isLinkLocal()) {
        L3AddressResolver().tryResolve(destAddressStr[k].c_str(), destAddresses[k]);
    }
    return destAddresses[k];
}

void UdpTraceBasedApp::sendPacket()
{
    std::ostringstream str;
    str << packetName << "-" << numSent;
    Packet *packet = new Packet(str.str().c_str());
    if(dontFragment)
        packet->addTag<FragmentationReq>()->setDontFragment(true);
    const auto& payload = makeShared<ApplicationPacket>();
    payload->setChunkLength(B(par("messageLength")));
    payload->setSequenceNumber(numSent);
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);
    L3Address destAddr = chooseDestAddr();
    emit(packetSentSignal, packet);
    socket.sendTo(packet, destAddr, destPort);
    numSent++;
}
