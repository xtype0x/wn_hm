function Attenuation = ...
	bodyShadowing(txVehicle,rxVehicle,txHeight,rxHeight,driverHeight,...
		frequency)
% input:
% txVehicle or rxVehiclee must be a 1x4 array
% 	col1: ID
% 	col2: x
% 	col3: y
% 	col6: angle
% txHeight,rxHeight : tx and rx height
% driverHeight: the height where driver sit on the scooter
% frequency:
	
% output: attenuation by body shadowing 

	x1 = txVehicle(:,2);
	y1 = txVehicle(:,3);
	angle1 = txVehicle(:,6);
	x2 = rxVehicle(:,2);
	y2 = rxVehicle(:,3);
	angle2 = rxVehicle(:,6);

	



	Attenuation = LOSNLOS.obstacleAttenuation(1.1,1.1,1.5,...
				20.0,19,2.4*10^9);