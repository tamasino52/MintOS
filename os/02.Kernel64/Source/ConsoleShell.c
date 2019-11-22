/**
 *  file    ConsoleShell.c
 *  date    2009/01/31
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �ܼ� �п� ���õ� �ҽ� ����
 */

#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "PIT.h"
#include "RTC.h"
#include "AssemblyUtility.h"
#include "Task.h"

// Ŀ�ǵ� ���̺� ����
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

// TCB �ڷᱸ���� ���� ����
static TCB gs_vstTask[2] = { 0, };
static QWORD gs_vstStack[1024] = { 0, };

// �½�ũ ��ȯ �׽�Ʈ �׽�ũ
void kTestTask(void)
{
	int i = 0;
	while (1)
	{
		// �޽����� ����ϰ� Ű �Է��� ���
		kPrintf("[%d] This message is from kTestTask. Press any key to switch kConsoleShell\n", i++);
		kGetCh();

		// Ű �Է� �� �½�ũ ��ȯ
		kSwitchContext(&(gs_vstTask[1].stContext), &(gs_vstTask[0].stContext));
	}
}

// �½�ũ 1
void kTestTask1(void)
{
	BYTE bData;
	int i = 0, iX = 0, iY = 0, iMargin;
	CHARACTER* pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS;
	TCB* pstRunningTask;

	// �ڽ� ID�� �� ȭ����������� ���
	pstRunningTask = kGetRunningTask();
	iMargin = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) % 10;

	// ȭ�� �� �����̸� ���鼭 ���� ���
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
		// ���� �� ���� ����
		pstScreen[iY * CONSOLE_WIDTH + iX].bCharactor = bData;
		pstScreen[iY * CONSOLE_WIDTH + iX].bAttribute = bData & 0x0F;
		bData++;

		// �ٸ� �½�ũ�� ��ȯ
		kSchedule();
	}
}

// �½�ũ 2
void kTestTask2(void)
{
	int i = 0, iOffset;
	CHARACTER* pstScreen = (CHARACTER*)CONSOLE_VIDEOMEMORYADDRESS;
	TCB* pstRunningTask;
	char vcData[4] = { '-','��','|','/' };

	// �ڽ��� ID�� �� ȭ�� ���������� ���
	pstRunningTask = kGetRunningTask();
	iOffset = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) * 2;
	iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - (iOffset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));

	while (1)
	{
		//ȸ�� �ٶ����� ǥ��
		pstScreen[iOffset].bCharactor = vcData[i % 4];
		pstScreen[iOffset].bAttribute = (iOffset % 15) + 1;
		i++;

		//�ٸ� �½�ũ�� ��ȯ
		kSchedule();
	}
}



void kRand(const char* pcParameterBuffer)
{
	BYTE bSecond, bMinute, bHour;
	// RTC ��Ʈ�ѷ����� �ð� �� ���ڸ� ����
	kReadRTCTime(&bHour, &bMinute, &bSecond);

	static long holdrand = 1L;
	holdrand += bSecond;
	int randvalue = (((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
	kPrintf("Rand: %d\n",randvalue);
}


//PIT ��Ʈ�ѷ��� ī���� 0 ����
void kSetTimer(const char* pcParameterBuffer)
{
	char vcParameter[100];
	PARAMETERLIST stList;
	long lValue;
	BOOL bPeriodic;

	//�Ķ���� �ʱ�ȭ
	kInitializeParameter(&stList, pcParameterBuffer);

	//milisecond ����
	if (kGetNextParameter(&stList, vcParameter) == 0)
	{
		kPrintf("ex)settimer 10(ms) 1(periodic)\n");
		return;
	}
	lValue = kAToI(vcParameter, 10);

	//Periodic ����
	if (kGetNextParameter(&stList, vcParameter) == 0)
	{
		kPrintf("ex)settimer 10(ms)1(periodic)\n" );
			return;
	}
	bPeriodic = kAToI(vcParameter, 10);
	kInitializePIT(MSTOCOUNT(lValue), bPeriodic);
	kPrintf("Time = %d ms , Periodic = %d Change Complete\n", lValue, bPeriodic);
}
// PIT ��Ʈ�ѷ��� ���� ����Ͽ� ms ���� ���
void kWaitUsingPIT(const char* pcParameterBuffer)
{
	char vcParameter[100];
	int iLength;
	PARAMETERLIST stList;
	long lMillisecond;
	int i;
	
	// �Ķ���� �ʱ�ȭ
	kInitializeParameter(&stList, pcParameterBuffer);
	if (kGetNextParameter(&stList, vcParameter) == 0)
	{
		kPrintf("ex)wait 100(ms)\n");
		return;
	}
	
	lMillisecond = kAToI(pcParameterBuffer, 10);
	kPrintf("%d ms Sleep Start ...\n", lMillisecond);
	
	// ���ͷ�Ʈ�� ��Ȱ��ȭ�ϰ� PIT ��Ʈ�ѷ��� ���� ���� �ð��� ����
	kDisableInterrupt();
	for (i = 0; i < lMillisecond / 30; i++)
	{
		kWaitUsingDirectPIT(MSTOCOUNT(30) );
	}
	kWaitUsingDirectPIT(MSTOCOUNT(lMillisecond % 30));
	kEnableInterrupt();
	kPrintf("%d ms Sleep Complete\n", lMillisecond);

	// Ÿ�̸� ����
	kInitializePIT(MSTOCOUNT(1), TRUE);
}

// Ÿ�� ������ ī���͸� ����
void kReadTimeStampCounter(const char* pcParameterBuffer)
{
	QWORD qwTSC;
	qwTSC = kReadTSC();
	kPrintf("Time Stamp Counter = %q\n" , qwTSC);
}
// ���μ����� �ӵ��� ����
void kMeasureProcessorSpeed(const char* pcParameterBuffer)
{
	int i;
	QWORD qwLastTSC, qwTotalTSC = 0;
	kPrintf("Now Measuring ." );
	// 10�� ���� ��ȭ�� Ÿ�� ������ ī���͸� �̿��Ͽ� ���μ����� �ӵ��� ���������� ����
	kDisableInterrupt();
	for (i = 0; i < 200; i++) {
		qwLastTSC = kReadTSC();
		kWaitUsingDirectPIT(MSTOCOUNT(50));
		qwTotalTSC += kReadTSC() - qwLastTSC;
		kPrintf(".");
	}
	// Ÿ�̸� ����
	kInitializePIT(MSTOCOUNT(1), TRUE);
	kEnableInterrupt();
	kPrintf("\nCPU Speed = %d MHz\n", qwTotalTSC / 10 / 1000 / 1000);
}

// RTC ��Ʈ�ѷ��� ����� ���� �� �ð� ������ ǥ��
void kShowDateAndTime(const char* pcParameterBuffer)
{
	BYTE bSecond, bMinute, bHour;
	BYTE bDayOfWeek, bDayOfMonth, bMonth;
	WORD wYear;

	// RTC ��Ʈ�ѷ����� �ð� �� ���ڸ� ����
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
//  ���� ���� �����ϴ� �ڵ�
//==============================================================================
/**
 *  ���� ���� ����
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
    
    // ������Ʈ ���
    kPrintf( CONSOLESHELL_PROMPTMESSAGE );
    
    while( 1 )
    {
        // Ű�� ���ŵ� ������ ���
        bKey = kGetCh();
        // Backspace Ű ó��
        if( bKey == KEY_BACKSPACE )
        {
            if( iCommandBufferIndex > 0 )
            {
                // ���� Ŀ�� ��ġ�� �� �� ���� ������ �̵��� ���� ������ ����ϰ� 
                // Ŀ�ǵ� ���ۿ��� ������ ���� ����
                kGetCursor( &iCursorX, &iCursorY );
                kPrintStringXY( iCursorX - 1, iCursorY, " " );
                kSetCursor( iCursorX - 1, iCursorY );
                iCommandBufferIndex--;
            }
        }
        // ���� Ű ó��
        else if( bKey == KEY_ENTER )
        {
            kPrintf( "\n" );
            
            if( iCommandBufferIndex > 0 )
            {
                // Ŀ�ǵ� ���ۿ� �ִ� ����� ����
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
            
            // ������Ʈ ��� �� Ŀ�ǵ� ���� �ʱ�ȭ
            kPrintf( "%s", CONSOLESHELL_PROMPTMESSAGE );            
            kMemSet( vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT );
            iCommandBufferIndex = 0;
			comPointer = index - 1;
        }
		else if( bKey == KEY_UP )
		{ // UP Ű�� �������ٸ� preCommand�� ����� ��ɾ� �����丮�� ��ȸ�Ͽ� ������ �Է��ߴ� ��ɾ �ҷ���
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
        // ����Ʈ Ű, CAPS Lock, NUM Lock, Scroll Lock�� ����
        else if( ( bKey == KEY_LSHIFT ) || ( bKey == KEY_RSHIFT ) ||
                 ( bKey == KEY_CAPSLOCK ) || ( bKey == KEY_NUMLOCK ) ||
                 ( bKey == KEY_SCROLLLOCK ) )
        {
            ;
        }
        else
        {
            // TAB�� �������� ��ȯ
            if( bKey == KEY_TAB )
            {
				bKey = ' ';

				//���� �Է����̴� ��ɾ��� ���̸� ��ȯ����
				iCommandBufferLength = kStrLen( vcCommandBuffer );

				// ���� �Է����̴� ��ɾ ����� ������ ������ �ε����� iSpaceIndex ������ ����
				for( iSpaceIndex = 0; iSpaceIndex < iCommandBufferLength; iSpaceIndex++ )
				{
					if( vcCommandBuffer[iSpaceIndex] == ' ' )
					{
						break;
					}
				}

				// Ŀ�ǵ� ���̺� �ִ� ��Ʈ���� ������ ����
				iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );

				// Ŀ�ǵ� ���̺��� ��ȸ�ϸ� ���� �Է����̴� ��ɾ� ���ۿ� ��ġ�ϴ��� ���ϸ� ��ȸ�Ͽ� ��ȿ�� ��ɾ��� �ε����� indexBuffer�� ����
				for( int i = 0; i < iCount; i++ )
				{
					if( kMemCmp( gs_vstCommandTable[i].pcCommand, vcCommandBuffer, iSpaceIndex ) == 0 )
					{
						indexBuffer[validCount] = i;
						validCount++;
					}
				}

				// ��ȿ�� ��ɾ 1���� �� ��ٷ� vcCommandBuffer�� �ش� ��ɾ �Է�
				if( validCount == 1 )
				{
					kGetCursor( &iCursorX, &iCursorY );
					kClearScreenLine( iCursorX, iCursorY, iSpaceIndex );

					int tmp = indexBuffer[0];
					int siz = kStrLen( gs_vstCommandTable[tmp].pcCommand );

					// ���� �ܼ� ��ɾ� �Է� �κп� �ĺ��� Ȯ���� ��ȿ ��ɾ ���.
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
				{ // ��ȿ�� ��ɾ 2�� �̻��� �� ��ȿ ��ɾ� �ĺ����� �ֿܼ� ��� �� ���� ��ɾ ��ٸ�
					bKey = kGetCh();
					// ��Ű�� �� �� �� ����������� ���
					if( bKey == KEY_TAB )
					{
						bKey=' ';
						kPrintf("\n");
						kGetCursor( &iCursorX, &iCursorY );
						kClearScreenLine( iCursorX, iCursorY, iSpaceIndex );

						// �Է��� ��ɾ�� ������ ��ɾ���� �ε����� �����Ͽ����Ƿ� �̸� ���� �ĺ����� ���
						for( int i = 0; i < validCount; i++ )
						{
							int tmp = indexBuffer[i];
							kPrintf( "%s ", gs_vstCommandTable[tmp].pcCommand );
						}

						// ��� �� ���Է��� ���� �ܼ� ��ɾ� â�� ����� �� �Է��� ������ ���
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
				// �ε����� ������ ���۸� 0���� �ʱ�ȭ
				for( int i = 0; i < validCount; i++ )
				{
					indexBuffer[i] = 0;
				}
				validCount = 0;
            }
            
            // ���ۿ� ������ �������� ���� ����
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
 *  Ŀ�ǵ� ���ۿ� �ִ� Ŀ�ǵ带 ���Ͽ� �ش� Ŀ�ǵ带 ó���ϴ� �Լ��� ����
 */
int kExecuteCommand( const char* pcCommandBuffer, int index, PRECOMMANDS preCommand[] )
{
    int i, iSpaceIndex;
    int iCommandBufferLength, iCommandLength;
    int iCount;
    
    // �������� ���е� Ŀ�ǵ带 ����
    iCommandBufferLength = kStrLen( pcCommandBuffer );
    for( iSpaceIndex = 0 ; iSpaceIndex < iCommandBufferLength ; iSpaceIndex++ )
    {
        if( pcCommandBuffer[ iSpaceIndex ] == ' ' )
        {
            break;
        }
    }
    
    // Ŀ�ǵ� ���̺��� �˻��ؼ� ������ �̸��� Ŀ�ǵ尡 �ִ��� Ȯ��
    iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );
    for( i = 0 ; i < iCount ; i++ )
    {
        iCommandLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
        // Ŀ�ǵ��� ���̿� ������ ������ ��ġ�ϴ��� �˻�
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

    // ����Ʈ���� ã�� �� ���ٸ� ���� ���
    if( i >= iCount )
    {
        kPrintf( "'%s' is not found.\n", pcCommandBuffer );
		return index;
    }
}

/**
 *  �Ķ���� �ڷᱸ���� �ʱ�ȭ
 */
void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter )
{
    pstList->pcBuffer = pcParameter;
    pstList->iLength = kStrLen( pcParameter );
    pstList->iCurrentPosition = 0;
}

/**
 *  �������� ���е� �Ķ������ ����� ���̸� ��ȯ
 */
int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter )
{
    int i;
    int iLength;

    // �� �̻� �Ķ���Ͱ� ������ ����
    if( pstList->iLength <= pstList->iCurrentPosition )
    {
        return 0;
    }
    
    // ������ ���̸�ŭ �̵��ϸ鼭 ������ �˻�
    for( i = pstList->iCurrentPosition ; i < pstList->iLength ; i++ )
    {
        if( pstList->pcBuffer[ i ] == ' ' )
        {
            break;
        }
    }
    
    // �Ķ���͸� �����ϰ� ���̸� ��ȯ
    kMemCpy( pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i );
    iLength = i - pstList->iCurrentPosition;
    pcParameter[ iLength ] = '\0';

    // �Ķ������ ��ġ ������Ʈ
    pstList->iCurrentPosition += iLength + 1;
    return iLength;
}
    
//==============================================================================
//  Ŀ�ǵ带 ó���ϴ� �ڵ�
//==============================================================================
/**
 *  �� ������ ���
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

    // ���� �� Ŀ�ǵ��� ���̸� ���
    for( i = 0 ; i < iCount ; i++ )
    {
        iLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
        if( iLength > iMaxCommandLength )
        {
            iMaxCommandLength = iLength;
        }
    }
    
    // ���� ���
    for( i = 0 ; i < iCount ; i++ )
    {
        kPrintf( "%s", gs_vstCommandTable[ i ].pcCommand );
        kGetCursor( &iCursorX, &iCursorY );
        kSetCursor( iMaxCommandLength, iCursorY );
        kPrintf( "  - %s\n", gs_vstCommandTable[ i ].pcHelp );
    }
}

/**
 *  ȭ���� ���� 
 */
void kCls( const char* pcParameterBuffer )
{
    // �� ������ ����� ������ ����ϹǷ� ȭ���� ���� ��, ���� 1�� Ŀ�� �̵�
    kClearScreen();
    kSetCursor( 0, 1 );
}

/**
 *  �� �޸� ũ�⸦ ���
 */
void kShowTotalRAMSize( const char* pcParameterBuffer )
{
    kPrintf( "Total RAM Size = %d MB\n", kGetTotalRAMSize() );
}

/**
 *  ���ڿ��� �� ���ڸ� ���ڷ� ��ȯ�Ͽ� ȭ�鿡 ���
 */
void kStringToDecimalHexTest( const char* pcParameterBuffer )
{
    char vcParameter[ 100 ];
    int iLength;
    PARAMETERLIST stList;
    int iCount = 0;
    long lValue;
    
    // �Ķ���� �ʱ�ȭ
    kInitializeParameter( &stList, pcParameterBuffer );
    
    while( 1 )
    {
        // ���� �Ķ���͸� ����, �Ķ������ ���̰� 0�̸� �Ķ���Ͱ� ���� ���̹Ƿ�
        // ����
        iLength = kGetNextParameter( &stList, vcParameter );
        if( iLength == 0 )
        {
            break;
        }

        // �Ķ���Ϳ� ���� ������ ����ϰ� 16�������� 10�������� �Ǵ��Ͽ� ��ȯ�� ��
        // ����� printf�� ���
        kPrintf( "Param %d = '%s', Length = %d, ", iCount + 1, 
                 vcParameter, iLength );

        // 0x�� �����ϸ� 16����, �׿ܴ� 10������ �Ǵ�
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
 *  PC�� �����(Reboot)
 */
void kShutdown( const char* pcParamegerBuffer )
{
    kPrintf( "System Shutdown Start...\n" );
    
    // Ű���� ��Ʈ�ѷ��� ���� PC�� �����
    kPrintf( "Press Any Key To Reboot PC..." );
    kGetCh();
    kReboot();
}

void kRaiseFault( const char* pcParamegerBuffer )
{
	long *ptr = (long*)0x1ff000;
	*ptr = 0;
}
