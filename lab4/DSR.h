#ifndef DSR_H
#define DSR_H

#define ROUTE_NO 10
#define RECORD_SIZE 10
typedef struct dsr_packet{
  int type;// RREQ: 1, RREP: 2, RERR:3 
  int src_id;
  int dest_id;
  int req_id;
  int route[ROUTE_NO];
  int length;
} DsrPacket;

class DSR {

	public:
			
		int nodeid;	//source id
		int req_id;
		int route[ROUTE_NO] ; //for record the route she got
		int cache[ROUTE_NO][ROUTE_NO] ;
		int req_record[RECORD_SIZE]; 
		int record_leng;
		
		DSR(int NODE_ID){
		  nodeid = NODE_ID;
		  record_leng=0;		  
		  memset(req_record, 0, sizeof(int)*RECORD_SIZE);
		  memset(route, 0, sizeof(int)*ROUTE_NO);
		  memset(cache, 0, sizeof(int)*ROUTE_NO*ROUTE_NO);
		} // src
		~DSR(){}
		DsrPacket send_request();
		int get_request();
		DsrPacket send_reply();
		int get_reply();
		void update_cache(int *);	
		void depkt(char * pkt);
};
/*
rreq
2: destination
1: success
0: has route, fail
*/
int DSR::get_request(DsrPacket * pkt)
{

  for(int i=0; i<record_leng; i++)
	if(pkt->req_id == req_record[i])
		return 0;
  
  for(int i=0 ; i < pkt->length; i++)
	route[i] = pkt->route[i];

  if( pkt->dest_id == nodeid )  // u got it!!!!
	{
		
		req_record[record_leng] = pkt->req_id;
		record_leng++ ;
		return 2 ;                  //time to send reply
	}
  else                          // u got a broadcast ,please append and just rebroadcast
  {
	route[ pkt->length] = nodeid;
	req_record[record_leng] = pkt->req_id;
	record_leng++ ;
	return 1
  }

/***
first check if the req has been here,
then check is this node target?
yes return 2 ,no append and return 1

***/  
  
  
}


DsrPacket DSR::send_request(int _dest, int * route , int length){
	DsrPacket send;
	
	memset(send.route, 0, sizeof(int)*ROUTE_NO);

	if(route[0] == 0){
		send.src_id = nodeid;
	}else{
		send.src_id = route[0];
	}
	send.dest_id = _dest;
	send.req_id = nodeid;
	send.type = 1;
	for (int i = 0; i < length; ++i){
		send.route[i] = route[i];	
	}
	send.length = length;
	return send;
}


//RREP
DsrPacket DSR::send_reply(int _dest , int * route ,int length){

	DsrPacket send;
	int cnt = 0;
	memset(send.route, 0, sizeof(int)*ROUTE_NO);
	
	
	send.dest_id = _dest;
	send.req_id = nodeid;
	send.type = 2;
	
	for(int i=0; i<length; i++ )
		send.route[i] = route[i];
	
	send.length = length;
		
	/***
	send_reply need target's id and the route attach to  packet
	***/
		
	return send;	
}

int DSR::get_reply(DsrPacket * pkt )
{
	int cnt=0;
	int for_update_route[ROUTE_NO];
	
	memset(update_cache, 0, sizeof(int)*ROUTE_NO);
	while( pkt->route[cnt]!= 0)
		cnt++;
	// cnt is route's length
	for(int i=0; i<cnt ; i++ )
	{
		if( nodeid == pkt->route[cnt-i] )
			break;
		for_update_route[i] = pkt->route[cnt-i] ;
	}
	update_cache(for_update_route);
	
	return 0;
	/***
	
	DSR has route S->A->C->D
	
	if A get reply from D the reply will be D->C
	
	***/	
}


void DSR::assign_route(int * recev)
{
	int i = 0;
	while(recev[i] != -1)
	{
		i++;
		route[i]=recev[i];
	}
	route[i] = -1;	
}

void DSR::route_append()
{
	int i = 0;
	while( route[i]!=-1 )i++;
	route[i] = nodeid;
	route[i+1] = -1 ;
	
}
void DSR::update_cache(int * rrep_route) //check the necessary of reverse route
{
	
	int tmp;
	int cnt = 0;
	while(rrep_route[cnt] != 0)
		cnt++;
	// cnt = route's length 

	tmp = rrep_route[0] ;
	for(int i = 0; i<cnt-1; i++)
		rrep_route[i]=rrep_route[i+1] ;
	rrep_route[cnt-1] =tmp;
	
	for(int i = 0; i<cnt; i++)
	{
		if( cache[0][rrep_route[cnt]] == 0 ) //no route to node tmp
			{
			for(int j = 0; j <= i ; j++)
				cache[j][rrep_route[cnt]] = rrep_route[j];
			
			for(int k = i+1 ; k<ROUTE_NO; k++)
				cache[k][rrep_route[cnt]] = 0;
			}
	}
}
/***
ex:  S DSR object get rrep_packet 
S ask D and she get reply  D,C,A 
so she can update S->A ,S->C ,S->D
  S  A  B  C  D  E
------------------------
  0 |A| 0 |A||A| 0
------------------------
  0  0  0 |C||C| 0
------------------------
  0  0  0  0 |D| 0 
------------------------
  0  0  0  0  0  0
------------------------
***/


#endif
