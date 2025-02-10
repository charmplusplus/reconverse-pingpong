#include <stdio.h>
#include "converse.h"
#include <pthread.h>
//#include "ping.cpm.h"

CpvDeclare(int, ping_index);
CpvDeclare(int, ackmsg_index);
CpvDeclare(int, stop_index);
CpvDeclare(int, msg_size);
CpvDeclare(int, ack_count);

/*
typedef struct myMsg
{
  char header[CmiMsgHeaderSizeBytes];
  int payload[1];
} *message;
*/

void print_results() {
  printf("msg_size\n%d\n", CpvAccess(msg_size));
}

//CpmInvokable ping_stop()
void ping_stop_handler(void *msg)
{
  CmiFree(msg);
  CsdExitScheduler();
}

void send_msg() {
  CmiMessage *msg = new CmiMessage;
  msg->header.handlerId = CpvAccess(ping_index);
  msg->header.messageSize = sizeof(CmiMessage);
  msg->header.destPE = CmiMyRank() + CmiMyNodeSize() / 2;
  //payload
  int ints_to_send[(CpvAccess(msg_size) - msg->header.messageSize) / sizeof(int)];
  for (int i = 0; i < (CpvAccess(msg_size) - msg->header.messageSize) / sizeof(int); ++i) ints_to_send[i] = i;
  msg->data = (char*) ints_to_send; //will need to fix this for off-node messages
  CmiSyncSendAndFree(msg->header.destPE, msg->header.messageSize, msg);
  /*
  struct myMsg *msg;
  msg = (message)CmiAlloc(CpvAccess(msg_size));
  // Fills payload with ints
  for (int i = 0; i < (CpvAccess(msg_size) - CmiMsgHeaderSizeBytes) / sizeof(int); ++i) msg->payload[i] = i;
  
  // DEBUG: Print ints stored in payload
  // for (int i = 0; i < (CpvAccess(msg_size) - CmiMsgHeaderSizeBytes) / sizeof(int); ++i) CmiPrintf("%d ", msg->payload[i]);
  // CmiPrintf("\n");

  CmiSetHandler(msg, CpvAccess(ping_index));
  //Send from my pe-i on node-0 to q+i on node-1
  CmiSyncSendAndFree(CmiNumPes() / 2 + CmiMyPe(), CpvAccess(msg_size), msg);
  */
}

void call_exit(){
  for(int i=0;i<CmiMyNodeSize();i++) {
    CmiMessage *msg = new CmiMessage;
    msg->header.handlerId = CpvAccess(stop_index);
    msg->header.messageSize = sizeof(CmiMessage);
    msg->header.destPE = i;
    CmiSyncSendAndFree(i, msg->header.messageSize, msg);
  }
}

void ping_handler(void *vmsg)
{
  int i;
  CmiMessage *msg = (CmiMessage*) vmsg;
  int *incoming_data = (int*) msg->data;
  // if this is a receiving PE
  if (CmiMyRank() >= CmiMyNodeSize() / 2) {
    long sum = 0;
    long result = 0;
    double num_ints = (CpvAccess(msg_size) - msg->header.messageSize) / sizeof(int);
    double exp_avg = (num_ints - 1) / 2;
    for (i = 0; i < num_ints; ++i) {
      sum += incoming_data[i];
    }
    if(result < 0) {
      printf("Error! in computation");
    }
    double calced_avg = sum / num_ints;
    if (calced_avg != exp_avg) {
      printf("Calculated average of %f does not match expected value of %f, exiting\n", calced_avg, exp_avg);
      call_exit();
//      Cpm_ping_stop(CpmSend(CpmALL)); 
    } 
    // else
    //   CmiPrintf("Calculation OK\n"); // DEBUG: Computation Check
      
    CmiFree(msg);
    msg = new CmiMessage;
    msg->header.handlerId = CpvAccess(ackmsg_index);
    msg->header.messageSize = sizeof(CmiMessage);
    msg->header.destPE = 0;
    /*
    msg = (message)CmiAlloc(CpvAccess(msg_size));
    CmiSetHandler(msg, CpvAccess(ackmsg_index));
    */
    CmiSyncSendAndFree(0, CpvAccess(msg_size), msg);
  } else
    printf("\nError: Only node-1 can be receiving node!!!!\n");
}

void pe0_ack_handler(void *vmsg)
{
  int pe;
  CmiMessage *msg = (CmiMessage*)vmsg;
   //Pe-0 receives all acks
  CpvAccess(ack_count) = 1 + CpvAccess(ack_count);

  if(CpvAccess(ack_count) == CmiMyNodeSize()/2) {
    CpvAccess(ack_count) = 0;

    CmiFree(msg);

    // print results
    print_results();
    call_exit();
//    Cpm_ping_stop(CpmSend(CpmALL));
  }
}

void ping_init()
{
  int totalpes = CmiMyNodeSize(); //p=num_pes
  int pes_per_node = totalpes/2; //q=p/2
  if (CmiMyNodeSize()%2 !=0) {
    printf("note: this test requires at multiple of 2 pes, skipping test.\n");
    printf("exiting.\n");
    CsdExitScheduler();
    call_exit();
//    Cpm_ping_stop(CpmSend(CpmALL));
  } else {
    if(CmiMyRank() < pes_per_node)
      send_msg();
  }
}

void ping_moduleinit(int argc, char **argv)
{
  CpvInitialize(int, ping_index);
  CpvInitialize(int, ackmsg_index);
  CpvInitialize(int, stop_index);
  CpvInitialize(int, msg_size);
  CpvInitialize(int, ack_count);

  CpvAccess(ping_index) = CmiRegisterHandler(ping_handler);
  CpvAccess(ackmsg_index) = CmiRegisterHandler(pe0_ack_handler);
  CpvAccess(stop_index) = CmiRegisterHandler(ping_stop_handler);
  CpvAccess(msg_size) = 16+sizeof(CmiMessage)+100;
//  void CpmModuleInit(void);
//  void CfutureModuleInit(void);
  //void CpthreadModuleInit(void);

//  CpmModuleInit();
//  CfutureModuleInit();
  //CpthreadModuleInit();
//  CpmInitializeThisModule();
  // Set runtime cpuaffinity
//  CmiInitCPUAffinity(argv);
  // Initialize CPU topology
//  CmiInitCPUTopology(argv);
  // Wait for all PEs of the node to complete topology init
        CmiNodeBarrier();

  // Update the argc after runtime parameters are extracted out
  //argc = CmiGetArgc(argv);
  if(CmiMyRank() < CmiMyNodeSize()/2)
    ping_init();
}

int main(int argc, char **argv)
{
	ConverseInit(argc,argv,ping_moduleinit);
}
