clear;
indata = dlmread('rayto3d_result.csv',' ');
points = [indata(:,1) -indata(:,3) indata(:,2)];
usedFrameIndex = indata(:,4);
usedRayNumber = indata(:,5);
% Load list of stable frame intervals
stableframes = dlmread('stableframes_m1_recording2.csv',';');

% visualization - show result point
h = figure;
hold on;
% show results using 2 rays
idx = find(usedRayNumber == 2);
plot3(points(idx,1),points(idx,2),points(idx,3),'bx','MarkerSize',5,'MarkerFaceColor','b');
% show results using 3 rays
idx = find(usedRayNumber == 3);
plot3(points(idx,1),points(idx,2),points(idx,3),'ro','MarkerSize',5,'MarkerFaceColor','r');
legend('2 rays','3 rays');
grid on;
xlabel('X');    % Chessboard coordinate system
ylabel('Z');
zlabel('Y');
%saveas(h,'m1_results.png','png');
%saveas(h,'m1_results.eps','eps');

% Visualization - show only stable points (locations of the robotic arm)
resultsWithLocationID=[];    % locationID, X,Y,Z, rayNum

% Stable point localization results (x, y, z)
LocationMean2Ray=[];
LocationMean3Ray=[];
LocationMeanAll=[];
LocationStd2Ray=[];
LocationStd3Ray=[];
LocationStdAll=[];
LocationEffectiveStd2Ray=[];
LocationEffectiveStd3Ray=[];
LocationEffectiveStdAll=[];


for i=1:size(stableframes,1)    % for every stable frame
    frameIndices = stableframes(i,1) : stableframes(i,2);
    resultMask = ismember(usedFrameIndex,frameIndices);
    resultIndices = find(resultMask == 1);
    
    locationIDs = ones(length(resultIndices),1)*i;
    pointsSub = points(resultIndices,1:3);
    usedRayNumSub = usedRayNumber(resultIndices);
    
    newRows = [ locationIDs, pointsSub, usedRayNumSub];
    
    startIdx = size(resultsWithLocationID,1)+1;
    endIdx = startIdx+size(newRows,1)-1;
    
    resultsWithLocationID(startIdx:endIdx,1:5) = newRows;
    
    % further processing (calculating means, std...)
    ray2idx = find(usedRayNumSub==2);
    ray3idx = find(usedRayNumSub==3);
    
    LocationMean2Ray(i,1:3)=mean(pointsSub(ray2idx,:));
    LocationMean3Ray(i,1:3)=mean(pointsSub(ray3idx,:));
    LocationMeanAll(i,1:3)=mean(pointsSub);
    LocationStd2Ray(i,1:3)=std(pointsSub(ray2idx,:));
    LocationStd3Ray(i,1:3)=std(pointsSub(ray3idx,:));
    LocationStdAll(i,1:3)=std(pointsSub);
    
    LocationEffectiveStd2Ray(i)=norm(LocationStd2Ray(i));
    LocationEffectiveStd3Ray(i)=norm(LocationStd3Ray(i));
    LocationEffectiveStdAll(i)=norm(LocationStdAll(i));
end

% For every location measurement in the stable frames
%   pointID, x, y, z, rayNum
%   pointID is the index of the robot arm path point (stable frame interval)
dlmwrite('stableframeDataProcessor_resultsWithLocationID.csv',resultsWithLocationID,' ');

% For every stable frame interval (location)
saveddata = ...
    [ ...
        LocationMean2Ray ...
        LocationMean3Ray ...
        LocationMeanAll ...
        LocationStd2Ray ...
        LocationStd3Ray ...
        LocationStdAll ...
        LocationEffectiveStd2Ray' ...
        LocationEffectiveStd3Ray' ...
        LocationEffectiveStdAll' ...
    ];
dlmwrite('stableframeDataProcessor_stats.csv',saveddata,' ');
