////////////////////////////////////////////////////////////////////////////
//  
//  本程序的文件名：OS实验参考程序2008.CPP
//
////////////////////////////////////////////////////////////////////////////
//
// 程序中假定：
//   FAT[K]存储FAT表，本程序中K设为5000，其中FAT[0]存放空盘块数；
//   Disk[K][R]用于存储对应磁盘块的内容(char型)，每个盘块最多存R(=64)个字符；
//   文件目录项FCB中，文件属性fattrib=1表示“只读”，=2表示“隐藏”，=4表示“系统”；
//   文件属性fattrib=16表示“子目录”而非文件，各属性可以互相组合。
//   用户打开文件表UOF中，状态state=0表示空登记栏，=1表示“建立”，=2表示“打开”状态；
//   UOF中“文件属性”即为被打开的文件的属性，对于“只读”文件，打开后只能读，不能写。
//
//   本系统对输入的命令，除文件名和目录名区分大小写外，其余部分字母都不区分大小写。
//
////////////////////////////////////////////////////////////////////////////
//
// 本模拟文件系统，包括如下操作命令：
// dir [<目录名>]——显示路径名指定的目录内容；
// cd [<目录名>]——指定当前目录。路径中“..”表示父目录；
// md <目录名>——创建子目录；
// rd <目录名>——删除子目录；
// create <文件名>[ <文件属性>]——创建文件；
// open <文件名>——打开文件；
// write <文件名> [<位置/app>[ insert]]——写文件；
// read <文件名> [<位置m> [<字节数n>]]——读文件；
// close <文件名>——关闭文件；
// ren <原文件名> <新文件名>——文件更名；
// copy <源文件名> [<目标文件名>]——复制文件；
// closeall——关闭当前用户的所有打开的文件
// del <文件名>——删除指定的文件
// type <文件名>——显示指定文件的内容；
// undel [<目录名>]——恢复指定目录中被删除的文件
// help——显示各命令的使用格式。
// attrib <文件名> [±<属性>]——显示[修改]文件/目录属性。
// rewind <文件名>——读、写指针移到文件开头(第一个字节处)
// fseek <文件名> <位置n>——将读、写指针都移到指定位置n处。
// block <文件名>——显示文件或目录占用的盘块号。
// uof——显示用户的UOF(文件打开表)。
// prompt——提示符中显示/不显示当前目录的切换。
// fat——显示模拟磁盘的空闲块数(FAT表中0的个数)。
// check——检查核对FAT表对应的空闲块数。
// exit——结束本程序的运行。
//
////////////////////////////////////////////////////////////////////////////

#include<iostream>	//cout,cin
#include<iomanip>	//setw(),setiosflags()
#include<cstdlib>		//exit(),atoi()
#include<string>		//strcpy(),_stricmp()
#include<fstream>	//文件操作用
#include<cstring>
using namespace std;
//定义若干符号常量
#define S 32			//假设最多同时打开32个文件 
#define K 5000			//假设磁盘共有5000个盘块
#define SIZE 64			//假设磁盘的大小是64字节
#define CK 8			//命令分解后的段数
#define INPUT_LEN 128	//输入缓冲区长度
#define COMMAND_LEN 11	//命令字符串长度
#define FILENAME_LEN 11	//文件名长度
#define PATH_LEN INPUT_LEN-COMMAND_LEN//输入的每个命令或者每个参数的长度 
#define DM 40			//恢复被删除文件表的表项数


struct FCB			//定义文件目录项FCB的结构(共16个字节)
{
	char FileName[FILENAME_LEN];//文件名(1～10字节)
	char Fattrib;				//文件属性
	short int Addr;				//文件首块号
	short int Fsize;			//文件长度
};

struct UOF			//定义用户打开文件表的结构
{
	char fname[PATH_LEN];		//文件全路径名
	char attr;					//文件属性，1=只可读；0=可读写
	short int faddr;			//文件的首块号
	short int fsize;			//文件大小(字节数)
	FCB *fp;					//该文件的目录项指针
	short int state;			//状态：0=空表项；1=新建；2=打开
	short int readp;			//读指针，指向某个要读的字符位置，0=空文件
	short int writep;			//写读指针，指向某个要写读的字符位置
};

struct CurPath		//定义存储当前目录的数据结构
{
	short int fblock;			//当前目录的首块号
	char cpath[PATH_LEN];		//当前目录绝对路径字符串(全路径名)
};

struct UnDel		//恢复被删除文件表的数据结构
{
	char gpath[PATH_LEN];		//被删除文件的全路径名(不含文件名)
	char ufname[FILENAME_LEN];	//被删除文件名
	short ufaddr;				//被删除文件名的首块号
	short fb;					//存储被删除文件块号的第一个块号(链表头指针)
								//首块号也存于fb所指的盘块中
};

//关于恢复被删除文件问题，还可以采用类似于Windows的回收站的方法。例如可以在根目录中
//建立一个特殊的文件夹recycled (其属性为：只读、隐藏、系统)，其FCB记录结构中的成员
//Fsize，不用来存储文件长度，而用来存储一个盘块号，该盘块中存储文件长度和文件的全路
//径名(不含文件名)，这里的“全路径名”就是文件的原位置，还原文件时该信息是不可或缺的。
//dir等命令处理recycled文件夹时，与普通文件夹略有不同(因其文件长度等信息要从Fsize号
//盘块中取出，不能直接获得)。rd命令应修改成不能删除文件夹recycled，copy,move,replace
//等命令也改成不能对文件夹recycled操作。

//当用del命令删除文件时,将该文件的有关信息保存到特殊的文件夹recycled中，亦即将文件“搬”
//到回收站，文件占用的磁盘空间并不释放；恢复时工作相反。清空回收站时才释放磁盘空间。
//此方案比前述UnDel结构的方案耗费更多的磁盘空间(删除的文件仍占用磁盘空间)

int FAT[K];						//FAT表,盘块数为K
//char (*Disk)[SIZE]=new char [K][SIZE];//定义磁盘空间，每个盘块容量为SIZE个字节
char Disk[K][SIZE];
UOF uof[S];						//用户打开文件表UOF,最多同时打开S个文件 
char comd[CK][PATH_LEN];		//分析命令时使用
char temppath[PATH_LEN];		//临时路径(全路径)
CurPath curpath;
UnDel udtab[DM];				//定义删除文件恢复表，退出系统时该表可存于文件UdTab.dat中
short Udelp=0;					//udtab表的第一个空表项的下标，系统初始化时它为0。
								//当Udelp=DM时，表示表已满，需清除最早的表项(后续表项依次前移)
short ffbp=1;
//0号盘快中存储如下内容：
//	short ffbp;		//从该位置开始查找空闲盘快(类似循环首次适应分配)
//	short Udelp;	//udtab表的第一个空表项的下标

int dspath=1;		//dspath=1,提示符中显示当前目录
char lastOperatefile[PATH_LEN];//这是用来记录当前操作文件的绝对名称的，适用于close关闭当前命令操作 

//函数原型说明
int CreateComd(int);			//create命令处理函数
int OpenComd(int);				//open命令处理函数
int ReadComd(int);				//read命令处理函数
int WriteComd(int);				//write命令处理函数
int CloseComd(int);				//close命令处理函数
void CloseallComd(int);			//closeaal命令处理函数, 关闭当前用户所有打开的文件
int DelComd(int);				//del命令处理函数
int UndelComd(int);				//undel命令处理函数，恢复被删除文件
int CopyComd(int);				//copy命令处理函数
int DirComd(int);				//dir命令处理函数，显示指定的文件目录——频繁使用
int CdComd(int);				//cd命令处理函数
int MdComd(int);				//md命令处理函数
int RdComd(int);				//rd命令处理函数
int TypeComd(int);				//type命令处理函数
int RenComd(int);				//ren命令处理函数
int AttribComd(int);			//attrib命令处理函数
void UofComd(void);				//uof命令处理函数
void HelpComd(void);			//help命令处理函数
int FindPath(char*,char,int,FCB* &);	//找指定目录(的首块号)
int FindFCB(char*,int,char,FCB* &);		//找指定的文件或目录
int FindBlankFCB(short s,FCB* &fcbp1);	//寻找首块号为s的目录中的空目录项
int RewindComd(int);			//rewind命令处理函数, 读、写指针移到文件开头(第一个字节处)
int FseekComd(int);				//fseek命令处理函数, 读、写指针移到文件第n个字节处
int blockf(int);				//block命令处理函数(显示文件占用的盘块号)
int delall(int);				//delall命令处理函数, 删除指定目录中的所有文件
void save_FAT(void);
void save_Disk(void);
int getblock(void);				//获得一个盘块
void FatComd(void);
void CheckComd(void);
int Check_UOF(char*);
void ExitComd(void);
bool IsName(char*);				//判断名字是否符合规则
void PromptComd(void);			//prompt命令，提示符是否显示当前目录的切换
void UdTabComd(void);			//udtab命令，显示udtab表内容
void releaseblock(short s);		//释放s开始的盘块链
int buffer_to_file(FCB* fcbp,char* Buffer);	//Buffer写入文件
int file_to_buffer(FCB* fcbp,char* Buffer);	//文件内容读到Buffer,返回文件长度
int ParseCommand(char *);		//将输入的命令行分解成命令和参数等
void ExecComd(int);				//执行命令
int FcComd(int);
int FcFunction(FCB*,FCB* );
int MoveComd(int);
int ReplaceComd(int);
//#define INIT	//决定初始化还是从磁盘读入
inline string change(char c)
{
     string data;
    for(int i=0;i<8;i++)
    {
        //  data+=c&(0x01<<i);
        if ( ( c >>(i-1) ) & 0x01 == 1 )
        {
            data+="1";
        }
        else
        {
            data+="0";
        }
    }
    for(int a=1;a<5;a++)
    {
        char x=data[a];
        data[a]=data[8-a];
        data[8-a]=x;
    }
   return data;
}

int main()
{
	
	char cmd[INPUT_LEN];			//命令行缓冲区，长度为 128
	int i,k;
	// 进入系统时，当前目录是根目录
	curpath.fblock=1;				//当前目录(根目录)的首块号为1 
	strcpy(curpath.cpath,"/");		//根目录的路径字符串

#ifdef INIT
	
	int j;
	FCB *fcbp;
	// *********** 初始化FAT和Disk ************
	for (i=0;i<K;i++)	//开始时所有盘块空闲
		FAT[i]=0;		//空闲盘块标记
	FAT[0]=K-1;			//FAT[0]中保存空闲盘块数
	for (i=1;i<30;i++)	//构造根目录盘块链
	{
		FAT[i]=i+1;		//初始化根目录的FAT表
		FAT[0]--;		//空盘块数减1
	}
	FAT[i]=-1;			//根目录尾标记
	FAT[0]--;			//空盘块数减1
	for (i++;i<40;i++)
	{
		FAT[i]=-1;		//各子目录尾标记
		FAT[0]--;
	}
	// *********** 初始化Disk ************
	fcbp=(FCB*) Disk[1];
	j=40*SIZE/sizeof(FCB);
	for (i=1;i<=j;i++)
	{
		fcbp->FileName[0]='\0';	//初始目录树各目录中初始化为空目录项
		fcbp++;
	}
	//以下建立初始目录树中各个子目录
	fcbp=(FCB*) Disk[1];
	strcpy(fcbp->FileName,"bin");	//子目录bin
	fcbp->Fattrib=16;				//表示是子目录
	fcbp->Addr=31;					//该子目录的首盘块号是31
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp++;							//指向下一个目录项
	strcpy(fcbp->FileName,"usr");	//子目录usr
	fcbp->Fattrib=16;				//表示是子目录
	fcbp->Addr=32;					//该子目录的首盘块号是32
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp++;
	strcpy(fcbp->FileName,"auto");	//文件unix的目录项
	fcbp->Fattrib=0;				//表示是普通文件
	fcbp->Addr=0;					//该子目录的首盘块号是0，表示是空文件
	fcbp->Fsize=0;					//该文件的长度为0
	fcbp++;
	strcpy(fcbp->FileName,"dev");	//子目录etc
	fcbp->Fattrib=16;				//表示是子目录
	fcbp->Addr=33;					//该子目录的首盘块号是33
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp=(FCB*) Disk[31];
	strcpy(fcbp->FileName,"..");	//bin的父目录对应的目录项
	fcbp->Fattrib=16;				//表示是目录而不是文件
	fcbp->Addr=1;					//父目录(此处是根目录)的首盘块号是1
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp=(FCB*) Disk[32];
	strcpy(fcbp->FileName,"..");	//usr的父目录对应的目录项
	fcbp->Fattrib=16;				//表示是目录而不是文件
	fcbp->Addr=1;					//父目录(此处是根目录)的首盘块号是1
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp++;
	strcpy(fcbp->FileName,"user");	//子目录lib
	fcbp->Fattrib=16;				//表示是子目录
	fcbp->Addr=34;					//该子目录的首盘块号是34
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp++;
	strcpy(fcbp->FileName,"lib");	//子目录user
	fcbp->Fattrib=16;				//表示是子目录
	fcbp->Addr=35;					//该子目录的首盘块号是35
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp++;
	strcpy(fcbp->FileName,"bin");	//子目录bin
	fcbp->Fattrib=16;				//表示是子目录
	fcbp->Addr=36;					//该子目录的首盘块号是36
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp=(FCB*) Disk[33];
	strcpy(fcbp->FileName,"..");	//etc的父目录对应的目录项
	fcbp->Fattrib=16;				//表示是目录而不是文件
	fcbp->Addr=1;					//父目录(此处是根目录)的首盘块号是1
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp=(FCB*) Disk[34];
	strcpy(fcbp->FileName,"..");	//lib的父目录对应的目录项
	fcbp->Fattrib=16;				//表示是目录而不是文件
	fcbp->Addr=32;					//父目录(此处是usr目录)的首盘块号是32
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp++;
	strcpy(fcbp->FileName,"lin");	//子目录liu
	fcbp->Fattrib=16;				//表示是子目录
	fcbp->Addr=37;					//该子目录的首盘块号是37
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp++;
	strcpy(fcbp->FileName,"sun");	//子目录sun
	fcbp->Fattrib=16;				//表示是子目录
	fcbp->Addr=38;					//该子目录的首盘块号是38
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp++;
	strcpy(fcbp->FileName,"ma");	//子目录fti
	fcbp->Fattrib=16;				//表示是子目录
	fcbp->Addr=39;					//该子目录的首盘块号是39
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp=(FCB*) Disk[35];
	strcpy(fcbp->FileName,"..");	//user的父目录对应的目录项
	fcbp->Fattrib=16;				//表示是目录而不是文件
	fcbp->Addr=32;					//父目录(此处是usr目录)的首盘块号是32
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp=(FCB*) Disk[36];
	strcpy(fcbp->FileName,"..");	//usr/bin的父目录对应的目录项
	fcbp->Fattrib=16;				//表示是目录而不是文件
	fcbp->Addr=32;					//父目录(此处是usr目录)的首盘块号是32
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp=(FCB*) Disk[37];
	strcpy(fcbp->FileName,"..");	//usr/lib/liu的父目录对应的目录项
	fcbp->Fattrib=16;				//表示是目录而不是文件
	fcbp->Addr=34;					//父目录(此处是usr/lib目录)的首盘块号是34
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp=(FCB*) Disk[38];
	strcpy(fcbp->FileName,"..");	//usr/lib/sun的父目录对应的目录项
	fcbp->Fattrib=16;				//表示是目录而不是文件
	fcbp->Addr=34;					//父目录(此处是usr/lib目录)的首盘块号是34
	fcbp->Fsize=0;					//约定子目录的长度为0
	fcbp=(FCB*) Disk[39];
	strcpy(fcbp->FileName,"..");	//usr/lib/fti的父目录对应的目录项
	fcbp->Fattrib=16;				//表示是目录而不是文件
	fcbp->Addr=34;					//父目录(此处是usr/lib目录)的首盘块号是34
	fcbp->Fsize=0;					//约定子目录的长度为0

	// *********** 初始化UnDel表 ************
	Udelp=0;

	ffbp=1;			//从FAT表开头查找空闲盘快

#else

	// 读入文件分配表FAT
	char yn;
	ifstream ffi("FAT2008.txt");//打开文件FAT2008.txt /////////////////////////////这边去掉了ios::nocreate参数 
	if (!ffi)
	{
		cout<<"Can't open FAT2008.txt!\n";
		cin>>yn;
		exit(0);
	}
	
	for (i=0;i<K;i++)		//从文件FAT2008.txt读入文件分配表FAT
		if(ffi)
			ffi>>FAT[i];
		else
			break;
	ffi.close();

	//读入磁盘块Disk[ ]信息
	ffi.open("Disk2008.dat",ios::binary);/////////////////////////////这边去掉了|ios::nocreate 
	if (!ffi)
	{
		cout<<"Can't open Disk2008.dat!\n";
		cin>>yn;
		exit(0);
	}
	for (i=0;i<K;i++)		//从文件Disk2008.dat读入盘块内容
		if(ffi)
			ffi.read((char*)&Disk[i],SIZE);
		else
			break;
	ffi.close();

	//读入恢复删除文件表UdTab.dat信息
	ffi.open("UdTab2008.dat",ios::binary);/////////////////////////////这边去掉了|ios::nocreate 
	if (!ffi)
	{
		cout<<"Can't open UdTab2008.dat!\n";
		cin>>yn;
		exit(0);
	}
	for (i=0;i<DM;i++)		//从文件Disk2008.dat读入盘块内容
		if(ffi)
			ffi.read((char*)&udtab[i],sizeof(udtab[0]));
		else
			break;
	ffi.close();

	short *pp=(short*) Disk[0];
	ffbp=pp[0];
	Udelp=pp[1];

#endif
 
	for (i=0;i<S;i++)		//初始化UOF。state：0＝空表项；1＝新建；2＝打开
		uof[i].state=0;		//初始化为空表项

	cout<<"\n现在你可以输入各种操作命令.\n"
		<<"Help —— 简易帮助信息.\n"
		<<"exit —— 退出本程序.\n";
	while (1)	//循环，等待用户输入命令，直到输入“exit”结束循环，程序结束
	{			//输入命令，分析并执行命令

		while (1)
		{
			cout<<"\nC:";					//显示提示符(本系统总假定是C盘)
			if (dspath)
				cout<<curpath.cpath;
			cout<<">";
			cin.getline(cmd,INPUT_LEN);		//输入命令
			if (strlen(cmd)>0)
				break;
		}
        k=ParseCommand(cmd);		//分解命令及其参数,k的值为输入参数的个数 
        //比如dir/usr，K的值为1，表示只有一个参数 
									//comd[0]中是命令，comd[1],comd[2]...是参数		
        ExecComd(k);				//执行命令
    }
    return 0;
}

/////////////////////////////////////////////////////////////////

void ExecComd(int k)		//执行命令
{
	int cid;				//命令标识

	//操作命令表 
	char CmdTab[][COMMAND_LEN]={"create","open","write","read","close",
		"del","dir","cd","md","rd","ren","copy","type","help","attrib",
		"uof","closeall","block","rewind","fseek","fat","check","exit",
		"undel","Prompt","udtab","fc","move","replace"};
	int M=sizeof(CmdTab)/COMMAND_LEN;	//统计命令个数
	for (cid=0;cid<M;cid++)			//在命令表中检索命令
		if (_stricmp(CmdTab[cid],comd[0])==0)//命令不区分大小写
			break;
	//以下命令函数中，命令参数通过全局变量comd[][]传递，故未出现在函数参数表中
	switch(cid)
	{
		 case 0:CreateComd(k);		//create命令，建立文件
				break;
		 case 1:OpenComd(k);		//open命令，打开文件 
				break;
		 case 2:WriteComd(k);		//write命令，k为命令中的参数个数(命令本身除外) 
				break;
		 case 3:ReadComd(k);		//read命令，读文件 
				break;
		 case 4:CloseComd(k);		//close命令，关闭文件 
				break;
		 case 5:DelComd(k);			//del命令，删除文件
				break;
		 case 6:DirComd(k);			//dir命令 
				break;
		 case 7:CdComd(k);			//cd命令 
				break;
		 case 8:MdComd(k);			//md命令 
				break;
		 case 9:RdComd(k);			//rd命令 
				break;
		 case 10:RenComd(k);		//ren命令，文件更名 
				 break;
		 case 11:CopyComd(k);		//copy命令，复制文件
				 break;
		 case 12:TypeComd(k);		//type命令，显示文件内容(块号) 
				 break;
		 case 13:HelpComd();		//help命令，帮助信息 
				 break;
		 case 14:AttribComd(k);		//attrib命令，修改文件属性 
				 break;
		 case 15:UofComd();			//uof命令，显示用户的UOF(文件打开表) 
				 break;
		 case 16:CloseallComd(1);	//closeall命令，关闭所有文件
				 break;
		 case 17:blockf(k);			//block命令，显示文件的盘块号
				 break;
		 case 18:RewindComd(k);		//rewind命令，将读指针移到文件开头 
				 break;
		 case 19:FseekComd(k);		//fseek命令：将读、写指针都移到指定记录号 
				 break;
		 case 20:FatComd();			//fat命令
				 break;
		 case 21:CheckComd();		//check命令
				 break;
		 case 22:ExitComd();		//exit命令
				 break;
		 case 23:UndelComd(k);		//undel命令
				 break;	
		 case 24:PromptComd();		//prompt命令
				 break;	
		 case 25:UdTabComd();		//udtab命令
				 break;
		 case 26:FcComd(k);
		         break;
		 case 27:MoveComd(k);
		         break;
		 case 28:ReplaceComd(k);
		         break;
		 default:cout<<"\n命令错:"<<comd[0]<<endl;
	}
}

void ParseHelpCommand(char* p){
	int cid;
	char HelpCmdTab[][COMMAND_LEN]={"1","2","3","4","5",
		"6","7","8","9","10","11","12","13","14","15",
		"16","17","18","19","20","21","22","23",
		"24","25","quit"};
		int M=sizeof(HelpCmdTab)/COMMAND_LEN;
		for (cid=0;cid<=M;cid++)			//在命令表中检索命令
		if (_stricmp(HelpCmdTab[cid],p)==0)//命令不区分大小写
			break;
 	switch(cid)
	{
		 case 0: cout<<"create <文件名>[ <文件属性>]\n命令中的“文件属性”规定了文件的使用权限，分为“只读”、“隐藏”、“系统”等，分别用'r'、'h'、's'的形式表示\ncreate命令中的“文件名”允许是绝对路径名或相对路径名，例如命令\ncreate /usr/user/boy,表示在根目录的usr子目录中的user子目录中建立文件boy，该文件为普通文件。\n又如create bin/mail rh,表示在当前目录中的bin子目录中建立文件mail，该文件为“只读”且“隐藏”属性的文件。"<<endl;		//create命令，建立文件
				break;
		 case 1: cout<<"open <文件名>\n打开文件。若指定文件存在且尚未打开，则打开之。其<文件名>可以是相对路径也可以是绝对路径\n例如:\nopen boy 表示将当前目录的boy文件打开。\nopen /usr/boy表示将usr目录的boy文件打开。"<<endl;		//open命令，打开文件 
				break;
		 case 2:cout<<"write <文件名> [<位置>[ insert]]\n命令中若无“位置”参数，则在写指针所指位置写入文件内容；若提供“位置”参数，则在对应位置写入内容。位置可以是整数n，是指在文件的第n个字节处开始写入(位置从1开始编号)。“位置”参数可以是“append”（前3个字符有效，不区分大小写），表示在文件尾部写入信息；参数“insert”（前3个字符有效，不区分大小写）表示新写入的内容插入到对应位置，对应位置开始的原内容后移。若无参数“insert”，写入内容代替文件原先的内容(改写方式)。写入内容中的“\\n”代表“回车”符。\n例如：\nwrite <文件名>，当前写指针位置开始写\nwrite <文件名> ins当前写指针位置开始插入\nwrite <文件名> app 表示在文件尾部开始写\nwrite <文件名> <n> 从当前n位置开始写\nwrite <文件名> <n> ins 从当前n位置开始插入"<<endl;		//write命令，k为命令中的参数个数(命令本身除外) 
				break;
		 case 3:cout<<"read <文件名> [<位置m> [<字节数n>]]\n命令中若无“位置”参数，则从读指针所指位置开始读。若有“位置”参数，则从指定位置处开始读。位置m是指从文件开头第m个字节处读（m从1开始编号）。若无“字节数n”参数，则从指定位置读到文件末尾；若有“字节数n”参数，则从指定位置开始读n个字节。其文件名可以是绝对地址也可以是相对地址\n例如\nread boy 11 表示读当前目录下boy文件，从第11个字节开始读，一直读到文件末尾。\nread boy 11 8 表示读当前目录下boy文件，从第11个字节开始读，读出8个字节的内容。"<<endl;	//read命令，读文件 
				break;
		 case 4:cout<<"close [文件名]\n关闭文件。若指定文件已打开，则关闭之。其文件名可以是绝对地址也可以是相对地址,其文件名参数可以省略，当无文件名参数的时候，则显示当前操作的文件内容，所谓“当前操作文件”，是指上一次使用create、open、write、read、rewind或fseek等命令操作过的文件。\n例如：\nclose boy 表示关闭当前目录下的boy文件。"<<endl;		//close命令，关闭文件 
				break;
		 case 5:cout<<"del <文件名>\n删除指定的文件，其文件名可以是绝对地址也可以是相对地址，若该文件是只读文件，经用户同意后才可以删除且该命令不可删除正在使用的文件。\n例如：\ndel boy 表示删除当前目录下未打开的boy文件"<<endl;			//del命令，删除文件
				break;
		 case 6:cout<<"dir [<目录名>] [<属性>]\n显示“目录名”指定的目录中文件名和第一级子目录名。若命令中无“属性”参数，则显示指定目录中“非隐藏”属性的全部文件名和第一级子目录名；若命令中有“属性”参数，则仅显示指定属性的文件名和目录名。属性参数的形式是“|属性符号”，其中属性符号有r、h和s三种（不区分大小写），分别表示“只读”、“隐藏”和“系统”三种属性，它们可以组合使用且次序不限。\n例如：\ndir |rh，表示要求显示同时具有“只读”和“隐藏”属性的文件和目录名。"<<endl;			//dir命令 
				break;
		 case 7:cout<<"cd [<目录名>]\n改变当前目录，即把指定目录变为当前目录（工作目录）。若命令中无目录名，则显示当前目录路径名。其目录名可以是相对路径也可以是绝对路径。\n例如：\ncd/ 表示切换到根目录中\ncd.. 表示切换到父目录中"<<endl;			//cd命令 
				break;
		 case 8:cout<<"md <目录名>[<属性>]\n用于在当前目录下创建子目录，若没有“属性”参数，则在当前目录下创建普通的子目录，属性包括R、H、S以及它们的组合(不区分大小写，顺序也不限)\n例如：md user rh\n其功能是在当前目录中创建具有“只读”和“隐藏”属性的子目录user。\nmd user\n其功能是在当前目录中创建普通子目录user"<<endl;			//md命令 
				break;
		 case 9:cout<<"rd <目录名>\n删除子目录。该命令不能删除非空的目录。若指定目录为空(仅有一个“..”目录项的目录为空目录)，则删除之。\n例如：\nrd user 表示删除当前目录下的子目录user"<<endl;			//rd命令 
				break;
		 case 10:cout<<"ren <原文件名> <新文件名>\n文件改名，即将指定文件的名字改为“新文件名”，新文件名要符合命名规则。\nren boy haohao 表示将boy文件改名成为haohao"<<endl;		//ren命令，文件更名 
				 break;
		 case 11:cout<<"copy <源文件名> <目标文件名>\n将源文件的内容复制到目标文件中。若目标文件与源文件所在的目录相同，则只能进行更名复制，此时目标文件名不能省；若目标文件与源文件所在的目录不同，则既可更名复制也可同名复制，同名复制时目标文件名可省。\n例如：\ncopy mail email\n(1) 若当前目录中不存在email(目录或文件)，则该命令将当前目录中的文件mail，复制成当前目录下的文件email;\n(2) 若当前目录下存在email，但email是子目录名，则将当前目录中的文件mail，复制到当前目录中的email子目录内，文件名为mail(同名复制)；此时若email目录内已经存在名字为mail的文件或目录，会出现重名错误；\n【注】在同一目录中，各目录项不能重名（不管是文件名还是子目录名）\n还可以使用如下命令：\ncopy boy / \ncopy boy ..\n使用如上命令时会将当前文件夹内的boy文件分别复制到根目录文件夹和父目录。"<<endl;		//copy命令，复制文件
				 break;
		 case 12:cout<<"type [文件名]显示指定文件的内容。文件名可以是绝对地址也可以是相对地址，当无文件名参数的时候，则显示当前操作的文件内容，所谓“当前操作文件”，是指上一次使用create、open、write、read、rewind或fseek等命令操作过的文件。\n例如：\ntype boy 表示将当前文件下boy文件的所有内容显示出来"<<endl;		//type命令，显示文件内容(块号) 
				 break;
		 case 13:cout<<"help \n帮助信息(显示各命令格式),当选择命令的编号时，会比较详细地介绍该命令，包括命令的各种形式，每种形式的功能，以及某些举例说明等等。"<<endl;		//help命令，帮助信息 
				 break;
		 case 14:cout<<"attrib <文件名> [±<属性>]\n若命令中无“文件属性”参数， 则显示指定文件的属性；若命令中有“文件属性”参数，则修改指定文件的属性。“文件属性”的形式有“+r或+h或+s”和“-r或-h或-s”两种形式，前者为设置指定文件为“只读”或“隐藏”或“系统”属性，后者为去掉指定文件的“只读”或“隐藏”或“系统”属性。各属性可组合使用且顺序不限。\n例如：\nattrib user/boy +r +h 其功能是设置当前目录下user子目录中的文件boy为只读、隐藏文件。\nattrib /usr/user/box -h -r -s\n上述命令的功能是取消文件/usr/user/box的“隐藏”、“只读”、“系统”属性。"<<endl;		//attrib命令，修改文件属性 
				 break;
		 case 15:cout<<"uof \n显示已打开文件表UOF的内容"<<endl;			//uof命令，显示用户的UOF(文件打开表) 
				 break;
		 case 16:cout<<"closeall\n关闭当前用户的所有文件"<<endl;	//closeall命令，关闭所有文件
				 break;
		 case 17:cout<<"block [文件名]\n显示文件或目录占用的盘块号，文件名可以是绝对地址也可以是相对地址，当无文件名参数的时候，则显示当前操作的文件内容，所谓“当前操作文件”，是指上一次使用create、open、write、read、rewind或fseek等命令操作过的文件。\n例如：\nblock boy 表示显示当前文件夹下名为boy文件或目录占用的盘块号"<<endl;			//block命令，显示文件的盘块号
				 break;
		 case 18:cout<<"rewind [文件名]\n将读、写指针移到文件开头， 文件名可以是绝对地址也可以是相对地址，当无文件名参数的时候，则显示当前操作的文件内容，所谓“当前操作文件”，是指上一次使用create、open、write、read、rewind或fseek等命令操作过的文件。\n例如：\nrewind boy 表示将当前当前文件夹下名为boy文件或目录的读、写指针移到文件开头"<<endl;		//rewind命令，将读指针移到文件开头 
				 break;
		 case 19:cout<<"fseek <文件名> |p<n> \n将读、写指针移到指定位置n处，文件名可以是绝对地址也可以是相对地址，当无文件名参数的时候，则显示当前操作的文件内容，所谓“当前操作文件”，是指上一次使用create、open、write、read、rewind或fseek等命令操作过的文件。\n例如：\nfseek boy |p11 表示将当前目录下的boy文件的读、写指针移到指定位置11处"<<endl;		//fseek命令：将读、写指针都移到指定记录号 
				 break;
		 case 20:cout<<"fat\n显示当前磁盘剩余的空闲盘块数"<<endl;			//fat命令
				 break;
		 case 21:cout<<"check\n让程序自动重新检查，显示剩余盘块数。"<<endl;		//check命令
				 break;
		 case 22:cout<<"exit\n退出程序命令。"<<endl;		//exit命令
				 break;
		 case 23:cout<<"undel [<目录名>]\n恢复指定目录中被删除的文件，若无目录名参数，则表示恢复当前目录中被删除的文件。"<<endl;		//undel命令
				 break;	
		 case 24:cout<<"prompt\n当使用此命令时，若提示符中已经显示了当前目录，则使用该命令之后会隐藏目录的切换。若提示符中的目录的已经被隐藏，使用此命令后提示符会显示目录。"<<endl;		//prompt命令
				 break;	
		 case 25:cout<<"udtab\n显示删除文件恢复表udtab的内容"<<endl;
		         break;
		 case 26:cout<<"fc <文件名1> <文件名2>\n例如：\nfc boy haohao 表示将当前文件夹下的boy文件和haohao文件相比较其文件内容"<<endl;
		         break;
		 case 27:cout<<"move <文件名> <目录名>\n将“文件名”指定的文件或目录，移动到“目标名”指定目录中(名字保持不变)。此命令也可改变子目录名。若转移的是一个文件，且目录名指定的目录原来已经存在，则将该文件转移到指定目录中；若指定目录中有同名文件，则询问是否覆盖，若同意则覆盖之。若转移的是一个子目录，而“目录名”指定的子目录不存在，则move命令执行子目录改名操作(必须是同一目录内)；若指定目录存在，则将“文件名”指定的目录转移到该目录中，但若指定目录中存在与“文件名”指定的目录同名的子目录，则报错。\n例如：\nmove lin /bin 表示当前目录中的子目录lin，转移到子目录/bin中，前提是lin是当前目录中的一个子目录，而根目录中已存在子目录bin，且/bin中无名字为lin的子目录。当同意覆盖时此命令可覆盖子目录/bin中的名字为lin的文件。\nmove /usr/boy bin 表示目录/usr中的文件boy，转移到当前目录下的bin子目录中，前提是boy是目录/usr中的一个文件，而当前目录中已存在子目录bin，且bin中无名字为boy子目录。若bin目录中已存在文件boy，则询问用户是否覆盖。"<<endl;
		         break;	
		 case 28:cout<<"replace <文件名> <目录名>\n以“文件名”指定的文件，取代“目录名”指定目录中的同名文件。若目录名参数缺省，则取代当前目录的同名文件；文件名指定的文件和被取代的文件不应该是同一个文件，即不能自己取代自己。\n【注】具有隐藏和系统属性的文件不能被取代。\n例如：\nreplace haohao /usr 表示取代/usr文件中与haohao同名的文件。"<<endl;
		         break;	         
		 default:cout<<"\n标号选择错误，请重新输入"<<endl;
	}
 }
//////////////////////////////////////////////////////////////////////////////////////

void HelpComd()				//help命令，帮助信息(显示各命令格式)
 {
 		char helpcmd[INPUT_LEN];
		 char quit[]="quit";			//命令行缓冲区，长度为 128
    cout<<"\n* * * * * * * * * 本系统主要的文件操作命令简述如下 * * * * * * * * * *\n\n";
    cout<<"1.create <文件名>[<文件属性>]　　　　　　  ——创建新文件,文件属性是r、h或s。\n";
    cout<<"2.open <文件名>                            ——打开文件，操作类型可为r、h或(与)s。\n";
	cout<<"3.write <文件名> [<位置>[ins][app]]          ——在指定位置写文件(有插入功能)。\n";
    cout<<"4.read <文件名> [<位置m> [<字节数n>]]      ——读文件，从第m字节处读n个字节。\n";
    cout<<"5.close <文件名>　　　　　　　　　　　　　 ——关闭文件。\n";
    cout<<"6.del <文件名>                             ——撤消(删除)文件。\n";
    cout<<"7.dir [<路径名>] [|<属性>]                 ——显示当前目录。\n";
	cout<<"8.cd [<路径名>]                            ——改变当前目录。\n";
	cout<<"9.md <路径名> [<属性>]                     ——创建指定目录。\n";
	cout<<"10.rd [<路径名>]                           ——删除指定目录。\n";
	cout<<"11.ren <旧文件名> <新文件名>               ——文件更名。\n";
	cout<<"12.attrib <文件名> [±<属性>]              ——修改文件属性(r、h、s)。\n";
	cout<<"13.copy <源文件名> [<目标文件名>]          ——复制文件。\n";
	cout<<"14.type <文件名>                           ——显示文件内容。\n";
	cout<<"15.rewind <文件名>                         ——将读、写指针移到文件第一个字符处。\n";
	cout<<"16.fseek <文件名> <位置>                   ——将读、写指针都移到指定位置。\n";
	cout<<"17.block <文件名>                          ——显示文件占用的盘块号。\n";
	cout<<"18.closeall                                ——关闭当前打开的所有文件。\n";
	cout<<"19.uof                                     ——显示UOF(用户打开文件表)。\n";
	cout<<"20.undel [<路径名>]                        ——恢复指定目录中被删除的文件。\n";
	cout<<"21.exit                                    ——退出本程序。\n";
	cout<<"22.prompt                                  ——提示符是否显示当前目录(切换)。\n";
	cout<<"23.fat                                     ——显示FAT表中空闲盘块数(0的个数)。\n";
	cout<<"24.check                                   ——核对后显示FAT表中空闲盘块数。\n";
	cout<<"\n请输入想要查询的命令的标号：（quit退出）"<<endl; 
	while (1)	//循环，等待用户输入命令，直到输入“exit”结束循环，程序结束
	{			//输入命令，分析并执行命令

		while (1)
		{				//显示提示符(本系统总假定是C盘)
			if (dspath)
			cout<<">>";
			cin.getline(helpcmd,INPUT_LEN);		//输入命令
			if (strlen(helpcmd)>0)
				break;
		}
		if(_stricmp(helpcmd,quit)==0){
        	break;
		} 
        ParseHelpCommand(helpcmd);		//分解命令及其参数,k的值为输入参数的个数 
        //比如dir/usr，K的值为1，表示只有一个参数 
									//comd[0]中是命令，comd[1],comd[2]...是参数		
        
    }
 }


/////////////////////////////////////////////////////////////////
//str是|h或者|hr,而h或者hr可以是大写,attrib是属性，通过判断str具体是什么，来给attrib加入不同的二进制位 
int GetAttrib(char* str,char& attrib)
{
	//char&是字符型引用，类似于重命名 
	int i,len;
	char ar='\01',ah='\02',as='\04';
	if (str[0]!='|')
	{
		cout<<"\n命令中属性参数错误。\n";
		return -1;
	}
	len=strlen(str);
//	cout<<"转换成小写之前："<<str<<endl; 
//	cout<<"str的长度"<<len<<endl;
	_strlwr(str);		//转换成小写字母
//	cout<<"转换成小写之后："<<str<<endl; 
	for (i=1;i<len;i++)
	{
		switch(str[i])
		{
			case 'r': attrib=attrib|ar;
					break;
			case 'h': attrib=attrib|ah;
					break;
			case 's': attrib=attrib|as;
					break;
			default: cout<<"\n命令中属性参数错误。\n";
					return -1;
		}
	}
	return 1;
}
//将md命令中的第二个属性参数进行处理 
int ProcessAttrib(char* str,char& attrib)
{
	int i,len;
	char ar='\01',ah='\02',as='\04';
//	if (str[0]!='|')
//	{
//		cout<<"\n命令中属性参数错误。\n";
//		return -1;
//	}
	len=strlen(str);
	_strlwr(str);		//转换成小写字母
//		cout<<"转换成小写之后："<<str<<endl; 
	for (i=0;i<len;i++)
	{
		switch(str[i])
		{
			case 'r': attrib=attrib|ar;
					break;
			case 'h': attrib=attrib|ah;
					break;
			case 's': attrib=attrib|as;
					break;
			default: cout<<"\n命令中属性参数错误。\n";
					return -1;
		}
	}
	return 1;
}

/////////////////////////////////////////////////////////////////

int DirComd(int k)	//dir命令，显示指定目录的内容（文件名或目录名等）
{
	// 命令形式：dir[ <目录名>[ <属性>]]
	// 命令功能：显示"目录名"指定的目录中文件名和第一级子目录名。若指
	// 定目录不存在，则给出错误信息。如果命令中没有指定目录名，则显示
	// 当前目录下的相应内容。若命令中无"属性"参数，则显示指定目录中"非
	// 隐藏"属性的全部文件名和第一级子目录名；若命令中有"属性"参数，则
	// 仅显示指定属性的文件名和目录名。h、r或s或两者都有，则显示隐藏属
	// 性或只读属性或既是隐藏又是只读属性的文件。属性参数的形式是"|<属
	// 性符号>"，其中属性符号有r、h和s三种（不区分大小写），分别表示"只
	// 读"、"隐藏"和"系统"三种属性,它们可以组合使用且次序不限。例如"|rh"
	// 和"|hr"都表示要求显示同时具有"只读"和"隐藏"属性的文件和目录名。显
	// 示文件名时，显示该文件长度；显示目录名时，同时显示"<DIR>"的字样。

	// 举例：
 	//		dir /usr |h	 
	// 上述命令显示根目录下usr子目录中全部"隐藏"属性的文件名和子目录名
 	//		dir ..		 
	// 上述命令显示当前目录的父目录中全部"非隐藏"属性的文件和子目录名(包
	// 括"只读"属性的也显示，但一般不显示"系统"属性的，因为"系统"属性的对
	// 象一般也是"隐藏"属性的)。
	//
	// 学生可考虑将此函数修改成命令中的路径的最后允许是文件名的情况。
	// 另外还可以考虑含通配符的问题。

	short i,s;
	short filecount,dircount,fsizecount;	//文件数、目录数、文件长度累计
	char ch,attrib='\0',attr,cc;
	FCB *fcbp,*p;//这是FCB的指针类型，用来指向文件的FCB 
	
	filecount=dircount=fsizecount=0;
	//cout<<"K的值为参数值验证其正确性："<<k<<endl; 
	if (k>2)	//命令中多于2个参数，错误(较复杂的处理应当允许有多个参数)
	{
		cout<<"\n命令错误：参数太多。\n";
		return -1;
	}
	if (k<1)	//命令无参数，显示当前目录
	{
//		cout<<"命令无参数，显示当前目录"<<endl;
		strcpy(temppath,curpath.cpath);//当前目录的绝对路径保存到temppath中 
//		cout<<"命令无参数的时候temppath的值为："<<temppath<<endl; 
		s=curpath.fblock;	//当前目录的首块号保存于s
	}
	else if (k==1)		//命令有1个参数(k=1)
	{
		if (comd[1][0]=='|')//说明还是当前目录，只不过要求显现带有输入参数的文件或者文件夹 
		{
//			cout<<"当前的参数是前："<<comd[1]<<endl; 
//			printf("%d\n",attrib);
			i=GetAttrib(comd[1],attrib);//这里attrib存的是comd[1]对应的属性值 
//			cout<<"当前的参数是前："<<endl; 
//			printf("%d\n",attrib);
			if (i<0) return i;
			strcpy(temppath,curpath.cpath);
			s=curpath.fblock;	//当前目录的首块号保存于s
		}
		else//说明dir的内容不是当前路径了，comd[1]是要dir的目录名 
		{
			//comd[1]可以是绝对路径，也可以是相对路径
			s=FindPath(comd[1],'\020',1,fcbp);	//找指定目录(的首块号)
			if (s<1)
			{
				cout<<"\n输入的路径错误！"<<endl;
				return -1;
			}
		}
	}
	else		//命令有2个参数(k=2)
	{
		s=FindPath(comd[1],'\020',1,fcbp);	//找指定目录(的首块号)
		if (s<1)
		{
			cout<<"\n输入的路径错误！"<<endl;
			return -1;
		}
		i=GetAttrib(comd[2],attrib);
		if (i<0) return i;
	}
	cout<<"\nThe Directory of C:"<<temppath<<endl;
	while (s>0)
	{
		p=(FCB*) Disk[s];	//p指向该目录的第一个盘块
		for (i=0;i<4;i++,p++)
		{
			ch=p->FileName[0];	//取文件(目录)名的第一个字符
//			cout<<"这是"<<ch<<endl;
			if (ch==(char)0xe5){
//				cout<<"这是空白目录项啊"<<endl;
			continue;
			}		//空目录项
				
			if (ch=='\0')		//已至目录尾部
				break;
			attr=p->Fattrib&'\07';	//不考虑文件还是目录,只考虑属性
			if (attrib==0)			//命令中没有指定属性
			{
				if (attr&'\02')		//不显示“隐藏”属性文件
					continue;
			}
			else
			{
				cc=attr & attrib;
				if (attrib!=cc)		//只显示指定属性的文件
					continue;
			}
			cout<<setiosflags(ios::left)<<setw(20)<<p->FileName;
			if (p->Fattrib>='\020')	//是子目录
			{
				cout<<"<DIR>\n";
				dircount++;
			}
			else
			{
				cout<<resetiosflags(ios::left);
				cout<<setiosflags(ios::right)<<setw(10)<<p->Fsize<<endl;
				filecount++;
				fsizecount+=p->Fsize;
			}
		}
		if (ch=='\0') break;
		s=FAT[s];		//指向该目录的下一个盘块
	}
	cout<<resetiosflags(ios::left)<<endl;
	cout<<setiosflags(ios::right)<<setw(6)<<filecount<<" file(s)";
	cout<<setw(8)<<fsizecount<<" bytes"<<endl;
	cout<<setw(6)<<dircount<<" dir(s) "<<setw(8)<<SIZE*FAT[0];
	cout<<" free"<<endl;
	return 1;
}

/////////////////////////////////////////////////////////////////

int CdComd(int k) 
{
	// 当前目录（工作目录）转移到指定目录下。指定目录不存在时，给出错误信息。
	// 若命令中无目录名，则显示当前目录路径。

	short i,s;
	char attrib=(char)16;
//	cout<<"attrib的值为："<<attrib<<endl;
	FCB* fcbp;
	if (k>1)	//命令中多于1个参数，错误
	{
		cout<<"\n命令错误：参数太多。\n";
		return -1;
	}
	if (k<1)	//命令无参数，显示当前目录
	{
		cout<<"\nThe Current Directory is C:"<<curpath.cpath<<endl;
		return 1;
	}
	else		//命令有一个参数，将指定目录作为当前目录
	{
		i=strlen(comd[1]);
		if (i>1 && comd[1][i-1]=='/')	//路径以"/"结尾，错误
		{
			cout<<"\n路径名错误！\n";
			return -1;
		}
//		cout<<"comd[]1"<<comd[1]<<endl;
		s=FindPath(comd[1],attrib,1,fcbp);	//找指定目录(的首块号)
//		cout<<"s的值为："<<s;
		if (s<1)
		{
			cout<<"\n路径名错误！"<<endl;
			return -1;
		}
		curpath.fblock=s;
//		cout<<"当前目录为"<<curpath.cpath<<endl;
//		cout<<"temppath为："<<temppath<<endl; 
		strcpy(curpath.cpath,temppath);
//		cout<<"当前目录为"<<curpath.cpath<<endl; 
		if (!dspath)
			cout<<"\n当前目录变为 C:"<<curpath.cpath<<endl;
		return 1;
	}
}

/////////////////////////////////////////////////////////////////

int M_NewDir(char *Name,FCB* p,short fs,char attrib)	//在p位置创建一新子目录
{
	//成功返回新子目录的首块号

	short i,b,kk;
	FCB *q;
	kk=SIZE/sizeof(FCB);
	b=getblock();		//新目录须分配一磁盘块用于存储目录项“..”
	if (b<0)
		return b;
	strcpy(p->FileName,Name);	//目录名
	p->Fattrib=attrib;			//目录项属性为目录而非文件
	p->Addr=b;					//该新目录的首块号
	p->Fsize=0;					//子目录的长度约定为0
	q=(FCB*) Disk[b];
	for (i=0;i<kk;i++,q++)
		q->FileName[0]='\0';	//置空目录项标志*/
	q=(FCB*) Disk[b];
	strcpy(q->FileName,"..");	//新目录中的第一个目录项名是“..”
	q->Fattrib=(char)16;		//目录项属性为目录而非文件
	q->Addr=fs;					//该目录的首块号是父目录的首块号
	q->Fsize=0;					//子目录的长度约定为0
	return b;					//成功创建，返回
}

/////////////////////////////////////////////////////////////////
//返回path所在目录的首块号，Name是path路径中最后一个文件名比如path为/usr，则Name为usr，
//k是参数的个数，与n配合使用，n为0的时候，参数任意。attrib返回的是目录的属性为，十进制为16 
//temppath的值是所在目录的绝对地址 
int ProcessPath(char* path,char* &Name,int k,int n,char attrib)
{
	// 将path中最后一个名字分离出来，并由引用参数Name带回；
	// 返回path中除掉Name后，最后一个目录的地址(首块号)；
	// 必要时调用函数FindPath()，并通过全局变量temppath返
	// 回path(去掉Name后)的全路径名(绝对路径名)


	short i,len,s;
	FCB* fcbp;

	if (n && k!=n)	//n=0,参数个数k任意,n>0,必须k=n
	{
		cout<<"\n命令参数个数错误！\n";
		return -1;
	}
	len=strlen(path);
	for (i=len-1;i>=0;i--)
		if (path[i]=='/')
			break;
	Name=&path[i+1];		//取路径中最后一个名字
//	cout<<"i的值为："<<i<<endl;
//	cout<<"path的值为："<<path<<endl;
//	cout<<"Name的值为："<<Name<<endl;
	if (i==-1)//说明path中没有"/" 
	{
		s=curpath.fblock;
		strcpy(temppath,curpath.cpath);
	}
	else
	{
		if (i==0)//说明path为"/文件名" ，所以当前目录s的值为1 
		{
			s=1;
			strcpy(temppath,"/");
		}
		else//说明path中有多个“/” 
		{
			path[i]='\0';//临时去除 
//			cout<<"path的值为："<<path<<endl;
			s=FindPath(path,attrib,1,fcbp); 
			if (s<1)
			{
				cout<<"\n路径名错误！\n";
				return -3;
			}
		}
	}
//	cout<<"tempPath的值为："<<temppath<<endl;
//	cout<<"attrib的值为：";
//	printf("%d\n",attrib); 
	return s;
}

/////////////////////////////////////////////////////////////////

int MdComd(int k)		//md命令处理函数
{
	// 命令形式：md <目录名>
	// 功能：在指定路径下创建指定目录，若没有指定路径，则在当前目录下创建指定目录。
	// 对于重名目录给出错误信息。目录与文件也不能重名。

	// 学生可以考虑命令中加“属性”参数，用于创建指定属性的子目录。命令形式如下：
	//		md <目录名>[ <属性>]
	// 属性包括R、H、S以及它们的组合(不区分大小写，顺序也不限)。例如：
	//		md user rh
	// 其功能是在当前目录中创建具有“只读”和“隐藏”属性的子目录user。

	short i,s,s0,kk;
	char attrib=(char)16,*DirName;//将它设置为目录 
	FCB *p;

	kk=SIZE/sizeof(FCB);

	if (k<1)
	{
		cout<<"\n错误：命令中没有目录名。\n";
		return -1;
	}
	if (k>2)
	{
		cout<<"\n错误：命令参数太多。\n";
		return -1;
	}
	//这里s返回的值为所建文件当前目录所在的首块号 
	//这里将DirName赋值为当前所要建立的文件名称 
	s=ProcessPath(comd[1],DirName,k,0,attrib);
//	cout<<"当前目录首块号："<<s<<endl; 
	if (s<0)
		return s;		//失败，返回
	if (!IsName(DirName))		//若名字不符合规则
	{
		cout<<"\n命令中的新目录名错误。\n";
		return -1;
	}
	i=FindFCB(DirName,s,'\040',p);//查新建子目录名是否存在，返回负数则证明不存在同名文件或子目录名 
	
	if (i>=0)
	{
		cout<<"\n错误：新目录名与文件或子目录重名！\n";
		return -1;
	}
	if (k==2)		//命令形式：md <目录名> |<属性符>
	{
//		cout<<"hahahah"<<endl;
//		cout<<comd[2]<<endl;
//		printf("%d\n",attrib);
		i=ProcessAttrib(comd[2],attrib);
		if (i<0)//返回负数说明命令出错了 
			return i;
	}
	s0=FindBlankFCB(s,p);//找空白目录项
//	cout<<"\ns的值为："<<s<<" "<<"s0的值为："<<s0<<endl;
	if (s0<0)			//磁盘满
		return s0;
	s0=M_NewDir(DirName,p,s,attrib);	//在p所指位置创建一新子目录项
//	cout<<"空白子目录首块号为："<<s0<<endl; 
//		printf("%d\n",attrib);
	if (s0<0)		//创建失败
	{
		cout<<"\n磁盘空间已满，创建目录失败。\n";
		return -1;
	}
	cout<<"\n创建新目录成功!"<<endl; 
	return 1;		//新目录创建成功，返回
}

/////////////////////////////////////////////////////////////////

int RdComd(int k)
{
	// 若指定目录为空，则删除之，否则，给出"非空目录不能删除"的提示。
	// 不能删除当前目录。

	short i,j,count=0,fs,s0,s;
	char attrib=(char)16,*DirName;
	FCB *p,*fcbp;
	fs=ProcessPath(comd[1],DirName,k,1,attrib);	//返回DirName的父目录的首块号
//	cout<<"\nfs的值为："<<fs<<endl;
	if (fs<0)
		return fs;				//失败，返回
	s0=s=FindFCB(DirName,fs,attrib,fcbp);//取DirName的首块号
	if (s<1)
	{
		cout<<"\n要删除的目录不存在。\n";
		return -1;
	}
	if (s==curpath.fblock)
	{
		cout<<"\n不能删除当前目录。\n";
		return 0;
	}
	while (s>0)		//循环查找，直到目录尾部
	{
		p=(FCB*) Disk[s];
		for (i=0;i<4;i++,p++)
		{
			if (p->FileName[0]!=(char)0xe5 && p->FileName[0]!='\0')//累计非空目录项
				count++;
		}
		//s0=s;			//记下上一个盘块号
		s=FAT[s];		//取下一个盘块号
	}
	if (count>1)
	{
		cout<<"\n目录"<<DirName<<"非空，不能删除。\n";
		return -1;
	}
	//s0=fcbp->Addr;		//取DirName的首块号
	while (s0>0)			//归还目录DirName所占的磁盘空间
	{
		s=FAT[s0];			//记下第s0块的后续块号		
		FAT[s0]=0;			//回收第s0块
		FAT[0]++;			//空闲盘块数增1
		s0=s;				//后续块号赋予s0
	}
	fcbp->FileName[0]=(char)0xe5;	//删除DirName的目录项
	if (strcmp(temppath,"/")==0)	//所删除的子目录在根目录
	{
		cout<<"\n删除目录成功！"<<endl;
		return 1;
	}
	//所删除的子目录DirName不在根目录时，对其父目录作以下处理
	s0=s=fs;				//取DirName父目录的首块号
	while (s>0)				//整理DirName的父目录空间(回收无目录项的盘块)
	{
		p=(FCB*) Disk[s];
		for (j=i=0;i<4;i++,p++)
			if (p->FileName[0]!=(char)0xe5 && p->FileName[0]!='\0')//累计非空目录项
				j++;
		if (j==0)
		{
			FAT[s0]=FAT[s];		//调整指针
			FAT[s]=0;			//回收s号盘块
			FAT[0]++;			//空闲盘块数增1
			s=FAT[s0];
		}
		else
		{
			s0=s;				//记下上一个盘块号
			s=FAT[s];			//s指向下一个盘块
		}
	}
	cout<<"\n删除目录成功！"<<endl;
	return 1;
}

/////////////////////////////////////////////////////////////////

int TypeComd(int k)		//type命令处理函数(显示文件内容)
{
	// 显示文件内容：type <文件名>，显示指定文件的内容。
	// 若指定文件不存在，则给出错误信息。

	short i,s,size,jj=0;
	char attrib='\0',*FileName;
	char *Buffer;
	char gFileName[PATH_LEN];	//存放文件全路径名
	FCB* fcbp;
    char nowOperatefile[PATH_LEN]; 

	if(k>1||k<0)
	{
		cout<<"\n命令参数个数错误！\n";
		return -1;
	}
	strcpy(nowOperatefile,lastOperatefile);
	if(k==0){
		s=ProcessPath(nowOperatefile,FileName,k,0,'\020');//取FileName所在目录的首块号
	}else if(k==1){
		s=ProcessPath(comd[1],FileName,k,0,'\020');//取FileName所在目录的首块号
	}
	if (s<1)			//路径错误
		return s;		//失败，返回
	s=FindFCB(FileName,s,attrib,fcbp);		//取FileName的首块号(查其存在性)
	strcpy(gFileName,temppath);
	i=strlen(temppath);
	if (temppath[i-1]!='/')
		strcat(gFileName,"/");
	strcat(gFileName,FileName);	//构造文件的全路径名
	if (s<0)
	{
		cout<<"\n文件"<<gFileName<<"不存在。\n";
		return -3;
	}
	if (s==0)
		cout<<"\n文件"<<gFileName<<"是空文件\n";
	else
	{
		size=fcbp->Fsize;
		Buffer=new char[size+1];		//分配动态内存空间
		while (s>0)
		{
			for (i=0;i<SIZE;i++,jj++)
			{
				if (jj==size)
					break;
				Buffer[jj]=Disk[s][i];
			}
			if (i<SIZE)
				break;
			s=FAT[s];
		}
		Buffer[jj]='\0';
		cout<<Buffer<<endl;
		delete [] Buffer;		//释放分配的动态内存空间
	}
	return 1;
}

/////////////////////////////////////////////////////////////////

int blockf(int k)	//block命令处理函数(显示文件或目录占用的盘块号)
{
	short s;
	char attrib='\040';		//32表示任意(文件或子目录)目录项都可以
	FCB* fcbp;

	if(k>1||k<0)
	{
		cout<<"\n命令中参数个数错误,只能有一个或者没有参数\n";
		return -1;
	}else if(k==0){
		s=FindPath(lastOperatefile,attrib,1,fcbp);
	}else if(k==1){
		s=FindPath(comd[1],attrib,1,fcbp);	//找指定目录(的首块号)
	}
//////如果s为0的话表示是空文件 
    if(s==0){
    	cout<<"\n"<<temppath<<"为空文件，不占用盘块号！"<<endl;
    	return -2;
	}else if (s<1)
	{
		cout<<"\n路径名错误！"<<endl;
		return -2;
	}
//	cout<<"\n"<<temppath<<"占用的盘块号为：";
	while (s>0)
	{
		cout<<s<<"  ";
		s=FAT[s];
	}
	cout<<endl;
	return 1;
}

/////////////////////////////////////////////////////////////////

void Put_UOF(char *gFileName,int i,short status,FCB* fcbp)
{
	strcpy(uof[i].fname,gFileName);	//复制文件全路径名
	uof[i].attr=fcbp->Fattrib;		//复制文件属性
	uof[i].faddr=fcbp->Addr;		//文件的首块号(0代表空文件)
	uof[i].fsize=fcbp->Fsize;
	uof[i].fp=fcbp;
	uof[i].state=status;					//打开状态
	if (fcbp->Fsize>0)				//若文件非空
		uof[i].readp=1;				//读指针指向文件开头
	else
		uof[i].readp=0;				//读指针指向空位置
	uof[i].writep=fcbp->Fsize+1;	//写指针指向文件末尾
}

/////////////////////////////////////////////////////////////////

int FindBlankFCB(short s,FCB* &fcbp1)	//寻找首块号为s的目录中的空目录项
{
	short i,s0;
	while (s>0)			//在首块号为s的目录找空登记栏，直到目录尾部
	{
		fcbp1=(FCB*) Disk[s];
		for (i=0;i<4;i++,fcbp1++)
			if (fcbp1->FileName[0]==(char)0xe5 || fcbp1->FileName[0]=='\0')
			{
				fcbp1->Addr=fcbp1->Fsize=0;		//假设为空目录项
				return 1;						//找到空目录项，成功返回
			}
		s0=s;		//记下上一个盘块号
		s=FAT[s];	//取下一个盘块号
	}
	if (strcmp(temppath,"/")==0)	//若是根目录
	{
		cout<<"\n根目录已满，不能再创建目录项。\n";
		return -1;
	}
	s=getblock();	//取一空闲盘快
	if (s<0)		//无空闲盘快
	{
		cout<<"\n磁盘空间已满，创建目录失败。\n";
		return -1;
	}
	FAT[s0]=s;		//构成FAT链
	fcbp1=(FCB*) Disk[s];
	for (i=0;i<4;i++,fcbp1++)
		fcbp1->FileName[0]='\0';	//置空目录标志
	fcbp1=(FCB*) Disk[s];
	fcbp1->Addr=fcbp1->Fsize=0;		//假设为空目录项
	return 1;
}

/////////////////////////////////////////////////////////////////
//create传入的参数为用户输入的参数个数，返回的结果暂时未知 
int CreateComd(int k)		//create命令处理函数：建立新文件 
{
	// 创建文件：create <文件名> [<文件属性>]，创建一个指定名字的新文件，
	// 即在目录中增加一目录项，不考虑文件的内容。对于重名文件给出错误信息。

	short i,i_uof,s0,s;
	char attrib='\0',*FileName;
	char gFileName[PATH_LEN];	//存放文件全路径名
	char ch,*p;
	FCB* fcbp1;
	if(k>2 || k<1)
	{
		cout<<"\n命令中参数个数不对。\n";
		return -1;
	}
//	cout<<"comd[1]的值为："<<comd[1]<<endl;
	//这里s返回的值为所建文件当前目录所在的首块号 
	//这里将FileName赋值为当前所要建立的文件名称 
	s=ProcessPath(comd[1],FileName,k,0,'\020');//取FileName所在目录的首块号
//	cout<<"S的值为："<<s<<endl; 
	if (s<1)			//路径错误
		return s;		//失败，返回
	if (!IsName(FileName))		//若名字不符合规则
	{
		cout<<"\n命令中的新文件名错误。\n";
		return -2;
	}
	//s0如果存在同名文件则返回正数 ，不存在则返回负数 
	s0=FindFCB(FileName,s,'\040',fcbp1);	//取FileName的首块号(查其存在性)
//	cout<<"FileNmae的首块号s0的值是："<<s0<<endl; 
	if (s0>=0)//这里有改动，将s0>0改成了>=0，这样就不能创建隐藏同名文件 
	{
		cout<<"\n有同名文件或目录，不能建立。\n";
		return -2;
	}
	strcpy(gFileName,temppath);
//	cout<<"copy完之后的gFileNmae"<<gFileName<<endl;
	i=strlen(temppath);
	if (temppath[i-1]!='/')
		strcat(gFileName,"/");
	strcat(gFileName,FileName);	//构造文件的全路径名
//	cout<<"gFileNmae现在的全路径名称"<<gFileName<<endl;
	if (k==2)
	{
		p=comd[2];
		while (*p!='\0')	//处理文件属性
		{
			ch=*p;
			ch=tolower(ch);
			switch(ch)
			{
				case 'r' : attrib=attrib | (char) 1;
					break;
				case 'h' : attrib=attrib | (char) 2;
					break;
				case 's' : attrib=attrib | (char) 4;
					break;
				default : cout<<"\n输入的文件属性错误。\n";
					return -3;
			}
			p++;
		}
	}
	for (i_uof=0;i_uof<S;i_uof++)			//在UOF中找空表项
		if (uof[i_uof].state==0)
			break;
	if (i_uof==S)
	{
		cout<<"\nUOF已满，不能创建文件。\n";
		return -4;
	}
	i=FindBlankFCB(s,fcbp1);			//寻找首块号为s的目录中的空目录项
	if (i<0)
	{
		cout<<"\n创建文件失败。\n";
		return i;
	}
	strcpy(fcbp1->FileName,FileName);	//目录项中保存文件名
	fcbp1->Fattrib=attrib;				//复制文件属性
	fcbp1->Addr=0;						//空文件首块号设为0
	fcbp1->Fsize=0;						//空文件长度为0
	Put_UOF(gFileName,i_uof,1,fcbp1);	//建立UOF登记项
	cout<<"\n文件"<<gFileName<<"建立成功\n";
	strcpy(lastOperatefile,gFileName);
//	cout<<"\n当前操作文件的全路径名为："<< lastOperatefile<<endl; 
//	cout<<"Attrib的值为：";
//	printf("%d\n",fcbp1->Fattrib); 
	return 1;							//文件创建成功，返回
}

/////////////////////////////////////////////////////////////////

int Check_UOF(char *Name)		//检查UOF中有无命令中指定的文件
{
	int i;
	for (i=0;i<S;i++)			//查用户打开文件表UOF
	{
//		cout<<"在Check_UOF的函数中："<<uof[i].fname<<endl;
		if (uof[i].state==0)	//空表项
			continue;
		if (strcmp(Name,uof[i].fname)==0)	//找到
			break;
	}
	return i;
}

/////////////////////////////////////////////////////////////////

int OpenComd(int k)			//open命令处理函数：打开文件 
{
	// 命令形式：open <文件名>
	// 若指定文件存在且尚未打开，则打开之，并在用户打开文件表（UOF）中登
	// 记该文件的有关信息。若指定文件已经打开，则显示"文件已打开"的信息；
	// 若指定文件不存在，则给出错误信息。只读文件打开后只能读不能写。

	short i,s0,s;
	char attrib='\0',*FileName;
	char gFileName[PATH_LEN];	//存放文件全路径名
	FCB* fcbp;

	s0=ProcessPath(comd[1],FileName,k,1,'\20');//取FileName所在目录的首块号
	if (s0<1)			//路径错误
		return s0;		//失败，返回
	s=FindFCB(FileName,s0,attrib,fcbp);		//取FileName的首块号(查其存在性)
//	cout<<"open命令的s表示的首块号为："<<s<<endl;
	if (s<0)
	{
		cout<<"\n要打开的文件不存在。\n";
		return -2;
	}
	strcpy(gFileName,temppath);
	i=strlen(temppath);
	if (temppath[i-1]!='/')
		strcat(gFileName,"/");
	strcat(gFileName,FileName);	//构造文件的全路径名
	i=Check_UOF(gFileName);		//查UOF
	if (i<S)					//该文件已在UOF中
	{
		cout<<"\n文件"<<gFileName<<"原先已经打开!\n";
		strcpy(lastOperatefile,gFileName);
		return -3;
	}
	for (i=0;i<S;i++)			//在UOF中找空表项
		if (uof[i].state==0)
			break;
//    cout<<"在UOF中找到的空表项为："<<i<<endl;
	if (i==S)
	{
		cout<<"\nUOF已满，不能打开文件。\n";
		return -4;
	}
	Put_UOF(gFileName,i,2,fcbp);
	cout<<"\n文件"<<gFileName<<"打开成功。\n";
	strcpy(lastOperatefile,uof[i].fname);
	return 1;
}

/////////////////////////////////////////////////////////////////

int getblock()	//获得一个空闲盘块，供fappend()函数调用
{
	short b;
	if (FAT[0]==0)	//FAT[0]中是磁盘空闲块数
		return -1;	//磁盘已满(已无空闲盘块)
	for(b=ffbp;b<K;b++)
		if (!FAT[b])
			break;
	if (b==K)
	{
		for (b=1;b<ffbp;b++)
			if (!FAT[b]) break;
	}
	ffbp=b+1;
	if (ffbp==K) ffbp=1;
	FAT[0]--;	//盘块数减1
	FAT[b]=-1;	//置盘块已分配标志(此处不妨假设其为文件尾)
	return b;	//返回取得的空闲盘块号
}

////////////////////////////////////////////////////////////////

int WriteComd(int k)		//write命令的处理函数
{
	// 写文件：write <文件名> [<位置>[ insert]]，命令中若无"位置"参数，则在写指
	// 针所指位置写入文件内容；若提供"位置"参数，则在对应位置写入内容。位置可以
	// 是整数n，是指在文件的第n个字节处开始写入(位置从1开始编号)。"位置" 还可以
	// 是 "append"（前3个字符有效，不区分大小写），表示在文件尾部写入信息；若有
	// 参数 "insert"（前3个字符有效，不区分大小写），则新写入的内容插入到对应位
	// 置，对应位置开始的原内容后移。若无参数 "insert" ，写入的内容代替文件原先
	// 的内容(对应位置的内容)。写入完毕调整文件长度和写指针值。
	// 若文件未打开或文件不存在，分别给出错误信息。

	// 可以有如下几种命令形式：
	//	write <文件名> ——在写指针当前所指位置写，写入内容代替原内容(代替方式或改写方式)
	//	write <文件名> <n>——在文件开头第n个字节处写，改写方式
	//	write <文件名> insert——在写指针所指位置写，写入处开始的原内容后移(插入方式)
	//	write <文件名> <n> insert——在文件开头第n个字节处写，插入方式
	//	write <文件名> append——在文件尾部写(添加方式)
	
	//【思考】如何使参数“insert”、“append”只要前3个字符对就可以，但多于3个字符也行。例如：
	// 对于“insert”，输入ins、inse、inser、insert(不区分大小写)都可以，输入其它不行。

	#define BSIZE 40*SIZE+1
	short int ii,ii_uof,len0,len,len1,pos,ins=0;
	short int bn0,bn1,jj,count=0;
	char attrib='\0',Buffer[BSIZE];		//为方便计，假设一次最多写入2560字节
	char *buf;
	FCB *fcbp;

	if (k<1)//命令中一定存在参数 
	{
		cout<<"\n命令中没有文件名。\n";
		return -1;
	}
	FindPath(comd[1],attrib,0,fcbp);	//构成全路径且去掉“..”存于temppath中
//	cout<<"findpath之后的temppath为："<<temppath<<endl;
	ii_uof=Check_UOF(temppath);			//查UOF
//	cout<<"查UOF之后的值为："<<ii_uof<<endl;
//	cout<<"查到的文件的状态为："<<uof[ii_uof].state;
//	cout<<"文件的属性值为："<<change(uof[ii_uof].attr)<<endl;
//	cout<<"这是if中的内容"<<(uof[ii_uof].attr&'\01')<<endl;
	if (ii_uof==S)
	{
		cout<<"\n文件"<<temppath<<"未打开或不存在，不能写文件。\n";
		return -2;
	}
	if (uof[ii_uof].attr&'\01' && uof[ii_uof].state!=1)
	{	//只读文件不是创建状态不能写
		cout<<"\n"<<temppath<<"是只读文件，不能写。\n";
		return -3;
	}
	if (k==1)
		pos=uof[ii_uof].writep;	//从写指针所指位置开始写(write <文件名>)
	else		//k=2或3
	{
		if (_strnicmp(comd[2],"app",3)==0)//不区分大小写地比较comd[2]和“app”前三位 
			pos=uof[ii_uof].fsize+1;	//文件尾部添加模式(write <文件名> append)
		else if (_strnicmp(comd[2],"ins",3)==0)
		{
			pos=uof[ii_uof].writep;	//从当前写指针位置开始写
			ins=1;					//插入模式(write <文件名> insert)
		}
		else
		{
			pos=atoi(comd[2]);		//从命令中指定位置写(write <文件名> <n>)
			if (pos<=0)
			{
				cout<<"\n命令中提供的写入位置错误。\n";
				return -4;
			}
			if (k==3)
			{
				if (_strnicmp(comd[3],"ins",3)==0)
					ins=1;			//插入模式(write <文件名> <n> insert)
				else
				{
					cout<<"\n命令参数"<<comd[2]<<","<<comd[3]<<"错误\n";
					return -5;
				}
			}
		}
	}
//	cout<<"这是pos的值："<<pos<<endl; 
//	cout<<"这是UFO的fsize值："<<uof[ii_uof].fsize<<endl;
//	cout<<"这是写指针的位置："<<uof[ii_uof].writep<<endl;
//	cout<<"这是读指针的位置："<<uof[ii_uof].readp<<endl;
	if (pos<=0)
	{
		cout<<"\n命令中提供的写入位置错误。\n";
		return -1;
	}
	if (pos>=uof[ii_uof].fsize+1)
	{
		pos=uof[ii_uof].fsize+1;
		ins=0;						//这种情况不会是插入方式
	}

	pos--;							//使pos从0开始

	cout<<"\n请输入写入文件的内容(最多允许输入"<<sizeof(Buffer)-1<<"个字节)：\n";
	cin.getline(Buffer,BSIZE);
	len1=strlen(Buffer);
	if (len1==0)			//输入长度为0,不改变文件
		return 0;
	fcbp=uof[ii_uof].fp;
	len0=uof[ii_uof].fsize;				//取文件原来的长度值
//	cout<<"len0的值为："<<len0<<endl;
	if (len0==0)						//若是空文件
	{
		ii=buffer_to_file(fcbp,Buffer);
//		cout<<"这是写入文件之后返回的值："<<ii<<endl; 
		if(ii==0)	//写文件失败
			return ii;
		uof[ii_uof].fsize=uof[ii_uof].fp->Fsize;
		uof[ii_uof].faddr=uof[ii_uof].fp->Addr;
		uof[ii_uof].readp=1;
		uof[ii_uof].writep=uof[ii_uof].fsize+1;
	////////////////////////////如果UFO的状态为1表示是刚刚创建的文件，这里要添加一个将UOF的状态设置为2，表示状态更改成为打开
//	cout<<"这是之前的状态："<<uof[ii_uof].state<<endl;
	if(uof[ii_uof].state==1){
		uof[ii_uof].state=2;
	} 
//	cout<<"这是之后的状态："<<uof[ii_uof].state<<endl;
	cout<<"\n写文件"<<uof[ii_uof].fname<<"成功.\n";
	strcpy(lastOperatefile,uof[ii_uof].fname);
//	cout<<"\n文件的首块号为："<<fcbp->Addr<<endl;
		return 1;
	}
	//以下处理文件非空的情况
	len=len1+pos+ins*(len0-pos);		//计算写入完成后文件的长度
	bn0=len0/SIZE+(short)(len0%SIZE>0);	//文件原来占用的盘块数
	bn1=len/SIZE+(short)(len%SIZE>0);		//写入后文件将占用的盘块数
	if (FAT[0]<bn1-bn0)
	{
		cout<<"\n磁盘空间不足,不能写入文件.\n";
		return -1;
	}
	buf=new char[len+1];
	if (buf==0)
	{
		cout<<"\n分配内存失败。\n";
		return -1;
	}
	file_to_buffer(fcbp,buf);		//文件读到buf
	if (ins)	//若是插入方式
	{
		for (ii=len0;ii>=pos;ii--)
			buf[ii+len1]=buf[ii];	//后移,空出后插入Buffer
		jj=pos;
		ii=0;
		while (Buffer[ii]!='\0')		//Buffer插入到buf
			buf[jj++]=Buffer[ii++];
	}
	else		//若是改写方式
		strcpy(&buf[pos],Buffer);
	buffer_to_file(fcbp,buf);
	delete [] buf;
	uof[ii_uof].fsize=uof[ii_uof].fp->Fsize;
	uof[ii_uof].writep=uof[ii_uof].fsize+1;
	cout<<"\n写文件"<<uof[ii_uof].fname<<"成功.\n";
	strcpy(lastOperatefile,uof[ii_uof].fname);
//	cout<<"\n文件的首块号为："<<fcbp->Addr<<endl;
	return 1;
}

////////////////////////////////////////////////////////////////

int CloseComd(int k)				//close命令的处理函数：关闭文件 
{
	// close <文件名>，若指定文件已打开，则关闭之，即从UOF中删除该文件
	// 对应的表项。若文件未打开或文件不存在，分别给出有关信息。

	int i_uof;
	char attrib='\0';
	FCB *p;
	if (k>1)
	{
		cout<<"\n命令参数个数过多，错误！\n";
		return -1;
	}
	if(k==1){
		FindPath(comd[1],attrib,0,p);	//构成全路径且去掉“..”存于temppath中
	i_uof=Check_UOF(temppath);		//查UOF
	}else if(k==0){
		FindPath(lastOperatefile,attrib,0,p);
        i_uof=Check_UOF(temppath);		//查UOF
	}
	if (i_uof==S)
		cout<<"\n文件"<<temppath<<"未打开或不存在，不能关闭。\n";
	else
	{
		uof[i_uof].state=0;			//在UOF中清除该文件登记栏
		p=uof[i_uof].fp;			//取该文件的目录项位置指针
		p->Addr=uof[i_uof].faddr;	//保存文件的首块号
		p->Fsize=uof[i_uof].fsize;	//保存文件的大小
		cout<<"\n关闭文件"<<temppath<<"成功。\n";
	}
	return 1;
}

/////////////////////////////////////////////////////////////////

void CloseallComd(int disp)    //closeall命令，关闭当前用户的所有文件
{
	int i_uof,j,k;
	FCB *p;
	for (k=i_uof=0;i_uof<S;i_uof++)
	{
		j=uof[i_uof].state;	//UOF中状态>0为有效登记项
		if (j>0)
		{
			k++;  //已打开文件计数
			uof[i_uof].state=0;			//在UOF中清除该文件登记栏
			p=uof[i_uof].fp;			//取该文件的目录项位置指针
			p->Addr=uof[i_uof].faddr;	//保存文件的首块号
			p->Fsize=uof[i_uof].fsize;	//保存文件的大小
			cout<<"\n文件"<<uof[i_uof].fname<<"已关闭.\n";
		}
	}
	if (!disp)
		return;
	if (k==0)
		cout<<"\n你没有打开文件，故无文件可关闭。\n\n";
	else
		cout<<"\n共关闭 "<<k<<" 个文件.\n\n";
}

/////////////////////////////////////////////////////////////////

short int SAVE_bn(short bb)
{
	// 在udtab中存储被删除文件的块号

	short i=0,b0,b,bs;
	if (bb==0)		//被删除文件是空文件
		return bb;
	bs=getblock();
	short *pb=(short*) Disk[bs];
	while (bb>0)
	{
		pb[i]=bb;
		bb=FAT[bb];
		i++;
		if (i==SIZE/2)
		{
			i=0;
			b0=b;
			b=getblock();
			FAT[b0]=b;
			pb=(short*) Disk[b];
		}
	}
	pb[i]=-1;
	return bs;
}

/////////////////////////////////////////////////////////////////

void Del1Ud(short a)
{
	// 在udtab表中删除一项，并前移后续表项

	short i,b,b0;
	b=udtab[a].fb;
	while (b>0)
	{	//回收存储文件块号的磁盘空间
		b0=b;
		b=FAT[b];
		FAT[b0]=0;
		FAT[0]++;
	}
	for (i=a;i<Udelp-1;i++)		//udtab表中表项前移一个位置
		udtab[i]=udtab[i+1];
	Udelp--;
}

/////////////////////////////////////////////////////////////////

int PutUdtab(FCB *fp)
{
	//在udtab中加入一表项

	short bb,bn,n,m,size;
	size=fp->Fsize;
	bn=size/SIZE+(size%SIZE>0)+1;	//文件的盘块号个数(含-1)
	n=SIZE/sizeof(short);			//每个盘块可存储的盘块号数
	m=bn/n+(short)(bn%n>0);			//共需m个盘块存储文件的块号
	if (Udelp==DM)
		Del1Ud(0);
	if (m>FAT[0])
	{
		cout<<"\n磁盘空间不足,不能保存删除恢复信息,该文件删除后将不能恢复.\n";
		return -1;
	}
	strcpy(udtab[Udelp].gpath,temppath);
	strcpy(udtab[Udelp].ufname,fp->FileName);
	bb=udtab[Udelp].ufaddr=fp->Addr;
	udtab[Udelp].fb=SAVE_bn(bb);	//保存被删除文件的盘块号
	Udelp++;						//调整指针位置
	return 1;
}

/////////////////////////////////////////////////////////////////

int DelComd(int k)			//del(删除文件)命令处理函数
{
	// 删除文件：del <文件名>，删除指定的文件，即清除其目录项和回收
	// 其所占用磁盘空间。对于只读文件，删除前应询问用户，得到同意后
	// 方能删除。当指定文件正在使用时，显示"文件正在使用，不能删除"
	// 的信息，当指定文件不存在时给出错误信息。
	// 删除文件时，将该文件的有关信息记录到删除文件恢复信息表udtab中，
	// 以备将来恢复时使用。

	short i,s0,s;
	char yn,attr;
	char attrib='\0',*FileName;
	char gFileName[PATH_LEN];	//存放文件全路径名
	FCB* fcbp;

	s0=ProcessPath(comd[1],FileName,k,1,'\20');//取FileName所在目录的首块号
	if (s0<1)			//路径错误
		return s0;		//失败，返回
	s=FindFCB(FileName,s0,attrib,fcbp);		//取FileName的首块号(查其存在性)
	if (s<0)
	{
		cout<<"\n要删除的文件不存在。\n";
		return -2;
	}
	strcpy(gFileName,temppath);
	i=strlen(temppath);
	if (temppath[i-1]!='/')
		strcat(gFileName,"/");
	strcat(gFileName,FileName);	//构造文件的全路径名
	i=Check_UOF(gFileName);		//查UOF
	if (i<S)					//该文件已在UOF中
	{
		cout<<"\n文件"<<gFileName<<"正在使用，不能删除!\n";
		return -3;
	}
	attr=fcbp->Fattrib & '\01';
	if (attr=='\01')
	{
		cout<<"\n文件"<<gFileName<<"是只读文件，你确定要删除它吗？(y/n) ";
		cin>>yn;
		if (yn!='Y' && yn!='y')
			return 0;		//不删除，返回
	}
	i=PutUdtab(fcbp);		//被删除文件的有关信息保存到udtab表中
	if (i<0)				//因磁盘空间不足，不能保存被删除文件的信息
	{
		cout<<"\n你是否仍要删除文件 "<<gFileName<<" ? (y/n) : ";
		cin>>yn;
		if (yn=='N' || yn=='n')
			return 0;				//不删除返回
	}
	fcbp->FileName[0]=(char) 0xe5;	//删除目录项
	while (s>0)						//回收磁盘空间
	{
		s0=s;//s0是所在目录的首块号 
		s=FAT[s];
		FAT[s0]=0;
		FAT[0]++;
	}
	cout<<"\n删除文件成功"<<endl;
	return 1;
}

/////////////////////////////////////////////////////////////////

int Udfile(FCB *fdp,short s0,char *fn,short &cc)
{
	// 在目录中找到被删除文件(文件名首字符为'\0xe5')的目录项后调用此函数
	// 本函数在udtab表中逐个查找，当找到与被删除文件的路径相同、名字(首字
	// 符除外)相同、首块号相同的表项时，显示“可能可以恢复字样”，询问用
	// 户得到肯定答复后，即开始恢复工作。恢复中若发现发生重名冲突时，由用
	// 户输入新文件名解决。恢复中若发现文件原先占用的盘块已作它用，则恢复
	// 失败。无论恢复成功与否，都将删除udtab中对应的表项。

	int i,j;
	char yn[11],Fname[INPUT_LEN];
	short *stp,b,b0,b1,s;
	FCB* fcbp;

	for (i=0;i<Udelp;i++)
	{
		if (strcmp(udtab[i].gpath,temppath)==0 && strcmp(&udtab[i].ufname[1],fn)==0  
			&& udtab[i].ufaddr==fdp->Addr)
		{
			cout<<"\n文件"<<udtab[i].ufname<<"可能可以恢复，是否恢复它？(y/n) ";
			cin.getline(yn,10);
			if (yn[0]=='y' || yn[0]=='Y')
			{
				if (udtab[i].ufaddr>0)
				{
					b=udtab[i].fb;			//取存储被删文件盘块号的第一个块号
					stp=(short*) Disk[b];	//stp指向该盘块
					b0=stp[0];				//取被删除文件的第一个块号到b0
					j=1;
					while (b0>0)
					{
						if (FAT[b0]!=0)		//若被删除文件的盘块已经不空闲
						{
							cout<<"\n文件"<<udtab[i].ufname<<"已不能恢复。\n";
							Del1Ud(i);		//删除udtab表中第i项(该表项已无用)
							return -1;
						}
						b0=stp[j++];		//取被删除文件的下一个块号到b0
						if (j==SIZE/2 && b0!=-1)
						{
							b=FAT[b];
							j=0;
							stp=(short*) Disk[b];
						}
					}
					b=udtab[i].fb;
					stp=(short*) Disk[b];
					b0=b1=stp[0];
					j=1;
					while (b1>0)
					{
						b1=stp[j];
						FAT[b0]=b1;
						FAT[0]--;
						b0=b1;
						j++;
						if (j==SIZE/2 && b1!=-1)
						{
							b=FAT[b];
							j=0;
							stp=(short*) Disk[b];
						}
					}
				}
				s=FindFCB(udtab[i].ufname,s0,'\0',fcbp);
				fdp->FileName[0]=udtab[i].ufname[0];	//恢复文件名
				if (s>=0)	//有重名文件
				{
					cout<<"\n该目录中已经存在名为"<<udtab[i].ufname<<"的文件，"
						<<"请为被恢复文件输入一个新的名字：";
					while (1)
					{
						cin.getline(Fname,INPUT_LEN);
						if (IsName(Fname))	//若输入的名字符合规则
						{
							s=FindFCB(Fname,s0,'\0',fcbp);	//查输入名字有否重名
							if (s>=0)
								cout<<"\n输入的文件名发生重名冲突。\n请重新输入文件名：";
							else
								break;			//输入名字合法且无重名文件存在。退出循环
						}
						else					//输入名字不符合命名规则
							cout<<"\n输入的文件名不合法。\n请重新输入文件名：";
					}
					strcpy(fdp->FileName,Fname);
				}
				cc++;		//被恢复文件数增1
				Del1Ud(i);	//删除udtab表中第i项
			}
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////

int UndelComd(int k)		//undel命令
{
	// 命令形式：undel [<目录名>]
	// 命令功能：恢复指定目录中被删除的文件
	// 具体有如下2种命令形式：
	//		undel——恢复当前目录中被删除的文件
	//		undel <目录名>——恢复指定目录中被删除的文件

	short i,s,s0,cc=0;		//cc是恢复文件计数变量
	char *fn;
	FCB *fcbp1;
	if (k>1)
	{
		cout<<"\n命令不能有参数。\n";
		return -1;
	}
	if (k<1)		//若命令中无参数
	{
		strcpy(temppath,curpath.cpath);
		s0=s=curpath.fblock;
	}
	else
	{
		s0=s=FindPath(comd[1],'\020',1,fcbp1);
		if (s<0)
		{
			cout<<"\n命令中所给的路径错误。\n";
			return -2;
		}
	}
	while (s>0)			//在首块号为s的目录找被删除文件的表项，直到目录尾部
	{
		fcbp1=(FCB*) Disk[s];
		for (i=0;i<4;i++,fcbp1++)
		{
			if (fcbp1->FileName[0]==(char)0xe5)		//找到可能进行删除恢复的目录项
			{
				fn=&(fcbp1->FileName[1]);
				Udfile(fcbp1,s0,fn,cc);
			}
		}
		s=FAT[s];	//取下一个盘块号
	}
	cout<<"\n共恢复了 "<<cc<<" 个被删除的文件。\n";
	return 1;
}

/////////////////////////////////////////////////////////////////

int ReadComd(int k)		//read命令的处理函数：读文件 
{
	// 读文件：read <文件名> [<位置m> [<字节数n>]，从已打开的文件读文件内容并显示。若无
	// “位置”参数，则从读指针所指位置开始读。若有"位置"参数，则从指定位置处开始读。位
	// 置m是指从文件开头第m个字节处读（m从1开始编号）。若无"字节数"参数，则从指定位置读
	// 到文件末尾；若有"字节数n"参数，则从指定位置开始读n个字节。每读一个字节，读指针后
	// 移一个字节。若文件未打开或文件不存在，分别给出错误信息。
	// read命令有如下几种形式：
	//		read <文件名>——从读指针开始读文件，一直读到文件末尾为止。
	//		read <文件名> <位置m>——从文件第m个字节开始，一直读到文件末尾为止。
	//		read <文件名> <位置m> <字节数n>>——从文件第m个字节开始，共读n个字节。
	// 说明：刚打开的文件，其读指针指向文件开头(即读指针等于1)，约定空文件的读指针等于0。

	short i,j,ii,i_uof,pos,offset;
	short b,b0,bnum,count=0,readc;
	char attrib='\0';
	char Buffer[SIZE+1];
	FCB* fcbp;

	if (k<1 || k>3)
	{
		cout<<"\n命令中参数个数太多或太少。\n";
		return -1;
	}
	FindPath(comd[1],attrib,0,fcbp);	//构成全路径且去掉“..”存于temppath中
	i_uof=Check_UOF(temppath);			//查UOF
	if (i_uof==S)
	{
		cout<<"\n文件"<<temppath<<"未打开或不存在，不能读文件。\n";
		return -2;
	}
	if (uof[i_uof].readp==0)
	{
		cout<<"\n文件"<<temppath<<"是空文件。\n";
		strcpy(lastOperatefile,uof[i_uof].fname);
		return 1;
	}
	if (k==1)				//参数个数k=1的情况(无参数m和n)
	{
		pos=uof[i_uof].readp;//从读指针所指位置开始读
		if (pos>uof[i_uof].fsize)
		{
			cout<<"\n读指针已指向文件尾部，无可读信息。\n";
			strcpy(lastOperatefile,uof[i_uof].fname);
			return 1;
		}
		readc=uof[i_uof].fsize-pos+1;	//读到文件尾部共需读readc个字节
	}
	else					//k=2或k=3的情况
	{
		pos=atoi(comd[2]);		//从命令中指定位置写
		if (pos<=0 || pos>uof[i_uof].fsize)
		{
			cout<<"\n命令中提供的读位置错误。\n";
			return -3;
		}
		readc=uof[i_uof].fsize-pos+1;	//读到文件尾部共需读readc个字节
		if (k==3)
		{
			readc=atoi(comd[3]);
			if (readc<1)
			{
				cout<<"\n命令中提供的读字节数错误。\n";
				return -4;
			}
			if (readc>uof[i_uof].fsize-pos+1)
				readc=uof[i_uof].fsize-pos+1;
		}
	}
	bnum=(pos-1)/SIZE;		//从文件的第bnum块读(bnum从0开始编号)
	offset=(pos-1)%SIZE;	//在第bnum块的偏移位置offset处开始读(offset从0开始)
	b=uof[i_uof].faddr;		//取文件首块号
//	cout<<"bnum的值为："<<bnum<<endl;
//	cout<<"文件的首块号为："<<b<<endl;
	for (i=0;i<bnum;i++)	//寻找读入的第一个盘块号
	{
		b0=b;
		b=FAT[b];
	}
//	cout<<"b的值为："<<b<<endl;
	ii=offset;
//	cout<<"ii的值为："<<ii<<endl;
//	cout<<"readc的值为："<<readc<<endl;
	while (count<readc)		//读文件至Buffer并显示之
	{
		for (i=ii,j=0;i<SIZE;i++,j++)
		{
			Buffer[j]=Disk[b][i];
		    
			count++;
			if (count==readc)
			{
				j++;
				break;
			}
		}
		Buffer[j]='\0';
		cout<<Buffer;
		ii=0;
		b=FAT[b];		//准备读下一个盘块
	}
//	cout<<"i的值为："<<i<<endl; 
	cout<<endl;
	uof[i_uof].readp=pos+readc;	//调整读指针
	strcpy(lastOperatefile,uof[i_uof].fname);
	return 1;
}

/////////////////////////////////////////////////////////////////

int CopyComd(int k)		//copy命令的处理函数：复制文件 
{
	// 复制文件：copy <源文件名> [<目标文件名>]
	// 命令功能：为目标文件建立目录项，分配新的盘块，并将源文件的内容复制到目标文件中
	// 和其他命令一样，这里的“文件名”，是指最后一个名字是文件的路径名。
	// 若目标文件与源文件所在的目录相同，则只能进行更名复制，此时目标文件名不能省；
	// 若目标文件与源文件所在的目录不同，则既可更名复制也可同名复制，同名复制时目标文件名可省。
	// 例如，命令
	//		copy mail email
	// (1) 若当前目录中不存在email(目录或文件)，则该命令将当前目录中的文件mail，复制成
	//     当前目录下的文件email;
	// (2) 若当前目录下存在email，但email是子目录名，则将当前目录中的文件mail，复制到当
	//     前目录中的email子目录内，文件名与源文件相同(同名复制)；此时若email目录内已经
	//     存在文件或目录mail，则出现重名错误；
	// (3) 若当前目录内存在email文件，则出现重名错误；
	// (4) 若当前目录内不存在源文件mail(或者虽然有mail，但它是子目录名)，则也报错。
	//【特例】命令中无目标文件时，将源文件同名复制到当前目录中。例如，当前目录为/usr
	//		copy /box
	// 则上述命令把根目录中的文件box复制到当前目录/usr中，文件名仍为box。

	//【注】在同一目录中，各目录项不能重名（不管是文件名还是子目录名）。

	// 学生还可考虑使用通配符的多文件同名复制的情况(目标文件与源文件所在目录必须不同)。

	short int i,size,s01,s02,s1,s2,s3,s22,b,c,b0,bnum,len;
	char *q;
	char cpath[PATH_LEN],tpath[PATH_LEN];
	char attrib='\0',*FileName1,*FileName2;
	char gFileName[PATH_LEN];	//存放文件全路径名
	char gFileName2[PATH_LEN];   //存放第二个文件的全路径名 
	FCB *fcbp,*fcbp1,*fcbp2;//fcbp1和fcbp2是各自文件的目录项 
	char yn; 
	bool flag=true;
	if (k<1 || k>2)
	{
		cout<<"\n命令中参数太多或太少。\n";
		return -1;
	}
	s01=ProcessPath(comd[1],FileName1,k,0,'\20');//取第一个参数（文件）所在目录的首块号，不能为空 
	if (s01<1)			//路径错误
		return s01;		//失败，返回
//	cout<<"s01是第一个参数（文件）所在目录的首块号："<<s01<<endl;
	s1=FindFCB(FileName1,s01,attrib,fcbp);	//取FileName(源文件)的首块号(查其存在性)
	if (s1<0)
	{
		cout<<"\n要复制的文件不存在。\n";
		return -1;
	}
	fcbp1=fcbp;			//记下源文件目录项指针值
	strcpy(gFileName,temppath);//temppath是目录的绝对路径 
	i=strlen(temppath);
	if (temppath[i-1]!='/')
		strcat(gFileName,"/");
	strcat(gFileName,FileName1);	//构造文件的全路径名
//	cout<<"\n第一个文件的完整名称为："<<gFileName;
	i=Check_UOF(gFileName);			//查UOF
	if (i<S)						//该文件已在UOF中
	{
		cout<<"\n文件"<<gFileName<<"已经打开，不能复制!\n";
		return -2;
	}	
	if (k==1)		//命令中无目标文件,同名复制到当前目录
	{
		s02=curpath.fblock;	//取当前目录的首块号
		FileName2=FileName1;
	}
	else	//k=2(命令中提供目标文件)的情况
	{
		
        if(strcmp(comd[2],"/")==0){//这边处理copy boy /的情况 
        	strcat(comd[2],FileName1);
		}else if(strcmp(comd[2],"..")==0){
//			cout<<"\n当前目录为："<<curpath.cpath;
            strcpy(cpath,curpath.cpath);
//            cout<<"\n当前目录2为："<<cpath;
			len=strlen(cpath);
			if(strcmp(cpath,"/")==0){
				cout<<"\n错误！根目录没有父目录";
				return -1;
			}
			q=&cpath[len-1];
			while (*q!='/') q--;
			*q='\0';
			if(*cpath=='\0'){//说明父目录是根目录 
				comd[2][0]='/';
				comd[2][1]='\0';
				strcat(comd[2],FileName1);
//				cout<<"\n现在的comd[2]为："<<comd[2];
			}else{//父目录不为根目录 
				strcat(cpath,"/");
				strcat(cpath,FileName1);
				strcpy(comd[2],cpath);
			}
		}
		s02=FindPath(comd[2],'\040',1,fcbp);//查看目录项是否存在 
		if(s02<1){
//			cout<<"temppath"<<temppath<<endl;
			strcpy(tpath,temppath);
//			cout<<"目标文件项不存在"<<endl;
			s02=ProcessPath(tpath,FileName2,k,0,'\20');
		}else {
		s02=FindPath(comd[2],'\020',1,fcbp);//查看第二个参数是否为目录 
		    FileName2=FileName1;
			if(s02<0){//目录二不是目录 则一定是文件 
//				s02=FindPath(comd[2],'\0',1,fcbp);
				s02=ProcessPath(comd[2],FileName2,k,0,'\20');
			}
		}
	}
//	cout<<"父目录所在首块号为："<<s02<<" "<<FileName2<<endl;
	if (!IsName(FileName2))		//若名字不符合规则
	{
		cout<<"\n命令中的目标文件名错误。\n";
		return -2;
	}
	s2=FindFCB(FileName2,s02,'\0',fcbp);	//取FileName2(目标子文件)的首块号(查其存在性)
//	cout<<"\ntemppath2的值为："<<temppath;
	if (s2>=0 && fcbp->Fattrib<='\07')	//存在同名目标文件 
	{	
	    cout<<"\n目标文件"<<fcbp->FileName<<"已存在，是否要覆盖该文件？(y/n) \n";
		cin>>yn;
		if (yn=='N' || yn=='n'){
			flag=false;
			cout<<"\n复制失败"<<endl;
			return -3;
		}
		if(flag){//覆盖其文件 
		short ss=s02,sd=s2;
		
		fcbp->FileName[0]=(char) 0xe5;	//删除目录项
        while (sd>0)						//回收磁盘空间
	    {
		    ss=sd;//ss是所在目录的首块号 
		    sd=FAT[sd];
	    	FAT[ss]=0;
	    	FAT[0]++;
	    }
//	cout<<"\n删除文件成功"<<endl;
		}
		
	}else{
		s3=FindFCB(FileName2,s02,'\020',fcbp);
		if(s3>=0){
			cout<<"错误！目录内存在同名子目录。"<<endl;
			return -1; 
		}
	} 
	s22=s02; 
//	if (s2<0)		//说明不存在同名文件 
//		s22=s02;
//	else			//说明存在同名文件，但它是目录名
//	{
//		s22=s02;
////		s22=s2;//s2的值是FileName2的首块号 
//		if (s2!=s01)		//目标子目录和源文件所在目录不同目录
//		{
////			cout<<"\n啊哈哈哈哈进入了"; 
//			b=FindFCB(FileName1,s2,'\020',fcbp);//需查FileName2目录中有没有文件FileName1
//			if (b>=0)
//			{
//				cout<<"\n错误！有同名目录名，不能复制。\n";
//				return -4;
//			}
//			c=FindFCB(FileName1,s2,'\0',fcbp);
//			if(c>=0){
//				cout<<"\n有同名文件"; 
//			}
//			FileName2=FileName1;	//缺省目标文件名，同名复制
//		}
//		else
//		{
//			cout<<"\n不能同目录同名复制。\n";
//			return -5;
//		}
//	}
//加下来开始找空的磁盘，开始复制 
	i=FindBlankFCB(s22,fcbp2);
	if (i<0)
	{
		cout<<"\n复制文件失败。\n";
		return i;
	}
	size=fcbp1->Fsize;		//源文件的长度
	bnum=size/SIZE+(short)(size%SIZE>0);	//计算源文件所占盘块数
	if (FAT[0]<bnum)
	{
		cout<<"\n磁盘空间已满，不能复制文件。\n";
		return -6;
	}
	*fcbp2=*fcbp1;						//源文件的目录项复制给目标文件
	strcpy(fcbp2->FileName,FileName2);	//写目标文件名
	b0=0;
	while (s1>0)		//开始复制文件内容
	{
		b=getblock();
		if (b0==0)
			fcbp2->Addr=b;		//目标文件的首块号
		else
			FAT[b0]=b;
		memcpy(Disk[b],Disk[s1],SIZE);	//复制盘块
		s1=FAT[s1];				//准备复制下一个盘块
		b0=b;
	}
	cout<<"\n复制文件成功！"<<endl; 
	return 1;					//文件复制成功，返回
}

/////////////////////////////////////////////////////////////////

int FseekComd(int k)	//fseek命令的处理函数 
{
	// 命令形式：fseek <文件名> <n>
	// 功能：将读、写指针移到指定位置n处

	int i_uof,n;
	char attrib='\0';
	char *pp;
	FCB* fcbp;

	if (k>2||k<1)
	{
		cout<<"\n出错啦，命令参数个数过多或过少。\n";
		return -1;
	}else if(k==2){
		if(comd[2][0]!='|'||comd[2][1]!='p'){
			cout<<"\n出错了，命令形式不正确！"<<endl;
			return -1;
		}
		pp=&comd[2][2];
	n=atoi(pp);
	FindPath(comd[1],attrib,0,fcbp);		//构成全路径且去掉“..”存于temppath中
	i_uof=Check_UOF(temppath);		//查UOF
	}else if(k==1){
		if(comd[1][0]!='|'||comd[1][1]!='p'){
			cout<<"\n出错了，命令形式不正确！"<<endl;
			return -1;
		}
		pp=&comd[1][2];
	n=atoi(pp);
	FindPath(lastOperatefile,attrib,0,fcbp);		//构成全路径且去掉“..”存于temppath中
	i_uof=Check_UOF(temppath);		//查UOF
	}

	if (i_uof==S)
	{
		cout<<"\n文件"<<temppath<<"未打开或不存在，不能操作。\n";
		return -2;					//操作失败返回
	}
	if (uof[i_uof].fsize==0)		//空文件
	{
		cout<<"\n"<<temppath<<"是空文件，不能进行此操作。\n";
		strcpy(lastOperatefile,uof[i_uof].fname);
		return -3;
	}
	if (n<=0 || n>uof[i_uof].fsize+1)
	{
		cout<<"\n位置参数错误。该参数必须在1和"<<uof[i_uof].fsize+1<<"之间。\n";
		return -4;
	}
	uof[i_uof].readp=n;				//读指针设定为n
	uof[i_uof].writep=n;			//写指针设定为n
	strcpy(lastOperatefile,uof[i_uof].fname);
	cout<<"\n修改成功！";
	return 1;						//修改成功，返回
}

/////////////////////////////////////////////////////////////////

int RenComd(int k)	//ren命令的处理函数：文件改名
{
	// 命令形式：ren <原文件名> <新文件名>
	// 若原文件不存在，给出错误信息。
	// 若原文件存在，但正在使用，也不能改名，同样显示出错信息。
	// 应检查新文件名是否符合命名规则。

	short i,s0,s;
	char attrib='\0',*FileName;
	char gFileName[PATH_LEN];	//存放文件全路径名
	FCB *fp,*fcbp;
	s0=ProcessPath(comd[1],FileName,k,2,'\20');//取FileName所在目录的首块号
	if (s0<1)			//路径错误
		return s0;		//失败，返回
	s=FindFCB(FileName,s0,attrib,fcbp);		//取FileName的首块号(查其存在性)
	if (s<0)
	{
		cout<<"\n要改名的文件不存在。\n";
		return -2;
	}
	strcpy(gFileName,temppath);
	i=strlen(temppath);
	if (temppath[i-1]!='/')
		strcat(gFileName,"/");
	strcat(gFileName,FileName);	//构造文件的全路径名
	i=Check_UOF(gFileName);		//查UOF
	if (i<S)					//该文件已在UOF中
	{
		cout<<"\n文件"<<gFileName<<"已经打开，不能改名!\n";
		return -3;
	}
	if (IsName(comd[2]))
	{
		fp=fcbp;						//保存指向要改名文件目录项的指针
		s=FindFCB(comd[2],s0,attrib,fcbp);	//查新文件名是否重名
		if (s<0)			//不重名
		{
			strcpy(fp->FileName,comd[2]);
			return 1;		//正确返回
		}
		cout<<"\n存在与新文件名同名的文件。\n";
		return -5;
	}
	cout<<"\n命令中提供的新文件名错误。\n";
	return -4;
}

/////////////////////////////////////////////////////////////////

int AttribComd(int k)	//attrib命令的处理函数：修改文件或目录属性 
{
//	 显示修改文件属性：attrib <文件名> [±<属性>]。若命令中无"文件属性"参数，
//	 则显示指定文件的属性；若命令中有"文件属性"参数，则修改指定文件的属性。"文
//	 件属性"的形式有“+r或+h或+s”和“-r或-h或-s”两种形式，前者为设置指定文件
//	 为"只读"或"隐藏"或"系统"属性，后者为去掉指定文件的"只读"或"隐藏"或"系统"
//	 属性。各属性可组合使用且顺序不限。例如：
//			attrib user/boy +r +h
//	 其功能是设置当前目录下user子目录中的文件boy为只读、隐藏文件。又如
//			attrib /usr/user/box -h -r -s
//	 上述命令的功能是取消文件/usr/user/box的"隐藏"、"只读"、"系统"属性。
	// 当命令中指定的文件已打开或不存在，给出错误信息；
	// 当命令中提供的参数错误，也显示出错信息。

	short i,j,i_uof,s;
	char Attrib,attrib='\40';
	char Attr[5],Attr1[4]="RHS";
	char attr[6][3]={"+r","+h","+s","-r","-h","-s"};
	char or_and[6]={'\01','\02','\04','\036','\035','\033'};
	FCB* fcbp;

	if (k<1)
	{
		cout<<"\n命令中没有指定文件名。\n";
		return -1;
	}
	s=FindPath(comd[1],attrib,1,fcbp);	//寻找指定的文件或目录并返回其首块号
	if (s<0)
	{
		cout<<'\n'<<temppath<<"文件或目录不存在。\n";
		return -2;
	}
	if (k==1)		//显示文件/目录的属性
	{
		Attrib=fcbp->Fattrib & '\07';
		if (Attrib=='\0')
			strcpy(Attr,"普通");
		else
		{
			for (i=0;i<3;i++)
			{
				if (Attrib & or_and[i])
					Attr[i]=Attr1[i];
				else
					Attr[i]=' ';
			}
			Attr[i]='\0';
		}
//		cout<<"\n"<<temppath<<"的属性是："<<Attr<<endl;
		return 1;
	}
	if (fcbp->Fattrib<='\07')		//若是文件，要查其是否已被打开
	{
		i_uof=Check_UOF(temppath);	//查UOF
		if (i_uof<S)
		{
			cout<<"\n文件"<<temppath<<"正打开着，不能修改属性。\n";
			return -3;
		}
	}
	for (i=2;i<=k;i++)		//处理属性参数
	{
		for (j=0;j<6;j++)
			if (_stricmp(comd[i],attr[j])==0)
			{
				if (j<3)
					fcbp->Fattrib=fcbp->Fattrib | or_and[j];
				else
					fcbp->Fattrib=fcbp->Fattrib & or_and[j];
				break;
			}
		if (j==6)
		{
			cout<<"\n命令中的属性参数错误。\n";
			return -4;
		}

	}
	return 1;	//修改属性完成，返回
}

/////////////////////////////////////////////////////////////////

int RewindComd(int k)	//rewind命令的处理函数：读、写指针移到文件开头 
{
	// 命令形式：rewind <文件名>
	// 对指定文件操作，同时该文件变为当前操作文件

	int i_uof;
	char attrib='\0';
	FCB* fcbp;

	if (k<0||k>1)
	{
		cout<<"\n命令参数个数错误。本命令只能有1个参数或无参数。\n";
		return -1;
	}else if(k==1){
		FindPath(comd[1],attrib,0,fcbp);		//构成全路径且去掉“..”存于temppath中
	    i_uof=Check_UOF(temppath);		//查UOF
	}else if(k==0){
		FindPath(lastOperatefile,attrib,0,fcbp);
		i_uof=Check_UOF(temppath);		//查UOF
	}

	if (i_uof==S)
	{
		cout<<"\n文件"<<temppath<<"未打开或不存在，不能操作。\n";
		return -1;					//操作失败返回
	}
	if (uof[i_uof].faddr>0)			//若是空文件
		uof[i_uof].readp=1;			//读指针设定为0
	else
		uof[i_uof].readp=0;			//非空文件的读指针设定为1
	uof[i_uof].writep=1;			//文件的写指针设定为1
	cout<<"\n成功将文件读写指针移到开头!";
	strcpy(lastOperatefile,uof[i_uof].fname);
	return 1;						// 修改成功，返回
}

/////////////////////////////////////////////////////////////////

void UofComd()	//uof命令，显示当前用户“打开文件表”
{
	//显示用户已打开文件表UOF的内容

	int i,k;
	char ch;
	for (k=i=0;i<S;i++)
		k+=uof[i].state;
	if (k>0)
	{
		cout<<"\n打开文件表UOF的内容如下:\n\n"
			<<"文件名                       文件属性  "
			<<"首块号  文件长度  状态  读指针  写指针\n";
		for (i=0;i<S;i++)
		{
			if (uof[i].state==0)
				continue;					//空目录项
//			cout.setf(std::ios::left);
//			cout.width(32);
//			cout<<uof[i].fname; 
			cout<<std::left<<setw(32)<<uof[i].fname;	//显示文件名 
			ch=uof[i].attr;
			switch(ch)
			{
			case '\0': cout<<"普通    ";
				break;
			case '\01': cout<<"R       ";
				break;
			case '\02': cout<<"H       ";
				break;
			case '\03': cout<<"RH      ";
				break;
			case '\04': cout<<"S       ";
				break;
			case '\05': cout<<"RS      ";
				break;
			case '\06': cout<<"HS      ";
				break;
			case '\07': cout<<"RHS     ";
				break;
			default: cout<<"错误    ";
			}
			cout<<setw(8)<<uof[i].faddr;	//首块号
			cout<<setw(8)<<uof[i].fsize;	//文件大小
			k=uof[i].state;
			if (k==1)
				cout<<" 建立   ";			//状态为“建立” 
			else
				cout<<" 打开   ";			//状态为“打开” 
			cout<<setw(8)<<uof[i].readp;
			cout<<uof[i].writep<<endl;		//读指针 
		}
	}
	else
		cout<<"目前尚无打开的文件。\n";
}
/*

  */
/////////////////////////////////////////////////////////////////

void save_FAT()	//保存文件分配表FAT到磁盘文件FAT.txt
{
	int i;
	ofstream ffo;
	ffo.open("FAT2008.txt");
	for (i=0;i<K;i++)
		ffo<<FAT[i]<<' ';
	ffo.close();
}

/////////////////////////////////////////////////////////////////

void save_Disk() //保存盘块中的文件内容
{
	int i;
	short *p=(short*) Disk[0];
	p[0]=ffbp;
	p[1]=Udelp;
	ofstream ffo("Disk2008.dat",ios::binary);
	for (i=0;i<K;i++)
		ffo.write((char*)&Disk[i],SIZE);
	ffo.close();
}

/////////////////////////////////////////////////////////////////

void save_UdTab()	//保存被删除文件信息表
{
	int i;
	ofstream ffo("UdTab2008.dat",ios::binary);
	for (i=0;i<DM;i++)
		ffo.write((char*)&udtab[i],sizeof(udtab[0]));
	ffo.close();
}

/////////////////////////////////////////////////////////////////

int FindFCB(char *Name,int s,char attrib,FCB* &fcbp)
{
	// 从第s块开始，查找名字为Name且符合属性attrib的文件或者目录 
	// 给定名字Name没有找到返回负数，找到返回非负数(找目录时返回恒正)
	// 函数正确返回时，引用参数指针变量fcbp指向Name目录项。

	int i;
	char ch,Attrib;
	while (s>0)
	{
		fcbp=(FCB*) Disk[s];
		for (i=0;i<4;i++,fcbp++)		//每个盘块4个目录项
		{
			ch=fcbp->FileName[0];
			if (ch==(char)0xe5)
				continue;
			if (ch=='\0')
				return -1;		//路径错误(至该目录尾部仍未找到)
			if (strcmp(Name,fcbp->FileName)==0)	//名字找到
			{
//				cout<<"attrib的值为：";
//				printf("%d\n",attrib); 
				if (attrib=='\040')		//attrib为32时，文件、子目录不限
					return fcbp->Addr;
				Attrib=fcbp->Fattrib;
//				cout<<"Attrib的值为：";
//				printf("%d\n",Attrib); 
				if (attrib=='\020' && Attrib>=attrib)	//子目录属性
					return fcbp->Addr;
				if (attrib=='\0' && Attrib<='\07')		//文件属性(找的是文件)
					return fcbp->Addr;
				return -1;			//名字符合但属性不对仍然没有找到
			}
		}
		s=FAT[s];		//取下一个盘块号
	}
	return -2;
}

/////////////////////////////////////////////////////////////////

//FindPath的结果是s为给定路径的的首块号 
int FindPath(char *pp,char attrib,int ffcb,FCB* &fcbp)
//pp为路径，pp可以是绝对路径，也可以是相对路径，attrib为属性， 
{
	// 查找命令中给定的路径，确定路径的正确性，并返回路径中最后一个
	// 名字(目录名)代表的目录的地址(首块号)；对路径进行处理（去掉路
	// 径中的“..”），构成一个全路径名存于temppath中；当函数参数ffcb
	// 非零时，通过调用FindFCB( )函数，使本函数成功返回时,FCB类型的
	// 引用参数指针变量fcbp指向路径最后一个目录的目录项。

	short i,j,len,s=0;
	char paths[60][FILENAME_LEN];	//分解路径用(路径中最多不超过60个名字)
	//这里paths存放用户输入的路径，比如dir /usr/bin ，paths[0]存放usr，paths[1]存放bin 
	char *q,Name[PATH_LEN];//这里Name表示存放的是最后一个目录名比如上一行的例子 ，此时Name存放的是bin 

	strcpy(temppath,"/");
	if (strcmp(pp,"/")==0)	//是根目录
		return 1;			//返回根目录的首块号
//		cout<<"这是pp的第一个内容："<<*pp<<endl;
//		cout<<"这是pp的全部内容："<<pp<<endl;
	if (*pp=='/')			//绝对路径，从根目录开始
	{
		s=1;				//根目录的首块号
		pp++;
//				cout<<"这是pp的第二个内容："<<*pp<<endl;
	}
	else
	{
		s=curpath.fblock;	//相对路径，从当前目录开始
		strcpy(temppath,curpath.cpath);
	}
	j=0;
//	cout<<"进行解析前的路径名为："<<pp<<endl;
	while (*pp!='\0')	//对命令中的路径分解
	{
		for (i=0;i<PATH_LEN;i++,pp++)
		{
			if (*pp!='/' && *pp!='\0'){
				Name[i]=*pp;
				
			}
				
			else
			{
				if (i>0)
				{
					Name[i]='\0';
					if (i>FILENAME_LEN-1)	//名字过长则截取前FILENAME_LEN-1个字符
						Name[FILENAME_LEN-1]='\0';
					strcpy(paths[j],Name);
//					cout<<paths[j]<<endl;
					j++;
				}
				else
					return -1;		//路径错误
				if (*pp=='/') pp++;
//				cout<<"到了该结束的时候了"<<endl;
				break;			//已处理到字符串尾部
			}
		}
	}
//	cout<<"这是Name的值："<<Name<<endl;
//	cout<<"这是paths的值"<<paths[0]<<endl;
//	cout<<"这是j的值："<<j<<endl;
//	cout<<"这是temppath的值："<<temppath<<endl;
	for (i=0;i<j;i++)
	{
		if (strcmp(paths[i],"..")==0)//处理dir ..的命令 
		{
			if (strcmp(temppath,"/")==0)
				return -1;		//路径错误(根目录无父目录)
			len=strlen(temppath);
			q=&temppath[len-1];
			while (*q!='/') q--;
			*q='\0';
			if (*temppath=='\0')
			{
				*temppath='/';
				temppath[1]='\0';
			}
			
		}
		else
		{
			if (strcmp(temppath,"/")!=0)
				strcat(temppath,"/");
			strcat(temppath,paths[i]);
			//这边将temppath变成完整的dir后面的目录名 比如dir /usr/bin,此时temppath的值为/usr/bin 
		}
		if (ffcb)
		{
//			cout<<"这里返回s的值前："<<s<<endl;
//			cout<<"paths[i]"<<paths[i]<<endl; 
//下面这一步很关键不然是不能准确找到文件或者目录的首块号的！！！ 
			if(i!=j-1){
				s=FindFCB(paths[i],s,'\020',fcbp);
			}else{
				s=FindFCB(paths[i],s,attrib,fcbp);
			}
			
//			cout<<"这里返回s的值后："<<s<<endl;
//			cout<<"temppath"<<temppath<<endl;
			if (s<0)
				return s;
		}
	}
	return s;
}
		
/////////////////////////////////////////////////////////////////

void FatComd()	//若输入"fat"
{
	cout<<"\n当前磁盘剩余空闲块数为"<<FAT[0]<<endl;
}

/////////////////////////////////////////////////////////////////

void CheckComd()		//check命令
{
	int i,j;
	cout<<"\n当前磁盘空闲是："<<FAT[0]<<endl;
	for ( j=0,i=2;i<K;i++)
		if (FAT[i]==0)
			j++;
	FAT[0]=j;
	cout<<"重新检查后，磁盘的空闲块是："<<FAT[0]<<endl;
	cout<<"\nffbp="<<ffbp<<endl;
	cout<<"Udelp="<<Udelp<<endl;
}

/////////////////////////////////////////////////////////////////

void ExitComd()		//exit命令处理
{
	char yn;
	CloseallComd(0);		//关闭所有打开的文件以防数据丢失
	cout<<"\n退出时FAT、Disk、Udtab是否要存盘？(y/n) ";
	cin>>yn;
	if (yn=='Y' || yn=='y')
	{
		save_FAT();			//FAT表存盘
		save_Disk();		//磁盘块中存储的内容
		save_UdTab();		//保存被删除文件信息表
	}
	//delete [] Disk;
	exit(0);
}

/////////////////////////////////////////////////////////////////

bool isunname(char ch)
{
	char cc[]="\"*+,/:;<=>?[\\]| ";
	for (int i=0;i<16;i++)
		if (ch==cc[i])
			return true;
	return false;
}

/////////////////////////////////////////////////////////////////

bool IsName(char* Name)
{
	// 判断名字是否符合如下规则：
	// (1) 名字长度不能超过FILENAME_LEN-1个字节，即10个字节。
	//     允许输入的名字超过10个字符，但只有前10个字符有效；
	// (2) 名字一般由字母（区分大小写）、数字、下划线等组成，名字允许是汉字；
	// (3) 名字不能包含以下16个字符之一：
	//		" * + , / : ; < = > ? [ \ ] | space(空格)
	// (4) 名字中允许包含字符“.”，但它不能是名字的第一个字符，故“.”、
	//    “.abc”、“..”和“..abc”等都是不合法的名字。

	int i,len,Len=FILENAME_LEN-1;
	bool yn=true;
	char ch;
	len=strlen(Name);
	if (len==0)
		return false;
	if (Name[0]=='.')		//名字第一个字符不能是字符'.'
		return false;
	if (len>Len)			//若名字过长，截去多余的尾部
	{
		Name[Len]='\0';
		len=Len;
	}
	for (i=0;i<len;i++)
	{
		ch=Name[i];
		if (isunname(ch))	//若名字中含有不合法符号
		{
			yn=false;
			break;
		}
	}
	if (!yn)
		cout<<"\n名字中不能包含字符'"<<ch<<"'。\n";
	return yn;
}

/////////////////////////////////////////////////////////////////

void PromptComd(void)			//prompt命令
{
	dspath=!dspath;
}

/////////////////////////////////////////////////////////////////

void UdTabComd(void)			//udtab命令
{
	//显示删除文件恢复表udtab的内容

	cout<<"\n恢复被删除文件信息表(UdTab)内容如下：\n\n";
	cout<<"文件路径名                      "<<"文件名        "
		<<"首块号      "<<"存储块号"<<endl;
	for (int i=0;i<Udelp;i++)
		cout<<setiosflags(ios::left)<<setw(32)<<udtab[i].gpath
		<<setw(15)<<udtab[i].ufname<<setw(12)<<udtab[i].ufaddr
		<<setw(8)<<udtab[i].fb<<endl;
}

/////////////////////////////////////////////////////////////////

int file_to_buffer(FCB* fcbp,char* Buffer)	//文件内容读到Buffer,返回文件长度
{
	//文件内容读到Buffer,返回文件长度

	short s,len,i,j=0;

	len=fcbp->Fsize;				//取文件长度
	s=fcbp->Addr;					//取文件首块号
	while (s>0)
	{
		for (i=0;i<SIZE;i++,j++)
		{
			if (j>=len)				//已读完该文件
				break;
			Buffer[j]=Disk[s][i];
		}
		s=FAT[s];					//取下一个盘块
	}
	Buffer[j]='\0';
	return len;						//返回文件长度
}

/////////////////////////////////////////////////////////////////

int buffer_to_file(FCB* fcbp,char* Buffer)	//Buffer写入文件
{
//	cout<<"buffer的值为："<<Buffer<<endl;
	//成功写入文件，返回1；写文件失败，返回0

	short bn1,bn2,i,j,s,s0,len,size,count=0;

	len=strlen(Buffer);	//取字符串Buffer长度
	s0=s=fcbp->Addr;		//取文件首块号
	if (len==0)
	{
		fcbp->Addr=fcbp->Fsize=0;	//文件变为空文件
		releaseblock(s);			//释放文件占用的磁盘空间
		return 1;
	}
	size=fcbp->Fsize;	//取文件长度
	bn1=len/SIZE+(short)(len%SIZE>0);		//Buffer若存盘占用的盘块数
	bn2=size/SIZE+(short)(size%SIZE>0);	//文件原先内容占用的盘块数
	if (FAT[0]<bn1-bn2)
	{
		cout<<"\n磁盘空间不足，不能将信息写入文件。\n";
		return 0;
	}
	if (s==0)				//若是空文件
	{
		s0=s=getblock();	//为其分配首个盘块
		fcbp->Addr=s0;		//记下首块号
	}
	j=0;
	while (j<len)		//Buffer写入FilName2
	{
		if (s<0)
		{
			s=getblock();
			FAT[s0]=s;
		}
		for (i=0;i<SIZE;i++,j++)
		{
			if (j==len)
				break;
			if (Buffer[j]=='\\' && Buffer[j+1]=='n')
			{
				Disk[s][i]='\n';
				j++;
				count++;
			}
			else{
				Disk[s][i]=Buffer[j];
//				cout<<"Buffer[j]的值为："<<change(Buffer[j])<<endl;
			}
				
		}
		s0=s;
		s=FAT[s];
	}
	if (s>0)
	{
		FAT[s0]=-1;			//目标文件结束盘块标记
		releaseblock(s);	//若FileName2仍有盘块未使用，应释放它们
	}
	fcbp->Fsize=len-count;		//改变文件的长度
	return 1;
}

/////////////////////////////////////////////////////////////////

void releaseblock(short s)	//回收磁盘空间
{	//释放s开始的盘块链
	short s0;
	while (s>0)				//循环操作，直到盘块链尾部
	{
		s0=s;				//s0记下当前块号		
		s=FAT[s];			//s指向下一个盘块
		FAT[s0]=0;			//释放盘块s0
		FAT[0]++;			//空闲盘块数增1
	}
}

/////////////////////////////////////////////////////////////////

int ParseCommand(char *p)	//将输入的命令行分解成命令和参数等
{
	int i,j,k,g=0;
	for (i=0;i<CK;i++)					//初始化comd[][] ck=8是分解之后的段数 
		comd[i][0]='\0';
	for (k=0;k<CK;k++)
	{	//分解命令及其参数，comd[0]中是命令，comd[1],comd[2]...是参数
		for (i=0;*p!='\0';i++,p++)
			if (*p!=' ')				//从输入命令的第一个字符开始，不为空格， 
				comd[k][i]=*p;		//取命令标识符
			else					//如果命令中为空格 
			{
				comd[k][i]='\0';
				if (strlen(comd[k])==0) k--;
				p++;
				break;
			}
		if (*p=='\0')
		{
			comd[k][i]=*p;
			break;
		}
	}
	///上面是根据空格来将输入的命令分了段 
	////////////这里来检查一下comd里面存的是什么东西
//	for(int g=0;comd[g][0]!='\0';g++){
//		cout<<comd[g]<<endl;
//	} 
//	cout<<"k的值是多少："<<k<<endl; 
	
	
	for (i=0;comd[0][i]!='\0';i++)
		if	(comd[0][i]=='/'||(comd[0][i]=='.'&&comd[0][i+1]=='.'))		//处理cd/，dir/usr等情况
			break;
	if (comd[0][i]!='\0')			//comd[0]中存在字符'/'
	{
		if (k>0)
		for (j=k;j>0;j--)
			strcpy(comd[j+1],comd[j]);	//后移
		strcpy(comd[1],&comd[0][i]);
		comd[0][i]='\0';
		k++;	//多出一个参数
	}
	return k;
}

/////////////////////////////////////////////////////////////////
int FcComd(int k){
	short int i,size,s01,s02,s1,s2,s22,b,b0,bnum;
	char attrib='\0',*FileName1,*FileName2;
	char gFileName[PATH_LEN];	//存放文件全路径名
	FCB *fcbp,*fcbp1,*fcbp2;
	if (k!=2)
	{
		cout<<"\n命令中参数太多或太少。\n";
		return -1;
	}
	s01=ProcessPath(comd[1],FileName1,k,0,'\20');//取FileName所在目录的首块号
//	cout<<"s01的值为："<<s01<<endl;
//	cout<<"fileName1的值为："<<FileName1<<endl;
	if (s01<1)			//路径错误
	{
		cout<<"\n路径错误！"<<endl;
		return s01;		//失败，返回
	}
	s1=FindFCB(FileName1,s01,attrib,fcbp);	//取FileName(源文件)的首块号(查其存在性)
//	cout<<"s1的值为："<<s1<<endl; 
	if (s1<0)
	{
		cout<<"\n要比较的第一个文件不存在。\n";
		return -1;
	}
	fcbp1=fcbp;			//记下源文件目录项指针值
	
	s02=ProcessPath(comd[2],FileName2,k,0,'\20');//取FileName2所在目录的首块号
//	cout<<"s02的值为："<<s02<<endl;
//	cout<<"fileName2的值为："<<FileName2<<endl;
	if (s02<1)			//路径错误
	{
		cout<<"\n路径错误！"<<endl;
		return s02;		//失败，返回
	}
	s2=FindFCB(FileName2,s02,attrib,fcbp);	//取FileName(源文件)的首块号(查其存在性)
//	cout<<"s2的值为："<<s1<<endl; 
	if (s2<0)
	{
		cout<<"\n要比较的第二个文件不存在。\n";
		return -1;
	}
	fcbp2=fcbp;			//记下源文件目录项指针值
	FcFunction(fcbp1,fcbp2);
}

int FcFunction(FCB* fcbp1,FCB* fcbp2){
	short s1,s2,smax;//文件1和文件2的大小，smax是比较大的那个 
	int addr1,addr2;
	int flag=0,tempmax,temp1,temp2;

	int difLocation;//用来记录字节位置不同的位置 
	int i;
    s1=fcbp1->Fsize;
    s2=fcbp2->Fsize;
    char buf1[s1+1];//用来存放出错位置之后的字节流 
	char buf2[s2+1]; 
    addr1=fcbp1->Addr;
    addr2=fcbp2->Addr;
//    cout<<"文件1的大小和首块号为："<<s1<<" "<<addr1<<endl;
//    cout<<"文件2的大小和首块号为："<<s2<<" "<<addr2<<endl;
    if(s1<=s2){
    	smax=s2;
	}else{
		smax=s1;
	}
	int time=smax/64+1;//总共有几个盘块 
//	cout<<"总共有"<<time<<"个盘块"<<endl;
//	cout<<"文件1下一个盘块为："<<FAT[addr1]<<endl;
//	cout<<"文件2下一个盘块为："<<FAT[addr2]<<endl;
	int ii1=0,ii2=0;
	for(int j=0;j<time;j++){
		if((smax-SIZE*j)<SIZE){//记录一个盘块中文件1和文件2能取到的最大值 
			tempmax=smax-SIZE*j;
		}else{
			tempmax=SIZE;
		}
		if((s1-SIZE*j)<SIZE){//记录文件1能取到值的位置 
			temp1=s1-SIZE*j;
		}else{
			temp1=SIZE;
		}
		if((s2-SIZE*j)<SIZE){//记录文件2能取到值的位置 
			temp2=s2-SIZE*j;
		}else{
			temp2=SIZE;
		}
//		cout<<"temp1的值为："<<temp1<<endl;
//		cout<<"temp2的值为："<<temp2<<endl; 
		for(int i=0;i<tempmax;i++){
			if(i<temp1&&i<temp2){//两个disk都不为空，都能取到字节的时候
			    if(flag==0){
			    if(Disk[addr1][i]!=Disk[addr2][i]){//文件内容不同 
                difLocation=SIZE*j+i+1; 
				flag=1;
				cout<<"\n文件内容不同！位置为第"<<difLocation<<"个字节处"<<endl;
			    }
				} 
			    if(flag==1){
			    buf1[ii1]=Disk[addr1][i];//这个时候就要记录两个不同位置之后的所有内容了 
			    buf2[ii2]=Disk[addr2][i];
//			    cout<<"i"<<i<<endl;
//			    cout<<"buf1的值为："<<change(buf1[ii1])<<"ii1的值为："<<ii1<<endl;
//			    cout<<"buf2的值为："<<change(buf2[ii2])<<"ii2的值为："<<ii2<<endl;
			    ii1++;
			    ii2++;
			    
				}
			}else if(i>=temp1){//disk1取不到了
//			    cout<<"disk1取不到了"<<i<<" "<<endl;
                if(flag==1){
                buf2[ii2]=Disk[addr2][i];
				ii2++;
//				cout<<"buf2的值为："<<change(buf2[ii2])<<endl;
				}else{
				flag=1; //两个文件内容不同
				difLocation=SIZE*j+i+1;  
				cout<<"\n文件内容不同！位置为第"<<difLocation<<"个字节处"<<endl;
				buf2[ii2]=Disk[addr2][i];
				ii2++;
//				cout<<"buf2的值为："<<change(buf2[ii2])<<endl;
				}

			}else if(i>=temp2){//disk2取不到了 
//			cout<<"disk2取不到了"<<i<<" "<<endl;
                if(flag==1){
                buf1[ii1]=Disk[addr1][i];
				ii1++;
//				cout<<"buf1的值为："<<change(buf1[ii1])<<endl;
				}else{
				flag=1;
			    difLocation=SIZE*j+i+1;  
				cout<<"\n文件内容不同！位置为第"<<difLocation<<"个字节处"<<endl;
				buf1[ii1]=Disk[addr1][i];
				ii1++;
//				cout<<"buf1的值为："<<change(buf1[ii1])<<endl;
				}

			}	
	    }
		addr1=FAT[addr1];
		addr2=FAT[addr2];
		if(addr1==-1||addr2==-1){//文件结束 
			if(flag==0){
				cout<<"\n文件内容相同!"<<endl;
				break;
			}else if(flag==1){
				break;
			}
		}
	}
	buf1[ii1]='\0';
	buf2[ii2]='\0'; 
	cout<<"\n第一个文件不同内容开始之后的所有值为：\n"<<buf1<<endl;
	cout<<"第二个文件不同内容开始之后的所有值为：\n"<<buf2<<endl;

//    	for (i=0;i<SIZE;i++,j++)
//		{
//			if (j==len)
//				break;
//			if (Buffer[j]=='\\' && Buffer[j+1]=='n')
//			{
//				Disk[s][i]='\n';
//				j++;
//				count++;
//			}
//			else
//				Disk[s][i]=Buffer[j];
//		}
}



int MoveComd(int k){
	short s,s0,count=0,i,j;
	FCB *p,*fcbp,*p11,*fcbp0;
	if(k!=2){
		cout<<"命令参数错误，必须要有两个参数！"<<endl;
		return -1;
	}
	//首选需要知道第一个参数是文件还是目录
	short s1,i1,s2,i0,s21,i00;
	short temps,fs; 
	char *pp;
	char attrib=(char)16,*DirName,*DirName1,yn;
	char tPath1[PATH_LEN],tPath3[PATH_LEN],temp22path[PATH_LEN],tPath4[PATH_LEN];//temp22path是参数二的父目录
	char temp1path[PATH_LEN],temp2path[PATH_LEN];//用来保存参数一和参数二的完整绝对路径 
	char *tPath,*tPath2;
	bool isFile=false;//如果第一个参数为文件名，则会设置为true; 
	FCB* fcbp1,* fcbp2;//这是第一和第二个参数的fcbp 
	FCB* Bfcbp1;//空的地方的fcbp 
	char *DirName11,*DirName22;//第一个参数的完整绝对路径的最后一个文件名,第二个参数的完整绝对路径的最后一个目录名 
	short fs1,fs2;
	i0=strlen(comd[1]);
	i1=strlen(comd[2]);
	if (i0>1 && comd[1][i0-1]=='/')	//路径以"/"结尾，错误
	{
		cout<<"\n路径名错误！\n";
		return -1;
	}
	if (i1>1 && comd[2][i1-1]=='/')	//路径以"/"结尾，错误
	{
		cout<<"\n路径名错误！\n";
		return -1;
	}
	//(文件或目录简称目录项) 
	s1=FindPath(comd[1],'\020',1,fcbp1);	//找第一个参数的(的首块号)
	//findPath只能验证是否存在，不能真正找到首块号 ，现在来看这句话不正确 
	//无论comd[1]是相对路径还是绝对路径 
		if (s1<1)//第一个参数名一定不是目录 
		{
			temps=FindPath(comd[1],'\040',1,fcbp1);
			if(temps<0){
				cout<<"\n第一个参数的文件或目录名不存在";
				return -1;
			}
			// 第一个参数名一定是文件名
			isFile=true;
		}
		strcpy(temp1path,temppath);//将第一个参数的全路径名保存在temp1path中 
		strcpy(tPath4,temppath);//过渡 
		fs1=ProcessPath(tPath4,DirName11,k,0,attrib);//参数一所在父目录的首块号 
		
	s2=FindPath(comd[2],attrib,1,fcbp2);	//找第二个参数的指定目录(的首块号)
		strcpy(temp2path,temppath);
		if (s2<1) //说明指定的目录名不存在
		{
			
			if(!isFile){//这时根据第一个参数是否为文件，若不为文件，则进行改名操作，若是文件，则报错 
			pp=comd[2];
			while(*pp!='\0'){
				if(*pp=='/'){
					cout<<"\n错误，不允许"<<comd[2]<<"名字中包含非法字符！"<<endl;
					return -1;
					break; 
				}
				pp++;
			}
			strcpy(fcbp1->FileName,comd[2]);
			}else{
				cout<<"\n出错了，指定目录"<<comd[2]<<"不存在！"<<endl;
				return -1;
			} 
			cout<<"\n成功将"<<DirName11<<"更名为"<<comd[2]<<endl;
			
			
		}else
		{//说明第二个参数指定的目录已经存在 
		///////////////////////////////////////////////////////// 
			if(isFile){//说明将文件转移到已存在的目录中 
			
			//接下来验证参数一的正确性，即此文件不能正在被使用 
//            cout<<"\ntemp1path的值为："<< temp1path<<endl;//参数一的全路径 
            i00=Check_UOF(temp1path);		//查UOF
            if (i00<S){					//该文件已在UOF中
	            cout<<"\n文件"<<temp1path<<"正在使用，不能删除并转移!\n";
	            return -3;
            }
            
            //接下来将转移的文件拷贝到目标目录中，首先要查看目标目录中是否存在该文件
			//若存在则询问用户，若用户说覆盖，则将原文件删除，若不存在则拷贝到该目录下，然后删除原文件 
//            fs1=ProcessPath(temp1path,DirName11,k,0,attrib);//参数一所在目录的首块号 
            //接下来验证参数二的正确性，即参数二中不能存在该文件
//            cout<<"DirName11的值为："<<DirName11<<endl; 
//            cout<<"temp2path的值为："<<temp2path<<endl;
			    if(strcmp(temp2path,"/")==0){
			    strcpy(tPath1,temp2path);
			    strcpy(tPath3,temp2path);//tPath3数组为后面的tPath2做准备 
			    tPath=tPath1;
			    tPath2=tPath3;
			    strcat(tPath,DirName11);
			    strcat(tPath2,DirName11);
				}else{
				strcpy(tPath1,temp2path);
				strcpy(tPath3,temp2path);//tPath3数组为后面的tPath2做准备 
			    tPath=tPath1;
			    tPath2=tPath3;
			    strcat(tPath,"/");
			    strcat(tPath2,"/");
			    strcat(tPath,DirName11);
			    strcat(tPath2,DirName11);
				}
//				short s11=ProcessPath(tPath,DirName1,k,0,attrib);
             short pp1=ProcessPath(tPath2,DirName1,k,0,attrib);//tPath2是第二个参数（目标文件 
//             cout<<"tPath2（目录）的值为："<<tPath2<<endl;
//			 cout<<"tPath（完整路径）的值为："<<tPath<<endl; 
//             cout<<"参数二所在的目录首块号为："<<pp1<<endl;
//             cout<<"DirNmae1的值为："<<DirName1<<endl;
//			 cout<< "DirNmae11的值为："<<DirName11<<endl;
			 short i44=FindFCB(DirName11,pp1,'\020',p11);//查新文件名是否存在，返回负数则证明不存在同名子目录名 
//			cout<<"i44第一次的值为："<<i44<<endl; 
			if(i44>=0){
			cout<<"\n错误：新文件名与子目录重名！\n";
		    return -1;
			}
			 i44=FindFCB(DirName11,pp1,'\0',p11);//查新建子目录名是否存在，返回负数则证明不存在同名子目录名 
//			cout<<"i44的值为："<<i44<<endl;
//			cout<<"p11的filename为："<<p11->FileName<<endl;
//			cout<<"pp1为第二个参数目录的首块数"<<endl;
			short sss=pp1;
			if(i44>=0){
				cout<<"\n新文件名与子文件名"<<tPath<<"重名！是否要覆盖子文件？(y/n) \n";
	        	cin>>yn;
	        	if (yn!='N' && yn!='n'){//即删除子文件 
	        		short	i33=Check_UOF(tPath);		//查UOF
	        			if (i33<S)					//该文件已在UOF中
	                {
		              cout<<"\n文件"<<tPath<<"正在使用，不能删除并转移!移动文件失败！！\n";
		              return -3;
	                }
	                //删除旧的重名文件 
	                p11->FileName[0]=(char) 0xe5;	//删除目录项
	                while (i44>0)						//回收磁盘空间
	                {//i11是文件 
		                sss=i44;
	                	i44=FAT[i44];
	                	FAT[sss]=0;
	                	FAT[0]++;
	                }
//                    	cout<<"\n原文件删除文件成功"<<endl;
//	               fcbp2->FileName[0]=(char) 0xe5;	//删除目录项
//		         	while (temps>0){						//回收磁盘空间
//		        	fs1=temps;//s0是所在目录的首块号 
//                    temps=FAT[temps];
//	            	FAT[fs1]=0;
//	            	FAT[0]++;
//	                }
//	                cout<<"\n旧的文件删除成功"<<endl;
	    
                    //接下来开始拷贝新文件 并且删除原文件 
	        		
				}else{
					cout<<"\n文件移动失败"<<endl;
					return -1;
				} 
			}
	//先复制原文件，然后再删除原文件 
//			fs1=ProcessPath(tPath4,DirName11,k,0,attrib);//参数一所在父目录的首块号 
//			cout<<"fs1的值为////////"<<fs1<<endl; 
//			cout<<"DirNme11的值为："<<DirName11<<endl;
//            s01=ProcessPath(comd[1],FileName1,k,0,'\20');//取FileName所在目录的首块号
	        s1=FindFCB(DirName11,fs1,'\0',fcbp1);	//取FileName(源文件)的首块号(查其存在性)
	        if (s1<0)
	        {
		      cout<<"\n要转移的文件不存在。\n";
		      return -1;
	       }
	       fcbp0=fcbp1;			//记下源文件目录项指针值
//	       cout<<"s1(原文件的首块号)为："<<s1<<endl;
//	cout<<"\n第一个文件的名称为："<<temp1path<<endl;
//	cout<<"目标文件完整路径为:"<<tPath<<endl; 
//    cout<<"目标文件目录的首块号"<<pp1<<endl;
    short h1=pp1;//pp1是第二个参数的目录的首块号为1 
    	short h2=FindBlankFCB(h1,Bfcbp1);
    		if (h2<0)
	{
		cout<<"\n转移文件失败。\n";
		return h2;
	}
	short	size=fcbp0->Fsize;		//源文件的长度
//	cout<<"目标文件的长度为："<<size<<endl;
	short	bnum=size/SIZE+(short)(size%SIZE>0);	//计算源文件所占盘块数
	    if (FAT[0]<bnum)
	{
		cout<<"\n磁盘空间已满，不能创建目录并转移文件。\n";
		return -6;
	}
	*Bfcbp1=*fcbp0;
//	cout<<"原文件的目录项的首块号为："<<fcbp0->Addr<<endl; 
//	cout<<"s1的大小为："<<s1<<endl;
	  strcpy(Bfcbp1->FileName,DirName11);	//写目标文件名
	  short	b0=0;
	  short b;
	  short ss1=s1;
	while (ss1>0)		//开始复制文件内容
	{
		b=getblock();
		if (b0==0)
			Bfcbp1->Addr=b;		//目标文件的首块号
		else
			FAT[b0]=b;
		memcpy(Disk[b],Disk[ss1],SIZE);	//复制盘块
		ss1=FAT[ss1];				//准备复制下一个盘块
		b0=b;
	}
//	cout<<"\n复制文件成功！"<<endl; 
//	cout<<"原文件目录的首块号"<<fs1<<endl;
//	cout<<"原文件首块号："<<s1<<endl;
	ss1=s1;
		fcbp1->FileName[0]=(char) 0xe5;	//删除目录项
	while (ss1>0)						//回收磁盘空间
	{
		fs1=ss1;//s0是所在目录的首块号 
		ss1=FAT[ss1];
		FAT[fs1]=0;
		FAT[0]++;
	}
//	cout<<"\n删除文件成功"<<endl;
	
	
			}else
			{//说明将子目录转移到已存在的目录中 ,首先要删除子目录，然后再添加 
				fs=ProcessPath(comd[1],DirName,k,0,attrib);	//返回DirName的父目录的首块号
					strcpy(temp22path,temppath); 
         	    if (fs<0)
		           return fs;				//失败，返回
		        s0=s=FindFCB(DirName,fs,attrib,fcbp);//取DirName的首块号
		        while (s>0)		//循环查找，直到目录尾部
	            {
		            p=(FCB*) Disk[s];
		            for (i=0;i<4;i++,p++)
		            {
			        if (p->FileName[0]!=(char)0xe5 && p->FileName[0]!='\0')//累计非空目录项
				       count++;
		            }
		          //s0=s;			//记下上一个盘块号
		        s=FAT[s];		//取下一个盘块号
	            }
	            if (count>1)
	            {
	            	
		         cout<<"\n目录"<<DirName<<"非空，不能删除并转移。\n";
		          return -1;
	            }
	            //下面检查第二个参数是否符合要求，检测 
//			 cout<<"\nDirName:"<<DirName<<endl;
//			cout<<"\ncomd[2]的值为："<<comd[2]<<endl;
			if(strcmp(comd[2],"/")==0){
			strcpy(tPath1,comd[2]);
			strcpy(tPath3,comd[2]);//tPath3数组为后面的tPath2做准备 
			tPath=tPath1;//需要一个tPath的指针指向tPath1 
			tPath2=tPath3;
			strcat(tPath,DirName);
			strcat(tPath2,DirName);//这个时候将完整路径保存下来 在tPath2中 
			}else{
				//			tPath3=comd[2];//需要为后面的tPAth2重新赋予新的空间 
			strcpy(tPath1,comd[2]);
			strcpy(tPath3,comd[2]);//tPath3数组为后面的tPath2做准备 
			tPath=tPath1;//需要一个tPath的指针指向tPath1 
			tPath2=tPath3;
			strcat(tPath,"/");
			strcat(tPath2,"/");
			strcat(tPath,DirName);
			strcat(tPath2,DirName);//这个时候将完整路径保存下来 在tPath2中 
			}

//			cout<<"\ntPath的 值为:"<<tPath<<endl; 
//            cout<<"\ntPath3的 值为:"<<tPath2<<endl; 
		    short s11=ProcessPath(tPath,DirName1,k,0,attrib);
		    s21=s11;
//	        cout<<"目录首块号："<<s11<<endl; 
	        if (s11<0)
		      return s11;		//失败，返回
			if (!IsName(DirName1))		//若名字不符合规则
	        {
		     cout<<"\n命令中的新目录名错误。\n";
		     return -1;
	        }
	        short i11=FindFCB(DirName1,s11,'\020',p11);//查新建子目录名是否存在，返回负数则证明不存在同名子目录名 
	       if (i11>=0)
	       {
		     cout<<"\n错误：新目录名子目录重名！\n";
		    return -1;
	        }			
	        i11=FindFCB(DirName1,s11,'\0',p11);//查找子文件是否重名 
	        if (i11>=0)
           	{
	        	cout<<"\n新目录名与子文件名"<<tPath2<<"重名！是否要覆盖子文件？(y/n) \n";
	        	cin>>yn;
	        	if (yn!='N' && yn!='n'){//即删除子文件 
	        		short	i33=Check_UOF(tPath2);		//查UOF
	        			if (i33<S)					//该文件已在UOF中
	                {
		              cout<<"\n文件"<<tPath2<<"正在使用，不能删除并转移!\n";
		              return -3;
	                }
	                p11->FileName[0]=(char) 0xe5;	//删除目录项
	                while (i11>0)						//回收磁盘空间
	                {
		                s11=i11;
	                	i11=FAT[i11];
	                	FAT[s11]=0;
	                	FAT[0]++;
	                }
//                    	cout<<"\n删除文件成功"<<endl;
	        		
				}else{
					return -1;
				} 
	        }
						    
	            //注意temppath的值，明天继续 下面开始删除参数一（子目录） 
//	            cout<<"\ntemppath的值为："<<temppath<<endl;
	            while (s0>0)			//归还目录DirName所占的磁盘空间
	            {
		                s=FAT[s0];			//记下第s0块的后续块号		
		                FAT[s0]=0;			//回收第s0块
		                FAT[0]++;			//空闲盘块数增1
		                s0=s;				//后续块号赋予s0
	            }	  
	            fcbp->FileName[0]=(char)0xe5;	//删除DirName的目录项
	            if (strcmp(temp22path,"/")==0)	//所删除的子目录在根目录
	            {
//		           cout<<"\n删除目录成功！"<<endl;
		           
	            }else{
	//所删除的子目录DirName不在根目录时，对其父目录作以下处理
	            s0=s=fs;				//取DirName父目录的首块号
	            while (s>0)				//整理DirName的父目录空间(回收无目录项的盘块)
	            {
		          p=(FCB*) Disk[s];
		          for (j=i=0;i<4;i++,p++)
			         if (p->FileName[0]!=(char)0xe5 && p->FileName[0]!='\0')//累计非空目录项
				        j++;
		          if (j==0)
		          {
			        FAT[s0]=FAT[s];		//调整指针
			        FAT[s]=0;			//回收s号盘块
			        FAT[0]++;			//空闲盘块数增1
			        s=FAT[s0];
		        }
		        else
		       {
			      s0=s;				//记下上一个盘块号
			      s=FAT[s];			//s指向下一个盘块
		    	}
	           }
//	         cout<<"\n删除目录成功！"<<endl;//删除了目录 
	     }
				          
				          
			//下面是重新新建目录

	        short s22=FindBlankFCB(s21,p11);//找空白目录项
//	        cout<<"\ns21的值为："<<s21<<" "<<"s22的值为："<<s22<<endl;
	        if (s22<0)			//磁盘满
	         	return s22;
	        s22=M_NewDir(DirName1,p11,s21,attrib);	//在p所指位置创建一新子目录项
//	        cout<<"空白子目录首块号为："<<s22<<endl; 
//		    printf("%d\n",attrib);
	        if (s22<0)		//创建失败
	        {
	        	cout<<"\n磁盘空间已满，创建目录并转移失败。\n";
	        	return -1;
            	}
             	cout<<"\n转移成功!"<<endl; 
                	return 1;		//新目录创建成功，返回
			}
		}
		cout<<"\n转移成功!"<<endl;
		return 1; 
}


int ReplaceComd(int k){
	short i0,i1;
	short s1,s2;//第一个参数的首块号，和第二个目录的首块号 
	short fs1;//第一个文件的父目录首块号 
	FCB* fcbp1,* fcbp2,* fcbp0,* Bfcbp1;
	char temp1path[PATH_LEN],temp1Fpath[PATH_LEN];//保存第一个参数的完整路径 ，和其父路径 
	char temp2path[PATH_LEN],temp1Cpath[PATH_LEN];//第二个参数的完整路径和加上第一个文件名的文件路径 
	char *path;//第二个参数的目录名加上第一个参数的文件名所构成的完整的文件名 
	char t2Path[PATH_LEN]; 
	char *DirName1;
	char attrib=(char)16; 
	char yn;
	bool flag=true;//用来判断是否要取代只读文件的 
	if(k>2||k<1){
		cout<<"命令参数错误，参数只能有一个或者两个"<<endl;
		return -1;
	}else if(k==1){
		i0=strlen(comd[1]);
	if (i0>1 && comd[1][i0-1]=='/')	//路径以"/"结尾，错误
	{
		cout<<"\n路径名错误！\n";
		return -1;
	}
	
	
	}else if(k==2){
		i0=strlen(comd[1]);
		i1=strlen(comd[2]);
	if (i0>1 && comd[1][i0-1]=='/')	//路径以"/"结尾，错误
	{
		cout<<"\n路径名错误！\n";
		return -1;
	}
	if (i1>1 && comd[2][i1-1]=='/')	//路径以"/"结尾，错误
	{
		cout<<"\n路径名错误！\n";
		return -1;
	}
			
	}
	
	s1=FindPath(comd[1],'\0',1,fcbp1);//查找第一个参数（文件）的首块号 
//	cout<<"参数一的首块号为："<<s1<<endl;
	if(s1<0){
		cout<<"\n错误！指定文件不存在！"<<endl;
		return -1;
	}
	strcpy(temp1path,temppath);//将第一个参数的全路径名保存在temp1path中 
	strcpy(temp1Fpath,temppath);
	fs1=ProcessPath(temp1Fpath,DirName1,k,0,attrib);//参数一所在父目录的首块号 
//	cout<<"参数一的全路径名为："<<temp1path<<endl; 
//	cout<<"参数一的父目录首块号为："<<fs1<<endl;
//	cout<<"参数一的文件名为："<<DirName1<<endl;
	if(k==2){
		s2=FindPath(comd[2],attrib,1,fcbp2);
	}else if(k==1){
		s2=FindPath(curpath.cpath,attrib,1,fcbp2);	//找第二个参数的指定目录(的首块号)
	}
//	cout<<"参数二的首块号为："<<s2<<endl;
	if(s2<1){
		cout<<"错误！指定的目录不存在！"<<endl;
		return -1;
	}
	strcpy(temp2path,temppath);
//	cout<<"参数二的绝对路径为："<<temp2path<<endl;
	//接下来构造在参数二目录中的完整文件名 
	if(strcmp(temp2path,"/")==0){
		strcpy(t2Path,temp2path);
		path=t2Path;//用指针指向t2Path 
		strcat(path,DirName1);
	}else{
		strcpy(t2Path,temp2path);
		path=t2Path;//用指针指向t2Path 
		strcat(path,"/");
		strcat(path,DirName1);
	}
//	cout<<"文件的完整路径名为："<<path<<endl;
	short i2=FindFCB(DirName1,s2,'\0',fcbp2);//查找被取代同名文件是否存在
//	cout<<"查找被取代文件是否存在的首块号为："<<i2<<endl;
	if(i2<0){
		cout<<"\n错误！被取代的文件不存在！"<<endl;
		return -1;
	}
//	cout<<"检查fs1和s2是否为相同值，即验证是否在同一目录中："<<fs1<<" "<<s2<<endl;
	if(fs1==s2){
		cout<<"\n错误！指定文件和被取代文件不能是同一个文件！"<<endl;
		return -1;
	} 
	//接下来就是来查看被取代文件的属性了
//	cout<<"被取代文件的属性为：";
//	printf("%d\n",fcbp2->Fattrib);
	if(fcbp2->Fattrib=='\02'||fcbp2->Fattrib=='\04'){
		cout<<"被取代文件属性不能为系统和隐藏文件！取代文件失败！"<<endl;
		return -1;
	}
	if(fcbp2->Fattrib=='\01'){//只读文件属性 
		cout<<"\n被取代文件"<<fcbp2->FileName<<"为只读文件，是否要取代该文件？(y/n) \n";
		cin>>yn;
		if (yn=='N' || yn=='n'){
			flag=false;
		}
	}
	if(flag){//即开始删除被取代文件，并且复制指定文件 
//		cout<<"开始取代被取代文件了哈哈哈"<<endl;
		short	i33=Check_UOF(path);		//查UOF
	    if (i33<S)					//该文件已在UOF中
	    {
		    cout<<"\n文件"<<path<<"正在使用，不能删除!删除失败！！\n";
		    return -3;
	    }
	    fcbp2->FileName[0]=(char) 0xe5;	//删除目录项
//	    cout<<"s2的值："<<s2<<endl;
	    short s0=s2;//s2是所在目录的首块号
		short s=i2;//i2是所在文件的首块号 
	    while (s>0)						//回收磁盘空间
	    {
		s0=s;//s0是所在目录的首块号 
		s=FAT[s];
		FAT[s0]=0;
		FAT[0]++;
	    }
//	    cout<<"\n删除文件成功"<<endl;
	    //接下来开始复制文件 
//	    cout<<"原文件的首块号："<<s1<<endl;
//	    cout<<"\n第一个文件的名称为："<<DirName1<<endl;
	    fcbp0=fcbp1;
//	    cout<<"目标文件目录的首块号"<<s2<<endl;
	    short h1=s2;
	    short h2=FindBlankFCB(h1,Bfcbp1);
	    if (h2<0)
	    {
		    cout<<"\n替换文件失败。\n";
		    return h2;

	    }
	    short	size=fcbp0->Fsize;		//源文件的长度
//	    cout<<"目标文件的长度为："<<size<<endl;
	    short	bnum=size/SIZE+(short)(size%SIZE>0);	//计算源文件所占盘块数
	    if (FAT[0]<bnum)
	    {
		    cout<<"\n磁盘空间已满，不能复制文件。\n";
		    return -1;
	    }
		*Bfcbp1=*fcbp0;
//		cout<<"原文件的目录项的首块号为："<<fcbp0->Addr<<endl;
//		cout<<"s1的大小为："<<s1<<endl;
		strcpy(Bfcbp1->FileName,DirName1);	//写目标文件名	
		short	b0=0,b,ss1=s1;    
	while (ss1>0)		//开始复制文件内容
	{
		b=getblock();
		if (b0==0)
			Bfcbp1->Addr=b;		//目标文件的首块号
		else
			FAT[b0]=b;
		memcpy(Disk[b],Disk[ss1],SIZE);	//复制盘块
		ss1=FAT[ss1];				//准备复制下一个盘块
		b0=b;
	}		
	cout<<"\n替换文件成功！"<<endl; 
		return 1;
		
	}else{
		cout<<"替换文件失败！"<<endl;
		return -1;
	}
	
}


