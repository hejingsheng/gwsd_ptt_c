// gwdemo.c : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

extern void pttClientStart(const char *account, const char *password, const char *dns, int port);
extern void pttClientQueryGroup(int pageNum, int pageSize);
extern void pttClientQueryMember(int gid, int online, int pageNum, int pageSize);
extern void pttClientSpeakStart();
extern void pttClientSpeakEnd();
extern void pttClientCreateGroup();
extern void pttClientGeneralGroupToken(int gid);
extern void pttClientEnterGroup(int token, int type);
extern void pttClientExitGroup(int gid);
extern void pttClientDeleteGroup(int gid);
extern void pttClientJoinGroup(int gid);
// 865223047568037
// chn-access2c.hawk-sight.com
int main()
{
	int cmd;
	int pageNum, pageSize;
	int gid, online;
	int token, type;
	pttClientStart("865223047568037", "111111", "chn-access2c.hawk-sight.com", 23003);
	Sleep(1000);
	while (1)
	{
		printf("0.Help\n");
		printf("1.Query group\n");
		printf("2.Query member\n");
		printf("3.Start speak\n");
		printf("4.Stop speak\n");
		printf("5.Greate group\n");
		printf("6.General token for group\n");
		printf("7.Enter group\n");
		printf("8.Exit group\n");
		printf("9.Delete group\n");
		printf("10.Join group\n");
		printf("Please input cmd:");
		scanf("%d", &cmd);
		switch (cmd)
		{
		case 1:
			printf("Please input pagesize pagenum:");
			scanf("%d,%d", &pageSize, &pageNum);
			pttClientQueryGroup(pageNum, pageSize);
			break;
		case 2:
			printf("Please input gid online(0/1) pagesize pagenum:");
			scanf("%d,%d,%d,%d", &gid, &online, &pageSize, &pageNum);
			pttClientQueryMember(gid, online, pageNum, pageSize);
			break;
		case 3:
			pttClientSpeakStart();
			break;
		case 4:
			pttClientSpeakEnd();
			break;
		case 5:
			pttClientCreateGroup();
			break;
		case 6:
			printf("Please input gid:");
			scanf("%d", &gid);
			pttClientGeneralGroupToken(gid);
			break;
		case 7:
			printf("Please input token type:");
			scanf("%d,%d", &token, &type);
			pttClientEnterGroup(token, type);
			break;
		case 8:
			printf("Please input gid:");
			scanf("%d", &gid);
			pttClientExitGroup(gid);
			break;
		case 9:
			printf("Please input gid:");
			scanf("%d", &gid);
			pttClientDeleteGroup(gid);
			break;
		case 10:
			printf("Please input gid:");
			scanf("%d", &gid);
			pttClientJoinGroup(gid);
			break;
		default:
			break;
		}
	}
	return 0;
}

