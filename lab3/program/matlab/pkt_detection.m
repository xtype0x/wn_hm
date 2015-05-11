function [lts_ind payload_ind] = pkt_detection(rx_ant, THRESH_LTS_CORR)
LTS_LEN = 160;

% Long preamble (LTS) for CFO and channel estimation
lts_f = [0 1 -1 -1 1 1 -1 1 -1 1 -1 -1 -1 -1 -1 1 1 -1 -1 1 -1 1 -1 1 1 1 1 0 0 0 0 0 0 0 0 0 0 0 1 1 -1 -1 1 1 -1 1 -1 1 1 1 1 1 1 -1 -1 1 1 -1 1 -1 1 1 1 1];
lts_t = ifft(lts_f, 64);

% Hint 1:
% Complex cross correlation of Rx waveform with time-domain LTS 
lts_corr = abs(conv(conj(fliplr(lts_t)), sign(rx_ant)));
% lts_corr = abs(xcorr(rx_ant,lts_t));

% Skip early and late samples
lts_corr = lts_corr(32:end-32);

% Hint 2:
% Get the lts_ind and payload_ind
% Don't forget that "You have skip early and late samples"
peak = find(lts_corr>THRESH_LTS_CORR*max(lts_corr));
[LTS1, LTS2] = meshgrid(peak,peak);
[second_peak_index, y] = find(LTS2-LTS1 == length(lts_t));

payload_ind = peak(max(second_peak_index))+64;
lts_ind = payload_ind - LTS_LEN;