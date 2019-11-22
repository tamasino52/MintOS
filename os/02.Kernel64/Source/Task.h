#ifndef __TASK_H__
#define __TASK_H__

#include "Types.h"
#include "List.h"


// 매크로
// SS, RSP, RFLAGS,CS, RIP + ISR 에서 저장하는 19개의 레지스터
#define TASK_REGISTERCOUNT		(5 + 19)
#define TASK_REGSTERSIZE		8

// Context 자료구조의 레지스터 오프셋
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


// 구조체
// 1바이트로 정렬
#pragma pack(push, 1)

// 컨텍스트 관련 자료구조
typedef struct kContextStruct
{
	QWORD vqRegister[TASK_REGISTERCOUNT];
} CONTEXT;

// 태스크 상태 관리 자료구조
typedef struct kTaskControlBlockStruct
{
	// 다음 데이터의 위치와 ID
	LISTLINK stLink;

	// ID 및 플래그
	QWORD qwID;
	QWORD qwFlags;

	// 컨텍스트
	CONTEXT stContext;

	// 스택의 어드레스와 크기
	void* pvStackAddress;
	QWORD qwStackSize;
} TCB;

// TCB 풀의 상태를 관리하는 자료구조
typedef struct KTCBPoolManagerStruct
{
	// 태스크 풀에 대한 정보
	TCB* pstStartAddress;
	int iMaxCount;
	int iUseCount;

	// TCB가 할당된 횟수
	int iAllocatedCount;
}TCBPOOLMANAGER;

typedef struct kSchedulerStruct
{
	//현재 수행 중인 태스크
	TCB* pstRunningTask;

	// 현재 수헹 중인 태스크가사용할 수 있는 프로세서 시간
	int iProcessorTime;

	//실행할 태스크가 준비중인 리스트
	LIST stReadyList;
}SCHEDULER;



#pragma pack(pop)

// ==========================================================
// 태스크 풀과 태스크 관련
// ==========================================================

void kSetUpTask(TCB* pstTCB, QWORD qwID, QWORD qwFlags, QWORD qwEntryPointAddress, void* pvStackAddress, QWORD qwStackSize);
void kInitializeTCBPool(void);
TCB* kAllocateTCB(void);
void kFreeTCB(QWORD qwID);


// ==========================================================
// 스케쥴러 관련
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
