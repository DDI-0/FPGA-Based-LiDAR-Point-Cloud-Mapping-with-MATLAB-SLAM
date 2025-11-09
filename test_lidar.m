
% Set SLAM parameters for RPLIDAR A1M8
maxLidarRange = 6; % lidar range
mapResolution = 20; % 5cm precision 
slamAlg = lidarSLAM(mapResolution, maxLidarRange);
slamAlg.LoopClosureThreshold = 210; 
slamAlg.LoopClosureSearchRadius = 8; 

% Load single scan from CSV
data = readmatrix('test_file.csv', 'NumHeaderLines', 2); % Skip header lines    
angles = data(:,1) * pi/180; %   from degrees to radians
distances = data(:,2) / 1000; %  from mm to meters

% Create lidarScan object
scan = lidarScan(distances, angles);

% Data check
[isScanAccepted, loopClosureInfo, optimizationInfo] = addScan(slamAlg, scan);
if isScanAccepted
    fprintf('Added single scan\n');
else
    fprintf('Scan not accepted\n');
end

% plot the result
figure;
show(slamAlg);
title('SLAM Map using RPLIDAR A1M8');