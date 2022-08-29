#include "stdio.h"
#include "stdlib.h"
#include "Buddy_System.h"

#ifdef WIN32
#include "windows.h"
#endif

static void Set_Value(unsigned int* p, int iSize)
{//注意，这里要以0层的用户块大小为单位，否则一会验算的时候有误
	for (int j = 0; j < iSize; j++)
	{
		p[j] = j;
	}
		
}

static void Test_Mem_Mgr(Mem_Mgr* poMem_Mgr)
{//对Mem_Mgr的内容验算
	int i, j, * p;

	for (i = 1; i < poMem_Mgr->m_iCur_Item; i++)
	{
		p = (int*)&poMem_Mgr->m_pBuffer[poMem_Mgr->m_pHash_Item[i].m_iPos * poMem_Mgr->m_iBytes_Per_Bottom_Sub_Block];
		for (j = 0; j < poMem_Mgr->m_pHash_Item[i].m_iSub_Block_Count * poMem_Mgr->m_iBytes_Per_Bottom_Sub_Block / 4; j++)
		{
			if (p[j] != j)
			{
				printf("Piece开始块:%d 第 %d 项的第 %d 个元素出错，本应：%d 错误:%d , 位置: Buffer[%d] ", (int)poMem_Mgr->m_pHash_Item[i].m_iPos, i, j, j, p[j], (int)((unsigned char*)&p[j] - poMem_Mgr->m_pBuffer));
				exit(0);
			}
		}
		printf("第 %d 项通过\n", i-1);
	}
	return;
}

void Random_Test_1()
{//对这个玩意做全面的测试，测试的方法是搞若干个unsigned int Buffer, Buffer是循环有序置值，
//干完以后演算，看看有没有断了
#define MAX_PIECE_COUNT 1023
	Mem_Mgr oMem_Mgr;
	void* Allocate[MAX_PIECE_COUNT] = { NULL };
	int i;
	Init_Mem_Mgr(&oMem_Mgr, 1000000023, 1024, MAX_PIECE_COUNT);

	for (i = 0; i < MAX_PIECE_COUNT; i++)
	{
		Allocate[i] = pMalloc(&oMem_Mgr, i * 2048);
		if (Allocate[i])
			Set_Value((unsigned int*)Allocate[i], i * 2048 / 4);
	}

	//删除
	for (i = 0; i < MAX_PIECE_COUNT; i += 2)
	{
		Free(&oMem_Mgr, Allocate[i], i);
	}
	for (i = 0; i < MAX_PIECE_COUNT; i += 2)
	{
		Allocate[i] = pMalloc(&oMem_Mgr, i * 2048);
		if (Allocate[i])
			Set_Value((unsigned int*)Allocate[i], i * 2048 / 4);
	}

	Test_Mem_Mgr(&oMem_Mgr);
	Disp_Mem(&oMem_Mgr,2);
	Free_Mem_Mgr(&oMem_Mgr);
	return;
#undef MAX_PIECE_COUNT
}

void Random_Test_2()
{//真正的随机测试，这个过了应该差不多了
#define MAX_PIECE_COUNT 1023
#define MAX_SUB_BLOCK 10240

	Mem_Mgr oMem_Mgr;
	void* Allocate[MAX_PIECE_COUNT] = { NULL };
	unsigned int Piece_Size[MAX_PIECE_COUNT] = { 0 };

	int i, iSub_Block_Count;
	Init_Mem_Mgr(&oMem_Mgr, 14000000023, 1024, MAX_PIECE_COUNT);
	if (!oMem_Mgr.m_pBuffer)
		return;
	
	for (i = 0; i < MAX_PIECE_COUNT; i++)
	{
		iSub_Block_Count = (int)((unsigned int)iGet_Random_No() % MAX_SUB_BLOCK);
		Allocate[i] = pMalloc(&oMem_Mgr, iSub_Block_Count * oMem_Mgr.m_iBytes_Per_Bottom_Sub_Block);
		if (Allocate[i])
		{
			Set_Value((unsigned int*)Allocate[i], iSub_Block_Count * oMem_Mgr.m_iBytes_Per_Bottom_Sub_Block / 4);
			Piece_Size[i] = iSub_Block_Count * oMem_Mgr.m_iBytes_Per_Bottom_Sub_Block;
		}
	}

	for (i = MAX_PIECE_COUNT - 1; i >= 0; i -= 2)
	{
		if (Allocate[i])
			Free(&oMem_Mgr, Allocate[i], i);
	}
	for (i = MAX_PIECE_COUNT - 1; i >= 0; i -= 2)
	{
		if (Piece_Size[i])
		{
			Allocate[i] = pMalloc(&oMem_Mgr, Piece_Size[i]);
			if (Allocate[i])
				Set_Value((unsigned int*)Allocate[i], Piece_Size[i] / 4);
		}
	}

	Disp_Mem(&oMem_Mgr,2);
	Test_Mem_Mgr(&oMem_Mgr);

	for (i = 0; i < MAX_PIECE_COUNT; i++)
	{
		if (Allocate[i])
			Free(&oMem_Mgr, Allocate[i], i);
	}
	Disp_Mem(&oMem_Mgr,2);
	Free_Mem_Mgr(&oMem_Mgr);
	return;
#undef MAX_PIECE_COUNT
#undef MAX_SUB_BLOCK
}
void Small_Size_2_Layer_Test()
{
#define MAX_PIECE_COUNT 1023
#define MAX_SUB_BLOCK 16
	Mem_Mgr oMem_Mgr;
	void* Allocate[MAX_PIECE_COUNT];
	unsigned int Piece_Size[MAX_PIECE_COUNT];
	int i, iSub_Block_Count;
	//Init_Mem_Mgr(&oMem_Mgr, 1024*64*32, 1024, MAX_PIECE_COUNT);	//仅一层
	Init_Mem_Mgr(&oMem_Mgr, 1024 * 64 * 129, 1024, MAX_PIECE_COUNT);	//两层
	for (i = 0; i < MAX_PIECE_COUNT; i++)
	{
		while ( (iSub_Block_Count = (int)((unsigned int)iGet_Random_No() % MAX_SUB_BLOCK)) == 0) {}

		Allocate[i] = pMalloc(&oMem_Mgr, iSub_Block_Count * oMem_Mgr.m_iBytes_Per_Bottom_Sub_Block);
		if (Allocate[i])
		{
			Set_Value((unsigned int*)Allocate[i], iSub_Block_Count * oMem_Mgr.m_iBytes_Per_Bottom_Sub_Block / 4);
			Piece_Size[i] = iSub_Block_Count * oMem_Mgr.m_iBytes_Per_Bottom_Sub_Block;
		}else
			printf("Fail to allocated size:%u\n", iSub_Block_Count * oMem_Mgr.m_iBytes_Per_Bottom_Sub_Block);
	}
	Disp_Mem(&oMem_Mgr, 2);
	
	for (i = MAX_PIECE_COUNT - 1; i >= 0; i -= 2)
	{
		if (Allocate[i])
			Free(&oMem_Mgr, Allocate[i], i);
	}
	for (i = MAX_PIECE_COUNT - 1; i >= 0; i -= 2)
	{
		if (Piece_Size[i])
		{
			Allocate[i] = pMalloc(&oMem_Mgr, Piece_Size[i]);
			if (Allocate[i])
				Set_Value((unsigned int*)Allocate[i], Piece_Size[i] / 4);
		}
	}

	Disp_Mem(&oMem_Mgr,1);
	Test_Mem_Mgr(&oMem_Mgr);
	Free_Mem_Mgr(&oMem_Mgr);
	return;
#undef MAX_PIECE_COUNT
#undef MAX_SUB_BLOCK
}

void Shrink_Test()
{
#define MAX_PIECE_COUNT 1023
	Mem_Mgr oMem_Mgr;
	unsigned char* p;
	Init_Mem_Mgr(&oMem_Mgr, 1024 * 64 * 129, 1024, MAX_PIECE_COUNT);	//两层
	p = (unsigned char*)pMalloc(&oMem_Mgr, 1024 * 100);
	Set_Value((unsigned int*)p, 1024 * 100/4);
	Disp_Mem(&oMem_Mgr, 2);
	Test_Mem_Mgr(&oMem_Mgr);
	Shrink(&oMem_Mgr, p, 1024 * 90);
	Set_Value((unsigned int*)p, 1024 * 90/4);
	Test_Mem_Mgr(&oMem_Mgr);
	Disp_Mem(&oMem_Mgr,2);
#undef MAX_PIECE_COUNT
}

void Expand_Test()
{//扩充实验
#define MAX_PIECE_COUNT 1023
	void* Allocate[MAX_PIECE_COUNT];
	unsigned int Piece_Size[MAX_PIECE_COUNT];

	Mem_Mgr oMem_Mgr;
	Init_Mem_Mgr(&oMem_Mgr, 14000000023, 1024, MAX_PIECE_COUNT);
	Allocate[0] = pMalloc(&oMem_Mgr, (unsigned int)(oMem_Mgr.m_Layer[2].m_iBytes_Per_Block * 2 + 1024));
	Piece_Size[0] = (unsigned int)(oMem_Mgr.m_Layer[2].m_iBytes_Per_Block * 2 + 1024);
	Allocate[1] = pMalloc(&oMem_Mgr, (unsigned int)(oMem_Mgr.m_Layer[2].m_iBytes_Per_Block * 20));
	Piece_Size[1] = (unsigned int)(oMem_Mgr.m_Layer[2].m_iBytes_Per_Block * 20);
	Allocate[2] = pMalloc(&oMem_Mgr, 1024 );
	Piece_Size[2] = 1024;

	Set_Value((unsigned int*)Allocate[0], Piece_Size[0] / 4);
	Set_Value((unsigned int*)Allocate[1], Piece_Size[1] / 4);
	Set_Value((unsigned int*)Allocate[2], Piece_Size[2] / 4);

	Test_Mem_Mgr(&oMem_Mgr);

	Disp_Mem(&oMem_Mgr, 2);
	Free(&oMem_Mgr, Allocate[1],0);
	bExpand(&oMem_Mgr, Allocate[0], (unsigned int)(oMem_Mgr.m_Layer[2].m_iBytes_Per_Block * 20));
	Piece_Size[0] = (unsigned int)(oMem_Mgr.m_Layer[2].m_iBytes_Per_Block * 20);
	Set_Value((unsigned int*)Allocate[0], Piece_Size[0] / 4);
	Test_Mem_Mgr(&oMem_Mgr);

	Disp_Mem(&oMem_Mgr, 2);
	Free_Mem_Mgr(&oMem_Mgr);
	return;
#undef MAX_PIECE_COUNT
}
int main()
{
	Expand_Test();
	//Shrink_Test();
	//Small_Size_2_Layer_Test();
	//Random_Test_1();
	//Random_Test_2();
#ifdef WIN32
	_CrtDumpMemoryLeaks();
#endif // WIN32
}