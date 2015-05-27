#include <iostream>
#include "DSR.h"

using namespace std;

int main(int argc, char const *argv[]){
	DSR nodeS(1), nodeA(2), nodeB(3), nodeC(4), nodeD(5);
	DsrPacket *pkt;
	int status;

	//source
	pkt = new DsrPacket(nodeS.send_request(5));
	status = nodeA.get_request(pkt);
	cout<<"S -> A: "<<((status==1)?"success":"failed")<<endl;
	for(int i=0; i<10;i++){
		cout<<pkt->route[i]<<" ";
	}
	cout<<pkt->length<<endl;
	delete pkt;

	pkt = new DsrPacket(nodeA.send_request(5));
	status = nodeS.get_request(pkt);
	cout<<"A -> S(dropped): "<<((status==0)?"success":"failed")<<endl;
	status = nodeB.get_request(pkt);
	cout<<"A -> B: "<<((status==1)?"success":"failed")<<endl;
	status = nodeC.get_request(pkt);
	cout<<"A -> C: "<<((status==1)?"success":"failed")<<endl;
	delete pkt;

	pkt = new DsrPacket(nodeC.send_request(5));
	status = nodeA.get_request(pkt);
	cout<<"C -> A(dropped): "<<((status==0)?"success":"failed")<<endl;
	status = nodeD.get_request(pkt);
	cout<<"C -> D(destination): "<<((status==2)?"success":"failed")<<endl;
	delete pkt;

	pkt = new DsrPacket(nodeD.send_reply(5));
	status = nodeC.get_reply(pkt);
	cout<<"D -> C: "<<((status==0)?"success":"failed")<<endl;
	delete pkt;

	pkt = new DsrPacket(nodeC.send_reply(5));
	status = nodeA.get_reply(pkt);
	cout<<"C -> A: "<<((status==0)?"success":"failed")<<endl;
	delete pkt;

	pkt = new DsrPacket(nodeA.send_reply(5));
	status = nodeS.get_reply(pkt);
	cout<<"A -> S: "<<((status==0)?"success":"failed")<<endl;
	delete pkt;
	for(int i =0; i< 10;i++){
		for (int j = 0; j < 10; ++j){
			cout<<nodeS.cache[i][j]<<" ";
		}
		cout<<endl;
	}

	return 0;
}