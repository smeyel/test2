clear;
% For every location measurement in the stable frames
%   LocationID, x, y, z, rayNum
%   LocationID is the index of the robot arm path point (stable frame interval)
resultsWithLocationID = dlmread('stableframeDataProcessor_resultsWithLocationID.csv',' ');
indata = dlmread('stableframeDataProcessor_stats.csv',' ');

% Statistics for every stable frame interval (location)
LocationMean2Ray = indata(:,1:3);
LocationMean3Ray = indata(:,4:6);
LocationMeanAll = indata(:,7:9);
LocationStd2Ray = indata(:,10:12);
LocationStd3Ray = indata(:,13:15);
LocationStdAll = indata(:,16:18);
LocationEffectiveStd2Ray = indata(:,19);
LocationEffectiveStd3Ray = indata(:,20);
LocationEffectiveStdAll = indata(:,21);

% visualization - show stable result point
res = resultsWithLocationID;
h = figure;
hold on;
% show results using 2 rays
idx = find(res(:,5) == 2);
plot3(res(idx,2),res(idx,3),res(idx,4),'bx','MarkerSize',5,'MarkerFaceColor','b');
% show results using 3 rays
idx = find(res(:,5) == 3);
plot3(res(idx,2),res(idx,3),res(idx,4),'ro','MarkerSize',5,'MarkerFaceColor','r');
legend('2 rays','3 rays');
grid on;
xlabel('X');    % Chessboard coordinate system
ylabel('Z');
zlabel('Y');
%saveas(h,'m1_results_stableOnly.png','png');
%saveas(h,'m1_results_stableOnly.eps','eps');

% Visualize statistics
h = figure;
bar(LocationStdAll(:,[1 3 2]));
axis([1 size(LocationStdAll,1) 0 2]);
legend('X','Y','Z');
xlabel('Location index');
ylabel('Standard deviation');

h = figure;
bar(LocationEffectiveStdAll);
axis([1 length(LocationStdAll) 0 2]);
xlabel('Location index');
ylabel('Standard deviation');

% Calculate distance between points after each other
diatance = [];
for i=1:(size(LocationMeanAll,1)-1)
    A = LocationMeanAll(i,:);
    B = LocationMeanAll(i+1,:);
    d = B - A;
    distance(i) = norm(d);
end

groundTruth=[ ...
    250 150 ...
    250 250 616 ...
    250 250 150 ...
    250 250 150 ...
    250 250 100 ...
    250 250 150 ...
    250 250 150 ...
    250 250 100 ...
    250 250 150 ...
    250 250 150 ...
    250 250 616 ...
    250 250 150 ...
    250 250 150 ...
    250 250 ...
    ];

h = figure;
bar(distance);
xlabel('Location index');
ylabel('relative distance');

h = figure;
bar(distance-groundTruth);
xlabel('Location index');
ylabel('relative distance error');


