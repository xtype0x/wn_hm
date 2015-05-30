#include <iostream>
#include "DSR.h"

using namespace std;

int main(int argc, char const *argv[]){
	DSR nodeS(1), nodeA(2), nodeB(3), nodeC(4), nodeD(5);
	DsrPacket pkt;
	int status;
	//cout<<sizeof(DsrPacket)<<endl;

	pkt = nodeS.source(3);
	pkt = nodeC.get_packet(&pkt);
	pkt = nodeB.get_packet(&pkt);
	pkt = nodeC.get_packet(&pkt);
	pkt = nodeS.get_packet(&pkt);

	cout<<"route index: "<<nodeS.route_index(3)<<endl;
	// for(int i=0;i<10;i++)
	// 	cout<<pkt.route[i]<<" ";
	// cout<<endl<<endl;
	for(int i=0;i<10;i++)
		cout<<pkt.route[i]<<" ";
	cout<<endl;

	return 0;
}