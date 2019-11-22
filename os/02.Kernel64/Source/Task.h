#ifndef __TASK_H__
#define __TASK_H__

#include "Types.h"
#include "List.h"


// ��ũ��
// SS, RSP, RFLAGS,CS, RIP + ISR ���� �����ϴ� 19���� ��������
#define TASK_REGISTERCOUNT		(5 + 19)
#define TASK_REGSTERSIZE		8

// Context �ڷᱸ���� �������� ������
#define TASK_GSOFFSET		0
#define TASK_FSOFFSET		1
#define TASK_ESOFFSET		2
#define TASK_DSOFFSET		3
#define TASK_R15OFFSET		4
#define TASK_R14OFFSET		5
#define TASK_R13OFFSET		6
#define TASK_R12OFFSET		7
#define TASK_R11OFFSET		8
#define TASK_R10OFFSET		9
#define TASK_R9OFFSET		10
#define TASK_R8OFFSET		11
#define TASK_RSIOFFSET		12
#define TASK_RDIOFFSET		13
#define TASK_RDXOFFSET		14
#define TASK_RCXOFFSET		15
#define TASK_RBXOFFSET		16
#define TASK_RAXOFFSET		17
#define TASK_RBPOFFSET		18
#define TASK_RIPOFFSET		19
#define TASK_CSOFFSET		20
#define TASK_RFLAGSOFFSET	21
#define TASK_RSPOFFSET		22
#define TASK_SSOFFSET		23

#define TASK_TCBPOOLADDRESS	0x800000
#define TASK_MAXCOUNT		1024

#define TASK_STACKPOOLADDRESS	(TASK_TCBPOOLADDRESS + sizeof(TCB) * TASK_MAXCOUNT)
#define TASK_STACKSIZE		(64 * 1024)

#define TASK_INVALIDID		0xFFFFFFFFFFFFFFFF

#define TASK_PROCESSORTIME	5

#define TASK_MAXREADYLISTCOUNT	5

#define TASK_FLAGS_HIGHEST		0
#define TASK_FLAGS_HIGH			1
#define TASK_FLAGS_MEDIUM		2
#define TASK_FLAGS_LOW			3
#define TASK_FLAGS_LOWEST		4
#define TASK_FLAGS_WAIT			0xFF

#define TASK_FLAGS_ENDTASK		0x8000000000000000
#define TASK_FLAGS_SYSTEM		0x4000000000000000
#define TASK_FLAGS_PROCESS		0x2000000000000000
#define TASK_FLAGS_THREAD		0x1000000000000000
#define TASK_FLAGS_IDLE			0x0800000000000000
#define TASK_FLAGS_USERLEVEL	0x0400000000000000


#define GETPRIORITY(x)				((x) & 0xFF)
#define SETPRIORITY(x, priority)	((x) = ((x) & 0xFFFFFFFFFFFFFF00) | \
		(priority))
#define GETTCBOFFSET(x)				((x) & 0xFFFFFFFF)

#define GETTCBFROMTHREADLINK(x)		(TCB*)((QWORD)(x) - offsetof(TCB, stThreadLink))

#define TASK_LOADBALANCINGID		0xFF


// ����ü
// 1����Ʈ�� ����
#pragma pack(push, 1)

// ���ؽ�Ʈ ���� �ڷᱸ��
typedef struct kContextStruct
{
	QWORD vqRegister[TASK_REGISTERCOUNT];
} CONTEXT;

// �½�ũ ���� ���� �ڷᱸ��
typedef struct kTaskControlBlockStruct
{
	// ���� �������� ��ġ�� ID
	LISTLINK stLink;

	// ID �� �÷���
	QWORD qwID;
	QWORD qwFlags;

	// ���ؽ�Ʈ
	CONTEXT stContext;

	// ������ ��巹���� ũ��
	void* pvStackAddress;
	QWORD qwStackSize;
} TCB;

// TCB Ǯ�� ���¸� �����ϴ� �ڷᱸ��
typedef struct KTCBPoolManagerStruct
{
	// �½�ũ Ǯ�� ���� ����
	TCB* pstStartAddress;
	int iMaxCount;
	int iUseCount;

	// TCB�� �Ҵ�� Ƚ��
	int iAllocatedCount;
}TCBPOOLMANAGER;

typedef struct kSchedulerStruct
{
	//���� ���� ���� �½�ũ
	TCB* pstRunningTask;

	// ���� ���� ���� �½�ũ������� �� �ִ� ���μ��� �ð�
	int iProcessorTime;

	//������ �½�ũ�� �غ����� ����Ʈ
	LIST stReadyList;
}SCHEDULER;



#pragma pack(pop)

// ==========================================================
// �½�ũ Ǯ�� �½�ũ ����
// ==========================================================

void kSetUpTask(TCB* pstTCB, QWORD qwID, QWORD qwFlags, QWORD qwEntryPointAddress, void* pvStackAddress, QWORD qwStackSize);
void kInitializeTCBPool(void);
TCB* kAllocateTCB(void);
void kFreeTCB(QWORD qwID);


// ==========================================================
// �����췯 ����
// ==========================================================
void kInitializeScheduler(void);
void kSetRunningTask(TCB* pstTask);
TCB* kGetRunningTask(void);
TCB* kGetNextTaskToRun(void);
void kAddTaskToReadyList(TCB* pstTask);
void kSchedule(void);
BOOL kScheduleInInterrupt(void);
void kDecreaseProcessorTime(void);
BOOL kIsProcessorTimeExpired(void);
TCB* kCreateTask(QWORD qwFlags, QWORD qwEntryPointAddress);

#endif
