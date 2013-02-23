clear
% Load localization data
%indata = dlmread('m1_rays_output_recording1.csv',';');
indata = dlmread('m1_timing_record1_output.csv',';');

%camcount=2;
%data=zeros(camcount,columns(indata)-1);
results=[];
resultcount=0;
currentFrame = 0;
markerCount = 0;
usedRayNumber = [];     % Number of used rays for a given result
usedFrameIndex = [];    % Frame index for a given result

for i=1:size(indata,1)
  if (indata(i,2) > currentFrame)
    if (markerCount >= 2)
      a=data(:,1:4)';
      b=data(:,5:8)';
      d=b-a;
      resultcount=resultcount+1;
      usedFrameIndex(resultcount)=currentFrame;
      if (markerCount == 2)
        D=[d(:,1),-d(:,2)]; % this was wrong
        A=[a(:,2)-a(:,1)];
        usedRayNumber(resultcount)=2;
      end
      if (markerCount == 3)
          D = zeros(8,3);
          D(1:4,1)=d(:,1);
          D(5:8,1)=d(:,1);
          D(1:4,2)=-d(:,2);
          D(5:8,3)=-d(:,3);

          %D=[-d(:,1),d(:,2),zeros(4,1);-d(:,1),zeros(4,1),d(:,2)];
          A=[a(:,2)-a(:,1);a(:,3)-a(:,1)];
        usedRayNumber(resultcount)=3;
      end
      
      % FIXME works only for two cameras
%      D=[b(1,:)'-a(1,:)',b(2,:)'-a(2,:)'];
%      C=(a(2,:)-a(1,:))';
      t=(D'*D)^-1*D'*A;
      results(resultcount,:)=(a(:,1)+d(:,1)*t(1))';
    
      
      % show rays
      X = [];
      Y = [];
      Z = [];
      for rayIdx = 1:size(a,2)
          pointA = a(1:3,rayIdx);   % a and b are column vectors
          pointB = b(1:3,rayIdx);
          % show camera locations
          %plot3(pointA(1),pointA(2),pointA(3),'ro','MarkerSize',10);
          %plot3(pointB(1),pointB(2),pointB(3),'rs','MarkerSize',5);

          %plot3([pointA(1) pointB(1)],[pointA(2) pointB(2)],[pointA(3) pointB(3)],'g','LineWidth',3);
      end
      

    end
    currentFrame = indata(i,2);
    markerCount = 0;
      end
  markerCount = markerCount + 1;
  data(markerCount,:) = indata(i,3:end);
end

%results = results .* (36/50);  % just for failed chessboard sizing :)
dlmwrite('rayto3d_result.csv',[results(:,1:3) usedFrameIndex' usedRayNumber'],' ');

