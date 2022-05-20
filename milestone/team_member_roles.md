# Team Member List and Roles

## Harrison Cusack (45375008)
Harrison will be responsible for parts of establishing the Bluetooth Mesh connection between Thingy:52 devices. This will involve incorporating the Zephyr Bluetooth Mesh network driver file, enabling each Thingy:52 to 1) flood the mesh network with request packets (directed at a specified addressee) for sensor data, 2) ignore request packets if the request has already been received, and 3) gather and send back requested sensor data to the requestor. Furthermore, the code from Practical 1 (i.e. weather station functionality) will be ported over to the current project's code, and will be used to define a custom configuration server in the mesh code (replacing generic configuration servers in the example code, such as turning an LED on/off). This will allow for weather data to be retrieved from any Thingy:52 device in the mesh network. 


## Thanh Do (45062148)
Thanh will also be responsible for parts of establishing the Bluetooth Mesh connection. Specifically, he will design the message protocol used to communicate between the devices. He will decide the format in which messages need to be sent, as well as the action that the target device takes once it receives a message. Additionally, Thanh will be responsible for creating the web dashboard viewer, as well as the way in which individual nodes send data to the web dashboard. He is also responsible for retrieving data from the SEN54 sensor, and interfacing it with the Argon device via the I2C interface. Finally, Thanh will be engineering the topology of our mesh network, allowing for lowest possible latency whilst also maximising coverage.


## Team Photo
![fifth](https://github.com/HarrisonCusack/csse4011-green-medusa/blob/main/milestone/team_photo.PNG)

