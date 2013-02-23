function retval = getFrameId(indata) 
  retval = indata(2);
endfunction

function retval = getCamId(indata) 
  retval = indata(1);
endfunction

indata = dlmread('input.csv',';');
camcount=2;
%data=zeros(camcount,columns(indata)-1);

resultcount=0;
currentFrame = 0;
markerCount = 0;
%results=0;

for i=1:rows(indata)
  if (indata(i,2) > currentFrame)
    if (markerCount >= 2)
      a=data(:,1:4)';
      b=data(:,5:8)';
      d=b-a;
      if (markerCount == 2)
	D=[-d(:,1),d(:,2)];
	A=[a(:,2)-a(:,1)];
      endif
      if (markerCount == 3)
	D=[-d(:,1),d(:,2),zeros(4,1);-d(:,1),zeros(4,1),d(:,2)];
	A=[a(:,2)-a(:,1);a(:,3)-a(:,1)];
      endif
      
      % FIXME works only for two cameras
%      D=[b(1,:)'-a(1,:)',b(2,:)'-a(2,:)'];
%      C=(a(2,:)-a(1,:))';
      t=(D'*D)^-1*D'*A;
      resultcount=resultcount+1;
      results(resultcount,:)=(a(:,1)+d(:,1)*t(1))';
    endif
    currentFrame = indata(i,2);
    markerCount = 0;
  endif
  markerCount = markerCount + 1;
  data(markerCount,:) = indata(i,3:end);
endfor

dlmwrite('3d-points.csv',results(:,1:3),' ');
