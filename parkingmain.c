#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ************** macro defines *************** */
#define PARKTYPE_GPARK		(0)
#define PARKTYPE_ECHARGE	(1)
#define PARKTYPE_ECAR		(2)

#define USER_TYPE_STUDENT	(0)
#define USER_TYPE_EMPLOYEE	(1)

#define MAX_CHARGE_MONEY	(1000.0F)
#define MIN_CHARGE_MONEY	(10.0F)
/* General park 10X15X15 */
#define GPARK_FLOOR_COUNT		(10)
#define GPARK_XPLACE_COUNT		(15)
#define GPARK_YPLACE_COUNT		(15) 

/* Electrocar park with charge 1X8X8 */
#define ECHARGE_FLOOR_COUNT		(1)
#define ECHARGE_XPLACE_COUNT	(8)
#define ECHARGE_YPLACE_COUNT	(8)

/* Electrocar park without charge 1X20X20 */
#define ECAR_FLOOR_COUNT		(1)
#define ECAR_XPLACE_COUNT		(20)
#define ECAR_YPLACE_COUNT		(20)

/* String length of user infos  */
#define USER_NAME_LENG			(11)	/* with '\0' */
#define USER_SID_LENG			(17)	/* with '\0' */
#define USER_ADDR_LENG			(64)	/* with '\0' */
#define USER_TEL_LENG			(15)	/* with '\0' */
#define USER_PLATE_LENG			(9)		/* with '\0' */

#define INVALID_USER_SERNO		(0x00000000UL)

/* Care type */
#define CAR_TYPE_GENERAL		(0)
#define CAR_TYPE_ECHARGE		(1)
#define CAR_TYPE_NCHARGE		(2)

#define TRUE					(1)
#define FALSE					(0)
#define SUCCESS					(0)
#define FAILURE					(-1)

#define LOCAL					static
#define GLOBAL					

#ifndef NDEBUG
#define LOG(x)		fprintf(stderr, "%s:%d: %s\n", __FUNCTION__, __LINE__, (x))
#else
#define LOG(x)
#endif

#define ArraySize(x)			(sizeof(x) / sizeof((x)[0]))

#define DEFAULT_DB_FILENAME "./parking.db"
#define EVENTBUFFERSIZE		BUFSIZ

/* ************** type defines **************** */
/* Primary data type */
typedef signed int		bool;
typedef unsigned char	uint8;
typedef unsigned short	uint16;
typedef unsigned int	uint32;
typedef signed   char	sint8;
typedef signed   short	sint16;
typedef signed   int	sint32;

/* User Structure */
typedef struct stUserInfo_t{
	uint32	m_u32SerialNo;
	char	m_cName[USER_NAME_LENG];//位段结构 节省存储空间 占一位
	char	m_cId[USER_SID_LENG];
	char	m_cAddr[USER_ADDR_LENG];
	char	m_cTel[USER_TEL_LENG];
	char	m_cPlateNumber[USER_PLATE_LENG];
	struct{
		uint8 m_u8isStudent : 1;
		uint8 m_u8isUsing	: 1;
		uint8 m_u8CareType	: 2;
		uint8 m_u8Unused	: 4;
	};
	struct{
		uint8	m_u8ParkType;
		uint8	m_u8Floor;
		uint8	m_u8X;
		uint8	m_u8Y;
		uint32	m_u32Timeout; //时间戳（1970年1月1日开始到指定时间的秒数）
	};
	float		m_fRemaining; //卡里剩余金额
	struct stUserInfo_t * next; //链表的头指针
}UserInfos_t;

/* Parking Structure */
typedef struct{ //每一个车位
	uint32	m_u32UserSerno;//存储用户的序号
}Parking_t;

typedef struct {                                 //数据库的头信息
	uint32		m_u32MagicNumber;               //魔术号 用于识别文件类型
	float		m_fParkPrice;                   //普通停车场的停车价格                  
	float		m_fChargePrice;                 //充电停车场的停车价格
	float		m_fNChargePrice;                //不充电的停车价格
	uint32		m_u32GParkUsing;                //普通停车位的使用情况
	Parking_t		m_astGParking[GPARK_FLOOR_COUNT * GPARK_XPLACE_COUNT * GPARK_YPLACE_COUNT]; //停车场的车位数据
	uint32		m_u32CParkUsing;
	Parking_t		m_astCParking[ECHARGE_FLOOR_COUNT * ECHARGE_XPLACE_COUNT * ECHARGE_YPLACE_COUNT];
	uint32		m_u32EParkUsing;
	Parking_t		m_astEParking[ECAR_FLOOR_COUNT * ECAR_XPLACE_COUNT * ECAR_YPLACE_COUNT];
	uint32			m_u32UserCount; //数据库中共多少个注册用户
	UserInfos_t* 	m_pstUserInfosHandle;
}DataBaseFormat_t;

typedef struct eventLoop {
	void *		retData;
	void *		argData;//未使用
	void(*pfun) (struct eventLoop *);//用于指向一个函数(函数指针)用于控制程序的运行逻辑
} EventLoopHandle_t;

typedef void(*Pfun) (EventLoopHandle_t *);

#define TIME_STRING_LENGTH	(28)//存储时间的字符串

#define SECONDS_OF_MINUTE	(60)//一分钟多少秒
#define MINUTES_OF_HOUR	(60)//一小时多少分


#define DBERROR_SUCCESS			(0) //数据库访问出错时的错误代号
#define DBERROR_OPENAGAIN		(-1)//重复打开
#define DBERROR_INVNAME			(-2)//错误的文件名
#define DBERROR_OPENFAILED		(-3)//打开失败
#define DBERROR_WRIEDATA		(-4)//写数据失败
#define DBERROR_READDATA		(-5)//读取数据失败
#define DBERROR_INVTYPE			(-6)//魔术号不同 文件类型错误
#define DBERROR_INVDATA			(-7)//数据库里的数据错误
#define DBERROR_EMPTYDATA		(-8)//空数据
#define DBERROR_NOMEM			(-9)//系统内存不足
#define DBERROR_BADDBFILE		(-10)//文件损坏
#define DBERROR_OPERATION		(-11)//对数据库进行了错误操作

#define FINDTYPE_NAME			(1)
#define FINDTYPE_ID				(2)
#define FINDTYPE_PLATE			(3)
#define FINDTYPE_TEL				(4)
#define FINDTYPE_SERNO			(5)



#define MAX_FIND_COUNT			(10)



 void showSystemMenu(EventLoopHandle_t *handle);

 bool openDatabase(const char *pcDbName);
 bool closeDatabase(void);
 uint32 getParkingStatus(uint8 parkType);//取得当前类型的停车数量
 uint32 getParkingTotal(uint8 parkType);//取得当前类型的车位数量
 uint32 getMaxSernoId(void);//取得数据库中当前最大的用户序号值

 bool appendUserInfo(const UserInfos_t * pstUserInfo);//添加一个用户信息
 bool deleteUserInfo(const UserInfos_t * pstUserInfo);//消除一个用户信息
 float getParkPrice(void);//取得停车车位价格
 void setPrakPrice(float price);//设置停车车位价格
 float getChargePrice(void);
 void setChargePrice(float price);
 float getNoChargePrice(void);
 void setNoChargePrice(float price);
 void findUserInfo(int type, const void *filter,UserInfos_t**result, uint8 *findCount);//找到用户信息
 bool findFreePlace(uint8 type, uint8 *fool, uint8 *x, uint8 *y);//查找空闲车位
 void setCarIn(UserInfos_t *pUserInfo);//停车
 void setCarOut(UserInfos_t *pUserInfo);//出库

 const char *getDbErrorString(void);//取得数据库出错的字符串

uint32 getTimeStamp(void);
const char *getCurTimeString(void);
const char *timeStamp2String(uint32 timeStamp);



/* ****************** Menu defines ************************ */
/*
                                              system menu	
                    ┌──────────────┼──────────┐                  
                 UserManage                    ParkManage           QuitSystem
        ┌──┬──┼──┬──┐    ┌──┬──┼──┬──┐        │
       Add    Del   Mod   Fin   Ret  Empty Check CarIn CarOut Ret       ○
*/

#define TIME_PRINT_FMT			"%38s\n"//输出系统时间的格式
#define PLACE_PRINT_FMT			" %c(%04d/%04d)"	//车位使用情况的输出格式		

#define SYSTEM_MENU \
"┌──────System─menu──────┐\n" \
"│1) User manage.                     │\n" \
"│2) Park manage.                     │\n" \
"│3) Park money manage.               │\n" \
"│4) Charge money manage.             │\n" \
"│5) No charge money manage.          │\n" \
"│6) Quit system.                     │\n" \
"└──────────────────┘\n"
#define MENU_CHOOSE_SYSTEM_OFFSET		(-1)
#define MENU_CHOOSE_SYSTEM_USER			(1)
#define MENU_CHOOSE_SYSTEM_PARK			(2)
#define MENU_CHOOSE_SYSTEM_PARKM		(3)
#define MENU_CHOOSE_SYSTEM_CHARGE		(4)
#define MENU_CHOOSE_SYSTEM_NCHARGE		(5)
#define MENU_CHOOSE_SYSTEM_QUIT			(6)

#define USER_MENU \
"┌───────User─menu──────┐\n" \
"│1) Add user.                        │\n" \
"│2) Del user.                        │\n" \
"│3) Modify User Infos.               │\n" \
"│4) Find User by Name.               │\n" \
"│5) Find User by Id.                 │\n" \
"│6) Find User by Tel.                │\n" \
"│7) Find User by Plate Number.       │\n" \
"│8) Find User by Serial No.          │\n" \
"│9) Return to system menu.           │\n" \
"└──────────────────┘\n"
#define MENU_CHOOSE_USER_OFFSET			(MENU_CHOOSE_SYSTEM_QUIT + MENU_CHOOSE_SYSTEM_OFFSET)
#define MENU_CHOOSE_USER_ADD				(1)	
#define MENU_CHOOSE_USER_DEL				(2)
#define MENU_CHOOSE_USER_MOD				(3)
#define MENU_CHOOSE_USER_FNAME			(4)
#define MENU_CHOOSE_USER_FID				(5)
#define MENU_CHOOSE_USER_FTEL				(6)
#define MENU_CHOOSE_USER_FPNUM			(7)
#define MENU_CHOOSE_USER_FSERNO			(8)
#define MENU_CHOOSE_USER_QUIT				(9)

#define PARK_MENU \
"┌───────Park─menu──────┐\n" \
"│1) Find free parking spaces.        │\n" \
"│2) Check the use of parking space.  │\n" \
"│3) Vehicle warehousing.             │\n" \
"│4) Vehicle outgoing.                │\n" \
"│5) Return to system menu.           │\n" \
"└──────────────────┘\n"
#define MENU_CHOOSE_PARK_OFFSET			(MENU_CHOOSE_USER_OFFSET+ MENU_CHOOSE_USER_QUIT)
#define MENU_CHOOSE_PARK_FREE				(1)
#define MENU_CHOOSE_PARK_USTAT			(2)
#define MENU_CHOOSE_PARK_IN				(3)
#define MENU_CHOOSE_PARK_OUT				(4)
#define MENU_CHOOSE_PARK_QUIT				(5)


 void showUserMenu(EventLoopHandle_t *handle);//显示用户菜单
 void showParkingmenu(EventLoopHandle_t *handle);//显示停车菜单
 void setParkMoney(EventLoopHandle_t *handle);//普通停车场的车位费
 void setChargeMoney(EventLoopHandle_t *handle);//充电停车位的车位费
 void setNoChargeMoney(EventLoopHandle_t *handle);//非充电停车位的车位费

 void operationAddUser(EventLoopHandle_t *handle);//添加用户
 void operationDelUser(EventLoopHandle_t *handle);//删除用户
 void operationModUser(EventLoopHandle_t *handle);//修改用户
 void operationFindByName(EventLoopHandle_t *handle);//按名字查找
 void operationFindById(EventLoopHandle_t *handle);//按ID查找
 void operationFindByTel(EventLoopHandle_t *handle);//按电话查找
 void operationFindByPlateNumber(EventLoopHandle_t *handle);//按车牌查找
 void operationFindBySerNO(EventLoopHandle_t *handle);//按用户序号查找

 void operationFindFree(EventLoopHandle_t *handle);//查找空余车位
 void operationCheckUse(EventLoopHandle_t *handle);//查找用户车位使用情况
 void operationCarIn(EventLoopHandle_t *handle);//停车
 void operationCarOut(EventLoopHandle_t *handle);//出库


 void createUserInfo(UserInfos_t *pusr);//创建用户信息
 void printUserInfo(const UserInfos_t* pstUser);//输出用户信息
 void showTitle(void);//输出菜单头
 uint32 getInteger(uint32 min, uint32 max);//输入一个整形
 uint8   getString(char *str, uint8 len);//输入一个字符串
 float getFloat(float min, float max);//输入一个浮点型
 char getChar(const char *content);//输入一个字符


typedef struct{
	int 		choose;//用户选择值
	Pfun		pfun;//用户选择以后对应的处理函数
}MenuProcess_t;

static char timeString[TIME_STRING_LENGTH];

uint32 getTimeStamp(void)
{
	return time(NULL);
}
const char *timeStamp2String(uint32 timeStamp)//把指定时间的时间戳转换为字符串
{
    struct tm	*tmValue;
	time_t		rawTime = timeStamp;
	
	tmValue		= localtime(&rawTime);
    memset(timeString, 0x00, sizeof(timeString));
    strftime(timeString, TIME_STRING_LENGTH, "%Y-%m-%d %H:%M:%S(%a)", tmValue);
	
	return timeString;
}
const char *getCurTimeString(void)//取得当前时间的字符串
{
	uint32 timeStamp;

	timeStamp = getTimeStamp();
 
	return timeStamp2String(timeStamp);
}



#define MAGIC_NUMBER			(0xABEC)		/* To check whether the document is DB */
#define FILE_NAME_MAX_LENGTH	(128)

typedef struct {
	bool				m_isOpen;//数据库是否在使用中
	FILE*				m_pf;//文件指针 用于指向一个已经打开的文件
	uint8				m_LastErrno;//数据库最后一次出错时的错误号
	char 				m_acDbFName[FILE_NAME_MAX_LENGTH];
	DataBaseFormat_t	m_stDBMemMap;//数据库的头信息
}DbHandle_t;

 DbHandle_t stHandle; //声明变量 数据库的管理结构

 void setDatabaseName(const char *pcDbName);//设置数据库的文件名
 bool initializationDatabase(void);//初始化数据库
 bool loadDatabase(void);//导入数据库
 bool saveDatabase(void);//导出数据库
 bool databaseIsExist(void);//判断数据库文件是否已经存在
 bool loadUserInfos(void);//从数据库加载用户信息
 bool freeUserInfos(void);//释放内存中的用户信息
 bool checkMagicNumber(void);//检查文件类型
 bool removeUserInfo(uint32 index);//从内存中删掉用户信息

 const char *getDbErrorString(void)
{
	static const char * errorStrings[] = {
		"SUCCESS",
		"DBERROR : DB file was open",
		"DBERROR : Invalid file name",
		"DBERROR : DB file open filed",
		"DBERROR : Write to db file error",
		"DBERROR : Read from db file error",
		"DBERROR : Invalid db file type",
		"DBERROR : Invalid db data format",
		"DBERROR : Empty db data",
		"DBERROR : Not enough memory",
		"DBERROR : Bad db file",
		"DBERROR : Invalid db operation",
	};

	return errorStrings[stHandle.m_LastErrno];
}

 bool openDatabase(const char *pcDbName)
{
	bool 	bRet 			= FALSE;
	uint32	u32nameLength;

	u32nameLength = strlen(pcDbName);

	if((NULL == pcDbName) || (u32nameLength == 0) || 
		(u32nameLength >= sizeof(stHandle.m_acDbFName))){
		stHandle.m_LastErrno = DBERROR_INVNAME;
		return bRet;
	}
	if(TRUE == stHandle.m_isOpen){
		stHandle.m_LastErrno = DBERROR_OPENAGAIN;
		return FALSE;
	}
	setDatabaseName(pcDbName);
	if(FALSE == databaseIsExist()){
		if(FALSE == initializationDatabase()){
			return FALSE;
		}
	}
	bRet = loadDatabase();

	return bRet;
}

 bool closeDatabase(void){
	if(FALSE == stHandle.m_isOpen){
		return FALSE;
	}
	if(! saveDatabase())
	{
		return FALSE;
	}
	if((stHandle.m_stDBMemMap.m_u32UserCount != 0) 
		&& (NULL != stHandle.m_stDBMemMap.m_pstUserInfosHandle)){
		freeUserInfos();
	}
	if(stHandle.m_pf != NULL){
		fclose(stHandle.m_pf);
		stHandle.m_pf = NULL;
	}
	memset(&stHandle, 0x00, sizeof(stHandle));

	return TRUE;
}

 uint32 getParkingStatus(uint8 parkType)
{
	uint32 count = 0U;
	
	switch(parkType){
		case PARKTYPE_GPARK:
			count = stHandle.m_stDBMemMap.m_u32CParkUsing; break;
		case PARKTYPE_ECHARGE:
			count = stHandle.m_stDBMemMap.m_u32CParkUsing; break;
		case PARKTYPE_ECAR:
			count = stHandle.m_stDBMemMap.m_u32EParkUsing; break;
		default:
			count = 0U;
	}
	return count;
}
 uint32 getParkingTotal(uint8 parkType)
{
	uint32 count = 0U;
	
	switch(parkType){
		case PARKTYPE_GPARK:
			count = ArraySize(stHandle.m_stDBMemMap.m_astGParking); break;
		case PARKTYPE_ECHARGE:
			count = ArraySize(stHandle.m_stDBMemMap.m_astCParking); break;
		case PARKTYPE_ECAR:
			count = ArraySize(stHandle.m_stDBMemMap.m_astEParking); break;
		default:
			count = 0U;
	}
	return count;


}


 bool checkMagicNumber(void)
{
	return (stHandle.m_stDBMemMap.m_u32MagicNumber == MAGIC_NUMBER);
}

 bool loadDatabase(void)
{
	bool bRet = FALSE;
	
	stHandle.m_pf = fopen(stHandle.m_acDbFName, "r");
	if(NULL == stHandle.m_pf){
		stHandle.m_LastErrno = DBERROR_OPENFAILED;
		return FALSE;
	}
	if(fread(&stHandle.m_stDBMemMap, sizeof(stHandle.m_stDBMemMap),1,stHandle.m_pf) != 1){
		stHandle.m_LastErrno = DBERROR_READDATA;
		return FALSE;
	}
	stHandle.m_stDBMemMap.m_pstUserInfosHandle = NULL;
	if(TRUE == checkMagicNumber()){
		bRet = loadUserInfos();
	}else{
		stHandle.m_LastErrno = DBERROR_INVTYPE;
		bRet = FALSE;
	}
	fclose(stHandle.m_pf);
	stHandle.m_pf = NULL;
	stHandle.m_isOpen = TRUE;
	return bRet;
}
 bool saveDatabase(void)
{
	FILE 			*fp 			= NULL;
	UserInfos_t	*pHandle 	= NULL;
	UserInfos_t	userInfo;

	fp = fopen(stHandle.m_acDbFName, "w+");
	if(NULL == fp){
		stHandle.m_LastErrno = DBERROR_OPENFAILED;
		return FALSE;
	}
	if(1 != fwrite(&stHandle.m_stDBMemMap, sizeof(stHandle.m_stDBMemMap), 1, fp)){
		stHandle.m_LastErrno = DBERROR_WRIEDATA;
		fclose(fp);
		return FALSE;
	}
	pHandle = stHandle.m_stDBMemMap.m_pstUserInfosHandle;
	while(pHandle != NULL){
		userInfo = *pHandle;
		userInfo.next = NULL;
		if(1 != fwrite(&userInfo, sizeof(userInfo), 1, fp)){
			stHandle.m_LastErrno = DBERROR_WRIEDATA;
			fclose(fp);
			return FALSE;
		}
		pHandle = pHandle->next;
	}
	fclose(fp);
	
	return TRUE;
	
}

 bool appendUserInfo(const UserInfos_t * pstUserInfo)
{
	UserInfos_t 	*pHeader;
	UserInfos_t 	*pPrev;
	
	if(NULL == pstUserInfo){
		stHandle.m_LastErrno = DBERROR_EMPTYDATA;
		return FALSE;
	}
	pHeader = pPrev = stHandle.m_stDBMemMap.m_pstUserInfosHandle;
	while(pHeader != NULL){
		pPrev   = pHeader;
		pHeader = pHeader->next;
	}
	pHeader = (UserInfos_t *)malloc(sizeof(UserInfos_t));
	if(NULL == pHeader){
		 stHandle.m_LastErrno = DBERROR_NOMEM;
		 return FALSE;
	}
	if(pPrev != NULL){
		pPrev->next = pHeader;
	}else{
		stHandle.m_stDBMemMap.m_pstUserInfosHandle = pHeader; 

	}
	*pHeader = *pstUserInfo;
	pHeader->next = NULL;
	stHandle.m_stDBMemMap.m_u32UserCount++;

	return TRUE;
}
 bool removeUserInfo(uint32 index)
{
	uint32			counter;
	UserInfos_t		*pProv;
	UserInfos_t		*pHeader;
	
	if(index >= stHandle.m_stDBMemMap.m_u32UserCount){
		stHandle.m_LastErrno = DBERROR_OPERATION;
		return FALSE;
	}
	/*
	if(0 == index){
		pProv = stHandle.m_stDBMemMap.m_pstUserInfosHandle;
		pHeader= pProv->next;
		free(pProv);
		pProv = pHeader;
	}
	*/
	pProv = pHeader = stHandle.m_stDBMemMap.m_pstUserInfosHandle;
	for(counter = 0; counter < index; counter++){
		pProv = pHeader;
		pHeader = pHeader->next;
		free(pProv);
		pProv = pHeader;
	}
	return TRUE;
}

 bool loadUserInfos(void)
{
	uint32		userCount	= stHandle.m_stDBMemMap.m_u32UserCount;
	UserInfos_t	userInfo;

	if(0 == userCount){
		return TRUE;
	}
	while(! feof(stHandle.m_pf)){
		if(1 != fread(&userInfo, sizeof(userInfo), 1, stHandle.m_pf)){
			if(feof(stHandle.m_pf)){
				return TRUE;
			}
			stHandle.m_LastErrno = DBERROR_BADDBFILE;
			return FALSE;
		}
		userInfo.next = NULL;
		if(FALSE == appendUserInfo(&userInfo)){
			return FALSE;
		}
	}
	return TRUE;
}

 bool freeUserInfos(void)
{
	bool	bRet = TRUE;
	uint32	counter = 0;

	for(counter = 0; counter < stHandle.m_stDBMemMap.m_u32UserCount; counter++){
		bRet = removeUserInfo(0);
	}

	return bRet;
}

 bool initializationDatabase(void){
	bool	bRet 	= TRUE;
	FILE 	*fp 	= NULL;
	int		iRet;

	fp = fopen(stHandle.m_acDbFName, "wb+");
	if(NULL == fp){
		stHandle.m_LastErrno = DBERROR_OPENFAILED;
		return FALSE;
	}
	stHandle.m_pf			= NULL;
	stHandle.m_isOpen 		= FALSE;
	stHandle.m_LastErrno 	= DBERROR_SUCCESS;
	memset(&stHandle.m_stDBMemMap, 0x00, sizeof(stHandle.m_stDBMemMap));
	stHandle.m_stDBMemMap.m_u32MagicNumber = MAGIC_NUMBER;

	iRet = fwrite(&stHandle.m_stDBMemMap, sizeof(stHandle.m_stDBMemMap), 1, fp);
	if(iRet != 1){
		stHandle.m_LastErrno = DBERROR_WRIEDATA;
		bRet = FALSE;
	}
	fflush(fp);
	fclose(fp);
	fp = NULL;
	
	return bRet;
}

 void setDatabaseName(const char *pcDbName)
{
	memset(stHandle.m_acDbFName, 0x00, sizeof(stHandle.m_acDbFName));
	
	strcpy(stHandle.m_acDbFName, pcDbName);
}

 bool databaseIsExist(void)
{
	FILE * fp;
	fp = fopen(stHandle.m_acDbFName, "r");
    if( fp == NULL ) {
        return FALSE;
    }
	fclose(fp);
	return TRUE;
}

 uint32 getMaxSernoId(void)
{
	UserInfos_t *	handle;
	uint32		maxSerno	= 0U;

	handle = stHandle.m_stDBMemMap.m_pstUserInfosHandle;

	while(handle){
		if(maxSerno < handle->m_u32SerialNo){
			maxSerno = handle->m_u32SerialNo;
		}
		handle = handle->next;
	}
	return maxSerno;
}

 void findUserInfo(int type, const void *filter,UserInfos_t**result, uint8 *findCount)

{
	UserInfos_t *	handle;
	char 	*pStr = (char *)filter;	
	uint32	serno = *((uint32 *)filter);
	uint8		foundCount = 0;
	
	handle = stHandle.m_stDBMemMap.m_pstUserInfosHandle;

	while(handle){
		if(foundCount >= *findCount){
			break;
		}
		switch(type)
		{
			case FINDTYPE_ID:
				if(0 == strcmp(pStr, handle->m_cId)){
					result[foundCount++] = handle;
				}
				break;
			case FINDTYPE_NAME:
				if(0 == strcmp(pStr, handle->m_cName)){
					result[foundCount++] = handle;
				}
				break;
			case FINDTYPE_PLATE:
				if(0 == strcmp(pStr, handle->m_cPlateNumber)){
					result[foundCount++] = handle;
				}
				break;
			case FINDTYPE_SERNO:
				if(serno == handle->m_u32SerialNo){
					result[foundCount++] = handle;
				}
				break;
			case FINDTYPE_TEL:
				if(0 == strcmp(pStr, handle->m_cTel)){
					result[foundCount++] = handle;
				}
				break;
			default:
				break;
		}
		handle = handle->next;
	}
	*findCount= foundCount;
}
 void setCarIn(UserInfos_t *pUserInfo)
{
	Parking_t		*pHead;
	uint32		offset;

	switch(pUserInfo->m_u8ParkType)
	{
	case CAR_TYPE_GENERAL:
		pHead= stHandle.m_stDBMemMap.m_astGParking;
		offset = (pUserInfo->m_u8Y- 1) + (pUserInfo->m_u8X - 1) *  GPARK_YPLACE_COUNT
			+ (pUserInfo->m_u8Floor - 1) * GPARK_XPLACE_COUNT * GPARK_YPLACE_COUNT;
		stHandle.m_stDBMemMap.m_u32GParkUsing++;
		break;
	case CAR_TYPE_ECHARGE:
		pHead = stHandle.m_stDBMemMap.m_astCParking;
		offset = (pUserInfo->m_u8Y- 1) + (pUserInfo->m_u8X - 1) * ECHARGE_YPLACE_COUNT
			+ (pUserInfo->m_u8Floor - 1) * ECHARGE_XPLACE_COUNT * ECHARGE_YPLACE_COUNT;
		stHandle.m_stDBMemMap.m_u32CParkUsing++;
		break;
	case CAR_TYPE_NCHARGE:
		pHead = stHandle.m_stDBMemMap.m_astEParking;
		offset = (pUserInfo->m_u8Y- 1) + (pUserInfo->m_u8X - 1) * ECAR_YPLACE_COUNT
			+ (pUserInfo->m_u8Floor - 1) * ECAR_XPLACE_COUNT * ECAR_YPLACE_COUNT;
		stHandle.m_stDBMemMap.m_u32EParkUsing++;
		break;
	default:
		return;
	}
	pHead[offset].m_u32UserSerno = pUserInfo->m_u32SerialNo;
}
 void setCarOut(UserInfos_t *pUserInfo)
{
	Parking_t		*pHead;
	uint32		offset;

	switch(pUserInfo->m_u8ParkType)
	{
	case CAR_TYPE_GENERAL:
		pHead= stHandle.m_stDBMemMap.m_astGParking;
		offset = (pUserInfo->m_u8Y- 1) + (pUserInfo->m_u8X - 1) *  GPARK_YPLACE_COUNT
			+ (pUserInfo->m_u8Floor - 1) * GPARK_XPLACE_COUNT * GPARK_YPLACE_COUNT;
		stHandle.m_stDBMemMap.m_u32GParkUsing--;
		break;
	case CAR_TYPE_ECHARGE:
		pHead = stHandle.m_stDBMemMap.m_astCParking;
		offset = (pUserInfo->m_u8Y- 1) + (pUserInfo->m_u8X - 1) * ECHARGE_YPLACE_COUNT
			+ (pUserInfo->m_u8Floor - 1) * ECHARGE_XPLACE_COUNT * ECHARGE_YPLACE_COUNT;
		stHandle.m_stDBMemMap.m_u32CParkUsing--;
		break;
	case CAR_TYPE_NCHARGE:
		pHead = stHandle.m_stDBMemMap.m_astEParking;
		offset = (pUserInfo->m_u8Y- 1) + (pUserInfo->m_u8X - 1) * ECAR_YPLACE_COUNT
			+ (pUserInfo->m_u8Floor - 1) * ECAR_XPLACE_COUNT * ECAR_YPLACE_COUNT;
		stHandle.m_stDBMemMap.m_u32EParkUsing--;
		break;
	default:
		return;
	}
	pHead[offset].m_u32UserSerno = INVALID_USER_SERNO;

}

 bool deleteUserInfo(const UserInfos_t * pstUserInfo)
{
	UserInfos_t *	handle;
	UserInfos_t *	tmphandle;
	uint32		maxSerno	= pstUserInfo->m_u32SerialNo;;

	tmphandle = handle = stHandle.m_stDBMemMap.m_pstUserInfosHandle;
	if(handle->m_u32SerialNo == maxSerno){
		tmphandle = handle->next;
		free(handle);
		stHandle.m_stDBMemMap.m_pstUserInfosHandle = tmphandle;
		stHandle.m_stDBMemMap.m_u32UserCount--;
		return TRUE;
	}
	handle = handle->next;
	while(handle){
		if(handle->m_u32SerialNo == maxSerno){
			tmphandle->next = handle->next;
			free(handle);
			stHandle.m_stDBMemMap.m_u32UserCount--;
			return TRUE;
		}	
		tmphandle = handle;
		handle = handle->next;
	}
	return FALSE;

}

 float getParkPrice(void)
{
	return stHandle.m_stDBMemMap.m_fParkPrice;
}
 void setPrakPrice(float price)
{
	stHandle.m_stDBMemMap.m_fParkPrice = price;
}
 float getChargePrice(void)
{
	return stHandle.m_stDBMemMap.m_fChargePrice;
}
 void setChargePrice(float price)
{
	stHandle.m_stDBMemMap.m_fChargePrice = price;
}
 float getNoChargePrice(void)
{
	return stHandle.m_stDBMemMap.m_fNChargePrice;
}
 void setNoChargePrice(float price)
{
	stHandle.m_stDBMemMap.m_fNChargePrice = price;
}

 bool findFreePlace(uint8 type, uint8 *fool, uint8 *x, uint8 *y)
{
	uint32 		offset = 0;
	uint32 		count = 0;
	uint8			countX, countY;
	Parking_t		*pHead;
	bool bFound = FALSE;

	switch(type)
	{
	case CAR_TYPE_GENERAL:
		pHead= stHandle.m_stDBMemMap.m_astGParking;
		count = ArraySize(stHandle.m_stDBMemMap.m_astGParking);
		countX = GPARK_XPLACE_COUNT;
		countY = GPARK_YPLACE_COUNT;
		break;
	case CAR_TYPE_ECHARGE:
		pHead = stHandle.m_stDBMemMap.m_astCParking;
		count = ArraySize(stHandle.m_stDBMemMap.m_astCParking);
		countX = ECHARGE_XPLACE_COUNT;
		countY = ECHARGE_YPLACE_COUNT;
		break;
	case CAR_TYPE_NCHARGE:
		pHead = stHandle.m_stDBMemMap.m_astEParking;
		count = ArraySize(stHandle.m_stDBMemMap.m_astEParking);
		countX = ECAR_XPLACE_COUNT;
		countY = ECAR_YPLACE_COUNT;
		break;
	default:
		return FALSE;
	}
	
	for(offset = 0; offset < count; offset++)
	{
		if(pHead->m_u32UserSerno == INVALID_USER_SERNO){
			bFound = TRUE;
			break;
		}
	}
	if(TRUE == bFound){
		*fool = (offset / (countX * countY)) + 1;
		offset %= *fool;
		*x = (offset / countY) + 1;
		*y = offset % countY;
	}
	return bFound;
}




 MenuProcess_t 		astMenuProcess[] = {
	{MENU_CHOOSE_SYSTEM_USER 		+ MENU_CHOOSE_SYSTEM_OFFSET, 	showUserMenu},
	{MENU_CHOOSE_SYSTEM_PARK 		+ MENU_CHOOSE_SYSTEM_OFFSET, 	showParkingmenu},
	{MENU_CHOOSE_SYSTEM_PARKM	+ MENU_CHOOSE_SYSTEM_OFFSET, 	setParkMoney},
	{MENU_CHOOSE_SYSTEM_CHARGE	+ MENU_CHOOSE_SYSTEM_OFFSET, 	setChargeMoney},
	{MENU_CHOOSE_SYSTEM_NCHARGE	+ MENU_CHOOSE_SYSTEM_OFFSET, 	setNoChargeMoney},
	{MENU_CHOOSE_SYSTEM_QUIT		+ MENU_CHOOSE_SYSTEM_OFFSET, 	NULL},
	{MENU_CHOOSE_USER_ADD 		+ MENU_CHOOSE_USER_OFFSET, 	operationAddUser},
	{MENU_CHOOSE_USER_DEL  		+ MENU_CHOOSE_USER_OFFSET, 	operationDelUser},
	{MENU_CHOOSE_USER_MOD 		+ MENU_CHOOSE_USER_OFFSET, 	operationModUser},
	{MENU_CHOOSE_USER_FNAME		+ MENU_CHOOSE_USER_OFFSET, 	operationFindByName},
	{MENU_CHOOSE_USER_FID			+ MENU_CHOOSE_USER_OFFSET, 	operationFindById},
	{MENU_CHOOSE_USER_FTEL		+ MENU_CHOOSE_USER_OFFSET, 	operationFindByTel},
	{MENU_CHOOSE_USER_FPNUM		+ MENU_CHOOSE_USER_OFFSET, 	operationFindByPlateNumber},
	{MENU_CHOOSE_USER_FSERNO		+ MENU_CHOOSE_USER_OFFSET, 	operationFindBySerNO},
	{MENU_CHOOSE_USER_QUIT			+ MENU_CHOOSE_USER_OFFSET, 	showSystemMenu},
	{MENU_CHOOSE_PARK_FREE		+ MENU_CHOOSE_PARK_OFFSET,	operationFindFree},
	{MENU_CHOOSE_PARK_USTAT		+ MENU_CHOOSE_PARK_OFFSET,	operationCheckUse},
	{MENU_CHOOSE_PARK_IN			+ MENU_CHOOSE_PARK_OFFSET,	operationCarIn},
	{MENU_CHOOSE_PARK_OUT			+ MENU_CHOOSE_PARK_OFFSET,	operationCarOut},
	{MENU_CHOOSE_PARK_QUIT		+ MENU_CHOOSE_PARK_OFFSET,	showSystemMenu},
};

 Pfun getOperationFuncAddr(uint32 optid);

 void showSystemMenu(EventLoopHandle_t *handle)//展示系统菜单
{
	uint8 choose;
	
	showTitle();
	printf(SYSTEM_MENU);
	choose = getInteger(MENU_CHOOSE_SYSTEM_USER, MENU_CHOOSE_SYSTEM_QUIT);
	handle->pfun = getOperationFuncAddr(choose + MENU_CHOOSE_SYSTEM_OFFSET);
}
 Pfun getOperationFuncAddr(uint32 optid)
{
	if(optid >= ArraySize(astMenuProcess)){
		printf("Invalid item : %d\n", optid);
		return NULL;
	}

	return astMenuProcess[optid].pfun;
}

 void showUserMenu(EventLoopHandle_t *handle)
{
	uint8 choose;
	
	showTitle();
	printf(USER_MENU);
	choose = getInteger(MENU_CHOOSE_USER_ADD, MENU_CHOOSE_USER_QUIT);
	handle->pfun = getOperationFuncAddr(choose + MENU_CHOOSE_USER_OFFSET);
}
 void showParkingmenu(EventLoopHandle_t *handle)
{
	uint8 choose;
	
	showTitle();
	printf(PARK_MENU);
	choose = getInteger(MENU_CHOOSE_PARK_FREE, MENU_CHOOSE_PARK_QUIT);
	handle->pfun = getOperationFuncAddr(choose + MENU_CHOOSE_PARK_OFFSET);
}
 void setParkMoney(EventLoopHandle_t *handle)
{
	float f;
	
	printf("Current Park price is: %.2f(Yuan/Huor)\n", getParkPrice());
	printf("Input now price:");
	f = getFloat(1.0F, 100.0F);
	setPrakPrice(f);
	printf("Price set success. Press 'Entry' to continue...");
	while(getchar() != '\n');
	handle->pfun = showSystemMenu;
}
 void setChargeMoney(EventLoopHandle_t *handle)
{
	float f;
	
	printf("Current Charge price is: %.2f(Yuan/Huor)\n", getChargePrice());
	printf("Input now price:");
	f = getFloat(1.0F, 100.0F);
	setChargePrice(f);
	printf("Price set success. Press 'Entry' to continue...");
	while(getchar() != '\n');
	handle->pfun = showSystemMenu;
}
 void setNoChargeMoney(EventLoopHandle_t *handle)
{
	float f;
	
	printf("Current No Charge price is: %.2f(Yuan/Huor)\n", getNoChargePrice());
	printf("Input now price:");
	f = getFloat(1.0F, 100.0F);
	setNoChargePrice(f);
	printf("Price set success. Press 'Entry' to continue...");
	while(getchar() != '\n');
	handle->pfun = showSystemMenu;
}

 void showTitle(void)
{
	printf(TIME_PRINT_FMT, getCurTimeString());
	printf(PLACE_PRINT_FMT,'G', getParkingStatus(PARKTYPE_GPARK), getParkingTotal(PARKTYPE_GPARK));
	printf(PLACE_PRINT_FMT,'C', getParkingStatus(PARKTYPE_ECHARGE), getParkingTotal(PARKTYPE_ECHARGE));
	printf(PLACE_PRINT_FMT,'E', getParkingStatus(PARKTYPE_ECAR), getParkingTotal(PARKTYPE_ECAR));
	printf("\n");
}
 uint8   getString(char *str, uint8 len)
{
	uint8 length = 0;
	const char *ptmp;

	ptmp = fgets(str, len, stdin);
	fflush(stdin);

	length = strlen(ptmp);
	if(str[length - 1] == '\n'){
		str[length - 1] = '\0';
		length --;
	}
	return length;
}
 uint32 getInteger(uint32 min, uint32 max)
{
	char 	str[12];
	char	*stop;
	uint8 	len;
	uint32	value;

	while(1){
		printf("Please input an integer(%u~%u):", min, max);
		fflush(stdin);
		memset(str, 0x00, sizeof(str));
		len = getString(str,sizeof(str));
		if(0 != len){ 
			value = strtol(str, &stop, 10);
			if(*stop == '\0'){
				if(value <= max && value >= min){
					return value;
				}
			}
		}
		printf("Input Error Please input Again!\n");
	}
	return 0;
}
 float getFloat(float min, float max){
	char 	str[24];
	char	*stop;
	uint8 	len;
	float	value;

	while(1){
		printf("Please input an real number(%.2f~%.2f):", min, max);
		fflush(stdin);
		memset(str, 0x00, sizeof(str));
		len = getString(str,sizeof(str));
		if(0 != len){ 
			value = (float)strtod(str, &stop);
			if(*stop == '\0'){
				if(value <= max && value >= min){
					return value;
				}
			}
		}
		printf("Input Error Please input Again!\n");
	}
	return .0F;
}

 char getChar(const char*content)
{
	char c;
	
	while(1){
		printf("Please input a char [%s]:", content);
		c = getchar();
		while(getchar() != '\n');
		if(strchr(content, c) != NULL){
			return c;
		}
		printf("Input Error Please input Again!\n");
	}
}

 void printUserInfo(const UserInfos_t* pstUser)
{
	printf("*****************User infos***********************\n");
	printf("serno : %u\n", pstUser->m_u32SerialNo);
	printf("name  : %s\n", pstUser->m_cName);
	printf("Type  : %d%s\n", pstUser->m_u8isStudent, "(1:Student, 1:Employee)");
	printf("stuid : %s\n", pstUser->m_cId);
	printf("Tel   : %s\n", pstUser->m_cTel);
	printf("Addr  : %s\n", pstUser->m_cAddr);
	printf("Plat  : %s\n", pstUser->m_cPlateNumber);
	printf("Type  : %d%s\n", pstUser->m_u8CareType, "(0:GenCar, 1:ElectCarWithCharge, 2:ElectCarWithoutCharge)");
	printf("Status: %d%s\n", pstUser->m_u8isUsing, "(0:Not use, 1:Using)");
	if(pstUser->m_u8isUsing){
		uint32	during;
		float	money;

		printf("Agent Park Place : (area=%d,fool=%d, x=%d, y=%d). Press Entry key to continue..\n",
					pstUser->m_u8ParkType, pstUser->m_u8Floor, pstUser->m_u8X, pstUser->m_u8Y);
		
		during = getTimeStamp() - pstUser->m_u32Timeout;
		during = (during / SECONDS_OF_MINUTE) +( (during % SECONDS_OF_MINUTE) ? 1 : 0);
		switch(pstUser->m_u8ParkType)
		{
			default:
				return;
			case CAR_TYPE_GENERAL:
				money = getParkPrice(); 
				break;
			case CAR_TYPE_ECHARGE:
				money = getChargePrice(); 
				break;
			case CAR_TYPE_NCHARGE:
				money = getNoChargePrice();
				break;
		}
		money = money * (during / MINUTES_OF_HOUR + ((during % MINUTES_OF_HOUR) ? 1 : 0));
		printf("Parking time(%dhour:%02dminute), collect money:%.2F(Yuan)\n", 
			during / MINUTES_OF_HOUR, during % MINUTES_OF_HOUR, money);
		if(money > pstUser->m_fRemaining){
			printf("Warning : not sufficient funds.\n");
		}
	}
	printf("Remain: %.2f(YUAN)\n", pstUser->m_fRemaining);
	printf("**************************************************\n");
}

 void operationAddUser(EventLoopHandle_t *handle)
{
	UserInfos_t	 userInfo;
	uint8			tmp;

	memset(&userInfo, 0x00, sizeof(userInfo));
	createUserInfo(&userInfo);

	printf("Save User Info?");
	tmp = getChar("YN");
	if(tmp == 'Y'){
		bool bRet;
		bRet = appendUserInfo(&userInfo);
		if(bRet == TRUE){
			printf("Save Successed. Press 'Enter' key to Continue...");
		}else{
			printf("Save Failed(%s).Press 'Enter' key to Continue...", getDbErrorString());
		}
		while(getchar() != '\n');
	}
	handle->pfun = showUserMenu;
}
 void createUserInfo(UserInfos_t *pusr)
{
	uint8			tmp;

	printf("Input user name(len < %d):", sizeof(pusr->m_cName));
	getString(pusr->m_cName,sizeof(pusr->m_cName) - 1);

	printf("Car type(0:GenCar, 1:ElectCarWithCharge, 2:ElectCarWithoutCharge):");
	pusr->m_u8CareType = getInteger(CAR_TYPE_GENERAL, CAR_TYPE_NCHARGE);

	printf("user type(S:Student, E:Employee). ");
	tmp = getChar("SE");
	pusr->m_u8isStudent = ((tmp == 'S') ? TRUE : FALSE);

	printf("Input EmployeeID or StudentID(len < %d):", sizeof(pusr->m_cId));
	getString(pusr->m_cId,sizeof(pusr->m_cId) - 1);

	printf("Input Address(len < %d):", sizeof(pusr->m_cAddr));
	getString(pusr->m_cAddr,sizeof(pusr->m_cAddr) - 1);

	printf("Input Telphone number(len < %d):", sizeof(pusr->m_cTel));
	getString(pusr->m_cTel,sizeof(pusr->m_cTel) - 1);

	printf("Input plate number(len < %d):", sizeof(pusr->m_cPlateNumber));
	getString(pusr->m_cPlateNumber,sizeof(pusr->m_cPlateNumber) - 1);

	printf("How much money do you charge. ");
	pusr->m_fRemaining = getFloat(MIN_CHARGE_MONEY,MAX_CHARGE_MONEY);
	
	pusr->m_u32SerialNo = getMaxSernoId() + 1;

	printUserInfo(pusr);

}
 void operationDelUser(EventLoopHandle_t *handle)
{
	uint32 serno;
	uint8			maxFindCount = 1;
	UserInfos_t*	userInfos = NULL;
	
	printf("Input Serial Number:");
	serno = getInteger(0, 0xFFFFFFFFUL);
	findUserInfo(FINDTYPE_SERNO,&serno,&userInfos,&maxFindCount);

	if(0 == maxFindCount){
		printf("Cannot find this user.Press Enter key To Display----------\n");
		while(getchar() != '\n');	
	}else{
		char c;
		printUserInfo(userInfos);
		printf("Delete this user?");
		c = getChar("YN");
		if(c == 'Y'){
			if(TRUE == deleteUserInfo(userInfos)){
				printf("Delete user successed.");
			}else{
				printf("Delete user failed..");
			}
			printf(".Press Enter key To Display----------\n");
			while(getchar() != '\n');	
		}
	}
	handle->pfun = showUserMenu;
}
 void operationModUser(EventLoopHandle_t *handle)
{
	uint32 serno;
	uint8			maxFindCount = 1;
	UserInfos_t*	userInfos = NULL;
	
	printf("Input Serial Number:");
	serno = getInteger(0, 0xFFFFFFFFUL);
	findUserInfo(FINDTYPE_SERNO,&serno,&userInfos,&maxFindCount);

	if(0 == maxFindCount){
		printf("Cannot find this user.Press Enter key To Display----------\n");
		while(getchar() != '\n');	
	}else{
		char c;
		UserInfos_t infos;
		memset(&infos, 0x00, sizeof(infos));
		printUserInfo(userInfos);
		printf("Modify this user?");
		c = getChar("YN");
		if(c == 'Y'){
			createUserInfo(&infos);
			printUserInfo(&infos);
			printf("Save Modify?");
			c= getChar("YN");
			if(c == 'Y'){
				*userInfos = infos;
				printf("Save user info done. Press 'Entry' to continue...");
				while(getchar() != '\n');
			}

			
			while(getchar() != '\n');	
		}
	}
	handle->pfun = showUserMenu;
	
}
 void operationFindAllType(int type, const void *filter)
{
	uint8			maxFindCount = MAX_FIND_COUNT;
	uint8			counter;
	UserInfos_t*	userInfos[MAX_FIND_COUNT] = {NULL};
	
	findUserInfo(type,filter,userInfos,&maxFindCount);
	printf("Found User Count:%d. Press Enter key To Display----------\n", maxFindCount);
	while(getchar() != '\n');
	for(counter = 0; counter < maxFindCount; counter++)
	{
		printf("==============List(%d/%d)==============\n", counter + 1, maxFindCount);
		printUserInfo(userInfos[counter]);
		printf("Press Enter key To Continue----------\n");
		while(getchar() != '\n');
	}
}
 void operationFindByName(EventLoopHandle_t *handle)
{
	char filter[USER_NAME_LENG] = "";	
	printf("Input user name(len < %d):", sizeof(filter));
	getString(filter,sizeof(filter) - 1);
	operationFindAllType(FINDTYPE_NAME, filter);
	handle->pfun = showUserMenu;
}
 void operationFindById(EventLoopHandle_t *handle)
{
	char filter[USER_SID_LENG] = "";	
	printf("Input ID(len < %d):", sizeof(filter));
	getString(filter,sizeof(filter) - 1);
	operationFindAllType(FINDTYPE_ID, filter);
	handle->pfun = showUserMenu;
}
 void operationFindByTel(EventLoopHandle_t *handle)
{
	char filter[USER_TEL_LENG] = "";	
	printf("Input Tel(len < %d):", sizeof(filter));
	getString(filter,sizeof(filter) - 1);
	operationFindAllType(FINDTYPE_TEL, filter);
	handle->pfun = showUserMenu;
}
 void operationFindByPlateNumber(EventLoopHandle_t *handle)
{
	char filter[USER_PLATE_LENG] = "";	
	printf("Input Plate Number(len < %d):", sizeof(filter));
	getString(filter,sizeof(filter) - 1);
	operationFindAllType(FINDTYPE_PLATE, filter);
	handle->pfun = showUserMenu;
}
 void operationFindBySerNO(EventLoopHandle_t *handle)
{
	uint32 serno;
	
	printf("Input Serial Number:");
	serno = getInteger(0, 0xFFFFFFFFUL);
	operationFindAllType(FINDTYPE_SERNO, &serno);
	handle->pfun = showUserMenu;
}
 void operationFindFree(EventLoopHandle_t *handle)
{
	uint8		carType;
	
	
	printf("Car type(0:GenCar, 1:ElectCarWithCharge, 2:ElectCarWithoutCharge):");
	carType = getInteger(CAR_TYPE_GENERAL, CAR_TYPE_NCHARGE);

	printf("Remaing Park Place Count = %d\n", getParkingTotal(carType) - getParkingStatus(carType));

	handle->pfun = showParkingmenu;
}
 void operationCheckUse(EventLoopHandle_t *handle)
{

}
 void operationCarIn(EventLoopHandle_t *handle)
{
	char 		buffer[USER_PLATE_LENG] = "";
	UserInfos_t	*pUserInfo = NULL;
	uint8			findCount = 1;
	uint8			fool, x, y;
	uint8			parkType;
	char		c;
	
	printf("Input plate number(len < %d):", USER_PLATE_LENG);
	getString(buffer,USER_PLATE_LENG - 1);

	findUserInfo(FINDTYPE_PLATE,buffer,&pUserInfo, &findCount);
	if(0 == findCount){
		printf("This car is not in user database. please add user first.\n");
		goto NoPlaceNoIn;
	}
	if(TRUE == pUserInfo->m_u8isUsing){
		printf("This car was parked.\n");
		goto NoPlaceNoIn;
	}
	parkType = pUserInfo->m_u8CareType;
	if(FALSE == findFreePlace(pUserInfo->m_u8CareType, &fool,&x,&y))
	{
		if((pUserInfo->m_u8CareType == CAR_TYPE_GENERAL) || (pUserInfo->m_u8CareType == CAR_TYPE_NCHARGE)){
			printf("Sorry, Parking FULL !");
			goto NoPlaceNoIn;
		}
		if(getParkingTotal(CAR_TYPE_NCHARGE) - getParkingStatus(CAR_TYPE_NCHARGE) <= 0){
			printf("Sorry, Parking FULL !");
			goto NoPlaceNoIn;
		}
		printf("Charge park place not found. wheather parking into park without charge?");
		c = getChar("YN");
		if(c != 'Y'){
			goto NoPlaceNoIn;
		}
		findFreePlace(CAR_TYPE_NCHARGE, &fool, &x, &y);
		parkType = CAR_TYPE_NCHARGE;
	}
	pUserInfo->m_u8ParkType = parkType;
	pUserInfo->m_u8isUsing = TRUE;
	pUserInfo->m_u8Floor = fool;
	pUserInfo->m_u8X = x;
	pUserInfo->m_u8Y = y;
	pUserInfo->m_u32Timeout = getTimeStamp();
	setCarIn(pUserInfo);
	printf("Agent Park Place : (area=%d,fool=%d, x=%d, y=%d). Press Entry key to continue..\n",
					parkType, fool, x, y);
NoPlaceNoIn:
	printf("Press 'Entry' key to continue...");
	while(getchar() != '\n');
	handle->pfun = showParkingmenu;
	return;
}
 void operationCarOut(EventLoopHandle_t *handle)
{
	char 		buffer[USER_PLATE_LENG] = "";
	UserInfos_t	*pUserInfo = NULL;
	uint8			findCount = 1;
	uint32		during;
	float 		money;
	
	printf("Input plate number(len < %d):", USER_PLATE_LENG);
	getString(buffer,USER_PLATE_LENG - 1);

	findUserInfo(FINDTYPE_PLATE,buffer,&pUserInfo, &findCount);
	if(0 == findCount){
		printf("This car is not in user database. please add user first.\n");
		goto NoParkNoOut;
	}
	if(TRUE != pUserInfo->m_u8isUsing){
		printf("This car was not parked.\n");
		goto NoParkNoOut;
	}
	during = getTimeStamp() - pUserInfo->m_u32Timeout;
	during = (during / SECONDS_OF_MINUTE) +( (during % SECONDS_OF_MINUTE) ? 1 : 0);
	switch(pUserInfo->m_u8ParkType)
	{
		default:
			goto NoParkNoOut;
		case CAR_TYPE_GENERAL:
			money = getParkPrice(); 
			break;
		case CAR_TYPE_ECHARGE:
			money = getChargePrice(); 
			break;
		case CAR_TYPE_NCHARGE:
			money = getNoChargePrice();
			break;
	}
	money = money * (during / MINUTES_OF_HOUR + ((during % MINUTES_OF_HOUR) ? 1 : 0));
	printf("Parking time(%dhour:%02dminute), collect money:%.2F(Yuan)\n", 
		during / MINUTES_OF_HOUR, during % MINUTES_OF_HOUR, money);

	if(money > pUserInfo->m_fRemaining){
		printf("Not sufficient funds. Please charge :");		
		pUserInfo->m_fRemaining += getFloat((money -pUserInfo->m_fRemaining) ,MAX_CHARGE_MONEY);
	}
	setCarOut(pUserInfo);
	pUserInfo->m_fRemaining -= money;
	pUserInfo->m_u8isUsing = FALSE;

NoParkNoOut:
	printf("Press 'Entry' key to continue...");
	while(getchar() != '\n');
	handle->pfun = showParkingmenu;
	return;

}



 bool initEventHandle(EventLoopHandle_t *event);
 void freeEventHandle(EventLoopHandle_t *event);


int main(int argc, char *argv[])
{
	EventLoopHandle_t handle;//声明一个handle

	if(FALSE == initEventHandle(&handle)){//初始化失败则退出程序
		exit(-1);
	}
	if(FALSE == openDatabase(DEFAULT_DB_FILENAME)){//打开数据库并把数据库数据加载到内存
		goto ErrorProcess;//进入错误处理
	}

	while(NULL != handle.pfun){
		handle.pfun(&handle);//存储函数的地址
	}

	closeDatabase();
	freeEventHandle(&handle);
	
	return EXIT_SUCCESS;
ErrorProcess:
	fprintf(stderr, "%s\n", getDbErrorString());
	closeDatabase();
	freeEventHandle(&handle);

	return EXIT_FAILURE;
}

 bool initEventHandle(EventLoopHandle_t *event)
{
	if(NULL == event){
		return FALSE;
	}
	memset(event, 0x00, sizeof(*event));
	event->argData = malloc(EVENTBUFFERSIZE);
	if(NULL == event->argData){
		goto ErrorInitEventHandle;
	}
	event->retData = malloc(EVENTBUFFERSIZE);
	if(NULL == event->retData){
		goto ErrorInitEventHandle;
	}
	memset(event->argData, 0x00, EVENTBUFFERSIZE);
	memset(event->retData, 0x00, EVENTBUFFERSIZE);
	event->pfun = showSystemMenu;
	
	return TRUE;
ErrorInitEventHandle:
	freeEventHandle(event);
	return FALSE;
}
 void freeEventHandle(EventLoopHandle_t *event)
{
	if(NULL == event){
		return;
	}

	event->pfun = NULL;
	if(NULL != event->argData){
		free(event->argData);
		event->argData = NULL;
	}
	if(NULL != event->retData){
		free(event->retData);
		event->retData = NULL;
	}
}



