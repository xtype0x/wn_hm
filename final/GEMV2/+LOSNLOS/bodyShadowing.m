function Attenuation = ...
	bodyShadowing(txPos,rxPos,...
		txHeight,rxHeight,driverHeight,...
		frequency)
% input:
% txVehicle or rxVehiclee must be a 1x4 array
% 	col1: ID
% 	col2: x
% 	col3: y
% 	col6: angle
% txHeight,rxHeight : tx and rx height
% driverHeight: the height where driver sit on the scooter
% frequency: signal frequency
% output: attenuation by body shadowing 

	% check if line of sight
	distance = sqrt((txPos(:,1)-rxPos(:,1)).^2+(txPos(:,2)-rxPos(:,2)).^2);
	distObs = distance - 0.43;
	d1 = sqrt((1.46 - rxHeight).^2+(0.43).^2);
	d2 = sqrt((1.46 - txHeight).^2 + distObs.^2);

	Attenuation = LOSNLOS.obstacleAttenuation(txHeight,rxHeight,1.46,...
				d1+d2,d1,frequency);