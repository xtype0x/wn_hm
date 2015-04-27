function rx_out = phaseTrack(rx_in, tx_in, cf)
	% figure(cf) can use to plot process of phaseTrack
	% TODO
	
    rx_buf = rx_in;
    tx_buf = tx_in;
    
	% hint 1: find your pilot index according to signal_generator.m
    % >> pilot_idx = ___;
    
    phase_shift = phase( rx_buf(pilot_idx) ./ tx_buf(pilot_idx) );  %rx : 不乾淨的(真正傳的資料)   tx : 乾淨的  %radian
	
	% hint 2: the phase shift is linear!
	%         there is a matlab function called "regress"
    % phase_shift is in unit [radian] 
    % phase_shift_regressed = ___ ;  % should be complex values!!
	
	% hint 3: Use phase_shift_regressed to remove SFO
	% rx_out = rx_in ./ ...
end
