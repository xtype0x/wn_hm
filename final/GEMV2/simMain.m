function[largeScalePwrCell,smallScaleVarCell,communicationPairsCell,...
    communicationPairsCellAll,commPairTypeCell,communicationPairsLatLon,...
    effectivePairRangeCell,numNeighborsPerVehPerIntervalCell,vehiclesLatLon,...
    vehiclesHeight,buildingsLatLon,foliageLatLon,numCommPairsPerTimestep,...
    numTimesteps,numVehiclesPerTimestep,vehicleMidpointsLatLon]...
    = simMain(vehiclesFile,bBoxVehicles,staticFile,foliageFile,toleranceBuildings,...
    commRange,numRowsPerVehicle,numCommPairs,vehDimensionParams,lengthThreshold,...
    maxNLOSbRange,maxNLOSvRange,antennaHeight,polarization,vehReflRelPerm,...
    buildingReflRelPerm,freq,txPower,Gt,Gr,PLENLOSb,minFadingLOS,minFadingNLOSv,...
    minFadingNLOSb,maxFadingLOS,maxFadingNLOSv,maxFadingNLOSb,minDensityRange,...
    NLOSvModelType,addNLOSvLoss,useReflDiffr,verbose,commPairArray)
% SIMMAIN   Sets up simulation environment.
%   Loads the object outlines (vehicles, buildings, foliage).
%   Initiates output variables.
%   Runs simulation for each time step.
% Input:   
%   see simSettings.m
% Output: 
%   largeScalePwrCell:  received power for each communication pair based on
%                       the large-scale path loss model
%   smallScaleVarCell:  small-scale variations for each pair
%   communicationPairsCell:  list of randomly generated communication
%                       pairs for which received power was deemed to be
%                       significant based on the communication ranges and
%                       link types
%   communicationPairsCellAll: list of all randomly generated communication
%                       pairs. NB: it can contain all the communication
%                       pairs in the system
%   commPairTypeCell:   comm. pair link type (1-LOS; 2-NLOSb; 3-NLOSv)
%   effectivePairRangeCell: effective communication range for a pair.
%                       Based on the link type comm. range in
%                       simSettings.m. Set to Inf if comm. pair is deemed
%                       to have insignificant rec. power based on current
%                       simulation settings
%   numNeighborsPerVehPerIntervalCell: 
%   vehiclesLatLon:     latitude and longitude coordinates of vehicle
%                       outlines
%   numNeighborsPerVehPerIntervalCell: number of neighbors per vehicle and
%                       per timestep
%   vehiclesHeight: 
%   buildingsLatLon:    latitude and longitude coordinates of buildings
%   foliageLatLon:      latitude and longitude coordinates of foliage
%   numCommPairsPerTimestep: number of comm. pair per timestep
%   numTimesteps:       number of simulation timesteps
%   numVehiclesPerTimestep: number of vehicles per simulation timestep
%   vehicleMidpointsLatLon: latitude and longitude coordinates of vehicle
%                       	midpoints
%
% Copyright (c) 2014, Mate Boban

if nargin~=33
    error('Wrong number of input parameters');
end

% Load vehicle outlines
if ~isempty(vehiclesFile)
    [SUMOFile,vehicles,vehiclesLatLon,numTimesteps,numVehicles,...
        numVehiclesPerTimestep,vehiclesHeight,vehicleMidpoints,...
        vehicleMidpointsLatLon] = ...
        loadFunctions.loadVehicles(vehiclesFile,bBoxVehicles,...
        numRowsPerVehicle,lengthThreshold,vehDimensionParams,verbose);
else
    error('Vehicle array cannot be empty.');
end

%% Load building and foliage outlines
[buildings,buildingsLatLon,boundingBoxesBuildings,objectCellsBuildings,...
    BigBoxesBuildings,foliage,foliageLatLon,boundingBoxesFoliage,...
    objectCellsFoliage,BigBoxesFoliage] =...
    loadFunctions.loadBuildingsFoliage(staticFile,foliageFile,...
    toleranceBuildings,verbose);

%% Load communication pairs (if any)
commPairArray = load(commPairArray);

%% Run simulation for each timestep
if round(numTimesteps)~=numTimesteps
    error('Something is wrong with the input file containing vehicle polygons');
end

% Initialize cells and arrays used to collect info from all timesteps
largeScalePwrCell = cell(numTimesteps,1);
smallScaleVarCell = cell(numTimesteps,1);
communicationPairsCell = cell(numTimesteps,1);
communicationPairsCellAll = cell(numTimesteps,1);
commPairTypeCell = cell(numTimesteps,1);
effectivePairRangeCell = cell(numTimesteps,1);
numNeighborsPerVehPerIntervalCell = cell(numTimesteps,1);
communicationPairsLatLon = zeros(0);
numCommPairsPerTimestep = zeros(numTimesteps,1);

% Counter for rows in vehicle array in case of SUMOFile (i.e., when
% there are non-fixed number of vehicles per timestep)
vehicleRowCounter = 0;

for kk = 1:numTimesteps
    % Get the current vehicle data (outlines and height)
    if ~SUMOFile
        currVehicles = vehicles((kk-1)*numVehicles*numRowsPerVehicle+...
            1:kk*numVehicles*numRowsPerVehicle,:);
        currVehiclesHeight = vehiclesHeight((kk-1)*numVehicles+...
            1:kk*numVehicles);
        currVehicleMidpoints = vehicleMidpoints((kk-1)*numVehicles+...
            1:kk*numVehicles,:);
        % Set the number of vehicles for current timestep
        numVehiclesPerTimestep=numVehicles;
    else
        currVehicles = vehicles(vehicleRowCounter+1:vehicleRowCounter...
            +numVehiclesPerTimestep(kk)*numRowsPerVehicle,:);
        currVehiclesHeight = vehiclesHeight(vehicleRowCounter/numRowsPerVehicle+...
            1:vehicleRowCounter/numRowsPerVehicle+numVehiclesPerTimestep(kk));
        currVehicleMidpoints = vehicleMidpoints(vehicleRowCounter/numRowsPerVehicle+...
            1:vehicleRowCounter/numRowsPerVehicle+numVehiclesPerTimestep(kk),:);
        vehicleRowCounter = vehicleRowCounter+size(currVehicles,1);
    end
    % Simulate current timestep
    [largeScalePwr,smallScaleVar,communicationPairs,commPairType,effectivePairRange,~]...
        = simOneTimestep(numCommPairs,currVehicles,BigBoxesBuildings,...
        BigBoxesFoliage,objectCellsBuildings,objectCellsFoliage,commRange,...
        numRowsPerVehicle,boundingBoxesBuildings,boundingBoxesFoliage,currVehiclesHeight,...
        maxNLOSbRange,maxNLOSvRange,antennaHeight,polarization,...
        vehReflRelPerm,buildingReflRelPerm,freq,txPower,Gt,Gr,...
        PLENLOSb,minFadingLOS,minFadingNLOSv,minFadingNLOSb,maxFadingLOS,...
        maxFadingNLOSv,maxFadingNLOSb,minDensityRange,NLOSvModelType,...
        addNLOSvLoss,verbose,useReflDiffr,commPairArray,currVehicleMidpoints);
    
    % Store the simulation results from current timestep in cells
    largeScalePwrCell{kk,1} = largeScalePwr;
    smallScaleVarCell{kk,1} = smallScaleVar;
    commPairTypeCell{kk,1} = commPairType;
    communicationPairsCellAll{kk,1} = communicationPairs;
    communicationPairsCell{kk,1} = communicationPairs(effectivePairRange~=Inf,:);
    effectivePairRangeCell{kk,1} = effectivePairRange;
    numCommPairsPerTimestep(kk) = length(largeScalePwr);
    
    %% Analyze the neighborhood of each vehicle
    uniqueVehiclesCommPairs = unique(communicationPairsCell{kk,1}(:));
    numNeighborsPerVehPerInterval = zeros(length(uniqueVehiclesCommPairs),1);
    for ii=1:length(uniqueVehiclesCommPairs)
        numNeighborsPerVehPerInterval(ii) = sum(communicationPairsCell{kk,1}(:)...
            ==uniqueVehiclesCommPairs(ii));
    end
    
    % Store vehicle IDs (from vehicleMidpoints) in first column of the
    % cell, and number of vehicles in second
    numNeighborsPerVehPerIntervalCell{kk,1} = uniqueVehiclesCommPairs;
    numNeighborsPerVehPerIntervalCell{kk,2} = numNeighborsPerVehPerInterval;
    fprintf('Time-step: %i\n',kk);
   
end
end