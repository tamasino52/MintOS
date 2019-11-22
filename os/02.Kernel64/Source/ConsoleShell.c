/**
 *  file    ConsoleShell.c
 *  date    2009/01/31
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   콘솔 셸에 관련된 소스 파일
 */

#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "PIT.h"
#include "RTC.h"
#include "AssemblyUtility.h"
#include "Task.h"

// 커맨드 테이블 정의
SHELLCOMMANDENTRY gs_vstCommandTable[] =
{
        { "help", "Show Help", kHelp },
        { "cls", "Clear Screen", kCls },
        { "totalram", "Show Total RAM Size", kShowTotalRAMSize },
        { "strtod", "String To Decial/Hex Convert", kStringToDecimalHexTest },
        { "shutdown", "Shutdown And Reboot OS", kShutdown },
		{ "raisefault", "Raise Fault in 0x1ff000", kRaiseFault },
		{"settimer", "Set PIT Controller Counter0, ex)settimer 10(ms) 1(periodic)", kSetTimer},
		{"wait", "Wait ms Using PIT, ex)wait 100(ms)",kWaitUsingPIT},
		{"rdtsc","Read Time Stamp Counter", kReadTimeStampCounter},
		{"cpuspeed","Measure Processor Speed",kMeasureProcessorSpeed},
		{"date","Show Date And TIme",kShowDateAndTime},
		{"rand","Generate Random Number", kRand},
		{ "stdcdt","dummy",},
		{ "tdsccs","dummy",},
		{ "totade","dummy",},
		{ "hello","dummy",},
		{ "shine","dummy",},
		{ "ccno","dummy",},
		{ "shutup","dummy",},
		{ "cusin","dummy",},
		{ "student","dummy",},
		{"createtask","Create Task, ex)createtask 1(type) 10(count)", kCreateTestTask},
};

// TCB 자료구조와 스택 정의
static TCB gs_vstTask[2] = { 0, };
static QWORD gs_vstStack[1024] = { 0, };

// 태스크 전환 테스트 테스크
void kTestTask(void)
{
	int i = 0;
	while (1)
	{
		// 메시지를 출력하고 키 입력을 대기
		kPrintf("[%d] This message is from kTestTask. Press any key to switch kConsoleShell\n", i++);
		kGetCh();

		// 키 입력 시 태스크 전환
		kSwitchContext(&(gs_vstTask[1].stContext), &(gs_vstTask[0].stContext));
	}
}

// 태스크 1
void kTestTask1(void)
{
	BYTE bData;
	int i = 0, iX = 0, iY = 0, iMargin;
	CHARACTER* pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS;
	TCB* pstRunningTask;

	// 자신 ID를 얻어서 화면오프셋으로 사용
	pstRunningTask = kGetRunningTask();
	iMargin = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) % 10;

	// 화면 네 귀퉁이를 돌면서 문자 출력
	while (1)
	{
		switch (i)
		{
		case 0:
			iX++;
			if (iX = (CONSOLE_WIDTH - iMargin))
			{
				i = 1;
			}
			break;
		case 1:
			iY++;
			if (iY >= (CONSOLE_HEIGHT - iMargin))
			{
				i = 2;
			}
			break;
		case 2:
			iX--;
			if (iX < iMargin)
			{
				i = 3;
			}
			break;
		case 3:
			iY--;
			if (iY < iMargin)
			{
				i = 0;
			}
			break;
		}
		// 문자 및 색깔 지정
		pstScreen[iY * CONSOLE_WIDTH + iX].bCharactor = bData;
		pstScreen[iY * CONSOLE_WIDTH + iX].bAttribute = bData & 0x0F;
		bData++;

		// 다른 태스크로 전환
		kSchedule();
	}
}

// 태스크 2
void kTestTask2(void)
{
	int i = 0, iOffset;
	CHARACTER* pstScreen = (CHARACTER*)CONSOLE_VIDEOMEMORYADDRESS;
	TCB* pstRunningTask;
	char vcData[4] = { '-','＼','|','/' };

	// 자신의 ID를 얻어서 화면 오프셋으로 사용
	pstRunningTask = kGetRunningTask();
	iOffset = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) * 2;
	iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - (iOffset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));

	while (1)
	{
		//회전 바람개비를 표시
		pstScreen[iOffset].bCharactor = vcData[i % 4];
		pstScreen[iOffset].bAttribute = (iOffset % 15) + 1;
		i++;

		//다른 태스크로 전환
		kSchedule();
	}
}



void kRand(const char* pcParameterBuffer)
{
	BYTE bSecond, bMinute, bHour;
	// RTC 컨트롤러에서 시간 및 일자를 읽음
	kReadRTCTime(&bHour, &bMinute, &bSecond);

	static long holdrand = 1L;
	holdrand += bSecond;
	int randvalue = (((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
	kPrintf("Rand: %d\n",randvalue);
}


//PIT 컨트롤러의 카운터 0 설정
void kSetTimer(const char* pcParameterBuffer)
{
	char vcParameter[100];
	PARAMETERLIST stList;
	long lValue;
	BOOL bPeriodic;

	//파라미터 초기화
	kInitializeParameter(&stList, pcParameterBuffer);

	//milisecond 추출
	if (kGetNextParameter(&stList, vcParameter) == 0)
	{
		kPrintf("ex)settimer 10(ms) 1(periodic)\n");
		return;
	}
	lValue = kAToI(vcParameter, 10);

	//Periodic 추출
	if (kGetNextParameter(&stList, vcParameter) == 0)
	{
		kPrintf("ex)settimer 10(ms)1(periodic)\n" );
			return;
	}
	bPeriodic = kAToI(vcParameter, 10);
	kInitializePIT(MSTOCOUNT(lValue), bPeriodic);
	kPrintf("Time = %d ms , Periodic = %d Change Complete\n", lValue, bPeriodic);
}
// PIT 컨트롤러를 직접 사용하여 ms 동안 대기
void kWaitUsingPIT(const char* pcParameterBuffer)
{
	char vcParameter[100];
	int iLength;
	PARAMETERLIST stList;
	long lMillisecond;
	int i;
	
	// 파라미터 초기화
	kInitializeParameter(&stList, pcParameterBuffer);
	if (kGetNextParameter(&stList, vcParameter) == 0)
	{
		kPrintf("ex)wait 100(ms)\n");
		return;
	}
	
	lMillisecond = kAToI(pcParameterBuffer, 10);
	kPrintf("%d ms Sleep Start ...\n", lMillisecond);
	
	// 인터럽트를 비활성화하고 PIT 컨트롤러를 통해 직접 시간을 측정
	kDisableInterrupt();
	for (i = 0; i < lMillisecond / 30; i++)
	{
		kWaitUsingDirectPIT(MSTOCOUNT(30) );
	}
	kWaitUsingDirectPIT(MSTOCOUNT(lMillisecond % 30));
	kEnableInterrupt();
	kPrintf("%d ms Sleep Complete\n", lMillisecond);

	// 타이머 복원
	kInitializePIT(MSTOCOUNT(1), TRUE);
}

// 타임 스탬프 카운터를 임음
void kReadTimeStampCounter(const char* pcParameterBuffer)
{
	QWORD qwTSC;
	qwTSC = kReadTSC();
	kPrintf("Time Stamp Counter = %q\n" , qwTSC);
}
// 프로세서의 속도를 측정
void kMeasureProcessorSpeed(const char* pcParameterBuffer)
{
	int i;
	QWORD qwLastTSC, qwTotalTSC = 0;
	kPrintf("Now Measuring ." );
	// 10초 동안 변화한 타임 스탬프 카운터를 이용하여 프로세서의 속도를 간접적으로 측정
	kDisableInterrupt();
	for (i = 0; i < 200; i++) {
		qwLastTSC = kReadTSC();
		kWaitUsingDirectPIT(MSTOCOUNT(50));
		qwTotalTSC += kReadTSC() - qwLastTSC;
		kPrintf(".");
	}
	// 타이머 복원
	kInitializePIT(MSTOCOUNT(1), TRUE);
	kEnableInterrupt();
	kPrintf("\nCPU Speed = %d MHz\n", qwTotalTSC / 10 / 1000 / 1000);
}

// RTC 컨트롤러에 저장된 일자 및 시간 정보를 표시
void kShowDateAndTime(const char* pcParameterBuffer)
{
	BYTE bSecond, bMinute, bHour;
	BYTE bDayOfWeek, bDayOfMonth, bMonth;
	WORD wYear;

	// RTC 컨트롤러에서 시간 및 일자를 읽음
	kReadRTCTime(&bHour, &bMinute, &bSecond);
	kReadRTCDate(&wYear, &bMonth, &bDayOfMonth, &bDayOfWeek);
	kPrintf(" Date : %d/%d/%d %s, ", wYear, bMonth, bDayOfMonth, 
		kConvertDayOfWeekToString(bDayOfWeek));
	kPrintf("Time: %d:%d:%d\n", bHour, bMinute , bSecond );
}







void kClearScreenLine( int iX, int iY, int length )
{
	CHARACTER* pstScreen = ( CHARACTER* ) CONSOLE_VIDEOMEMORYADDRESS;
	int i;
	int iLinearValue;

	iLinearValue = iY * CONSOLE_WIDTH;

	for( i = iLinearValue; i < CONSOLE_WIDTH + iLinearValue; i++ )
	{
		pstScreen[i].bCharactor = ' ';
		pstScreen[i].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
	}

	kSetCursor( 0, iY );
}

//==============================================================================
//  실제 셸을 구성하는 코드
//==============================================================================
/**
 *  셸의 메인 루프
 */
void kStartConsoleShell( void )
{
    char vcCommandBuffer[ CONSOLESHELL_MAXCOMMANDBUFFERCOUNT ];
    int iCommandBufferIndex = 0;
    BYTE bKey;
    int iCursorX, iCursorY;

	PRECOMMANDS preCommand[10] = {0, };
	int index = 0;
	int maxIndex = 0;
	int comPointer = 0;

	int iSpaceIndex, iCount;
	int iCommandBufferLength;
	int indexBuffer[30];
	int validCount = 0;
    
    // 프롬프트 출력
    kPrintf( CONSOLESHELL_PROMPTMESSAGE );
    
    while( 1 )
    {
        // 키가 수신될 때까지 대기
        bKey = kGetCh();
        // Backspace 키 처리
        if( bKey == KEY_BACKSPACE )
        {
            if( iCommandBufferIndex > 0 )
            {
                // 현재 커서 위치를 얻어서 한 문자 앞으로 이동한 다음 공백을 출력하고 
                // 커맨드 버퍼에서 마지막 문자 삭제
                kGetCursor( &iCursorX, &iCursorY );
                kPrintStringXY( iCursorX - 1, iCursorY, " " );
                kSetCursor( iCursorX - 1, iCursorY );
                iCommandBufferIndex--;
            }
        }
        // 엔터 키 처리
        else if( bKey == KEY_ENTER )
        {
            kPrintf( "\n" );
            
            if( iCommandBufferIndex > 0 )
            {
                // 커맨드 버퍼에 있는 명령을 실행
                vcCommandBuffer[ iCommandBufferIndex ] = '\0';
                index = kExecuteCommand( vcCommandBuffer, index, &preCommand[ index ] );

				if( maxIndex == 9 )
				{
					maxIndex++;
				}

				if( index > maxIndex )
				{
					maxIndex = index;
				}
            }
            
            // 프롬프트 출력 및 커맨드 버퍼 초기화
            kPrintf( "%s", CONSOLESHELL_PROMPTMESSAGE );            
            kMemSet( vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT );
            iCommandBufferIndex = 0;
			comPointer = index - 1;
        }
		else if( bKey == KEY_UP )
		{ // UP 키가 눌려졌다면 preCommand에 저장된 명령어 히스토리를 순회하여 이전에 입력했던 명령어를 불러옴
			if( comPointer < 0 )
			{
				comPointer = maxIndex - 1;
			}

			kGetCursor( &iCursorX, &iCursorY );
			kClearScreenLine( iCursorX, iCursorY, preCommand[comPointer].iLength );
			kPrintf( "MINT64>%s", preCommand[comPointer].commandBuffer );
			
			for( iCommandBufferIndex = 0; iCommandBufferIndex < preCommand[comPointer].iLength; iCommandBufferIndex++ )
			{
				vcCommandBuffer[iCommandBufferIndex] = ((char*)preCommand[comPointer].commandBuffer)[iCommandBufferIndex];
			}

			comPointer--;
		}
		else if( bKey == KEY_DOWN )
		{
			comPointer++;

			if( comPointer >= maxIndex )
			{
				comPointer = 0;
			}

			kGetCursor( &iCursorX, &iCursorY );
			kClearScreenLine( iCursorX, iCursorY, preCommand[comPointer].iLength );
			kPrintf( "MINT64>%s", preCommand[comPointer].commandBuffer );
			
			for( iCommandBufferIndex = 0; iCommandBufferIndex < preCommand[comPointer].iLength; iCommandBufferIndex++ )
			{
				vcCommandBuffer[iCommandBufferIndex] = ((char*)preCommand[comPointer].commandBuffer)[iCommandBufferIndex];
			}
		}
        // 시프트 키, CAPS Lock, NUM Lock, Scroll Lock은 무시
        else if( ( bKey == KEY_LSHIFT ) || ( bKey == KEY_RSHIFT ) ||
                 ( bKey == KEY_CAPSLOCK ) || ( bKey == KEY_NUMLOCK ) ||
                 ( bKey == KEY_SCROLLLOCK ) )
        {
            ;
        }
        else
        {
            // TAB은 공백으로 전환
            if( bKey == KEY_TAB )
            {
				bKey = ' ';

				//현재 입력중이던 명령어의 길이를 반환받음
				iCommandBufferLength = kStrLen( vcCommandBuffer );

				// 현재 입력중이던 명령어가 저장된 버퍼의 공백인 인덱스를 iSpaceIndex 변수에 저장
				for( iSpaceIndex = 0; iSpaceIndex < iCommandBufferLength; iSpaceIndex++ )
				{
					if( vcCommandBuffer[iSpaceIndex] == ' ' )
					{
						break;
					}
				}

				// 커맨드 테이블에 있는 엔트리의 갯수를 저장
				iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );

				// 커맨드 테이블을 순회하며 현재 입력중이던 명령어 버퍼와 일치하는지 비교하며 순회하여 유효한 명령어의 인덱스를 indexBuffer에 저장
				for( int i = 0; i < iCount; i++ )
				{
					if( kMemCmp( gs_vstCommandTable[i].pcCommand, vcCommandBuffer, iSpaceIndex ) == 0 )
					{
						indexBuffer[validCount] = i;
						validCount++;
					}
				}

				// 유효한 명령어가 1개일 때 곧바로 vcCommandBuffer에 해당 명령어를 입력
				if( validCount == 1 )
				{
					kGetCursor( &iCursorX, &iCursorY );
					kClearScreenLine( iCursorX, iCursorY, iSpaceIndex );

					int tmp = indexBuffer[0];
					int siz = kStrLen( gs_vstCommandTable[tmp].pcCommand );

					// 현재 콘솔 명령어 입력 부분에 후보로 확정된 유효 명령어를 출력.
					kPrintf( "MINT64>%s", gs_vstCommandTable[tmp].pcCommand );
					int leng=0;
					while(*(gs_vstCommandTable[tmp].pcCommand+leng)!='\0')
					{
						vcCommandBuffer[leng]=*(gs_vstCommandTable[tmp].pcCommand+leng);
						leng++;
					}
					iCommandBufferIndex=leng;
				}
				else if( validCount > 1 )
				{ // 유효한 명령어가 2개 이상일 때 유효 명령어 후보군을 콘솔에 출력 후 다음 명령어를 기다림
					bKey = kGetCh();
					// 탭키가 한 번 더 눌러지기까지 대기
					if( bKey == KEY_TAB )
					{
						bKey=' ';
						kPrintf("\n");
						kGetCursor( &iCursorX, &iCursorY );
						kClearScreenLine( iCursorX, iCursorY, iSpaceIndex );

						// 입력한 명령어와 유사한 명령어들의 인덱스를 저장하였으므로 이를 토대로 후보군을 출력
						for( int i = 0; i < validCount; i++ )
						{
							int tmp = indexBuffer[i];
							kPrintf( "%s ", gs_vstCommandTable[tmp].pcCommand );
						}

						// 출력 후 재입력을 위해 콘솔 명령어 창을 비워준 후 입력이 들어오길 대기
						kPrintf("\nMINT64>");
						//int leng=0;
						kMemSet( vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT );
						iCommandBufferIndex = 0;
						comPointer = index - 1;
						bKey = kGetCh();
						
					/*	char tmpcommand[30];
						while(vcCommandBuffer[leng]!=' ')
						{
							kPrintf("%c",vcCommandBuffer[leng]);
							tmpcommand[leng]=vcCommandBuffer[leng];

							leng++;
						}*/	
						/*while(vcCommandBuffer[leng]!='\0')
						{
							vcCommandBuffer[leng]=tmpcommand[leng];
							leng++;
						}*/
						//aiCommandBufferIndex=leng;
						
					}
				
				}
				// 인덱스를 저장한 버퍼를 0으로 초기화
				for( int i = 0; i < validCount; i++ )
				{
					indexBuffer[i] = 0;
				}
				validCount = 0;
            }
            
            // 버퍼에 공간이 남아있을 때만 가능
            if( iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT )
            {
		   // kPrintf("%d",iCommandBufferIndex);
                vcCommandBuffer[ iCommandBufferIndex++ ] = bKey;
		
                kPrintf( "%c", bKey );
            }
        }
    }
}

/*
 *  커맨드 버퍼에 있는 커맨드를 비교하여 해당 커맨드를 처리하는 함수를 수행
 */
int kExecuteCommand( const char* pcCommandBuffer, int index, PRECOMMANDS preCommand[] )
{
    int i, iSpaceIndex;
    int iCommandBufferLength, iCommandLength;
    int iCount;
    
    // 공백으로 구분된 커맨드를 추출
    iCommandBufferLength = kStrLen( pcCommandBuffer );
    for( iSpaceIndex = 0 ; iSpaceIndex < iCommandBufferLength ; iSpaceIndex++ )
    {
        if( pcCommandBuffer[ iSpaceIndex ] == ' ' )
        {
            break;
        }
    }
    
    // 커맨드 테이블을 검사해서 동일한 이름의 커맨드가 있는지 확인
    iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );
    for( i = 0 ; i < iCount ; i++ )
    {
        iCommandLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
        // 커맨드의 길이와 내용이 완전히 일치하는지 검사
        if( ( iCommandLength == iSpaceIndex ) &&
            ( kMemCmp( gs_vstCommandTable[ i ].pcCommand, pcCommandBuffer,
                       iSpaceIndex ) == 0 ) )
        {
            gs_vstCommandTable[ i ].pfFunction( pcCommandBuffer + iSpaceIndex + 1 );

			kMemCpy( preCommand -> commandBuffer, pcCommandBuffer, 80 );
			preCommand -> iLength = iCommandBufferLength;
			preCommand -> iIndex = index;

			if( index < 9 )
			{
				index++;
				return index;
			}
			else
			{
				return 0;
			}
            break;
        }
    }

    // 리스트에서 찾을 수 없다면 에러 출력
    if( i >= iCount )
    {
        kPrintf( "'%s' is not found.\n", pcCommandBuffer );
		return index;
    }
}

/**
 *  파라미터 자료구조를 초기화
 */
void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter )
{
    pstList->pcBuffer = pcParameter;
    pstList->iLength = kStrLen( pcParameter );
    pstList->iCurrentPosition = 0;
}

/**
 *  공백으로 구분된 파라미터의 내용과 길이를 반환
 */
int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter )
{
    int i;
    int iLength;

    // 더 이상 파라미터가 없으면 나감
    if( pstList->iLength <= pstList->iCurrentPosition )
    {
        return 0;
    }
    
    // 버퍼의 길이만큼 이동하면서 공백을 검색
    for( i = pstList->iCurrentPosition ; i < pstList->iLength ; i++ )
    {
        if( pstList->pcBuffer[ i ] == ' ' )
        {
            break;
        }
    }
    
    // 파라미터를 복사하고 길이를 반환
    kMemCpy( pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i );
    iLength = i - pstList->iCurrentPosition;
    pcParameter[ iLength ] = '\0';

    // 파라미터의 위치 업데이트
    pstList->iCurrentPosition += iLength + 1;
    return iLength;
}
    
//==============================================================================
//  커맨드를 처리하는 코드
//==============================================================================
/**
 *  셸 도움말을 출력
 */
void kHelp( const char* pcCommandBuffer )
{
    int i;
    int iCount;
    int iCursorX, iCursorY;
    int iLength, iMaxCommandLength = 0;
    
    
    kPrintf( "=========================================================\n" );
    kPrintf( "                    MINT64 Shell Help                    \n" );
    kPrintf( "=========================================================\n" );
    
    iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );

    // 가장 긴 커맨드의 길이를 계산
    for( i = 0 ; i < iCount ; i++ )
    {
        iLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
        if( iLength > iMaxCommandLength )
        {
            iMaxCommandLength = iLength;
        }
    }
    
    // 도움말 출력
    for( i = 0 ; i < iCount ; i++ )
    {
        kPrintf( "%s", gs_vstCommandTable[ i ].pcCommand );
        kGetCursor( &iCursorX, &iCursorY );
        kSetCursor( iMaxCommandLength, iCursorY );
        kPrintf( "  - %s\n", gs_vstCommandTable[ i ].pcHelp );
    }
}

/**
 *  화면을 지움 
 */
void kCls( const char* pcParameterBuffer )
{
    // 맨 윗줄은 디버깅 용으로 사용하므로 화면을 지운 후, 라인 1로 커서 이동
    kClearScreen();
    kSetCursor( 0, 1 );
}

/**
 *  총 메모리 크기를 출력
 */
void kShowTotalRAMSize( const char* pcParameterBuffer )
{
    kPrintf( "Total RAM Size = %d MB\n", kGetTotalRAMSize() );
}

/**
 *  문자열로 된 숫자를 숫자로 변환하여 화면에 출력
 */
void kStringToDecimalHexTest( const char* pcParameterBuffer )
{
    char vcParameter[ 100 ];
    int iLength;
    PARAMETERLIST stList;
    int iCount = 0;
    long lValue;
    
    // 파라미터 초기화
    kInitializeParameter( &stList, pcParameterBuffer );
    
    while( 1 )
    {
        // 다음 파라미터를 구함, 파라미터의 길이가 0이면 파라미터가 없는 것이므로
        // 종료
        iLength = kGetNextParameter( &stList, vcParameter );
        if( iLength == 0 )
        {
            break;
        }

        // 파라미터에 대한 정보를 출력하고 16진수인지 10진수인지 판단하여 변환한 후
        // 결과를 printf로 출력
        kPrintf( "Param %d = '%s', Length = %d, ", iCount + 1, 
                 vcParameter, iLength );

        // 0x로 시작하면 16진수, 그외는 10진수로 판단
        if( kMemCmp( vcParameter, "0x", 2 ) == 0 )
        {
            lValue = kAToI( vcParameter + 2, 16 );
            kPrintf( "HEX Value = %q\n", lValue );
        }
        else
        {
            lValue = kAToI( vcParameter, 10 );
            kPrintf( "Decimal Value = %d\n", lValue );
        }
        
        iCount++;
    }
}

/**
 *  PC를 재시작(Reboot)
 */
void kShutdown( const char* pcParamegerBuffer )
{
    kPrintf( "System Shutdown Start...\n" );
    
    // 키보드 컨트롤러를 통해 PC를 재시작
    kPrintf( "Press Any Key To Reboot PC..." );
    kGetCh();
    kReboot();
}

void kRaiseFault( const char* pcParamegerBuffer )
{
	long *ptr = (long*)0x1ff000;
	*ptr = 0;
}
