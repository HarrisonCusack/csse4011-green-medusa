# Project Overview/Scenario

## Project and scenario description
This project involves implementing an air quality and weather data station across a Bluetooth Mesh network. A number of Thingy:52 devices will be connected via the mesh network, each of which implements the code from Practical 1 of the course (i.e. weather station functionality, collecting data on temperature, humidity, etc.).

The Thingy:52 devices will be spaced far apart. This will allow for greater coverage of the region in question, similar to how a network of sensors would need to ensure coverage of the entirety of UQ. It also means that functionality of the mesh network is critical (as the separating distances between Thingy:52 devices are very large).

Users will be able to request a particular reading from a specified device in the network via a commandline interface on a PC. A base device (nRF52840 Dongle) connected to the PC will broadcast this request to a Thingy:52 device, which will then flood the network with the request packet. Once the reading has been received from the Thingy:52 device in question, it will be sent back to the PC and displayed for the user.

The data recorded by each sensor will also be displayed on a web dashboard viewer in real time.

## Key performance indicators
1. **Accuracy:** The air quality and weather data readings from the Thingy:52 is accurate - i.e. they closely match the 'real' values (15% leniency), and do not fluctuate extensively around these (15% leniency).
2. **Coverage:** The region in which readings can be attained is large (i.e. more space than a single classroom). This ensures that the Bluetooth Mesh network is functioning as expected.
3. **Data variety:** A variety of weather data related information can be requested from each sensor - i.e. at least four different types.
4. **Latency:** Upon a data request, the user does not have to wait long for the reading to be relayed back to them - i.e. less than 3 seconds. Furthermore, the web dashboard updates in close to realtime (i.e. within 5 seconds).
5. **Reliability:** Readings can be reliably obtained anytime - i.e. throughout the entire testing period, there are no software crashes.
6. **Presentation:** The commands that users type to obtain data readings are simple and intuitive. The data is relayed back to the user in a simple, interpretable way. The web dashboard is visually concise and well presented.


## System overview

**Block diagram of system**

![first](https://github.com/HarrisonCusack/csse4011-green-medusa/blob/main/milestone/hardware_diagram.png)

**Top level flowchart**

![second](https://github.com/HarrisonCusack/csse4011-green-medusa/blob/main/milestone/top_level_flowchart.png)

**Detailed software flowcharts for PC and mote**

![third](https://github.com/HarrisonCusack/csse4011-green-medusa/blob/main/milestone/detailed_flowcharts.png)




## Sensor integration
Sensors that will be used:
- Thingy:52
  - HTS221 Capacitive Digital Humidity Sensor
  - LPS22 MEMS Nano Pressure Sensor
  - Adafruit CCS811 Air Quality Sensor

Types of data required:
- Temperature
- Humidity
- Air pressures
- VOC index
- eCO2 Index

How the sensors will be integrated:
- Thingy:52 device includes all sensors and will be accessed using Zephyr sensor API in an identical fashion to Practical 1.
- The devices themselves will interact with each other by sending messages via configuration servers established in the mesh network. These configuration servers will be specific to a particular application key and network key (each device will be bound to the same application key).

## Wireless network communication
- The Thingy:52 devices will be connected amongst themselves via Bluetooth - specifically, the mesh network.
- nRF52840 Dongle will join the mesh network when in range and flood the bluetooth message through the mesh network until the specified mote device receives the message. 
This is done by sending the message to all nearby neighbours. When a mote receives a message, it checks if the message belongs to it before decided next action. If the message was for the mote, take a measurement depending on HCI and send back to original address through the mesh. If the message was not for the mote, send to nearest neighbour. Refer to diagram in system overview for a detailed software flowchart.
- The Thingy:52s will use configuration servers to communicate with each other. Specifically, a custom configuration server will be established which operates based on the message protocol (see below for the diagram). This configuration server will belong to a single attribute key that all devices are bound to.

**Message protocol diagram**

![fourth](https://github.com/HarrisonCusack/csse4011-green-medusa/blob/main/milestone/message_protocol_diagram.png)

The HCI format involves an array of bytes sent through bluetooth.
This involves a preamble, type, length and data.
![hci](https://github.com/HarrisonCusack/csse4011-green-medusa/blob/main/milestone/hci.png)



## Algorithm schemes
The Bluetooth Mesh algorithm scheme works as follows:

If a device receives a message, check if it has already received the message. If so, discard it. If not, then check if the message was addressed to itself. If so, process it appropriately (which may include constructing another message to flood the network with). If not, flood the network with the same message.

Furthermore, devices in range of the PC will send messages to it. The PC itself will process messages addressed to itself that it hasn't received before, and discard others.
