clc;
clear;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%��Ƭ�����ڽ��յ������ݴ���
image = textread('1.txt','%c');
image_data_length = size(image);

frame_size = 56*94/8;

%����һ���յ�ͼƬ
Use_Image(56,94,3) = uint8(0);

%dec2hex(image(1))    dec2hex(abs(a(1)))
ImageByteCount = 1;


%ͼƬͷ�ж�   ֡ͷ 0x55 0x55
if hex2dec('55') == image(ImageByteCount) && hex2dec('55') == image(ImageByteCount+1)
    ImageByteCount = ImageByteCount + 2; %����֡ͷ
    
    for high = 0:6
        for j = 1:94        %��������
            for i = 1:8         %ת��һ���ֽ�
                PixleTemp = bitget(abs(image(ImageByteCount)),i) * 255;
                Use_Image(i+high*8,j,:) = [PixleTemp, PixleTemp, PixleTemp];
            end
            ImageByteCount = ImageByteCount + 1;
        end
    end

end

imshow(Use_Image)