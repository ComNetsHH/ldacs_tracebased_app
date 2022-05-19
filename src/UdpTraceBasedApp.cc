#include "UdpTraceBasedApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
//////////////////////////////////////////////////////////////////////////
// Hop Count Signal (Musab)
//////////////////////////////////////////////////////////////////////////
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/TimeTag_m.h"



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
        // Emit application packet sent Signal (Musab)
        //////////////////////////////////////////////////////////////////////////
        appPacketSentSignal = registerSignal("appPacketSent");
        //////////////////////////////////////////////////////////////////////////
        // Register Hop Count Signal (Musab)
        //////////////////////////////////////////////////////////////////////////
        hopCountSignal = registerSignal("hopCount");
    }
}

void UdpTraceBasedApp::parseTraceFile2Vector(const char* filename, std::vector<double>& vecOfDoubles)
{
    std::vector<std::string> vecOfStrs;
    std::ifstream in(filename, std::ios::in);
    // Check if the file is opened (we modified the error message here to just  return in order to enable scripting the application)
    if (in.fail())
            return;
    std::string str;
    // Read the file line by line until the end.
    while (std::getline(in, str))
    {
        // Append the contents of a non-empty line to the vector of strings
        if (str.size() > 0)
            vecOfStrs.push_back(str);
    }
    // Close The File
    in.close();
    // Assign the elements of vecOfStrs to vecOfDoubles after converting it from string to double
    vecOfDoubles.resize(vecOfStrs.size());
    std::transform(vecOfStrs.begin(), vecOfStrs.end(), vecOfDoubles.begin(),
        [](std::string const& val) {return stod(val); });
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
    //////////////////////////////////////////////////////////////////////////
    // Emit Hop Count Signal (Musab)
    //////////////////////////////////////////////////////////////////////////
    // Initially create a packet with a hop count value of 0 (Musab)
    auto HopCountTag = packet->addTag<hopCountTag>();
    HopCountTag->setHopCount(0);
    EV_INFO << "Current hop count = " << HopCountTag->getHopCount() << endl;
    L3Address destAddr = chooseDestAddr();
    emit(packetSentSignal, packet);
    socket.sendTo(packet, destAddr, destPort);
    numSent++;
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

void UdpTraceBasedApp::processPacket(Packet *pk)
{
    emit(packetReceivedSignal, pk);
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    //////////////////////////////////////////////////////////////////////////
    // Emit Hop Count Signal (Musab)
    //////////////////////////////////////////////////////////////////////////
    auto HopCountTag = pk->getTag<hopCountTag>();
    EV_INFO << "Hop count for application packet = " << endl;
    EV_INFO << "Hop count for application packet = " << HopCountTag->getHopCount() << endl;
    delete pk;
    numReceived++;
}

void UdpTraceBasedApp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processPacket(packet);
}

void UdpTraceBasedApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case SEND:
                processSend();
                break;

            case STOP:
                processStop();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else
        socket.processMessage(msg);
}
