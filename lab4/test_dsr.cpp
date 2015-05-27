#include <iostream>
#include "DSR.h"

using namespace std;

int main(int argc, char const *argv[]){
	DSR nodeS(1), nodeA(2), nodeB(3), nodeC(4), nodeD(5);
	DsrPacket pkt;
	int status;

	pkt = nodeS.source(5);
	pkt = nodeA.get_packet(&pkt);
	pkt = nodeC.get_packet(&pkt);
	pkt = nodeD.get_packet(&pkt);

	pkt = nodeC.get_packet(&pkt);
	pkt = nodeA.get_packet(&pkt);
	pkt = nodeS.get_packet(&pkt);

	cout<<"type:"<<pkt.type<<endl;
	cout<<"length: "<<pkt.length<<endl;
	cout<<"route:"<<endl;
	for(int i=0;i<10;i++)cout<<pkt.route[i]<<" ";
	cout<<endl;

	return 0;
}