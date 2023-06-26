[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.8082929.svg)](https://doi.org/10.5281/zenodo.8082929)

    The L-Band Digital Aeronautical Communications System (LDACS) Trace-Based App implements a trace-based application in OMNeT++ for the LDACS Air-Air Medium Access Control simulator.
    Copyright (C) 2023  Sebastian Lindner, Konrad Fuger, Musab Ahmed Eltayeb Ahmed, Andreas Timm-Giel, Institute of Communication Networks, Hamburg University of Technology, Hamburg, Germany

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

This repository is part of the L-Band Digital Aeronautical Communications System (LDACS) Air-Air (A/A) mode simulator that implements the proposed Medium Access Control (MAC) protocol "Multi Channel Self-Organized TDMA (MCSOTDMA)".

This repository implements a trace-based application for OMNeT++ that allows trace-file-based application definitions.
For example, it can realize realistic Air Traffic Control (ATC) communications based on trace files that describe these.


# UdpTraceBasedApp
OMNeT++ Trace-Based Data Traffic Application extends the INET's `UdpBasicApp` by using a trace file to trigger the packet transmission. The file is a plain text file, where every line describes the trigger time of one packet, e.g., 

- t<sub>1</sub>      
- t<sub>2</sub> 
- ...
- t<sub>last</sub> 

The `UdpTraceBasedApp` application send the first packet at **t<sub>1</sub>** and the last packet at **t<sub>last</sub>**. 
## Getting Started
1. Please download [inet-4.2.5](https://github.com/inet-framework/inet/tree/v4.2.5) for OMNeT++ 5.6.2 and import it into the simulator.
2. Please clone this repo `UdpTraceBasedApp` and import it into OMNet++. To build it you need to do the following steps:
    1. `inet` must be set as a project reference. This is done by right-clicking on the `UdpTraceBasedApp` project in the Project Explorer and navigate to `Properties-> Project References -> Select inet -> Apply and Close`
    2. Ensure the build modes of this project and `inet` are identical (both `release` or both `debug`): This is done by right-clicking on each project in the Project Explorer and navigating to `Build Configuration -> Set Active -> <mode>`
    3. The `include paths` from INET must be added to this project such that you can write classes in your project which inherit `inet` classes. For that, you need to right-click on the `UdpTraceBasedApp` in the Project Explorer and navigate to `Properties -> OMNeT++ -> Makemake`, select the `src` folder in the list and then click "Options" in the right panel. 
    4. Go to the `Target` tab and make sure that `Shared library ...` is checked.
    5. Now go to the `Compile` tab and make sure all checkboxes are checked. 
    6. Move to the `Link` tab and make sure that the `Link with libraries exported from...` checkbox is checked. 
    7. Finally, `Apply and Close` and you should be able to `build` the `UdpTraceBasedApp` project without errors as a project that reference the INET project.

