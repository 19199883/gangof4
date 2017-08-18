#include<stdio.h> 
#include<time.h>
#include<stdlib.h>
#include<string.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define TEST
//#define DEBUG

#define BASE_YEAR 2014
#define BASE_DAYS 735964  //2014/12/31
#define BASE_YUAN_YEAR 1900
#define BASE_MONTH 1

#define FPGA_WR_BASE_ADDR 0x80000100
#define FPGA_RD_BASE_ADDR 0x80000000


enum {
	PRICE_LOW_OFFSET = 0x3,
	PRICE_HIGH_OFFSET,
	VALUME_OFFSET
};

long c_one_price[8] = {0x0B09020C0D050107, 0x0B090C070D050102, 0x0B0C02070D050109,0x0C0902070D05010B,0x0B0902070D05010C,0x0B0902070D050C01,0x0b0902070d0c0105,0x0B0902070C05010D};
long c_one_valume[8] = {0x07030109, 0x07030901, 0x07090103, 0x09030107, 0x07030109, 0x07030901, 0x07090103, 0x09030107};

int is_leap_year(long year)
{
	if( year%400 == 0 || (year%4 == 0 && year%100 != 0))
		return 1;
	else
		return 0;
}

struct tm *get_cur_system_time()
{
	time_t rawtime;
	struct tm * timeinfo;
#ifdef TEST
	int y = 0;
	int m = 0;
	int d = 0;
#endif
	time( &rawtime );
	timeinfo = localtime( &rawtime );
#ifdef TEST
	printf("please input time by the format: year/month/day:\n");
	scanf("%d/%d/%d", &y, &m, &d);
	timeinfo->tm_year = y - BASE_YUAN_YEAR;
	timeinfo->tm_mon = m - BASE_MONTH;
	timeinfo->tm_mday = d;
#endif
	return timeinfo;

}

int cal_cur_year_days(struct tm * timeinfo)
{
	long y, m, d;
	long days = 0;
	long flag = 0;
	if (timeinfo == NULL) {
		printf("can't get system time\n");
		return -1;
	}
	y = timeinfo->tm_year + BASE_YUAN_YEAR;
	m = timeinfo->tm_mon + BASE_MONTH;
	d = timeinfo->tm_mday;
    if(is_leap_year(y)) 
		flag=1;  
	switch(m) 
	{ 
		case 1:
			days=0;
			break; 
		case 2:
			days=31;
			break; 
		case 3:
			days=31+28;
			break; 
		case 4:
			days=31+28+31;
			break; 
		case 5:
			days=31+28+31+30;
			break; 
		case 6:
			days=31+28+31+30+31;
			break; 
		case 7:
			days=31+28+31+30+31+30;
			break; 
		case 8:
			days=31+28+31+30+31+30+31;
			break; 
		case 9:
			days=31+28+31+30+31+30+31+31;
			break; 
		case 10:
			days=31+28+31+30+31+30+31+31+30;
			break; 
		case 11:
			days=31+28+31+30+31+30+31+31+30+31;
			break; 
		case 12:
			days=31+28+31+30+31+30+31+31+30+31+30;
			break; 
	}; 
	days=days+d; 
	if ( m > 2) 
		days = days + flag; 
	
	return days;
}

int get_total_days()
{
	long days = 0;
	int i = 0;
	long years = 0;
	long one_year = 0;
	struct tm * timeinfo;
	days = BASE_DAYS;
	timeinfo = get_cur_system_time();
#ifdef DEBUG	
	printf("timeinfo->tm_year =%ld, timeinfo->mon =%ld, timeinfo->tm_mday=%ld\n", \
			timeinfo->tm_year + BASE_YUAN_YEAR, timeinfo->tm_mon + 1, timeinfo->tm_mday);
#endif
	years = timeinfo->tm_year + BASE_YUAN_YEAR - BASE_YEAR - 1;
	for (i = 1; i <= years; i++) {
		one_year = BASE_YEAR + i;
		if (is_leap_year(one_year))
			days += 366;
		else
			days +=365;
	}
	days += cal_cur_year_days(timeinfo);
	return days;
	
}

int get_price_valume(long * pprice, long *pvalume)
{
	long days = 0;
	int i = 0;
	
	if (pprice == NULL || pvalume == NULL)
		return -1;
	days = get_total_days();
	i = days % 8;
	*pprice = c_one_price[i];
	*pvalume = c_one_valume[i];
#ifdef DEBUG	
	printf("price = 0x%lx, valume=0x%lx, i = %d, days=%ld\n", *pprice, *pvalume, i, days);
#endif
	return 0;
}

int write_fpga_reg(int toe_id, int addr_offset, int value)
{
	char buf[1024];
	int count = 0;
	int addr = 0;
	addr = FPGA_WR_BASE_ADDR + addr_offset;
	memset(buf, 0, sizeof(buf));
	if (toe_id == 0) {
		count += sprintf(buf + count, "myreg w 0 0 0xc29 0x%x;", value);
		count += sprintf(buf + count, "myreg w 0 0 0xc28 0x0;");
		count += sprintf(buf + count, "myreg w 0 0 0xc28 0x%x;", addr_offset);
		count += sprintf(buf + count, "myreg w 0 0 0xc28 0x%x", addr);
	}
	else {
		count += sprintf(buf + count, "myreg w 0 0 0x1029 0x%x;", value);
		count += sprintf(buf + count, "myreg w 0 0 0x1028 0x0;");
		count += sprintf(buf + count, "myreg w 0 0 0x1028 0x%x;", addr_offset);
		count += sprintf(buf + count, "myreg w 0 0 0x1028 0x%x", addr);
	}	
	system(buf);	
	return 0;
}

int read_fpga_reg(int toe_id, int addr_offset)
{
	char buf[1024];
	int count = 0;
	int addr = 0;
	addr = FPGA_RD_BASE_ADDR + addr_offset;
	memset(buf, 0, sizeof(buf));
	if (toe_id == 0) {
		count += sprintf(buf + count, "myreg w 0 0 0xc28 0x%x;", addr_offset);
		count += sprintf(buf + count, "myreg w 0 0 0xc28 0x%x;", addr);
		count += sprintf(buf + count, "myreg r 0 0 0xc2a;");
		count += sprintf(buf + count, "myreg w 0 0 0xc28 0x0");
	}
	else {
		count += sprintf(buf + count, "myreg w 0 0 0x1028 0x%x;", addr_offset);
		count += sprintf(buf + count, "myreg w 0 0 0x1028 0x%x;", addr);
		count += sprintf(buf + count, "myreg r 0 0 0x102a;");
		count += sprintf(buf + count, "myreg w 0 0 0x1028 0x0");
	}	
	system(buf);	
	return 0;
}

void set_time()
{
	long days = 0;
	long price = 0;
	long valume = 0;
	int value = 0;
	
	get_price_valume(&price, &valume);
	value = price & 0xFFFFFFFF;
	write_fpga_reg(0, PRICE_LOW_OFFSET, value);
	read_fpga_reg(0, PRICE_LOW_OFFSET);
	value = price >> 32;
	write_fpga_reg(0, PRICE_HIGH_OFFSET, value);
	read_fpga_reg(0, PRICE_HIGH_OFFSET);
	value = valume & 0xFFFFFFFF;
	write_fpga_reg(0, VALUME_OFFSET, value);
	read_fpga_reg(0, VALUME_OFFSET);  

}

//void main()
//{
//	long days = 0;
//	long price = 0;
//	long valume = 0;
//	int value = 0;
//
//	get_price_valume(&price, &valume);
//	value = price & 0xFFFFFFFF;
//	write_fpga_reg(0, PRICE_LOW_OFFSET, value);
//	read_fpga_reg(0, PRICE_LOW_OFFSET);
//	value = price >> 32;
//	write_fpga_reg(0, PRICE_HIGH_OFFSET, value);
//	read_fpga_reg(0, PRICE_HIGH_OFFSET);
//	value = valume & 0xFFFFFFFF;
//	write_fpga_reg(0, VALUME_OFFSET, value);
//	read_fpga_reg(0, VALUME_OFFSET);
//}

#ifdef __cplusplus
}
#endif
