
class DSR {

	public:
			
		int _src;	
		int _type ;
		int _req_id ;
		int route[ROUTE_NO] ;
		int cache[ROUTE_NO][ROUTE_NO] ;
		
		DSR(uint8_t NODE_ID,char type){ _src = NODE_ID ,_type = type }; // src
		void assign_route(int *);
		void route_append(int * , int );
		void route_reverse( int *);
				
		void depkt(char * pkt);
};

void DSR::assign_route(int * recev)
{
	int i = 0;
	while(recev[i] != -1)
	{
	i++
	route[i]=recev[i]
	}
	route[i] = -1;	
}

void DSR::route_append()
{
	int i = 0;
	while( route[i]!=-1 )
		i++
	route[i] = _src;
	route[i+1] = -1 ;
	
}

