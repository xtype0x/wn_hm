% RUNSIMULATION Runs the simulation. 
%   Loads the settings from simSettings.m. 
%   Saves the simulation output to .mat and .csv files. 
%   Generates Google Earth Visualization.
%   For details on variables, see simSettings.m and simMain.m
%
% Usage: runSimulation
%
% Copyright (c) 2014, Mate Boban

clear all;
clear global;

fprintf('GEMV^2: Geometry-based Efficient propagation Model for V2V communication\n');
fprintf('Copyright (c) 2014, Mate Boban\n');

% Load the simulation settings (contained and explained in simSettings.m)
simSettings;

%% Run simMain (for explanation of returned variables, see simMain.m)
[largeScalePwrCell,smScaleVarCell,commPairsCell,commPairsCellAll,...
        commPairTypeCell,commPairsLatLon,effectivePairRangeCell,...
        numNeighborsCell,vehiclesLatLon,vehiclesHeight,buildingsLatLon,foliageLatLon,...
        numCommPairsPerTimestep,numTimesteps,numVehiclesPerTimestep,...
        vehicleMidpointsLatLon] = simMain(vehiclesFile,bBoxVehicles,...
        staticFile,foliageFile,toleranceBuildings,commRange,numRowsPerVehicle,...
        numCommPairs,vehDimensionParams,lengthThreshold,maxNLOSbRange,...
        maxNLOSvRange,antennaHeight,polarization,vehReflRelPerm,...
        buildingReflRelPerm,freq,txPower,Gt,Gr,PLENLOSb,minFadingLOS,minFadingNLOSv,...
        minFadingNLOSb,maxFadingLOS,maxFadingNLOSv,maxFadingNLOSb,minDensityRange,...
        NLOSvModelType,addNLOSvLoss,useReflDiffr,verbose,commPairArray);

%% Save output in a matlab .mat and comma-separated .csv file.
% Description of output variables:
%   
%   largeScalePwr:              received power for each comm. pair based on
%                               the large-scale path loss model
%   smScaleVar:                 small-scale variation for each comm. pair
%   commPairs:                  list of comm. pairs that are within
%                               the maximum comm. range for the link type
%                               they form. Max comm. range for link type is
%                               defined in simSettings.m
%   commPairsAll:               list of all generated communication pairs.
%                               NB: the list can, but may not contain all
%                               the communication pairs in the system
%                               (determined by variable numCommPairs in
%                               simSettings.m (if set to Inf, commPairsAll
%                               contains all pairs within comm. range in
%                               the system)
%   commPairType:               comm. pair link type (1-LOS; 2-NLOSb; 3-NLOSv)
%   effectivePairRange:         effective communication range for a pair.
%                               It is set to Inf if comm. pair if the
%                               vehicles are not within the maximum comm.
%                               range for the link type
%   numNeighborsPerVehicle:     Number of neighboring vehicles for which
%                               the received power is above rec. threshold
%   numCommPairsPerTimestep:    Number of feasible communication pairs for
%                               each timestep (i.e., number of pairs for
%                               which effectivePairRange~=Inf). This
%                               variable is useful for getting the
%                               per-timestep information from e.g.,
%                               largeScalePwr or smScaleVar

% Save variables to .mat file
fileName = ['outputSim/',date,'_outputSim.mat'];
save(fileName,'largeScalePwrCell','smScaleVarCell','commPairsCell',...
    'commPairsCellAll','commPairTypeCell','effectivePairRangeCell','numCommPairsPerTimestep');

% Turn cells to arrays
largeScalePwrCelltoArray = cell2mat(largeScalePwrCell);
smScaleVarCelltoArray = cell2mat(smScaleVarCell);
commPairsCelltoArray = cell2mat(commPairsCell);
commPairsCellAlltoArray = cell2mat(commPairsCellAll);
commPairTypeCelltoArray = cell2mat(commPairTypeCell);
effectivePairRangeCelltoArray = cell2mat(effectivePairRangeCell);
numNeighborsCelltoArray = cell2mat(numNeighborsCell);

% Save variables to .csv files
fileNamelargeScalePwrCell = ['outputSim/',date,'_largeScalePwr.csv'];
fileNamesmScaleVarCell = ['outputSim/',date,'_smScaleVar.csv'];
fileNamecommPairsCell = ['outputSim/',date,'_commPairs.csv'];
fileNamecommPairsCellAll = ['outputSim/',date,'_commPairsAll.csv'];
fileNamecommPairTypeCell = ['outputSim/',date,'_commPairType.csv'];
fileNameeffectivePairRangeCell = ['outputSim/',date,'_effectivePairRange.csv'];
fileNamenumNeighborsCell = ['outputSim/',date,'_numNeighborsPerVehicle.csv'];
fileNamenumCommPairsPerTimestepCell = ['outputSim/',date,'_numCommPairsPerTimestep.csv'];

dlmwrite(fileNamelargeScalePwrCell,largeScalePwrCelltoArray);
dlmwrite(fileNamesmScaleVarCell,smScaleVarCelltoArray);
dlmwrite(fileNamecommPairsCell,commPairsCelltoArray);
dlmwrite(fileNamecommPairsCellAll,commPairsCellAlltoArray);
dlmwrite(fileNameeffectivePairRangeCell,effectivePairRangeCelltoArray);
dlmwrite(fileNamenumNeighborsCell,numNeighborsCelltoArray);
dlmwrite(fileNamenumCommPairsPerTimestepCell,numCommPairsPerTimestep);

%% Google Earth Visualization
if GEVisualize
    % Run Google Earth output function
    plotFunctions.GEOutput(plotPoly,plotRxPwr,plotNeighbors,...
        vehicleMidpointsLatLon,vehiclesLatLon,vehiclesHeight,...
        numRowsPerVehicle,buildingsLatLon,foliageLatLon,verbose,...
        numNeighborsCell,numCommPairsPerTimestep,commPairsCell,0,...
        largeScalePwrCell,smScaleVarCell,numTimesteps,numVehiclesPerTimestep);
end