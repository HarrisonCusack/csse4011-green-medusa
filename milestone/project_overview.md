# Project Overview/Scenario

## Project and scenario description
This project involves implementing an air quality and weather data station across a Bluetooth Mesh network. A number of Thingy:52 devices will be connected via the mesh network, each of which implements the code from Practical 1 of the course (i.e. weather station functionality, collecting data on temperature, humidity, etc.).

The Thingy:52 devices will be spaced far apart. This will allow for greater coverage of the region in question, similar to how a network of sensors would need to ensure coverage of the entirety of UQ. It also means that functionality of the mesh network is critical (as the separating distances between Thingy:52 devices are very large).

Users will be able to request a particular reading from a specified device in the network via a commandline interface on a PC. A base device (nRF52840 Dongle) connected to the PC will broadcast this request to a Thingy:52 device, which will then flood the network with the request packet. Once the reading has been received from the Thingy:52 device in question, it will be sent back to the PC and displayed for the user.

The data recorded by each sensor will also be displayed on a web dashboard viewer in real time.

## Key performance indicators
1. **Accuracy:** The air quality and weather data readings from the Thingy:52 is accurate - i.e. they closely match the 'real' values (15% leniency), and do not fluctuate extensively around these (15% leniency).
2. **Coverage:** The region in which readings can be attained is large (i.e. more space than a single classroom). This ensures that the Bluetooth Mesh network is functioning as expected.
3. **Data variety:** A variety of weather data related information can be requested from each sensor - i.e. more than four different types.
4. **Latency:** Upon a data request, the user does not have to wait long for the reading to be relayed back to them - i.e. less than 3 seconds. Furthermore, the web dashboard updates in close to realtime (i.e. within 5 seconds).
5. **Reliability:** Readings can be reliably obtained anytime - i.e. throughout the entire testing period, there are no software crashes.
6. **Presentation:** The commands that users type to obtain data readings are simple and intuitive. The data is relayed back to the user in a simple, interpretable way. The web dashboard is visually concise and well presented.


## System overview
**Detailed software flowcharts for PC and mote**

![first](https://github.com/HarrisonCusack/csse4011-green-medusa/blob/main/milestone/detailed_flowcharts.png)

**Top level flowchart**

![second](https://github.com/HarrisonCusack/csse4011-green-medusa/blob/main/milestone/top_level_flowchart.png)

**Block diagram of system**

![third](https://github.com/HarrisonCusack/csse4011-green-medusa/blob/main/milestone/hardware_diagram.png)

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
- nRF52840 Dongle connects to Thingy:52 via Bluetooth Mesh
- The Thingy:52 devices will connected amongst themselves via Bluetooth - specifically, the mesh network. **NEED MORE INFO HERE ABOUT HOW THE COMMUNICATION ACTUALLY HAPPENS. IS IT VIA GATT?**

## Algorithm schemes
The Bluetooth Mesh algorithm scheme works as follows:

If a device receives a message, check if it has already received the message. If so, discard it. If not, then check if the message was addressed to itself. If so, process it appropriately (which may include constructing another message to flood the network with). If not, flood the network with the same message.

Furthermore, devices in range of the PC will send messages to it. The PC itself will process messages addressed to itself that it hasn't received before, and discard others.
