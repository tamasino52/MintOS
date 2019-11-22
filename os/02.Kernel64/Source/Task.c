#include "Task.h"
#include "Utility.h"
#include "AssemblyUtility.h"
#include "Descriptor.h"
#include "Console.h"

// �����췯 ���� �ڷᱸ��
static SCHEDULER gs_stScheduler;
static TCBPOOLMANAGER gs_stTCBPoolManager;


//===================================
//  �½�Ʈ Ǯ�� �½�ũ ����
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
// �����췯 ����
//===================================
// �����췯 �ʱ�ȭ


void kInitializeScheduler(void)
{
	// �½�ũ �� �ʱ�ȭ
	kInitializeTCBPool();

	//�غ񸮽�Ʈ �ʱ�ȭ
	kInitializeList(&(gs_stScheduler.stReadyList));
	
	// TCB �Ҵ�޾� �������� �½�ũ�� �����Ͽ� ������ ������ �½�ũ�� ������ TCB �غ�
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

	// ��ȯ�� �½�ũ�� �־�� ��
	if (kGetListCount(&(gs_stScheduler.stReadyList)) == 0)
	{
		return;
	}
	// ��ȯ�ϴ� ���� ���ͷ�Ʈ�� �߻��Ͽ� �½�ũ ��ȯ�� �� �Ͼ�� ����ϹǷ�
	// ��ȯ�ϴ� ���� ���ͷ�Ʈ�� �߻����� ���ϵ��� ����
	bPreviousFlag = kSetInterruptFlag(FALSE);
	// ������ ���� �½�ũ�� ����
	pstNextTask = kGetNextTaskToRun();
	if (pstNextTask == NULL)
	{
		kSetInterruptFlag(bPreviousFlag);
		return;
	}
		
	pstRunningTask = gs_stScheduler.pstRunningTask;
	//kAddTaskToRunList(pstRunningTask);
	// ���μ��� ��� ���Ҷ� ������Ʈ
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	
	// ���� �½�ũ�� ���� ���� ���� �½�ũ�� ������ �� ���ؽ�Ʈ ��ȯ
	gs_stScheduler.pstRunningTask = pstNextTask;
	kSwitchContext(&(pstRunningTask->stContext), &(pstNextTask->stContext));
	kSetInterruptFlag(bPreviousFlag);

}

// ���ͷ�Ʈ�� �߻����� �� / �ٸ� �½�ũ�� ���� ��ȯ
// �ݵ�� ���ͷ�Ʈ�� ���ܰ� �߻����� �� ȣ���ؾ� ��
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
	// �½�ũ ��ȯ ó��
	// ���ͷ�Ʈ �ڵ鷯���� ������ ���ؽ�Ʈ�� �ٸ� ���ؽ�Ʈ�� ����� ������� ó��
	// ==========================================================
	pcContextAddress = (char*)IST_STARTADDRESS - sizeof(CONTEXT);

	// ���� �½�ũ�� �� 1ST �� | �ִ� ���ؽ�Ʈ�� �����ϰ� ���� �½�ũ�� �غ� ����Ʈ�� �ű�
	pstRunningTask = gs_stScheduler.pstRunningTask;
	kMemCpy(&(pstRunningTask->stContext), pcContextAddress, sizeof(CONTEXT));
	kAddTaskToReadyList(pstRunningTask);

	// ��ȯ�ؼ� ������ �½�ũ�� Running Task�� �����ϰ� ���ؽ�Ʈ�� 1ST�� �����ؼ�
	// �ڵ����� �½�ũ ��ȯ�� �Ͼ���� ��
	gs_stScheduler.pstRunningTask = pstNextTask;
	kMemCpy(pcContextAddress , &(pstNextTask->stContext), sizeof(CONTEXT));

	// ���μ��� ��� �ð��� ������Ʈ
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	return TRUE;
}

// ���μ����� ����� �� �ִ� �ð��� �ϳ� ����
void kDecreaseProcessorTime(void)
{
	if (gs_stScheduler.iProcessorTime > 0)
	{
		gs_stScheduler.iProcessorTime--;
	}
}

// ���μ����� ����� �� �ִ� �ð��� �� �Ǿ����� ���θ� ��ȯ
BOOL kIsProcessorTimeExpired(void)
{
	if (gs_stScheduler.iProcessorTime <= 0)
	{
		return TRUE;
	}
	return FALSE;
}