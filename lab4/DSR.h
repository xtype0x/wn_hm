#ifndef DSR_H
#define DSR_H

#define ROUTE_NO 10

typedef struct dsr_packet{
  int type;// RREQ: 1, RREP: 2, RERR:3 
  int src_id;
  int dest_id;
  int req_id;
  int route[ROUTE_NO];
} DsrPacket;

class DSR {

	public:
			
		int nodeid;	//source id
		int req_id;
		int route[ROUTE_NO] ;
		int cache[ROUTE_NO][ROUTE_NO] ;
		
		DSR(int NODE_ID){
		  nodeid = NODE_ID; 
		  memset(route, 0, sizeof(int)*ROUTE_NO);
		  memset(cache, 0, sizeof(int)*ROUTE_NO*ROUTE_NO);
		} // src
		~DSR(){}
		DsrPacket send_request();
		int get_request();
		DsrPacket send_reply();
		int get_reply();

		void assign_route(int *);
		void route_append(int * , int );
		void route_reverse( int *);
		void update_cache(int *);	
		void depkt(char * pkt);
};
/*
rreq
2: destination
1: success
0: has route, fail
*/
int DSR::get_request(int _src, int _des, int _req, int _route[]){
  //check if route exist
  if(route[0] != 0){
  	return 0;
  }
  //check if dest
  if(_des == nodeid){
  	return 2;
  }
  int i;
  for (i = 0; i < ROUTE_NO; ++i){
  	if(route[i] == 0){
  		break;
  	}else{
  		route[i]=_route[i];
  	}
  }
  route[i+1] = nodeid;
  return 1;
}

DsrPacket DSR::send_request(int _dest){
	DsrPacket send;
	if(route[0] == 0){
		send.src_id = nodeid;
	}else{
		send.src_id = route[0];
	}
	send.dest_id = _dest;
	send.req_id = nodeid;
	send.type = 1;
	for (int i = 0; i < ROUTE_NO; ++i){
		send.route = 0;	
	}
	return send;
}
//RREP
DsrPacket DSR::send_reply(){
	DsrPacket send;
	send.dest_id = _dest;
	send.req_id = nodeid;
	send.type = 2;
	for (int i = 0; i < ROUTE_NO; ++i){
		send.route = 0;	
	}
	return send;	
}

int DSR::get_reply(){

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
	int tmp = 0;
	while(rrep_route[tmp] != 0)
		tmp++;
	
	for(int i = 0; i<tmp; i++)
	{
		if( cache[0][rrep_route[tmp]] == 0 ) //no route to node tmp
			{
			for(int j = 0; j <= i ; j++)
				cache[j][rrep_route[tmp]] = rrep_route[j];
			
			for(int k = i+1 ; k<ROUTE_NO; k++)
				cache[k][rrep_route[tmp]] = 0;
			}
	}
}
/***
ex:  S DSR object get rrep_packet 

S ask D and she get reply  D,C,A 
so she can update S->A ,S->C ,S->D

  S  A  B  C  D  E
------------------------
  0 |D| 0 |D||D| 0
------------------------
  0 |C| 0 |C| 0  0
------------------------
  0 |A| 0  0  0  0 
------------------------
  0  0  0  0  0  0
------------------------


***/


#endif
