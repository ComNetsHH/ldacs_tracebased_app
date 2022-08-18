#include "UdpTraceBasedApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/lifecycle/ModuleOperations.h"

Define_Module(UdpTraceBasedApp);

//////////////////////////////////////////////////////////////////////////
// Allow multuple groundstations (Musab) 
//////////////////////////////////////////////////////////////////////////
//Declaring Vector to store the ground station coordinates and ethernet


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
        //new code added here
        if (multiGroundStationUsed){
            host = getContainingNode(this);
            mobility = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
            const char *file_name = par("groundstationsTraceFile");
            parseGroundstationTraceFile2Vector(file_name);
        }
        
        // GSx = par("GSx");
        // GSy = par("GSy");
        // GSz = par("GSz");
        // GS2x = par("GS2x");
        // GS2y = par("GS2y");
        // GS2z = par("GS2z");
        // std::vector<std::vector<double>> locations
        // {
        //     { GSx, GSy, GSz },
        //     { GS2x, GS2y, GS2z }
        // };
        // GSLocations = locations;
        // if(multiGroundStationUsed){
        //     host = getContainingNode(this);
        //     mobility = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
        // }
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

//////////////////////////////////////////////////////////////////////////
// Allow multuple groundstations (Musab) 
//////////////////////////////////////////////////////////////////////////
////The following function reads a trace file to store the coordinates of the ground stations

void UdpTraceBasedApp::parseGroundstationTraceFile2Vector(const char* filename)
{
    std::vector<double> groundstationCoordinate;
    std::ifstream in(filename, std::ios::in);
    // Check if the file is opened (we modified the error message here to just  return in order to enable scripting the application)
    if (in.fail())
        return;
    std::string lineStr;
    std::string ethernetInterface;
    double x;
    double y;
    double z;
    std::istringstream iss;
    // Read the file line by line until the end.
    while (std::getline(in, lineStr))
    {
        std::istringstream iss(lineStr);
        iss >> x >> y >> z >> ethernetInterface;
        //std::cout << "x=" << x << ",y=" << y << ",z=" << z << ",interface=" << ethernetInterface << std::endl;
        // insert the x, y, z coordinates of one groundstation into groundstationCoordinate vector
        groundstationCoordinate.insert(groundstationCoordinate.end(), { x,y,z });
        ground_stations_coordinates_array.insert(ground_stations_coordinates_array.end(), { groundstationCoordinate });
        // clear the vector that contains a row of the grounstation trace file
        groundstationCoordinate.clear();
        // insert the groundstationCoordinate vector into the vector of vectors groundstationCoordinates
        ethernet_vector.insert(ethernet_vector.end(), { ethernetInterface });
    }
    for (int i = 0; i < ground_stations_coordinates_array.size(); i++){
        EV_INFO << "Ground station " << i << " location: " << ground_stations_coordinates_array.at(i).at(0) << ", " << ground_stations_coordinates_array.at(i).at(1) << ", " << ground_stations_coordinates_array.at(i).at(2) << endl;
    }

    // Close The File
    in.close();
}

//////////////////////////////////////////////////////////////////////////
// Allow multuple groundstations (Musab) 
//////////////////////////////////////////////////////////////////////////
////The following function calculates the distance to all ground stations and finds the closest one
int UdpTraceBasedApp::findClosestGroundStation()
{
    int closest_ground_station=0;
    //getting the position of the current aircraft
    // Coord aircraft_position = check_and_cast<IMobility *>(getContainingNode(this)->getSubmodule("mobility"))->getCurrentPosition();
    Coord aircraft_position = mobility->getCurrentPosition();
    EV << " aircraft_position: " << aircraft_position << " \n";
    
    std::vector<double> distance_vector;
    //applying the formula sqrt((x_2-x_1)^2+(y_2-y_1)^2+(z_2-z1)^2) to get the distance between aircraft and all ground stations
    double min_distance = sqrt(pow((aircraft_position.x-ground_stations_coordinates_array.at(0).at(0)),2) + pow((aircraft_position.y-ground_stations_coordinates_array.at(0).at(1)),2)+ pow((aircraft_position.z-ground_stations_coordinates_array.at(0).at(2)),2));    
    for(int i = 0; i < ground_stations_coordinates_array.size(); i++){
        double distance = sqrt(pow((aircraft_position.x-ground_stations_coordinates_array.at(i).at(0)),2) + pow((aircraft_position.y-ground_stations_coordinates_array.at(i).at(1)),2)+ pow((aircraft_position.z-ground_stations_coordinates_array.at(i).at(2)),2));
        // distance_vector.push_back(distance);
        distance_vector.insert(distance_vector.end(), { distance });

        //find the closest ground station
        if (distance < min_distance){
            min_distance = distance;
            closest_ground_station=i;
        }
    }
    EV << " closest_ground_station is: " << closest_ground_station << " \n";
    EV << " Corresponding Ethernet is: " << ethernet_vector.at(closest_ground_station) << " \n";
    //new code upto here
    return closest_ground_station;
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
        destination_index = findClosestGroundStation();
        return destAddresses[destination_index];
        // const Coord GroundStationLocation = Coord(GSx, GSy, GSz);
        // const Coord GroundStationLocation2 = Coord(GS2x, GS2y, GS2z);
        // m distanceToGroundStation = m(mobility->getCurrentPosition().distance(GroundStationLocation));
        // m distanceToGroundStation2 = m(mobility->getCurrentPosition().distance(GroundStationLocation2));
        // if (distanceToGroundStation < distanceToGroundStation2)
        //     return destAddresses[0];
        // else
        //     return destAddresses[1];
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
