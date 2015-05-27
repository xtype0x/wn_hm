#include <iostream>
#include "DSR.h"

using namespace std;

int main(int argc, char const *argv[]){
	DSR nodeS(1), nodeA(2), nodeB(3), nodeC(4), nodeD(5);
	DsrPacket pkt;
	int status;
	//cout<<sizeof(DsrPacket)<<endl;

	pkt = nodeS.source(2);
	pkt = nodeA.get_packet(&pkt);
	pkt = nodeS.get_packet(&pkt);

	// for(int i=0;i<10;i++)
	// 	cout<<pkt.route[i]<<" ";
	// cout<<endl<<endl;
	for(int i=0;i<10;i++){for(int j=0;j<10;j++)cout<<nodeA.cache[i][j];cout<<endl;}
	cout<<endl<<nodeS.in_cache(2)<<endl;

	return 0;
}