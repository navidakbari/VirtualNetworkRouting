# Virtual Network Routes

This project representing a virtual network router node. every node can connect to another node with routing algorithms and find their way to others. It uses [Distance Vector](https://en.wikipedia.org/wiki/Distance-vector_routing_protocol) routing protocol for routing and forwarding messages. It also supports `Traceroute` command to find nodes and interfaces.


## How to run

In project directory run 
```bash
vagrant up
```
vagrant configs are to run a virtual machine on your computer. You had to installed Virtual Box or other virtual machines.
after that you can connect to virtual machine using `vagrant ssh` command. 

**NOTE:** You also can make codes on your machine with standard c++ compiler `g++`.

You can write a network configs with bellow syntax. 
```
node <node_name> <physical_ip>
<node_name1> <-> <node_name2>
```
For example a net config for a tree could be something like this : 
```
node A localhost
node B localhost
node C localhost
node D localhost
node E localhost
A <-> B
B <-> C
C <-> D
C <-> E
```

After writing a config you should make some `lnx` file that will run with each node program. 
```bash
./tools/netconv config
```
After that some lnx file will be made. For example for node `A` in our tree nets we have something like this : 
```
localhost 5000
localhost 5001 192.168.0.1 192.168.0.2
localhost 5003 192.168.0.10 192.168.0.9
```

**NOTE:** There is some example net config in nets folder.

After that it is time to run each node with a `lnx` file.
first make codes : 
```bash
make
```
then for running that for each node run this command : 
```
./node <node_name>.lnx
```

## How to use
With help command you see how the whole node program works : 
```
> help
- help, h: Print this list of commands
- interfaces, li: Print information about each interface, one per line
- routes, lr: Print information about the route to each known destination, one per line
- up [integer]: Bring an interface "up" (it must be an existing interface, probably one you brought down)
- down [integer]: Bring an interface "down"
- send [ip] [protocol] [payload]: sends payload with protocol=protocol to virtual-ip ip
- traceroute [ip]: show traceroute
- q: quit this node
```

