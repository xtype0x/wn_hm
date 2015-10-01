function[largeScalePwr,smallScaleVar,communicationPairs,commPairType,...
    effectivePairRange,objectCellsVehicles]...
    = simOneTimestep(numCommPairs,vehicles,RTreeB,RTreeF,...
    objectCellsBuildings,objectCellsFoliage,commRange,numRowsPerVehicle,...
    boundingBoxesBuildings,boundingBoxesFoliage,vehiclesHeight,...
    MaxNLOSbRange,MaxNLOSvRange,antennaHeight,polarization,vehReflRelPerm,...
    buildingReflRelPerm,freq,txPower,Gt,Gr,PLENLOSb,...
    minFadingLOS,minFadingNLOSv,minFadingNLOSb,maxFadingLOS,maxFadingNLOSv,...
    maxFadingNLOSb,minDensityRange,NLOSvModelType,addNLOSvLoss,verbose,...
    useReflDiffr,commPairArray,vehicleMidpoints)

% SIMONETIMESTEP Executes one simulation timestep. 
%   Builds vehicle, buildings, and foliage R-tree
%   Generates random communication pairs (if not provided)
%   Determines the link type (LOS, NLOSv, NLOSb) between each comm. pair
%   Calculates large- and small-scale signal variation for each comm. pair
%   Calculates received power for each comm. pair
%   (optionally) Calculates 1-interaction reflections and diffractions
% Input:
%   See simSettings.m and simMain.m
%
% Output:
%   largeScalePwr:      large-scale received power (including path-loss and
%                       shadowing) for each communication pair
%   smallScaleVar:      small-scale signal variations for each
%                       communication pair
%   communicationPairs: array containing pairwise IDs of vehicles
%                       participating in communication pairs
%   commPairType:       type of link between a communication pair: LOS = 1,
%                       NLOSb/NLOSf = 2, NLOSv = 3 
%   effectivePairRange: for each communication pair, effectivePairRange is 
%                       the maximum allowed distance from any object to
%                       both Tx and Rx (i.e., it defines an ellipse with Tx
%                       and Rx as foci). Used to search for objects around
%                       Tx and Rx that could have impact on signal
%                       variation (see paper for details).
%   objectCellsVehicles:cell containing outline of all vehicles
%
% Copyright (c) 2014, Mate Boban
ticForTimestep=tic;
% Speed of light (m/s) constant
c = 299792458;
% Build the vehicle R-tree
[bBoxesVehicles,objectCellsVehicles,RTreeV,~] = ...
    RTree.prepareData(vehicles,verbose);
% Get the vehicle midpoints
%vehicleMidpoints = vehicleDimensions.getVehicleMidpoint...
%    (vehicles,numRowsPerVehicle,verbose);

% NB: in comments, the term "link" and "communication pair" are used
% interchangeably 
 
%% Generate and select communication pairs (if not provided)
if isempty(commPairArray)
    % Get all communicating vehicle pairs within commRange
    if exist('rangesearch') == 5
        % Rangesearch command is available in Statistics Toolbox (at
        % least from Matlab version 2011b)
        tic
        communicationPairs = rangesearch...
            (vehicleMidpoints,vehicleMidpoints,commRange);
        if verbose
            fprintf(['Matlab rangesearch command takes %f seconds '...
                'to map the communication pairs.\n'],toc);
        end
    else
        % Use the much slower function for getting all communicating
        % vehicle pairs within commRange
        communicationPairs = commPairs.mapVehicleCommPairs...
            (vehicleMidpoints,commRange,verbose);
    end
    
    % Get a given number of random communication pairs
    communicationPairs = commPairs.getRandCommPairs...
        (communicationPairs,numCommPairs);
    
    % NB: Depending on maximum communication ranges, there might be no
    % communication pairs for the current timestep
    if isempty(communicationPairs)
        fprintf(['There are no communication pairs given the current\n'...
            'maximum communication ranges! Moving to the next timestep'...
            '(if any)...\n']);
        largeScalePwr = [];
        smallScaleVar=[];
        commPairType=[];
        communicationPairs = [];
        effectivePairRange = [];
        objectCellsVehicles = cell(0,0);
        return
    end
else
    if size(commPairArray,2)==2 && max(max(commPairArray))<=...
            size(vehicleMidpoints,1)
        communicationPairs = commPairArray;
    else
        error(['Something is wrong with the supplied communication '...
            'pairs file! The structure of the file is: two columns,'...
            'each row represents vehicle ID for a communication pair.']);
    end
end


%% Calculate the distance between randomly selected communication pairs.
commPairVeh1 = vehicleMidpoints(communicationPairs(:,1),:);
commPairVeh2 = vehicleMidpoints(communicationPairs(:,2),:);

commPairDist = sqrt((commPairVeh1(:,1)-commPairVeh2(:,1)).^2+...
    (commPairVeh1(:,2)-commPairVeh2(:,2)).^2);
    
%% Get links obstructed by buildings    
if ~isempty(RTreeB)
    obstructedCommPairsB = polygonManipulation.getObstructingObjects...
        (communicationPairs,objectCellsBuildings,vehicleMidpoints,RTreeB,-1,1,verbose);
else
    obstructedCommPairsB = zeros(size(communicationPairs,1),1);
end

%% Get links obstructed by foliage (among those not obstructed by buildings)
if ~isempty(RTreeF)
    obstructedCommPairsFList = polygonManipulation.getObstructingObjects...
        (communicationPairs(~obstructedCommPairsB,:),objectCellsFoliage,...
        vehicleMidpoints,RTreeF,-1,0,verbose);
    obstructedCommPairsF2 = ~cellfun(@isempty,obstructedCommPairsFList);
    obstructedCommPairsF = zeros(size(communicationPairs,1),1);
    obstructedCommPairsF(~obstructedCommPairsB) = obstructedCommPairsF2;
else
    obstructedCommPairsF = zeros(size(communicationPairs,1),1);
end
    
%% Get links obstructed by vehicles (among those not obstructed by buildings or foliage)
obstructedCommPairsV = polygonManipulation.getObstructingObjects...
    (communicationPairs(~obstructedCommPairsB & ~obstructedCommPairsF,:),...
    objectCellsVehicles,vehicleMidpoints,RTreeV,numRowsPerVehicle,0,verbose);    
    % Convert array to cell; merge NLOSb and NLOSv. -1 represents comm.
    % pairs that are obstructed by buildings 
    obstructedCommPairs = num2cell(obstructedCommPairsB.*-1);    
    obstructedCommPairs(~(obstructedCommPairsB|obstructedCommPairsF),1)...
        = obstructedCommPairsV;

%% Categorize links/comm. pairs 
%   LOS (line of sight)
%   NLOSb (obstructed by buildings)
%   NLOSf (obstructed by foliage)
%   NLOSv (obstructed by vehicles) 
LOSPairs = cellfun(@isempty,obstructedCommPairs);
NLOSbPairs = logical(obstructedCommPairsB);
NLOSfPairs = logical(obstructedCommPairsF);
NLOSvPairs = ~(NLOSfPairs|NLOSbPairs|LOSPairs);

%% Calculate effective range for each link/comm. pair 
% effectivePairRange is the maximum distance from any object to both Tx and
% Rx in a comm. pair (i.e., effectivePairRange =
% dist(Tx,point)+dist(point,Rx)). Defines an ellipse with Tx and Rx as
% foci. Used to search for objects around Tx and Rx that could have impact
% on signal variation (see paper for details).
effectivePairRange = ones(size(communicationPairs,1),1);
% Effective range for LOS pairs
LOSPairsRange = max(commPairDist(LOSPairs),commRange);
% Effective range for NLOSb pairs
NLOSbPairsRange = max(commPairDist(NLOSbPairs | NLOSfPairs),MaxNLOSbRange);
% Effective range for NLOSv pairs
NLOSvPairsRange = max(commPairDist(NLOSvPairs),MaxNLOSvRange);
% If distance for a NLOSb/NLOSf/NLOSv comm. pair is above respective
% maximum range, assume the pair cannot communicate and set the range to
% Inf. Used for filtering "unfeasible" links
NLOSbPairsRange(NLOSbPairsRange>MaxNLOSbRange)=Inf;
NLOSvPairsRange(NLOSvPairsRange>MaxNLOSvRange)=Inf;
% Set the effective range
effectivePairRange(LOSPairs) = LOSPairsRange;
effectivePairRange(NLOSbPairs | NLOSfPairs) = NLOSbPairsRange;
effectivePairRange(NLOSvPairs) = NLOSvPairsRange;
    
%% Calculate the "real" LOS distance: distance between Tx and Rx that
% accounts for the height of the antennas (i.e., not only location of
% vehicle midpoints).
realLOSDists = sqrt(commPairDist.^2+(vehiclesHeight(communicationPairs...
    (:,1))+antennaHeight-(vehiclesHeight(communicationPairs(:,2))+...
    antennaHeight)).^2);
 
%% Get only those comm. pairs that are designated as "feasible" 
feasibleCommPairs = communicationPairs(effectivePairRange~=Inf,:);
feasibleEffRange = effectivePairRange(effectivePairRange~=Inf);

% Get the IDs of vehicles that are in feasibleCommPairs
IDVehMidpoints = ismember(1:size(vehicleMidpoints,1),...
    unique(feasibleCommPairs));

%% Get objects inside comm. pair ellipse
% Get buildings inside comm. pair ellipse
if ~isempty(boundingBoxesBuildings)
    [jointBuildings,jointDistances] = ...
        polygonManipulation.getObjectsInsideEllipse...
        (feasibleCommPairs,vehicleMidpoints,commRange,feasibleEffRange,...
        IDVehMidpoints,boundingBoxesBuildings,verbose);
else
    jointBuildings = cell(1,size(feasibleCommPairs,1));
    jointDistances = cell(1,size(feasibleCommPairs,1));
end

% Get vehicles inside comm. pair ellipse
if ~isempty(bBoxesVehicles)
    [jointVehicles,jointVehicleDistances] = ...
        polygonManipulation.getObjectsInsideEllipse...
        (feasibleCommPairs,vehicleMidpoints,commRange,feasibleEffRange,...
        IDVehMidpoints,bBoxesVehicles,verbose);
else
    error(['There needs to be at least two vehicles in the system for '...
        'the simulation to make sense!']);
end

% Get foliage inside comm. pair ellipse
if ~isempty(boundingBoxesFoliage)
    [jointFoliage,~] = ...
        polygonManipulation.getObjectsInsideEllipse...
        (feasibleCommPairs,vehicleMidpoints,commRange,feasibleEffRange,...
        IDVehMidpoints,boundingBoxesFoliage,verbose);
else
    jointFoliage = cell(1,size(feasibleCommPairs,1));    
end

% Exclude the vehicles in the comm. pair from the jointVehicles and
% jointVehicleDistances 
fh = str2func('commPairs.excludeCommPairJointVeh');
[jointVehicles,jointVehicleDistances] = cellfun(fh,jointVehicles,...
    jointVehicleDistances,num2cell(feasibleCommPairs,2),'uni',false);

%% Set the comm. pair/link type
% LOS = 1, NLOSb/NLOSf = 2, NLOSv = 3
commPairType = ones(size(communicationPairs,1),1);
commPairType(LOSPairs) = 1;
commPairType(NLOSbPairs | NLOSfPairs) = 2;
commPairType(NLOSvPairs) = 3;

% Get the link type of all feasibleCommPairs and their distances
feasibleCommPairType = commPairType(effectivePairRange~=Inf,:);
feasibleCommPairDists = realLOSDists(effectivePairRange~=Inf);
% Distance for NLOSb comm. pairs (used for reflections and diffractions) 
feasibleCommPairDistsNLOSb = feasibleCommPairDists(feasibleCommPairType==2);

%% Calculate power for NLOSf links
if sum(NLOSfPairs(effectivePairRange~=Inf))>0
    % Get obstructing foliage IDs for NLOSf comm. pairs
    obstructedCommPairsFListTemp = cell(size(communicationPairs,1),1);
    obstructedCommPairsFListTemp(~obstructedCommPairsB) = obstructedCommPairsFList;
    obstructedCommPairsFListObsOnly2 = obstructedCommPairsFListTemp...
        (NLOSfPairs==1 & effectivePairRange~=Inf);
    % Calculate power for NLOSf links
    [powerNLOSf,~] = LOSNLOS.NLOSf(objectCellsFoliage,...
        NLOSfPairs(effectivePairRange~=Inf),feasibleCommPairs,...
        obstructedCommPairsFListObsOnly2,feasibleCommPairDists,...
        vehicleMidpoints,txPower,c,freq,Gt,Gr,verbose);
else
    powerNLOSf = [];    
end

%% Calculate reflections and diffractions
if ~useReflDiffr
    finalReflRays=[];
    reflEfields=[];
    reflDist=[];
    diffrEfields=[];
    finalDiffrRays=[];
    diffrDist=[];
else
    [finalReflRays,feasibleCommPairsNLOSb,jointBuildingsNLOSb,...
        jointDistancesNLOSb,reflEfields,reflDist] = reflections.reflect...
        (feasibleCommPairs,feasibleCommPairType,antennaHeight,jointBuildings,...
        jointDistances,jointVehicles,objectCellsBuildings,objectCellsVehicles,...
        objectCellsFoliage,numRowsPerVehicle,vehicleMidpoints,...
        vehiclesHeight,RTreeB,RTreeV,RTreeF,buildingReflRelPerm,vehReflRelPerm,...
        polarization,txPower,Gt,verbose);
    if ~isempty(finalReflRays)
        [diffrEfields,finalDiffrRays,diffrDist] = diffractions.diffract...
            (finalReflRays,feasibleCommPairsNLOSb,feasibleCommPairDistsNLOSb,...
            feasibleEffRange,feasibleCommPairType,jointBuildingsNLOSb,...
            jointDistancesNLOSb,objectCellsBuildings,objectCellsVehicles,...
            vehicleMidpoints,vehiclesHeight,antennaHeight,RTreeB,RTreeV,...
            txPower,Gt,Gr,c,freq,verbose);
    else
        diffrEfields=[];
        finalDiffrRays=[];
        diffrDist=[];
    end
end

%% Calculate rec. power for both LOS and NLOSv links
[powerNLOSv,~] = LOSNLOS.LOSNLOSv(objectCellsVehicles,LOSPairs,...
    NLOSvPairs,communicationPairs,obstructedCommPairs,effectivePairRange,...
    realLOSDists,vehiclesHeight,vehicleMidpoints,antennaHeight,txPower,c,...
    freq,Gt,Gr,polarization,NLOSvModelType,addNLOSvLoss,verbose);

%% Calculate small-scale signal variations
smallScaleVar = smallScaleVariations.smallScaleVariation(feasibleCommPairs,...
    jointVehicles,jointBuildings,jointFoliage,feasibleEffRange,...
    effectivePairRange,minDensityRange,objectCellsBuildings,objectCellsFoliage,...
    LOSPairs,NLOSvPairs,NLOSbPairs|NLOSfPairs,minFadingLOS,minFadingNLOSv,...
    minFadingNLOSb,maxFadingLOS,maxFadingNLOSv,maxFadingNLOSb);

%% Combine large-scale signal variation results for all comm. pair types in one array
largeScalePwr = powerCalculations.largeScaleVariations...
    (communicationPairs,LOSPairs,NLOSvPairs,NLOSbPairs,NLOSfPairs,...
    effectivePairRange,useReflDiffr,powerNLOSv,powerNLOSf,finalReflRays,...
    reflEfields,realLOSDists,reflDist,finalDiffrRays,diffrEfields,...
    diffrDist,c,freq,txPower,Gr,Gt,PLENLOSb);

%% body shadowing by wn group 10 ^.<
shadowingAttenuation = LOSNLOS.bodyShadowing(commPairVeh1,commPairVeh2,...
    vehiclesHeight(communicationPairs(:,1)),vehiclesHeight(communicationPairs(:,2)),1.46,freq);

largeScalePwr = largeScalePwr - shadowingAttenuation(effectivePairRange~=Inf);

%% Display the time taken to simulate one timestep
if verbose
    fprintf('Simulating current time-step took %f seconds.\n',...
        toc(ticForTimestep));
end
