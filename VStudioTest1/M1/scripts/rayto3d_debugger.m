clear
% Load localization data
indata = dlmread('m1_timing_record2_output.csv',';');

doVerboseRays=0;
maxFrameNum=0;  % if 0, all frames are processed
skipCAM0=1;

%camcount=2;
%data=zeros(camcount,columns(indata)-1);
results=[];
resultcount=0;
currentFrame = 0;
markerCount = 0;
usedRayNumber = [];     % Number of used rays for a given result
usedFrameIndex = [];    % Frame index for a given result
n=0;
h = figure;
hold on;
for i=1:size(indata,1)
    if(n>maxFrameNum && maxFrameNum>0)
        break;
    end
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
      
      n=n+1;
      
      % FIXME works only for two cameras
%      D=[b(1,:)'-a(1,:)',b(2,:)'-a(2,:)'];
%      C=(a(2,:)-a(1,:))';
      t=(D'*D)^-1*D'*A;
      results(resultcount,:)=(a(:,1)+d(:,1)*t(1))';
    
      if (doVerboseRays>0)
          plot3(results(resultcount,1),results(resultcount,2),results(resultcount,3),'bx','MarkerSize',5);

          % show rays
          X = [];
          Y = [];
          Z = [];
          for rayIdx = 1:size(a,2)
              pointA = a(1:3,rayIdx);   % a and b are column vectors
              pointB = b(1:3,rayIdx);
              % show camera locations
              plot3(pointA(1),pointA(2),pointA(3),'ro','MarkerSize',10);
              plot3(pointB(1),pointB(2),pointB(3),'rs','MarkerSize',5);

              %plot3([pointA(1) pointB(1)],[pointA(2) pointB(2)],[pointA(3) pointB(3)],'g','LineWidth',3);
              plot3([pointA(1) pointB(1)],[pointA(2) pointB(2)],[pointA(3) pointB(3)],'g','LineWidth',1);
          end
      end
      

    end
    currentFrame = indata(i,2);
    markerCount = 0;
  end
  if (indata(i,1)~=0 && skipCAM0==1)   % skip CAM0
      markerCount = markerCount + 1;
      data(markerCount,:) = indata(i,3:end);
  end
end

dlmwrite('rayto3d_result.csv',[results(:,1:3) usedFrameIndex' usedRayNumber'],' ');

