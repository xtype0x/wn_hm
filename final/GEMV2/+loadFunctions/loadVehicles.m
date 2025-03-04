function[SUMOFile,vehicles,vehiclesLatLon,numTimesteps,numVehicles,...
    numVehiclesPerTimestep,vehiclesHeight,vehicleMidpoints,...
    vehicleMidpointsLatLon] = loadVehicles(vehiclesFile,bBoxVehicles,...
    numRowsPerVehicle,lengthThreshold,vehDimensionParams,verbose)
% LOADVEHICLES  Loads vehicle file.
%   Assigns dimensions to vehicles (if not already provided).
%   Saves preprocessed vehicle outlines (so that the vehicle outline
%   processing can be done only once).
%
% Input: see simMain.m
%
% Output:
%   SUMOFile:                   Whether or not the provided vehicles file
%                               is a SUMO (xml) file
%   vehicles                    vehicle outlines in UTM in a three-column
%                               format: [ID,X,Y]  
%   vehiclesLatLon:             vehicle outlines in lat/lon in a
%                               three-column format: [ID,Lat,Lon]  
%   numTimesteps:               number of simulation timesteps
%   numVehicles:                number of vehicles
%   numVehiclesPerTimestep:     number of vehicles per timestep
%   vehiclesHeight:             vehicle heights
%
% Copyright (c) 2014, Mate Boban


SUMOFile=0;
numVehicles = zeros(0);
numVehiclesPerTimestep = zeros(0);
numTimesteps = zeros(0);

%% Loading or  saving vehicle file
if ischar(vehiclesFile)
    [pathstrVehicles, nameVehicles, extVehicles] = fileparts(vehiclesFile);
    if strcmpi(extVehicles,'.xml')
        % If XML file is provided, simulation assumes that the mobility
        % file is generated by SUMO (more specifically, Floating Car Data:
        % http://sumo-sim.org/userdoc/Simulation/Output/FCDOutput.html).
        SUMOFile=1;
    end
    if SUMOFile
        if length(dir([pathstrVehicles,'/', nameVehicles, '_preprocessed.mat'])) == 1
            disp('Loading preprocessed SUMO mobility file ...');
            % Load the stored vehicle mobility data.
            load([pathstrVehicles,'/', nameVehicles, '_preprocessed.mat']);
        else
            [vehicles,numTimesteps,numVehiclesPerTimestep] = ...
                parsing.parseSUMOMobility(vehiclesFile,bBoxVehicles);
            % Set all vehicle types that are not trucks (i.e., ==1) to
            % cars (i.e., =0)
            vehicles(vehicles(:,4)~=1,4)=0;
            disp('Saving preprocessed SUMO mobility file...');
            save([pathstrVehicles,'/',nameVehicles,'_preprocessed.mat'],...
                'vehicles','numTimesteps','numVehiclesPerTimestep');
        end
    else
        vehicles = load(vehiclesFile);
        % Assumption: if the provided mobility file is not SUMO, then
        % there is a fixed number of vehicles per timestep, which do
        % not change (i.e., the vehicle IDs are the same in all
        % timesteps).
        vehIDs = unique(vehicles(:,1));
        numVehicles = numel(vehIDs);
        numVehiclesPerTimestep = numVehicles;
        % Number of timesteps is equal to the number of rows in the
        % input file divided by the number of unique vehicles.
        numTimesteps = size(vehicles,1)/(numVehicles*numRowsPerVehicle);
    end
else
    vehicles = vehiclesFile;
end

%% Generate vehicle outlines and set their height
if size(vehicles,2)==3 || size(vehicles,2)>4
    % Simulation needs the Lat/Long coordinates for Google Earth Visualization
    vehiclesLatLon = vehicles;
    % Convert Lat/Long coordinates to UTM
    [xxV,yyV,zzV] = externalCode.deg2utm.deg2utm(vehicles(:,2),vehicles(:,3));
    vehicles(:,2) = yyV;
    vehicles(:,3) = xxV;
    if size(vehicles,2)==3
        % The supplied file contains vehicle outlines
        vehicleMidpointsLatLon = vehicleDimensions.getVehicleMidpoint...
            (vehiclesLatLon,numRowsPerVehicle,verbose);
        vehicleMidpoints = vehicleDimensions.getVehicleMidpoint...
            (vehicles,numRowsPerVehicle,verbose);
    else
        vehicleMidpointsLatLon = vehiclesLatLon(:,[2 3]);
        vehicleMidpoints = [yyV,xxV];
        % The supplied file contains vehicle midpoints, vehicle type, and
        % bearing. Generate vehicle polygons.
        vehicles = vehicleDimensions.generateVehiclePolygons(vehicles,...
            vehDimensionParams,verbose);
        % Convert newly created points to Lat/Lon (for Google Earth Visualization)
        [LatV,LonV] = externalCode.utm2deg.utm2deg(vehicles(:,3),...
        vehicles(:,2),repmat(zzV(1,:),length(vehicles),1));
        vehiclesLatLon = [vehicles(:,1),LatV,LonV];               
    end    
    % Set the height of the vehicles.
    vehiclesHeight = vehicleDimensions.getVehicleHeight(vehicles,...
        numRowsPerVehicle,vehDimensionParams,lengthThreshold,verbose);   
else
    error(['Vehicle array needs to be in a three-column format: [ID,Lat,Lon] '...
           'or six-column format: [ID,midpointLat,midpointLong,vehicleType,vehicleBearing,angle]']);
end