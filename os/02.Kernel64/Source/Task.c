#include "Task.h"
#include "Utility.h"
#include "AssemblyUtility.h"
#include "Descriptor.h"
#include "Console.h"

// 스케쥴러 관련 자료구조
static SCHEDULER gs_stScheduler;
static TCBPOOLMANAGER gs_stTCBPoolManager;


//===================================
//  태스트 풀과 태스크 관련
//===================================

void kInitializeTCBPool(void)
{
	int i;

	kMemSet(&(gs_stTCBPoolManager), 0, sizeof(gs_stTCBPoolManager));

	gs_stTCBPoolManager.pstStartAddress = (TCB*)TASK_TCBPOOLADDRESS;
	kMemSet((void*)TASK_TCBPOOLADDRESS, 0, sizeof(TCB) * TASK_MAXCOUNT);

	for (i = 0; i < TASK_MAXCOUNT; i++)
	{
		gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;
	}

	gs_stTCBPoolManager.iMaxCount = TASK_MAXCOUNT;
	gs_stTCBPoolManager.iAllocatedCount = 1;
}

 TCB* kAllocateTCB(void)
{
	TCB* pstEmptyTCB;
	int i;



	if (gs_stTCBPoolManager.iUseCount == gs_stTCBPoolManager.iMaxCount)
	{
		return NULL;
	}

	for (i = 0; i < gs_stTCBPoolManager.iMaxCount; i++)
	{
		if ((gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID >> 32) == 0)
		{
			pstEmptyTCB = &(gs_stTCBPoolManager.pstStartAddress[i]);
			break;
		}
	}

	// set TCB
	pstEmptyTCB->stLink.qwID = ((QWORD)gs_stTCBPoolManager.iAllocatedCount << 32) | i;
	gs_stTCBPoolManager.iUseCount++;
	gs_stTCBPoolManager.iAllocatedCount++;
	if (gs_stTCBPoolManager.iAllocatedCount == 0)
	{
		gs_stTCBPoolManager.iAllocatedCount = 1;
	}

	return pstEmptyTCB;
}


 void kFreeTCB(QWORD qwID)
{
	int i;

	i = GETTCBOFFSET(qwID);

	kMemSet(&(gs_stTCBPoolManager.pstStartAddress[i].stContext), 0, sizeof(CONTEXT));
	gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;
	
	gs_stTCBPoolManager.iUseCount--;
}

TCB* kCreateTask(QWORD qwFlags, QWORD qwEntryPointAddress)
{
	TCB* pstTask;
	void* pvStackAddress;

	pstTask = kAllocateTCB();
	if (pstTask == NULL)
	{
		return NULL;
	}

	// allocate stack
	pvStackAddress = (void*)(TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * (pstTask->stLink.qwID & 0xFFFFFFFF)));

	kSetUpTask(pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE);
	kAddTaskToReadyList(pstTask);

	return pstTask;
}


void kSetUpTask(TCB* pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress,
		void* pvStackAddress, QWORD qwStackSize)
{
	kMemSet(pstTCB->stContext.vqRegister, 0, sizeof(pstTCB->stContext.vqRegister));

	// set RSP, RBP
	pstTCB->stContext.vqRegister[TASK_RSPOFFSET] = (QWORD)pvStackAddress +
		qwStackSize;
	pstTCB->stContext.vqRegister[TASK_RBPOFFSET] = (QWORD)pvStackAddress +
		qwStackSize;

	pstTCB->stContext.vqRegister[TASK_CSOFFSET] = GDT_KERNELCODESEGMENT;
	pstTCB->stContext.vqRegister[TASK_DSOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_ESOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_FSOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_GSOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_SSOFFSET] = GDT_KERNELDATASEGMENT;

	// set RIP register
	pstTCB->stContext.vqRegister[TASK_RIPOFFSET] = qwEntryPointAddress;

	// activate interrupt by set IF bit and allow user access IO port
	// by set IOPL bit
	pstTCB->stContext.vqRegister[TASK_RFLAGSOFFSET] |= 0x0200;

	// save ID, stack, flags
	pstTCB->pvStackAddress = pvStackAddress;
	pstTCB->qwStackSize = qwStackSize;
	pstTCB->qwFlags = qwFlags;
}



//===================================
// 스케쥴러 관련
//===================================
// 스케쥴러 초기화


void kInitializeScheduler(void)
{
	// 태스크 폴 초기화
	kInitializeTCBPool();

	//준비리스트 초기화
	kInitializeList(&(gs_stScheduler.stReadyList));
	
	// TCB 할당받아 실행중인 태스크로 설정하여 부팅을 수행한 태스크를 저장할 TCB 준비
	gs_stScheduler.pstRunningTask = kAllocateTCB();
}

void kSetRunningTask(TCB* pstTask)
{
	gs_stScheduler.pstRunningTask = pstTask;
}

TCB* kGetRunningTask( void )
{
	return gs_stScheduler.pstRunningTask;
}


TCB* kGetNextTaskToRun(void)
{
	if (kGetListCount(&(gs_stScheduler.stReadyList)) == 0)
	{
		return NULL;
	}
	return (TCB*)kRemoveListFromHeader(&(gs_stScheduler.stReadyList));
}

void kAddTaskToReadyList(TCB* pstTask)
{
	kAddListToTail(&(gs_stScheduler.stReadyList), pstTask);
}


void kSchedule(void)
{
	TCB* pstRunningTask, * pstNextTask;
	BOOL bPreviousFlag;

	// 전환할 태스크가 있어야 함
	if (kGetListCount(&(gs_stScheduler.stReadyList)) == 0)
	{
		return;
	}
	// 전환하는 도중 인터럽트가 발생하여 태스크 전환이 또 일어나면 곤란하므로
	// 전환하는 동안 인터럽트가 발생하지 못하도록 설정
	bPreviousFlag = kSetInterruptFlag(FALSE);
	// 실행할 다음 태스크를 얻음
	pstNextTask = kGetNextTaskToRun();
	if (pstNextTask == NULL)
	{
		kSetInterruptFlag(bPreviousFlag);
		return;
	}
		
	pstRunningTask = gs_stScheduler.pstRunningTask;
	//kAddTaskToRunList(pstRunningTask);
	// 프로세서 사용 시죠떨 업데이트
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	
	// 다음 태스크를 현재 수행 중인 태스크로 설정한 후 콘텍스트 전환
	gs_stScheduler.pstRunningTask = pstNextTask;
	kSwitchContext(&(pstRunningTask->stContext), &(pstNextTask->stContext));
	kSetInterruptFlag(bPreviousFlag);

}

// 인터럽트가 발생했을 때 / 다른 태스크를 참아 전환
// 반드시 인터럽트나 예외가 발생했을 때 호출해야 함
BOOL kScheduleInInterrupt(void)
{
	TCB* pstRunningTask, * pstNextTask;
	char* pcContextAddress;

	pstNextTask = kGetNextTaskToRun();
	if (pstNextTask == NULL)
	{
		return FALSE;
	}

	// ==========================================================
	// 태스크 전환 처리
	// 인터럽트 핸들러에서 저장한 콘텍스트를 다른 콘텍스트로 덮어쓰는 방법으로 처리
	// ==========================================================
	pcContextAddress = (char*)IST_STARTADDRESS - sizeof(CONTEXT);

	// 현재 태스크를 얻어서 1ST 어 | 있는 콘텍스트를 복사하고 현재 태스크를 준비 리스트로 옮김
	pstRunningTask = gs_stScheduler.pstRunningTask;
	kMemCpy(&(pstRunningTask->stContext), pcContextAddress, sizeof(CONTEXT));
	kAddTaskToReadyList(pstRunningTask);

	// 전환해서 실행할 태스크를 Running Task로 설정하고 콘텍스트를 1ST에 복사해서
	// 자동으로 태스크 전환이 일어나도록 항
	gs_stScheduler.pstRunningTask = pstNextTask;
	kMemCpy(pcContextAddress , &(pstNextTask->stContext), sizeof(CONTEXT));

	// 프로세서 사용 시간을 업데리트
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	return TRUE;
}

// 프로세서를 사용할 수 있는 시간을 하나 줄임
void kDecreaseProcessorTime(void)
{
	if (gs_stScheduler.iProcessorTime > 0)
	{
		gs_stScheduler.iProcessorTime--;
	}
}

// 프로세서를 사용할 수 있는 시간이 다 되었는지 여부를 반환
BOOL kIsProcessorTimeExpired(void)
{
	if (gs_stScheduler.iProcessorTime <= 0)
	{
		return TRUE;
	}
	return FALSE;
}