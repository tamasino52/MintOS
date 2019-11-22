#include "RTC.h"
#include "AssemblyUtility.h"

// CMOS 메모리에서 RTX 컨트롤러가 저장한 현재 시간을 읽음
void kReadRTCTime(BYTE* pbHour, BYTE* pbMinute, BYTE* pbSecond)
{
	BYTE bData;

	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_HOUR);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbHour = RTC_BCDTOBINARY(bData);

	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_MINUTE);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbMinute = RTC_BCDTOBINARY(bData);

	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_SECOND);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbSecond = RTC_BCDTOBINARY(bData);
}

// CMOS 메모리에서 RTX 컨트롤러가 저장한 현재 일자를 읽음
void kReadRTCDate(WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth,
		BYTE* pbDayOfWeek)
{
	BYTE bData;

	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_YEAR);
	bData = kInPortByte(RTC_CMOSDATA);
	*pwYear = RTC_BCDTOBINARY(bData) + 2000;

	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_MONTH);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbMonth = RTC_BCDTOBINARY(bData);

	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFMONTH);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbDayOfMonth = RTC_BCDTOBINARY(bData);

	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFWEEK);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbDayOfWeek = RTC_BCDTOBINARY(bData);
}

// 요일 값을 이용해서 해당 요일의 문자열 반환
char* kConvertDayOfWeekToString(BYTE bDayOfWeek)
{
	static char* vpcDayOfWeekString[8] = { "Error", "Sunday", "Monday",
		"Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

	if (bDayOfWeek >= 8)
		return vpcDayOfWeekString[0];

	return vpcDayOfWeekString[bDayOfWeek];
}


