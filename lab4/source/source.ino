/*

Run this sketch on two Zigduinos, open the serial monitor at 9600 baud, and type in stuff
Watch the Rx Zigduino output what you've input into the serial port of the Tx Zigduino

*/

#include <ZigduinoRadio.h>
#include <string.h>
#include "DSR.h"

#define NODE_ID 0x0001  // node id of this node. change it with different boards

#define CHANNEL 23     // check correspond frequency in SpectrumAnalyzer
#define TX_TRY_TIMES 5  // if TX_RETRY is set, pkt_Tx() will try x times before success
#define TX_DO_CARRIER_SENSE 1
#define TX_SOFT_ACK 0   // only affect RX part(send ACK by hw/sw). TX still check ACK by  hardware in this code. modify libraries if necessary.
#define TX_SOFT_FCS 1
#define TX_RETRY 1      // pkt_Tx() retransmit packets if failed.
#define TX_BACKOFF 100  // sleep time in ms
#define TX_HEADER_LEN 9
uint8_t TxBuffer[128]; // can be used as header and full pkt.
uint8_t RxBuffer[128];
uint8_t softACK[8];

uint8_t TX_available; // set to 1 if need a packet delivery, and use need_TX() to check  its value
// here are internal variables, please do not modify them.
uint8_t retry_c;
uint8_t RX_available; // use has_RX() to check its value
uint8_t RX_pkt_len;

DsrPacket pkt;
DSR node(NODE_ID);
unsigned long t1,t2;
int flag;

// the setup() function is called when Zigduino staets or reset
void setup()
{
  init_header();
  t1 = 0;
  flag = 0;
  retry_c = 0;
  TX_available = 1;
  RX_available = 1;
  ZigduinoRadio.begin(CHANNEL,TxBuffer);
  ZigduinoRadio.setParam(phyPanId,(uint16_t)0xABCD );
  ZigduinoRadio.setParam(phyShortAddr,(uint16_t)NODE_ID );
  ZigduinoRadio.setParam(phyCCAMode, (uint8_t)3);
  Serial.begin(9600);
  Serial.print("Node ID:");
  Serial.println(node.nodeid);
  // register event handlers
  ZigduinoRadio.attachError(errHandle);
  //ZigduinoRadio.attachTxDone(onXmitDone);
  ZigduinoRadio.attachReceiveFrame(pkt_Rx);
  
  if(NODE_ID == 0x0001){
    pkt = node.source(4);
    packet_send(pkt,0xffff);
  }
}

// the function is always running
void loop()
{
  uint8_t inbyte;
  uint8_t inhigh;
  uint8_t inlow;
  
  if(NODE_ID == 0x0001 && flag && t1 == 0)ping();
  
  //char txData[80]={};
  //memcpy(txData,&pkt,sizeof(pkt));
  
  if(has_RX()){
    //Serial.println();
    //Serial.print("Rx: ");
    char dataBuffer[256]={};
    for(uint8_t i=TX_HEADER_LEN;i<RX_pkt_len-4;i++){
      dataBuffer[i-TX_HEADER_LEN] = RxBuffer[i]; 
    }
    DsrPacket *rxpkt = (DsrPacket*) dataBuffer;
    pkt = node.get_packet(rxpkt);
    switch(pkt.type != 0){
      case 1:
        Serial.println("RREQ Send~");
        packet_send(pkt,0xffff);
        break;
      case 2:
        Serial.println("RREP Send~");
        packet_send(pkt, pkt.dest_id);
        break;
      case 4:
        if(pkt.dest_id == pkt.req_id){//ping end
          t2 = millis();
          Serial.print("RTT:");
          Serial.println(t2-t1);
          t1 = 0;
        }else{
          int target = node.in_cache(pkt.dest_id);
          if(target != 0){
            packet_send(target,pkt);
          }
        }
        break;
      case 5:
        Serial.println("RREQ done");
        flag = 1;
        break;
      case 0:
      default:
        break;
    }
    
    // Serial.println();
    // Serial.print("LQI: ");
    // Serial.print(ZigduinoRadio.getLqi(), 10);
    // Serial.print(", RSSI: ");
    // Serial.print(ZigduinoRadio.getLastRssi(), 10);
    // Serial.print(" dBm, ED: ");
    // Serial.print(ZigduinoRadio.getLastEd(), 10);
    // Serial.println("dBm");
  }
  //delay(100);
}

void packet_send(DsrPacket pkt,uint8_t addr){
  uint8_t tx_suc;
  if(need_TX()){
    delay(TX_BACKOFF);
    tx_suc = pkt_Tx(addr,(char*) &pkt,sizeof(DsrPacket));
    TX_available = 1;
  }
}

void init_header(){
  if(TX_SOFT_ACK){
    TxBuffer[0] = 0x61; // ack required
  }else{
    TxBuffer[0] = 0x41; // no ack required
  }
  TxBuffer[1] = 0x88;
  TxBuffer[2] = 0;    // seqence number
  TxBuffer[3] = 0xCD;
  TxBuffer[4] = 0xAB; //Pan Id
  TxBuffer[5] = 0x01; //dest address low byte
  TxBuffer[6] = 0x00; //dest address hight byte
  TxBuffer[7] = NODE_ID & 0xff; //source address low byte
  TxBuffer[8] = NODE_ID >> 8; //source address hight byre
  softACK[0] = 0x42;
  softACK[1] = 0x88;
}

/* the packet transfer function
 * parameters :
 *     dst_addr : destnation address, set 0xffff for broadcast. Pkt will be dropped if  NODE_ID != dst addr in packet header.
 *     msg : a null-terminated string to be sent. This function won't send the '\0'. Pkt  header will be paded in this function.
 * return values : this function returns 0 if success, not 0 if problems (no ACK after  some retries).
 *
 * This function set the headers to original msg then transmit it according your policy  setting.
 * The most important function is ZigduinoRadio.txFrame(TxBuffer, pkt_len) which transmit  pkt_len bytes from TxBuffer, where TxBuffer includes header.
 * Note that onXmitDone(radio_tx_done_t x) is right called after txFrame(), you can do  some status checking at there.
 *
 * Feel free to modify this function if needed.
 */
uint8_t pkt_Tx(uint16_t dst_addr, char* msg, size_t datalength){
  uint16_t fcs;
  uint8_t i;
  uint8_t pkt_len;
  uint8_t tmp_byte;
  uint8_t checksum = 0;
  radio_cca_t cca = RADIO_CCA_FREE;
  int8_t rssi;

  // process the dst addr, 0xffff for broadcast
  TxBuffer[5] = dst_addr & 0x00ff;
  TxBuffer[6] = dst_addr >> 8;
  tmp_byte = TxBuffer[0];
  if(dst_addr == 0xffff){ // broadcast, no ACK required
    TxBuffer[0] = 0x41;
  }
  // fill the payload
  for(i = 0; i< datalength; i++){
    TxBuffer[TX_HEADER_LEN + i] = msg[i];
    checksum += (int)msg[i] % 10;
  }
  checksum %= 10;
  TxBuffer[2] = checksum;
  pkt_len = TX_HEADER_LEN + i;
  // fill the software fcs
  if(TX_SOFT_FCS){
    fcs = cal_fcs(TxBuffer, pkt_len);
    TxBuffer[pkt_len++] = fcs & 0xff;
    TxBuffer[pkt_len++] = fcs >> 8;
  }
  // hardware fcs, no use
  pkt_len += 2;
  // transmit the packet
  // retry_c will be set to RETRY_TIMES by onXmitDone() if packet send successfully
  if(TX_RETRY){
    for(retry_c = 0; retry_c < TX_TRY_TIMES; retry_c++){
      if(TX_DO_CARRIER_SENSE){
//        cca = ZigduinoRadio.doCca();
        rssi = ZigduinoRadio.getRssiNow();
//        if(cca == RADIO_CCA_FREE)
        if(rssi == -91){
          ZigduinoRadio.txFrame(TxBuffer, pkt_len);
        }else{
          Serial.print("ca fail with rssi = ");
          Serial.println(rssi);
        }
      }else{
        ZigduinoRadio.txFrame(TxBuffer, pkt_len);
      }
      delay(TX_BACKOFF);
    }
    retry_c--; // extra 1 by for loop, if tx success retry_c == TX_TRY_TIMES
  }else{
    if(TX_DO_CARRIER_SENSE){
//      cca = ZigduinoRadio.doCca();
      rssi = ZigduinoRadio.getRssiNow();
//      if(cca == RADIO_CCA_FREE)
      if(rssi == -91){
        ZigduinoRadio.txFrame(TxBuffer, pkt_len);
      }else{
        Serial.print("ca fail with rssi = ");
        Serial.println(rssi);
      }
    }else{
      ZigduinoRadio.txFrame(TxBuffer, pkt_len);
    }
  }
  TxBuffer[0] = tmp_byte;
  return retry_c == TX_TRY_TIMES;
}

/* the event handler which is called when Zigduino got a new packet
 * don't call this function yourself
 * do sanity checks in this function, and set RX_available to 1 at the end
 * the crc_fail parameter is a fake, please ignore it
 */
uint8_t* pkt_Rx(uint8_t len, uint8_t* frm, uint8_t lqi, uint8_t crc_fail){
  uint16_t fcs;
  // This function set RX_available = 1 at the end of this function.
  // You can use has_RX() to check if has packet received.

  // Software packet filter :
  // Check pkt_len, dst_addr, FCS. Drop pkt if fails
  if(len < TX_HEADER_LEN){
    return RxBuffer;
  }
  // keep the pkt only if broadcast or dst==me
  if( (frm[5] != NODE_ID&0xff) || (frm[6] != NODE_ID>>8) ){
    if(frm[5] != 0xff || frm[6] != 0xff){
      return RxBuffer;
    }
  }
  // check fcs first, drop pkt if failed
  if(TX_SOFT_FCS){
    fcs = cal_fcs(frm, len-2);
    if(fcs != 0x0000){
      return RxBuffer;
    }
  }
  //checksum checked
  uint8_t checksum = 0;
  for(uint8_t i=TX_HEADER_LEN;i<len-4;i++){
    checksum += (int)frm[i] % 10;
  }
  checksum %= 10;
  if(frm[2] != checksum){
    Serial.println("checksum failed");
    return RxBuffer;
  }
  
  // send software ack
  if(frm[0] & 0x20){
    softACK[2] = frm[2];
    ZigduinoRadio.txFrame(softACK, 5);
  }
  // now all checks are passed, copy out the received packet
  for(uint8_t i=0; i < len; i++){
    RxBuffer[i] = frm[i];
  }
  RX_pkt_len = len;
  RX_available = 1;
  return RxBuffer;
}

// this function returns TX_available and reset it to 0
uint8_t need_TX(){
  if(TX_available){
    TX_available = 0;
    return 1;
  }
  return 0;
}


// this function returns RX_available and reset it to 0
uint8_t has_RX(){
  if(RX_available){
    RX_available = 0;
    return 1;
  }
  return 0;
}

// calculate error detecting code
// choose an algorithm for it
uint16_t cal_fcs(uint8_t* frm, uint8_t len){
  uint16_t fcs = frm[0];
  for(uint8_t i = 1; i < len; i += 1){
    fcs ^= frm[i];
  }
  return fcs;
}

uint8_t printable(uint8_t in){
  if(32 <= in && in <= 126){
    return 1;
  }
  return 0;
}

void errHandle(radio_error_t err)
{
  Serial.println();
  Serial.print("Error: ");
  Serial.print((uint8_t)err, 10);
  Serial.println();
}

// this function is called after the packet transmit function
void onXmitDone(radio_tx_done_t x)
{
  Serial.println();
  Serial.print("TxDone: ");
  Serial.print((uint8_t)x, 10);
  if(x==TX_NO_ACK){
    Serial.print(" NO ACK ");
  }else if(x==TX_OK){
    Serial.print("(OK)");
    retry_c = TX_TRY_TIMES;
  }else if(x==TX_CCA_FAIL){ // not implemented
    Serial.print("(CS busy)");
  }
  Serial.println();
}

void ping(){
  Serial.println();
  Serial.println("-- start ping --");
  
  //start ping
  t1 = millis();
  pkt = node.ping(20);
  int dest = node.in_cache(20);
  if(dest!=0)
    packet_send(dest,pkt);
}
