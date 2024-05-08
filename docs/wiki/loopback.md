# Loopback

- Loopback (also written **loop-back**) **is the routing of electronic signals or digital data streams back to their source without intentional processing or modification**.
- It is primarily a means of testing the communications infrastructure.

- Loopback can take the form of communication channels with only one **communication endpoint**. Any message transmitted by such a channel is immediately and only received by that same channel.
- In telecommunications, loopback devices perform transmission tests of access lines from the serving switching center. **Loop around** is a method of testing between stations that are not necessarily adjacent, wherein two lines are used, with the test being done at one station and the two lines are interconnected at the distant station.

- Where a system (such as a modem) involves round-trip analog-to-digital processing, a distinction is made between **analog loopback**, where the analog signal is looped back directly, and **digital loopback**, where the signal is processed in the digital domain before being re-converted to an analog signal and returned to the source.

## 1. Telecommunications

## 2. Serial Interfaces

- A serial communications **transceiver** can use loopback for testing its functionality. For example, a device's **transmit pin** connected to its **receive pin** will result in the device receiving exactly what it transmits.

## 3. Virtual Loopback Interface

- `localhost`.
- Implementations of the Internet include a **virtual network interface** through which network applications can communicate when executing on the same machine. It is implemented entirely within the OS's networking software and passes no packets to any **network interface controller**. Any traffic that a computer program sends to loopback IP address is simply and immediately passed back up the network software stack as if it had been received from another device.

- Unix-like system usually name this loopback interface `lo` or `lo0`.

- Various **Internet Engineering Task Force (IETF)** standards reserve the IPv4 address block `127.0.0.0/8` and the IPv6 address `::1/128` for this purpose.

- The most common IPv4 address used is `127.0.0.1`. Commonly these loopback addresses are mapped to the host name `localhost`.

### 3.1. Management Interface

- Some computer network equipment use the term `loopback` for a virtual interface used for management purposes. Unlike a proper loopback interface, this type of loopback device is not used to talk with itself.

- Such an interface is assigned an address that can be accessed from management equipment over a network but is not assigned to any of the physical interfaces on the device. Such a loopback device is also used for management datagrams, such as alarms, originating from the equipment. The property that makes this virtual interface special is that applications that use it will send or receive traffic using the address assigned to the virtual interface as opposed to the address on the physical interface through which the traffic passes.
