REM ==================================================================================
REM 
REM �� ������ C:/MINT64 ���͸��� �ִ� Disk.img �����̳� DiskWithPackage.img ������
REM �ε��ϵ��� �Ǿ� �ֽ��ϴ�. 
REM
REM ������ ���͸��� ���ϸ��� �ٲٰ� �ʹٸ� ���� �Ʒ��� �ִ� qemu-system-x86_64.exe 
REM ������ �����ϸ� �˴ϴ�.
REM 
REM ==================================================================================

REM Start qemu on windows.
@ECHO OFF

REM SDL_VIDEODRIVER=directx is faster than windib. But keyboard cannot work well.
SET SDL_VIDEODRIVER=windib

REM SDL_AUDIODRIVER=waveout or dsound can be used. Only if QEMU_AUDIO_DRV=sdl.
SET SDL_AUDIODRIVER=dsound

REM QEMU_AUDIO_DRV=dsound or fmod or sdl or none can be used. See qemu -audio-help.
SET QEMU_AUDIO_DRV=dsound

REM QEMU_AUDIO_LOG_TO_MONITOR=1 displays log messages in QEMU monitor.
SET QEMU_AUDIO_LOG_TO_MONITOR=0

REM PCI-based PC(default): -M pc 
REM ISA-based PC         : -M isapc
REM -M isapc is added for NE2000 ISA card.

REM qemu-system-x86_64.exe -L . -m 64 -fda c:/MINT64/Disk.img -boot a -localtime -M pc -serial tcp::4444,server,nowait 

REM qemu-system-x86_64.exe -L . -m 64 -fda c:/MINT64/Disk.img -hda c:/MINT64/HDD.img -boot a -localtime -M pc -serial tcp::4444,server,nowait -smp 2

REM qemu-system-x86_64.exe -L . -m 64 -fda c:/MINT64/Disk.img -hda c:/MINT64/HDD.img -boot a -localtime -M pc -serial tcp::4444,server,nowait -smp 2

REM qemu-system-x86_64.exe -L . -m 64 -fda c:/MINT64/Disk.img -boot a -localtime -M pc -serial tcp::4444,server,nowait -smp 2

REM qemu-system-x86_64.exe -L . -m 64 -fda c:/MINT64/DiskWithPackage.img -hda c:/MINT64/HDD.img -boot a -localtime -M pc -serial tcp::4444,server,nowait -smp 2

REM ==================================================================================
REM 
REM C:/MINT64 ���͸��� �ִ� DiskWithPackage.img ������ �ε��ϴ� �ɼ�
REM �ϵ� ��ũ �̹����� -hda �ɼ����� �ε� ����(���� ���� ����)
REM
REM ==================================================================================
qemu-system-x86_64.exe -L . -m 64 -fda c:/MINT64/DiskWithPackage.img -boot a -localtime -M pc -serial tcp::4444,server,nowait -smp 2