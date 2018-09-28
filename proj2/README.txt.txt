Project	2: Implementing	Reliable Data Transfer Protocol
(CS3516	– A18)

Dan Duff
dfduff@wpi.edu
9/26/18

This project is a representation of the Alternating Bit Protocol used in a 
simulated network environment.
Messages are sent from A (sender) to B (reciever) using an RDT 3.0 system
of ACK/NAK confirmation.

If specified by the user, packets can be lost, corrupted, or sent out of order
to simulate the unreliability within the network layer. 

To Build the program:
By running either 'make' or 'make all' after navigating to the files' location
in terminal will build the project into an executable: altbit

To Run the program:
Run the program in terminal by executing the command './altbit'. This will start
the prompts from the program

Arguments for the project (from description)

Number of messages to simulate:
The emulator will stop as soon as this number of messages have been passed
down from layer 5, regardless of whether or not all of the messages have been correctly
delivered. Note that if you set this value to 1, your program will terminate
immediately, before the message is delivered to the other side. Thus, this value should always be
greater than 1.

Loss:
You are asked to specify a packet loss probability. A value of 0.1 would mean that one in ten
packets (on average) are lost and not delivered to the destination.

Corruption:
You are asked to specify a packet loss probability. A value of 0.2 would mean that two in ten
packets (on average) are corrupted. Note that the contents of payload, sequence, ack, or
checksum fields can be corrupted.

Out Of Order:
You are asked to specify an out-of-order probability. A value of 0.2 would mean that two in ten
packets (on average) are reordered.

Tracing:
Setting a tracing value of 1 or 2 will print out useful information about what is going on inside the
emulation (e.g., what's happening to packets and timers). A tracing value of 0 will turn this off. A
tracing value of 5 will display all sorts of odd messages that are for emulator-debugging
purposes. THE DEBUGGING LEVEL I SET FOR MY OWN ERROR MESSAGES IS 3.

Randomization:
The simulation works by using a random number generator to determine if packets will or will not
be modified in some fashion. Setting 0 here (no randomization) means that you will get the same
result for each of your runs.

Bidirectional:
Should always be 0 for this project.

How to Clean the project:
In terminal, run 'make clean' to remove the executable.

Tracefile:
The tracefile contains various emulation conditions