#include "Types.h"
#include "AssemblyUtility.h"
#include "Keyboard.h"

///////////////////////////////////////////////////////////////////////////
// 키보드 컨트롤러와 키보드 제어에 관련된 함수
///////////////////////////////////////////////////////////////////////////
// 출력 버퍼 (포트 0x60)에 수신된 데이터가 있는 여부를 반환

BOOL kIsOutputBufferFull(void)
{
	// 상태 레지스터 ( 포트 0x64)에서 있은 값에 출력 버퍼 상태 비트 (비트 0)가 1로 설정되어 있으면 출력 버퍼에 키보드가 전송한 데이터가 존재함
	if (kInPortByte(0x64) & 0x01)
	{
		return TRUE;
	}
	return FALSE;
}

// 입력 버퍼(포트 0x60)에 프로세서가 쓴 데이터가 남아있는지 여부를 반환
BOOL kIsInputBufferFull(void)
{
	// 상태 레지스터 (포트 0x64)에서 읽은 값에 입력 버퍼 상태 비트(비트1) 가 1로 설정되어 있으면 아직 키보드가 데이터를 가져가지 않았음
	if (kInPortByte(0x64) & 0x02)
	{
		return TRUE;
	}
	return FALSE;
}

// 키보드를 활성화
BOOL kActivateKeyboard(void) {
	int i;
	int j;

	// 컨트롤 레지스터 포트 (0x64) 에 키보드 활성화 커맨드 (0xAE)를 전달하여 키보드 디바이스 활성화
	kOutPortByte(0x64, 0xAE);

	// 입력 버퍼 (포트 0x60)가 빌 때까지 기다렸다가 키보드에 활성화 커맨드를 전송
	// 0xFFFF만큼 루프를 수행할 시간이면 충분히 커맨드가 전송될 수 있음
	// 0xFFFF 루프를 수행한 이후에도 입력 버퍼 (포트 0x60)가 비지 않으면 무시하고 전송
	for (i=0; i<0xFFFF; i++)
	{
		//입력 버퍼(포트 0x64)가 비어있으면 키보드 커맨드 전송 가능
		if (kIsInputBufferFull() == FALSE)
		{
			break;
		}
	}
	// 입력 버퍼 (포트 0x60)로 키보드 활성화(0xF4) 커맨드를 전달하여 키보드로 전송
	kOutPortByte(0x60, 0xF4);

	// ACK 가 올 때 까지 대기함
	// ACK가 오기 전에 키보드 출력 버퍼(포트 0x60)에 키 데이터가 저장되어 있을 수 있으므로
	// 키보드에서 전달된 데이터를 최대 100개까지 수신하여 ACK를 확인

	for (j = 0; j < 100; j++)
	{
		// 0xFFFF만큼 루프를 수행할 시간이면 충분히 커맨드 응답이 올 수 있음
		// 0xFFFF 루프를 수행한 이후에도 출력버퍼가 차있지 않으면 무시하고 읽음
		for (i = 0; i < 0xFFFF; i++)
		{
			// 출력버퍼(포트 0x60)가 차있으면 데이터를 읽을 수 있음
			if (kIsOutputBufferFull() == TRUE)
			{
				break;
			}
		}
	}
	// 출력 버퍼에서 읽은 데이터가 ACK(0xFA)이면 성공
	if (kInPortByte(0x60) == 0xFA)
	{
		return TRUE;
	}
	return FALSE;
}


// 출력 버퍼에서 키를 읽음
BYTE kGetKeyboardScanCode(void)
{
	//출력 버퍼에 데이터가 있을 때까지 대기
	while (kIsOutputBufferFull() == FALSE)
	{
		;
	}
	return kInPortByte(0x60);
}

// 키보드 LED의 ON/OFF를 변경
BOOL kChangeKeyboardLED(BOOL bCapLockOn, BOOL bNumLockOn, BOOL bScrollLockOn)
{
	int i, j;

	// 키보드에 LED 변경 커맨드 전송하고 커맨드가 처리될 때까지 대기
	for (i = 0; i < 0xFFFF; i++)
	{
		//입력 버퍼가 비었으면 커맨드 전송가능
		if (kIsOutputBufferFull() == FALSE)
		{
			break;
		}
	}

	// 출력 버퍼(포트 0x60)로 LED 상태 변경 커맨드(0xED) 전송
	kOutPortByte(0x60, 0xED);
	for (i = 0; i < 0xFFFF; i++) {
		//입력 버퍼가 비어있으면 키보드가 커맨드를 가져간 것임
		if (kIsInputBufferFull() == FALSE)
		{
			break;
		}
	}

	// 키보드가 LED 상태 변경 커맨드를 가져갔으므로 ACK가 올 때까지 대기
	for (j = 0; j < 100; j++)
	{
		for (i = 0; i < 0xFFFF; i++)
		{
			// 출력 버퍼가 차있으면 데이터 읽을 수 없음
			if (kIsOutputBufferFull() == TRUE)
			{
				break;
			}
		}

		// 출력 버퍼에서 읽은 데이터가 ACK(0xFA)이면 성공
		if (kInPortByte(0x60) == 0xFA)
		{
			break;
		}
	}
	if (j >= 100)
	{
		return FALSE;
	}

	// LED변경값을 키보드로 전송하고 데이터가 처리가 완료될 때까지 대기
	kOutPortByte(0x60, (bCapLockOn << 2) | (bNumLockOn << 1) | bScrollLockOn);
	for (i = 0; i < 0xFFFF; i++)
	{
		if (kIsInputBufferFull() == FALSE)
		{
			break;
		}
	}

	// 키보드가 LED 데이터를 가져갔으므로 ACK 가 올때까지 대기함
	for (j = 0; j < 100; j++)
	{
		for (i = 0; i < 0xFFFF; i++)
		{
			// 출력버퍼(포트 0x60)가 차있으면 데이터를 읽을 수 있음
			if (kIsOutputBufferFull() == TRUE)
			{
				break;
			}
		}
		if (kInPortByte(0x60) == 0xFA)
		{
			break;
		}
	}

	if (j >= 100)
	{
		return FALSE;
	}
	return TRUE;
}

void kEnableA20Gate(void)
{
	BYTE bOutputPortData;
	int i;

	// send a command (0xD0) to the control register to read the
	// output port value of the keyboard controller
	kOutPortByte(0x64, 0xD0);

	for (i = 0; i < 0xFFFF; i++)
	{
		if (kIsOutputBufferFull() == TRUE)
		{
			break;
		}
	}
	bOutputPortData = kInPortByte(0x60);

	// set A20 Gate bit
	bOutputPortData |= 0x01;

	for (i = 0; i < 0xFFFF; i++)
	{
		if (kIsInputBufferFull() == FALSE)
		{
			break;
		}
	}
	// send output port setting command(0xD1) to the command register
	kOutPortByte(0x64, 0xD1);

	kOutPortByte(0x60, bOutputPortData);
}

void kReboot(void)
{
	int i;
	for (i = 0; i < 0xFFFF; i++)
	{
		if (kIsInputBufferFull() == FALSE)
		{
			break;
		}
	}
	kOutPortByte(0x64, 0xD1);
	kOutPortByte(0x60, 0x00);
	while (1)
	{
		;
	}
}

//////////////////////////////////////////////////////////////////////////
// 스캔 코드를 아스키 코드로 변환하는 기능에 관련된 함수들
//////////////////////////////////////////////////////////////////////////

// 키보드 상태를 관리하는 키보드 매니저
static KEYBOARDMANAGER gs_stKeyboardManager = { 0, };

// 스캔 코드를 아스키 코드로 변환하는 테이블
static KEYMAPPINGENTRY gs_vstKeyMappingTable[KEY_MAPPINGTABLEMAXCOUNT] =
{
	{ KEY_NONE, KEY_NONE },		// 0
	{ KEY_ESC, KEY_ESC },		// 1
	{ '1', '!' },				// 2
	{ '2', '@' },				// 3
	{ '3', '#' },				// 4
	{ '4', '$' },				// 5
	{ '5', '%' },				// 6
	{ '6', '^' },				// 7
	{ '7', '&' },				// 8
	{ '8', '*' },				// 9
	{ '9', '(' },				// 10
	{ '0', ')' },				// 11
	{ '-', '_' },				// 12
	{ '=', '+' },				// 13
	{ KEY_BACKSPACE, KEY_BACKSPACE },	// 14
	{ KEY_TAB, KEY_TAB },		// 15
	{ 'q', 'Q' },				// 16
	{ 'w', 'W' },				// 17
	{ 'e', 'E' },				// 18
	{ 'r', 'R' },				// 19
	{ 't', 'T' },				// 20
	{ 'y', 'Y' },				// 21
	{ 'u', 'U' },				// 22
	{ 'i', 'I' },				// 23
	{ 'o', 'O' },				// 24
	{ 'p', 'P' },				// 25
	{ '[', '{' },				// 26
	{ ']', '}' },				// 27
	{ '\n', '\n' },				// 28
	{ KEY_CTRL, KEY_CTRL },		// 29
	{ 'a', 'A' },				// 30
	{ 's', 'S' },				// 31
	{ 'd', 'D' },				// 32
	{ 'f', 'F' },				// 33
	{ 'g', 'G' },				// 34
	{ 'h', 'H' },				// 35
	{ 'j', 'J' },				// 36
	{ 'k', 'K' },				// 37
	{ 'l', 'L' },				// 38
	{ ';', ':' },				// 39
	{ '\'', '\"' },				// 40
	{ '`', '~' },				// 41
	{ KEY_LSHIFT, KEY_LSHIFT },	// 42
	{ '\\', '|' },				// 43
	{ 'z', 'Z' },				// 44
	{ 'x', 'X' },				// 45
	{ 'c', 'C' },				// 46
	{ 'v', 'V' },				// 47
	{ 'b', 'B' },				// 48
	{ 'n', 'N' },				// 49
	{ 'm', 'M' },				// 50
	{ ',', '<' },				// 51
	{ '.', '>' },				// 52
	{ '/', '?' },				// 53
	{ KEY_RSHIFT, KEY_RSHIFT },	// 54
	{ '*', '*' },				// 55
	{ KEY_LALT, KEY_LALT },		// 56
	{ ' ', ' ' },				// 57
	{ KEY_CAPSLOCK, KEY_CAPSLOCK },		// 58
	{ KEY_F1, KEY_F1 },			// 59
	{ KEY_F2, KEY_F2 },			// 60
	{ KEY_F3, KEY_F3 },			// 61
	{ KEY_F4, KEY_F4 },			// 62
	{ KEY_F5, KEY_F5 },			// 63
	{ KEY_F6, KEY_F6 },			// 64
	{ KEY_F7, KEY_F7 },			// 65
	{ KEY_F8, KEY_F8 },			// 66
	{ KEY_F9, KEY_F9 },			// 67
	{ KEY_F10, KEY_F10 },		// 68
	{ KEY_NUMLOCK, KEY_NUMLOCK },		// 69
	{ KEY_SCROLLLOCK, KEY_SCROLLLOCK },	// 70
	{ KEY_HOME, '7' },			// 71
	{ KEY_UP, '8' },			// 72
	{ KEY_PAGEUP, '9' },		// 73
	{ '-', '-' },				// 74
	{ KEY_LEFT, '4' },			// 75
	{ KEY_CENTER, '5' },		// 76
	{ KEY_RIGHT, '6' },			// 77
	{ '+', '+' },				// 78
	{ KEY_END, '1' },			// 79
	{ KEY_DOWN, '2' },			// 80
	{ KEY_PAGEDOWN, '3' },		// 81
	{ KEY_INS, '0' },			// 82
	{ KEY_DEL, '.' },			// 83
	{ KEY_NONE, KEY_NONE },		// 84
	{ KEY_NONE, KEY_NONE },		// 85
	{ KEY_NONE, KEY_NONE },		// 86
	{ KEY_F11, KEY_F11 },		// 87
	{ KEY_F12, KEY_F12 },		// 88
};

BOOL kIsAlphabetScanCode(BYTE bScanCode)
{
	if ('a' <= gs_vstKeyMappingTable[bScanCode].bNormalCode &&
		gs_vstKeyMappingTable[bScanCode].bNormalCode <= 'z')
		return TRUE;
	return FALSE;
}


BOOL kIsNumberOrSymbolScanCode(BYTE bScanCode)
{
	if (2 <= bScanCode && bScanCode <= 53 && kIsAlphabetScanCode(bScanCode) == FALSE)
		return TRUE;
	return FALSE;
}

BOOL kIsNumberPadScanCode(BYTE bScanCode)
{
	if (71 <= bScanCode && bScanCode <= 83)
		return TRUE;
	return FALSE;
}

BOOL kIsUseCombinedCode(BYTE bScanCode)
{
	BYTE bDownScanCode;
	BOOL bUseCombinedKey = FALSE;

	bDownScanCode = bScanCode & 0x7F;

	if (kIsAlphabetScanCode(bDownScanCode) == TRUE)
	{	// alphabet keys are affected by shift and caps lock
		if (gs_stKeyboardManager.bShiftDown ^ gs_stKeyboardManager.bCapsLockOn)
			bUseCombinedKey = TRUE;
		else
			bUseCombinedKey = FALSE;
	}
	else if (kIsNumberOrSymbolScanCode(bDownScanCode) == TRUE)
	{	// number and symbol keys are affected by shift
		if (gs_stKeyboardManager.bShiftDown == TRUE)
			bUseCombinedKey = TRUE;
		else
			bUseCombinedKey = FALSE;
	}
	else if (kIsNumberPadScanCode(bDownScanCode) == TRUE &&
		gs_stKeyboardManager.bExtendedCodeIn == FALSE)
	{	// number pad keys are affected by num lock
		// use combination codes only when no extended key code is received
		if (gs_stKeyboardManager.bNumLockOn == TRUE)
			bUseCombinedKey = TRUE;
		else
			bUseCombinedKey = FALSE;
	}

	return bUseCombinedKey;
}


void kUpdateCombinationKeyStatusAndLED(BYTE bScanCode)
{
	BOOL bDown;
	BYTE bDownScanCode;
	BOOL bLEDStatusChanged = FALSE;

	if (bScanCode & 0x80)
	{
		bDown = FALSE;
		bDownScanCode = bScanCode & 0x7F;
	}
	else
	{
		bDown = TRUE;
		bDownScanCode = bScanCode;
	}

	// Shift scan code
	if (bDownScanCode == 42 || bDownScanCode == 54)
	{
		gs_stKeyboardManager.bShiftDown = bDown;
	}
	// Caps Lock
	else if (bDownScanCode == 58 && bDown == TRUE)
	{
		gs_stKeyboardManager.bCapsLockOn ^= TRUE;
		bLEDStatusChanged = TRUE;
	}
	// Num Lock
	else if (bDownScanCode == 69 && bDown == TRUE)
	{
		gs_stKeyboardManager.bNumLockOn ^= TRUE;
		bLEDStatusChanged = TRUE;
	}
	// Scroll Lock
	else if (bDownScanCode == 70 && bDown == TRUE)
	{
		gs_stKeyboardManager.bScrollLockOn ^= TRUE;
		bLEDStatusChanged = TRUE;
	}

	if (bLEDStatusChanged == TRUE)
		kChangeKeyboardLED(gs_stKeyboardManager.bCapsLockOn,
			gs_stKeyboardManager.bNumLockOn, gs_stKeyboardManager.bScrollLockOn);
}

BOOL kConvertScanCodeToASCIICode(BYTE bScanCode, BYTE* pbASCIICode, BOOL* pbFlags)
{
	BOOL bUseCombinedKey;

	// If the Pause key was previously received, ignore the remaining Pause scan code
	if (gs_stKeyboardManager.iSkipCountForPause > 0)
	{
		gs_stKeyboardManager.iSkipCountForPause--;
		return FALSE;
	}

	// handle Pause key
	if (bScanCode == 0xE1)
	{
		*pbASCIICode = KEY_PAUSE;
		*pbFlags = KEY_FLAGS_DOWN;
		gs_stKeyboardManager.iSkipCountForPause = KEY_SKIPCOUNTFORPAUSE;
		return TRUE;
	}
	// just set flag when extended key code is received
	else if (bScanCode == 0xE0)
	{
		gs_stKeyboardManager.bExtendedCodeIn = TRUE;
		return FALSE;
	}

	bUseCombinedKey = kIsUseCombinedCode(bScanCode);

	if (bUseCombinedKey == TRUE)
	{
		*pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bCombinedCode;
	}
	else
	{
		*pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bNormalCode;
	}

	if (gs_stKeyboardManager.bExtendedCodeIn == TRUE)
	{
		*pbFlags = KEY_FLAGS_EXTENDEDKEY;
		gs_stKeyboardManager.bExtendedCodeIn = FALSE;
	}
	else
	{
		*pbFlags = 0;
	}

	if ((bScanCode & 0x80) == 0)
	{
		*pbFlags |= KEY_FLAGS_DOWN;
	}

	kUpdateCombinationKeyStatusAndLED(bScanCode);
	return TRUE;
}

