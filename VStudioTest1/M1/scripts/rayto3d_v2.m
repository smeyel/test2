clear
% Load localization data
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
      A=data(:,1:3)';   % We omit the last (0) coordinate...
      B=data(:,5:7)';
      V=B-A;
      resultcount=resultcount+1;
      usedFrameIndex(resultcount)=currentFrame;
      usedRayNumber(resultcount)=markerCount;
        
      p = getIntersection3D(A,V);

      results(resultcount,:)=p';
      % Reset data buffer.
      % Otherwise if the next frame has only cam0 and cam2,
      %     there may remain some garbage for cam1...
      data = [];
    end
    currentFrame = indata(i,2);
    markerCount = 0;
      end
  markerCount = markerCount + 1;
  data(markerCount,:) = indata(i,3:end);
end

dlmwrite('rayto3d_result.csv',[results(:,1:3) usedFrameIndex' usedRayNumber'],' ');
