function rx_out = phaseTrack(rx_in, tx_in, cf)
	pkg load statistics;
	% figure(cf) can use to plot process of phaseTrack
	% TODO
	
    rx_buf = rx_in;
    tx_buf = tx_in;
    
	% hint 1: find your pilot index according to signal_generator.m
    % >> pilot_idx = ___;
    pilot_idx = [8 22 44 58];
    X = [ones(size(pilot_idx)); pilot_idx];
    phase_shift = angle( rx_buf(pilot_idx) ./ tx_buf(pilot_idx) );  %rx : ¤£°®²bªº(¯u¥¿¶Çªº¸ê®Æ)   tx : °®²bªº  %radian

	% hint 2: the phase shift is linear!
	%         there is a matlab function called "regress"
    % phase_shift is in unit [radian] 
    % phase_shift_regressed = ___ ;  % should be complex values!!
	
    [B, bint, r, rint, stats]=regress(phase_shift,X');
    % size(B)

    phase_shift_regressed=B'*[1:64; rx_buf'];
    phase_shift_regressed=abs(phase_shift_regressed).*exp(i*angle(phase_shift_regressed));



	% hint 3: Use phase_shift_regressed to remove SFO
		rx_out = rx_in ./ phase_shift_regressed'
end
