indata = dlmread('input.csv',';');
camcount=2;
data=zeros(camcount,columns(indata)-1);

resultcount=0;

for i=1:rows(indata)
  data(indata(i,1)+1,:) = indata(i,2:end);
  ok=1;
  for j=1:camcount
    if (data(1,1) != data(j,1))
      ok = 0;
    endif
  endfor
  if (ok == 1)
    a=data(:,2:5);
    b=data(:,6:9);
    % FIXME works only for two cameras
    D=[b(1,:)'-a(1,:)',b(2,:)'-a(2,:)'];
    C=(a(2,:)-a(1,:))';
    t=(D'*D)^-1*D'*C;
    resultcount=resultcount+1;
    results(resultcount,:)=(a(1,:)+(b(1,:)-a(1,:))*t(1));
  endif
endfor

dlmwrite('3d-points.csv',results(:,1:3),' ');