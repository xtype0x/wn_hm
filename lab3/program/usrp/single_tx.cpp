#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <complex>
#include <cmath>

#include <csignal>
#include <stdio.h>

#include "single_tx.h"

namespace po = boost::program_options;

//System parameters 
double freq, gain, thres;
double inter;
double rate;

//USRP
uhd::usrp::multi_usrp::sptr usrp;
string usrp_ip;

//TX/RX metadata
uhd::rx_metadata_t rx_md;
uhd::tx_metadata_t tx_md;

//Buffer
gr_complex pkt[MAX_PKT_LEN];
gr_complex zeros[SYM_LEN];

//File
FILE* in_file;
FILE* out_file;
string in_name, out_name; 

//Evaluation
size_t r_cnt;
static bool stop_signal = false;

void init_usrp() {
	usrp = uhd::usrp::multi_usrp::make(usrp_ip);
	usrp->set_rx_rate(rate);
	usrp->set_tx_rate(rate);

	usrp->set_rx_freq(freq);
	usrp->set_tx_freq(freq);

	usrp->set_rx_gain(gain);
}

void sync_clock() {
	cout << "SYNC Clock" << endl;
	usrp->set_clock_config(uhd::clock_config_t::external());
	usrp->set_time_next_pps(uhd::time_spec_t(0.0));
}


void init_sys() {
	int cnt;
	
	// Buffer initialize	
	memset(pkt, 0, sizeof(pkt));
	in_file = fopen(in_name.c_str(), "rb");

	while((cnt = fread(pkt+sample_cnt, sizeof(gr_complex), 1, in_file)) > 0)
		sample_cnt++;

	fclose(in_file);

	/*
	for( int i = 0; i < sample_cnt; i++ )
		printf("%3d: %.4lf %.4lf\n", i, pkt[i].real(), pkt[i].imag());
	*/

	memset(zeros, 0, sizeof(zeros));

	// USRP setting
	rate = 1e8/inter;
	freq = 1e9*freq;

	init_usrp();
	//sync_clock();
}

void sig_int_handler(int){stop_signal = true;}



int UHD_SAFE_MAIN(int argc, char *argv[]){
	uhd::set_thread_priority_safe();
	uhd::time_spec_t refer;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "help message")
		("r0", po::value<string>(&usrp_ip)->default_value("addr=192.168.10.3"), "usrp's IP")
		("in", po::value<string>(&in_name)->default_value("wn_trace/src_data_1.bin"), "binary samples file")
		("out", po::value<string>(&out_name)->default_value("rx_log.dat"), "signal file")
		("i", po::value<double>(&inter)->default_value(SAMPLE_P), "interval of two sampling")
		("f", po::value<double>(&freq)->default_value(2.49), "RF center frequency in Hz")
		("g", po::value<double>(&gain)->default_value(25.0), "gain for the RF chain")
		("c", po::value<size_t>(&r_cnt)->default_value(90), "round count");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")){
		cout << boost::format("UHD TX samples from file %s") % desc << endl;
		return ~0;
	}

	// Init
	init_sys();

	// Setup time
	boost::this_thread::sleep(boost::posix_time::milliseconds(WARM_UP_TIME));

	std::signal(SIGINT, &sig_int_handler);
	std::cout << "Press Ctrl + C to stop streaming..." << std::endl;

	//tx_md.time_spec = usrp->get_time_now() + uhd::time_spec_t(0, SYM_CNT*start, 1e8/inter);

	tx_md.start_of_burst    = true;
	tx_md.end_of_burst      = false;
	tx_md.has_time_spec     = false;

	usrp->get_device()->send(zeros, SYM_LEN, tx_md, C_FLOAT32, S_ONE_PKT);

	tx_md.start_of_burst    = false;
	tx_md.end_of_burst		= false;

	// TODO:
	// Send Signals until press ^C
	// HINT: You have to send signals here
	// How many symbols you have to send? Ans: sym_cnt
	// pkt: records the samples which we want to send
	// using offset += usrp->get_device()->send(...)
	// remove content of within while loop

	while(!stop_signal) {
		size_t sym_cnt = sample_cnt/SYM_LEN;
		size_t offset  = 0;
		// add here to send the sym_cnt symbols
		for(size_t i = 0; i < sym_cnt; i++){
			offset += usrp->get_device()->send(pkt+offset, SYM_LEN, tx_md, C_FLOAT32, S_ONE_PKT);
			cout<<abs(*(pkt+offset))<<endl;
		}

		//clean the buffer of USRP
		for(size_t j = 0; j < 20; j++)
			usrp->get_device()->send(zeros, SYM_LEN, tx_md, C_FLOAT32, S_ONE_PKT);
		//cout<<offset<<" sended"<<endl;
		cout<<endl;
	}

	tx_md.start_of_burst    = false;
	tx_md.end_of_burst		= true;

	usrp->get_device()->send(zeros, SYM_LEN, tx_md, C_FLOAT32, S_ONE_PKT);

    boost::this_thread::sleep(boost::posix_time::seconds(1));
	cout << "Terminate systems ... " << endl;
	return 0;
}
