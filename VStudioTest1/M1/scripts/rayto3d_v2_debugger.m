clear
% Load localization data
indata = dlmread('m1_timing_record1_output.csv',';');

doVerboseRays = 0;
doVerboseResults = 0;
maxFrameNum=0;  % if 0, all frames are processed
processOnlyThisFrame = 0;   % 0: Process all frames
skipCAM0=0;

%camcount=2;
%data=zeros(camcount,columns(indata)-1);
results=[];
resultcount=0;
currentFrame = 0;
markerCount = 0;
usedRayNumber = [];     % Number of used rays for a given result
usedFrameIndex = [];    % Frame index for a given result
maxError = 0;
maxErrorAt = 0;
n=0;
h = figure;
hold on;
grid on;
for i=1:size(indata,1)
    if(n>maxFrameNum && maxFrameNum>0)
        break;
    end
  if (indata(i,2) > currentFrame)
    if (markerCount >= 2)
      n=n+1;
      if (processOnlyThisFrame > 0 && processOnlyThisFrame ~= currentFrame)
          continue;
      end
      % ---------- old method
      A=data(:,1:3)';   % We omit the last (0) coordinate...
      
      % Skip if the same camera provided more tham one result
      if (hasTwoEqualColumns(A)>0)
          %continue;
      end
      
      B=data(:,5:7)';
      V=B-A;
      [p,e] = getIntersection3D(A,V);
      sumE = sum(e);
     
      resultcount=resultcount+1;
      sumError(resultcount) = sumE;
      results_new(resultcount,:)=p';
      % ---------- old method
      a=data(:,1:4)';
      b=data(:,5:8)';
      d=b-a;
      if (markerCount == 2)
        Do=[d(:,1),-d(:,2)]; % this was wrong
        Ao=[a(:,2)-a(:,1)];
      end
      if (markerCount == 3)
          Do = zeros(8,3);
          Do(1:4,1)=d(:,1);
          Do(5:8,1)=d(:,1);
          Do(1:4,2)=-d(:,2);
          Do(5:8,3)=-d(:,3);
          Ao=[a(:,2)-a(:,1);a(:,3)-a(:,1)];
      end
      t=(Do'*Do)^-1*Do'*Ao;
      p_old = a(:,1)+d(:,1)*t(1);
      results_old(resultcount,:)=p_old';
      % -------------
      usedFrameIndex(resultcount)=currentFrame;
      usedRayNumber(resultcount)=markerCount;

      new_old_diff=norm(p-p_old(1:3));
      if (new_old_diff>maxError)
          disp 'Highest error...'
          maxError = new_old_diff;
          maxErrorAt = currentFrame;
      end
      
      if (doVerboseResults>0)
          plot3(results_new(resultcount,1),results_new(resultcount,2),results_new(resultcount,3),'bx','MarkerSize',5);
          plot3(results_old(resultcount,1),results_old(resultcount,2),results_old(resultcount,3),'bo','MarkerSize',5);
      end
      if (doVerboseRays>0)
          % show rays
          X = [];
          Y = [];
          Z = [];
          for rayIdx = 1:size(A,2)
              pointA = A(1:3,rayIdx);   % a and b are column vectors
              pointB = B(1:3,rayIdx);
              % show camera locations
              plot3(pointA(1),pointA(2),pointA(3),'ro','MarkerSize',10);
              plot3(pointB(1),pointB(2),pointB(3),'rs','MarkerSize',5);

              %plot3([pointA(1) pointB(1)],[pointA(2) pointB(2)],[pointA(3) pointB(3)],'g','LineWidth',3);
              plot3([pointA(1) pointB(1)],[pointA(2) pointB(2)],[pointA(3) pointB(3)],'g','LineWidth',1);
          end
      end
      data = [];
    end
    currentFrame = indata(i,2);
    markerCount = 0;
  end
  if (indata(i,1)~=0 || skipCAM0==0)   % skip CAM0
      markerCount = markerCount + 1;
      data(markerCount,:) = indata(i,3:end);
  end
end
results = results_old;
dlmwrite('rayto3d_result.csv',[results(:,1:3) usedFrameIndex' usedRayNumber'],' ');

figure;
idx = find(sumError < 5000);
plot3(results_new(idx,1),results_new(idx,2),results_new(idx,3),'bx','MarkerSize',5);
title('Exact newresults only');

figure;
plot3(results_old(:,1),results_old(:,2),results_old(:,3),'bo','MarkerSize',5);
title('Old results');


maxError
maxErrorAt
