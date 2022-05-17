# Project Overview/Scenario

## Project and scenario description
This project involves implementing an air quality and weather data station across a Bluetooth Mesh network. A number of Thingy:52 devices will be connected via the mesh network, each of which implements the code from Practical 1 of the course (i.e. weather station functionality, collecting data on temperature, humidity, etc.). Each Thingy:52 device will also interface with a SEN54 sensor, which will provide additional air quality and weather data. 

The Thingy:52 devices (and accompanying SEN54 sensors) will be spaced far apart. This will allow for greater coverage of the region in question, similar to how a network of sensors would need to ensure coverage of the entirety of UQ. It also means that functionality of the mesh network is critical (as the separating distances between Thingy:52 devices are very large).

Users will be able to request a particular reading from a specified device in the network via a commandline interface on a PC. A base device (nRF52840 Dongle) connected to the PC will broadcast this request to a Thingy:52 device, which will then flood the network with the request packet. Once the reading has been received from the Thingy:52 device in question, it will be sent back to the PC and displayed for the user.

The data recorded by each sensor will also be displayed on a web dashboard viewer in real time.

## Key performance indicators

## System overview

## Sensor integration

## Wireless network communication

## Algorithm schemes
