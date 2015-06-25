% test vehicle
clear all;
clear global;
fprintf('Loading Vehicle file ^.<\n');

% initial parameters
vehiclesFile = 'inputMobilitySUMO/CologneSUMOMobility.xml';
bBoxVehicles = [6.93,50.913,7,50.96];
numRowsPerVehicle = 6;
lengthThreshold = 7;

carMeanHeight = 1.13;
carStdDev =.02;
truckMeanHeight = 3; 
truckStdDev = .1;

carMeanWidth = 0.68;
carStdDevWidth = .05;
truckMeanWidth = 2;
truckStdDevWidth = .1;

carMeanLength = 1.85;
carStdDevLength = .2;
truckMeanLength = 9;
truckStdDevLength = .6;

vehDimensionParams = ...
    [carMeanHeight     carStdDev        truckMeanHeight     truckStdDev;...
     carMeanWidth      carStdDevWidth   truckMeanWidth      truckStdDevWidth;...
     carMeanLength     carStdDevLength  truckMeanLength     truckStdDevLength];
verbose =1;

SignalFrequency = 2.4*(10^9);

% [vehicles,numTimesteps,numVehiclesPerTimestep] =...
%     parsing.parseSUMOMobility(vehiclesFile,bBoxVehicles);
% save(['inputMobilitySUMO/CologneSUMOMobility_preprocessed.mat'],...
                % 'vehicles','numTimesteps','numVehiclesPerTimestep');
% load('inputMobilitySUMO/CologneSUMOMobility_preprocessed.mat')
% fprintf('Loading Done~~\n');

% load vehicle file
[SUMOFile,vehicles,vehiclesLatLon,numTimesteps,numVehicles,...
        numVehiclesPerTimestep,vehiclesHeight,vehicleMidpoints,...
        vehicleMidpointsLatLon] = ...
        loadFunctions.loadVehicles(vehiclesFile,bBoxVehicles,...
        numRowsPerVehicle,lengthThreshold,vehDimensionParams,verbose);
fprintf('Loading Complete. Try Try~~\n');

% commPairVeh1 = vehicles(1,:);
% commPairVeh2 = vehicles(2,:);

% % commPairDistance = sqrt((commPairVeh1(:,1)-commPairVeh2(:,1)).^2+...
% %     (commPairVeh1(:,2)-commPairVeh2(:,2)).^2);
% Rxheight = 1.13;
% driverHeight = 1.4;
% commPairDistance = 5;

% attenuation = LOSNLOS.bodyShadowing(commPairVeh1,commPairVeh2,1.1,1.1,1.6,2.4*10^9);

