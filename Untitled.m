%clear all
%close all
f = fopen('log9.txt', 'r');

A = fscanf(f, '%c');
fclose(f);
data = [];

strBuf = '';
str = '';
input = 1;
endsOfStrings = strfind(A, ';');
endsOfData = strfind(A, ',');
startOfBlock = 1;
endOfBlock = endsOfData(1);
counter = 1;
row = 1;
startOfRow = 1;
endOfRow = endsOfStrings(1);

for i=startOfRow:length(endsOfStrings)
    for j = startOfRow:endOfRow
        strBuf = A(j);
        if strBuf ~= ',' && strBuf ~= ';'
        str = strcat(str,strBuf);
        else
        data(i, counter) = str2double(str);
        counter = counter + 1;
        str = '';
        end
    end
    counter = 1;
    strBuf = '';
    startOfRow = endOfRow+1;
    if i+1 >= length(endsOfStrings)
        break;
    else
        endOfRow = endsOfStrings(i+1);
    end
end
%%
figure 
plot(data(:,1), data(:,2));
xlabel("�����");
ylabel("��������� �");

figure 
plot(data(:,1), data(:,3));
xlabel("�����");
ylabel("��������� Y");

figure 
plot(data(:,1), data(:,4));
xlabel("�����");
ylabel("��������� Z");

figure 
plot(data(:,1), data(:,8));
xlabel("�����");
ylabel("��������� �� ���� ����");

figure 
plot(data(:,1), data(:,9));
xlabel("�����");
ylabel("�������� X");

figure 
plot(data(:,1), data(:,10));
xlabel("�����");
ylabel("�������� Y");

figure 
plot(data(:,1), data(:,1));
xlabel("�����");
ylabel("�������� Z");

figure 
plot(data(:,1), data(:,12));
xlabel("�����");
ylabel("��������");

figure 
plot(data(:,1), data(:,13));
xlabel("�����");
ylabel("������");

