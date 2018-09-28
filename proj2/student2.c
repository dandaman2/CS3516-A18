#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project2.h"
 
/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/



/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */

//General Variables
extern int TraceLevel;

//ACK and NAK message codes
char ACKcode[MESSAGE_LENGTH] = {'A', 'C', 'K', 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
char NAKcode[MESSAGE_LENGTH] = {'N', 'A', 'K', 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//prototypes
int gimme_Checksum(struct pkt packet);
int is_Corrupt(struct pkt packet);
int is_good_ack_A(struct pkt packet);
int is_good_pkt_B(struct pkt packet);
void queue_msg(char message[20]);
char *dequeue_msg();
void mns_PacketA();

//Sender Checking variables
struct pkt lastpkt_transmitted;

//Sender queue Node variables/structure
struct Node* front = NULL;
struct Node* rear = NULL;
struct Node {
  char data[20];
  struct Node* next;
};

//reciever variables
int comp_seqnum;

/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {

	//If timer is running, put message data in the queue
	if(getTimerStatus(AEntity)){
		queue_msg(message.data);
	}
	else{
		//Create and send the packet
		struct pkt sndpkt;
		int seqnum;

		seqnum = (lastpkt_transmitted.seqnum == 0)? 1 : 0;

		sndpkt.seqnum = seqnum;
		sndpkt.acknum = 0;
		strncpy(sndpkt.payload, message.data, MESSAGE_LENGTH);
		sndpkt.checksum = gimme_Checksum(sndpkt);


		if(TraceLevel >=3){
			printf("A sending packet with seqnum %d and checksum %d\n ", seqnum, sndpkt.checksum);
		}
		//Save data about last packet sent
		lastpkt_transmitted.seqnum = sndpkt.seqnum;
		lastpkt_transmitted.acknum = sndpkt.acknum;
		strncpy(lastpkt_transmitted.payload, message.data, MESSAGE_LENGTH);
		lastpkt_transmitted.checksum = gimme_Checksum(lastpkt_transmitted);

		if(TraceLevel >=3) printf("A sending packet to B\n");
		tolayer3(AEntity, sndpkt);
		startTimer(AEntity, 500);
	}
}

/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message)  {
	//no bi-directional so not necessary
}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {
	stopTimer(AEntity);

	if(TraceLevel >= 3) printf("Recieved an ACK/NAK...Checking response...\n");

	if(is_good_ack_A(packet)){
		if(TraceLevel >= 3){
			printf("Got an ACK snACK that smiles bACK!\n");
			printf("Sending next data from the queue!\n");
		}
		//make and send the next data from the queue
		mns_PacketA();
	}
	else{
		//ACK was not recieved properly, so resend data
		if(is_Corrupt(packet)){

			if(TraceLevel >= 3) printf("A: no valid ACK, got %s instead with corruption. Resending packet with checksum %d and data %.20s.\n",
			packet.payload, lastpkt_transmitted.checksum, lastpkt_transmitted.payload);
			tolayer3(AEntity, lastpkt_transmitted);
			startTimer(AEntity, 500);

		}
		else{
			if(packet.seqnum != lastpkt_transmitted.seqnum){
				//Wrong sequence number, send next packet
				if(TraceLevel >=3) printf("Not corrupt, but bad sequence number, sending next boi\n");
				mns_PacketA();
			}
			else{
				if(TraceLevel >=3) printf("Packet with checksum %d and data %.20s corrupted from A to B, resending\n",lastpkt_transmitted.checksum, lastpkt_transmitted.payload);
				tolayer3(AEntity, lastpkt_transmitted);
				startTimer(AEntity, 500);
			}

		}

	}
}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
	if(TraceLevel >= 3) printf("A timed out. Sending packet with seqnum %d, and checksum %d again\n",
	lastpkt_transmitted.seqnum, lastpkt_transmitted.checksum);
	tolayer3(AEntity, lastpkt_transmitted);
	startTimer(AEntity, 500);
}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
	//For recording the last transmitted packet
	lastpkt_transmitted.seqnum = 1;
	lastpkt_transmitted.acknum = 0;
	for(int i = 0; i < MESSAGE_LENGTH; i++){
		lastpkt_transmitted.payload[i] = 0;
	}
	lastpkt_transmitted.checksum = 0;
}


/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {
	struct pkt backpkt;
	struct msg smessage;
	if(TraceLevel >= 3) printf("B gettin' a packet with seqnum %d, and checksum %d\n", packet.seqnum, packet.checksum);
	if(is_good_pkt_B(packet)){
		backpkt.seqnum = packet.seqnum;
		backpkt.acknum = 1;
		for(int i = 0; i < MESSAGE_LENGTH; i++){
		      backpkt.payload[i] = ACKcode[i];
		}
		backpkt.checksum = gimme_Checksum(backpkt);

		if(TraceLevel >= 3) printf("B Sending bACK an ACK with checksum: %d\n", backpkt.checksum);
		tolayer3(BEntity, backpkt);

		for(int j = 0; j < MESSAGE_LENGTH; j++){
		      smessage.data[j] = packet.payload[j];
		}


		if(TraceLevel >= 4) printf("B sending message up to layer 5 and updating newest recieved data\n");
		tolayer5(BEntity, smessage);


		if(TraceLevel >= 3) printf("Setting comp_seqnum from %d", comp_seqnum);
		//set the seqnum to the next expected value
		comp_seqnum = 1 - comp_seqnum;

		if(TraceLevel >= 3) printf(" to %d\n", comp_seqnum);
	}
	else{
		backpkt.seqnum = comp_seqnum;
		backpkt.acknum = 0;
		for(int i = 0; i < MESSAGE_LENGTH; i++){
			backpkt.payload[i] = NAKcode[i];
		}
		backpkt.checksum = gimme_Checksum(backpkt);
		if(TraceLevel >= 3) printf("Sending bACK a NAK!\n");
		tolayer3(BEntity, backpkt);

	}
}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
	comp_seqnum = 0;
}


//Creates and sends a packet from A to B
void mns_PacketA(){
	//Create and send the packet
	char* next_msg = dequeue_msg();
	if(next_msg == NULL){
		//return if no other messages in the queue
		return;
	}
		//Send new packet and restart timer
	struct pkt sndpkt;
	int seqnum;
	seqnum = (lastpkt_transmitted.seqnum == 0)? 1 : 0;

	sndpkt.seqnum = seqnum;
	sndpkt.acknum = 0;
	strncpy(sndpkt.payload, next_msg, MESSAGE_LENGTH);
	sndpkt.checksum = gimme_Checksum(sndpkt);
	if(TraceLevel >=3){
		printf("A sending packet with seqnum %d and checksum %d from the queue\n ", seqnum, sndpkt.checksum);
	}
	//Save data about last packet sent
	lastpkt_transmitted.seqnum = sndpkt.seqnum;
	lastpkt_transmitted.acknum = sndpkt.acknum;
	strncpy(lastpkt_transmitted.payload, sndpkt.payload, MESSAGE_LENGTH);
	lastpkt_transmitted.checksum = sndpkt.checksum;

	if(TraceLevel >=3) printf("A sending packet to B\n");
	tolayer3(AEntity, sndpkt);
	startTimer(AEntity, 500);
}

//determines the checksum based on the seqnum, acknum, and data of a packet
int gimme_Checksum(struct pkt packet){
	int checksum=0;
	checksum-=2* packet.acknum-1;
	checksum+=3* packet.seqnum+2;
	if(packet.payload==NULL){
		return checksum;
	}
	int i;
	for(i=1;i<MESSAGE_LENGTH+1;i++){
		checksum+=(int)packet.payload[i-1]*i*i-4;
	}
	return checksum;
}

//returns 1 if packet is corrupt (checksums dont match), 0 if checksums match
int is_Corrupt(struct pkt packet){
	int calc_checksum = gimme_Checksum(packet);
	if(calc_checksum == packet.checksum){
		return 0;
	}
	else{
		if(TraceLevel >= 3) {
			printf("Packet Corrupt!\n");
			printf("Recieved checksum: %d, seqnum: %d, acknum: %d\n", packet.checksum, packet.seqnum, packet.acknum);
			printf("Recieved packet data is: %.20s\n", packet.payload);
			printf("Expected checksum: %d\n", calc_checksum);
		}

		return 1;
	}
}

//determines whether an ACK has been recieved with/without errors for A side
//with errors: return 0		without errors: return 1
int is_good_ack_A(struct pkt packet){
	if((is_Corrupt(packet) == 0) && (strncmp(packet.payload, ACKcode, strlen(ACKcode)) == 0) &&
			(packet.seqnum== lastpkt_transmitted.seqnum)){

		return 1;
	}
	else{
		if(TraceLevel >= 3){
			printf("Bad ACK or NAK determined by A\n");
			if(packet.seqnum != lastpkt_transmitted.seqnum){
				printf("ACK seq number and last transmitted seqnum don't match!\n");
				printf("packet's seqnum: %d. last_transmitted seqnum: %d\n", packet.seqnum, lastpkt_transmitted.seqnum);
				printf("Last sent packet Checksum: %d\n", lastpkt_transmitted.checksum);
			}
		}

		return 0;
	}
}

//determines whether a packets has been recieved with/without errors for B side
int is_good_pkt_B(struct pkt packet){
	if((is_Corrupt(packet) == 0) && (packet.seqnum == comp_seqnum)){
		if(TraceLevel >= 3) printf("Expected packet compared and confirmed\n");
		return 1;
	}
	else{
		if(TraceLevel >= 3){
			if((is_Corrupt(packet) == 0)){
				printf("incorrect seqnum\n");
				printf("expected seqnum %d, got seqnum %d", comp_seqnum, packet.seqnum);
			}
			else{
				printf("Packet corrupt\n");
			}

		}
		return 0;
	}
}

//Queue is on Sender Side only

//place the message data on the quque
void queue_msg(char message[20]) {
  struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
  strncpy(temp->data, message, 20);
  temp->next = NULL;
  if(front == NULL && rear == NULL){
    front = rear = temp;
    return;
  }
  rear->next = temp;
  rear = temp;
  return;
}

//Pop the next message data off the queue
char *dequeue_msg() {
  struct Node* temp = front;
  if(front == NULL) {
    return NULL;
  }
  if(front == rear) {
    front = rear = NULL;
  } else {
    front = front->next;
  }
  if(TraceLevel >= 3) printf("Data being dequeued: %.20s\n", temp->data);
  return temp->data;
}




