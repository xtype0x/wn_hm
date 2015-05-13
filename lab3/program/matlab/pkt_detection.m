function [lts_ind payload_ind] = pkt_detection(rx_ant, THRESH_LTS_CORR)
pkg load signal;
LTS_LEN = 160;

% Long preamble (LTS) for CFO and channel estimation
lts_f = [0 1 -1 -1 1 1 -1 1 -1 1 -1 -1 -1 -1 -1 1 1 -1 -1 1 -1 1 -1 1 1 1 1 0 0 0 0 0 0 0 0 0 0 0 1 1 -1 -1 1 1 -1 1 -1 1 1 1 1 1 1 -1 -1 1 1 -1 1 -1 1 1 1 1];
lts_t = ifft(lts_f, 64);

% Hint 1:
% Complex cross correlation of Rx waveform with time-domain LTS 
% rx_ant = real(rx_ant).^2;
% lts_corr = abs(conv(conj(fliplr(lts_t)), sign(rx_ant)));
[lts_corr lag] = xcorr(rx_ant,lts_t);
% lts_corr = abs(xcorr(rx_ant,lts_t));

% Skip early and late samples
% lts_corr = lts_corr(32:end-32);
plot(abs(lts_corr));

% Hint 2:
% Get the lts_ind and payload_ind
% Don't forget that "You have skip early and late samples"
peak = find(lts_corr>THRESH_LTS_CORR*max(lts_corr));
lag(peak(1))
% [LTS1, LTS2] = meshgrid(peak,peak);
% [second_peak_index, y] = find(LTS2-LTS1 == length(lts_t));
% pick_peak = second_peak_index(1);
% for i = 2:size(second_peak_index)
% 	if second_peak_index(i) - pick_peak > 1
% 		break;
% 	end
% 	pick_peak = second_peak_index(i);
% end
lts_ind = lag(peak(1)) - 32 + 1;
payload_ind = lts_ind + 320;
% payload_ind = peak(pick_peak)+32;
% lts_ind = payload_ind - LTS_LEN;