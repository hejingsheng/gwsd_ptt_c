// gwdemo.c : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
extern void pttClientOffline();

void GetCurrentTimeString(char* buffer, int bufferSize) {
	time_t now;
	struct tm *local;

	time(&now);
	local = localtime(&now);

	strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", local);
}

void showMenuScreen(char *imei, char *dns, int port) 
{
	char timeStr[64];
	GetCurrentTimeString(timeStr, sizeof(timeStr));
	system("cls");

	printf("╔═══════════════════════════════════════════════════════╗\n");
	printf("║              GWSD Intercom System Platform            ║\n");
	printf("╠═══════════════════════════════════════════════════════╣\n");
	printf("║ Version: v1.0.1              Developer: Jack He       ║\n");
	printf("╠═══════════════════════════════════════════════════════╣\n");
	printf("║ Function Options:                                      ║\n");
	printf("║  ┌─────────────────────────────────────────────┐      ║\n");
	printf("║  │  0. System Help                            │      ║\n");
	printf("║  │  1. Query Groups                           │      ║\n");
	printf("║  │  2. Query Members                          │      ║\n");
	printf("║  │  3. Start Speaking                         │      ║\n");
	printf("║  │  4. Stop Speaking                          │      ║\n");
	printf("║  │  5. Create Group                           │      ║\n");
	printf("║  │  6. Generate Token                         │      ║\n");
	printf("║  │  7. Enter Group                            │      ║\n");
	printf("║  │  8. Leave Group                            │      ║\n");
	printf("║  │  9. Delete Group                           │      ║\n");
	printf("║  │ 10. Join Group                             │      ║\n");
	printf("║  │ 11. Logout                                 │      ║\n");
	printf("║  └─────────────────────────────────────────────┘      ║\n");
	printf("╠═══════════════════════════════════════════════════════╣\n");
	printf("║ Current User: %-10s | Time: %-21s ║\n", imei, timeStr);
	printf("║ Server Address: %s:%d ║\n", dns, port);
	printf("╠═══════════════════════════════════════════════════════╣\n");
	printf("║ Please select operation (0-11) >                      ║\n");
	printf("╚═══════════════════════════════════════════════════════╝\n");
	printf(" > ");
}

void showWelcomeScreen() 
{
	system("cls");

	printf("\n\n\n");
	printf("              ╔══════════════════════════════════════╗\n");
	printf("              ║                                      ║\n");
	printf("              ║    Welcome to GWSD Intercom System   ║\n");
	printf("              ║                                      ║\n");
	printf("              ║          Developer: Jack He          ║\n");
	printf("              ║          Version: v1.0.1             ║\n");
	printf("              ║           Year: 2026                 ║\n");
	printf("              ║                                      ║\n");
	printf("              ╚══════════════════════════════════════╝\n");

	Sleep(2000);  // Show for 2 seconds

	// Loading animation
	printf("\n\n              System initializing");
	for (int i = 0; i < 3; i++) {
		printf(".");
		Sleep(500);
	}
	printf("\n\n");
}

void showGoodbyeScreen() 
{
	system("cls");

	printf("\n\n\n");
	printf("              ╔══════════════════════════════════════╗\n");
	printf("              ║                                      ║\n");
	printf("              ║    Thank you for using GWSD System   ║\n");
	printf("              ║                                      ║\n");
	printf("              ║          Developer: Jack He          ║\n");
	printf("              ║          Goodbye!                    ║\n");
	printf("              ║                                      ║\n");
	printf("              ╚══════════════════════════════════════╝\n");
}

static int ValidatePort(int port) 
{
	return (port >= 1024 && port <= 65535);
}

// 验证DNS格式（简单验证）
static int ValidateDNS(const char* dns) 
{
	return (strlen(dns) > 0 && strlen(dns) < 256);
}

// 验证IMEI格式（简单验证）
static int ValidateIMEI(const char* imei) 
{
	return (strlen(imei) > 0 && strlen(imei) < 32);
}

void showConfigScreen(char* dns, int* port, char* imei) {
	int valid = 0;

	do {
		system("cls");

		printf("╔═══════════════════════════════════════════════════════╗\n");
		printf("║           GWSD System Configuration                   ║\n");
		printf("╠═══════════════════════════════════════════════════════╣\n");
		printf("║                                                       ║\n");
		printf("║  Please enter the required configuration:             ║\n");
		printf("║                                                       ║\n");
		printf("║  ┌─────────────────────────────────────────────┐      ║\n");
		printf("║  │                                             │      ║\n");
		printf("║  │  DNS Server Address:                       │      ║\n");
		printf("║  │  (Required for connection)                 │      ║\n");
		printf("║  │  (Default: chn-access2c.hawk-sight.com)    │      ║\n");
		printf("║  │                                             │      ║\n");
		printf("║  │  > ");
		fflush(stdout);
		scanf("%s", dns);

		// 添加默认值提示
		if (strlen(dns) == 0) {
			strcpy(dns, "chn-access2c.hawk-sight.com");
		}

		printf("║  │                                             │      ║\n");
		printf("║  │                                             │      ║\n");
		printf("║  │  Server Port:                              │      ║\n");
		printf("║  │  (1024-65535, Default: 23003)               │      ║\n");
		printf("║  │                                             │      ║\n");
		printf("║  │  > ");
		fflush(stdout);
		scanf("%d", port);

		// 验证端口
		if (*port <= 0) {
			*port = 23003; // 默认端口
		}

		printf("║  │                                             │      ║\n");
		printf("║  │                                             │      ║\n");
		printf("║  │  Device Identifier:                        │      ║\n");
		printf("║  │  (Unique device ID, e.g., 865223047568037) │      ║\n");
		printf("║  │                                             │      ║\n");
		printf("║  │  > ");
		fflush(stdout);
		scanf("%s", imei);

		printf("║  │                                             │      ║\n");
		printf("║  └─────────────────────────────────────────────┘      ║\n");
		printf("║                                                       ║\n");

		// 验证输入
		valid = ValidateDNS(dns) && ValidatePort(*port) && ValidateIMEI(imei);

		if (!valid) {
			printf("║  Invalid input detected. Please check:            ║\n");
			if (!ValidateDNS(dns)) printf("║     DNS address is required                        ║\n");
			if (!ValidatePort(*port)) printf("║     Port must be between 1024-65535                ║\n");
			if (!ValidateIMEI(imei)) printf("║     Device ID is required                          ║\n");
			printf("║                                                       ║\n");
			printf("║  Press Enter to try again...                         ║\n");
			printf("║                                                       ║\n");
			getchar();
			getchar();
		}

	} while (!valid);

	// 显示配置确认
	system("cls");
	printf("╔═══════════════════════════════════════════════════════╗\n");
	printf("║           Configuration Summary                      ║\n");
	printf("╠═══════════════════════════════════════════════════════╣\n");
	printf("║                                                       ║\n");
	printf("║  Your configuration has been saved:                  ║\n");
	printf("║                                                       ║\n");
	printf("║  ┌─────────────────────────────────────────────┐      ║\n");
	printf("║  │                                             │      ║\n");
	printf("║  │  DNS Server:   %-25s       │      ║\n", dns);
	printf("║  │  Server Port:  %-25d       │      ║\n", *port);
	printf("║  │  Device ID:    %-25s       │      ║\n", imei);
	printf("║  │                                             │      ║\n");
	printf("║  └─────────────────────────────────────────────┘      ║\n");
	printf("║                                                       ║\n");
	printf("║  Press Enter to start the system...                   ║\n");
	printf("║                                                       ║\n");
	printf("╚═══════════════════════════════════════════════════════╝\n");
	getchar();
	getchar();
}

void showGroupPageConfigScreen(int *pageSize, int *pageNum)
{
	system("cls");

	printf("┌───────────────────────────────────────────────────────┐\n");
	printf("│               Pagination Configuration                │\n");
	printf("├───────────────────────────────────────────────────────┤\n");
	printf("│                                                       │\n");
	printf("│  Enter pagination parameters (Format: size,number)    │\n");
	printf("│  Example: 20,1 for 20 items per page, page 1          │\n");
	printf("│                                                       │\n");
	printf("│                                                       │\n");
	printf("│  Input format: [pageSize],[pageNumber]                │\n");
	printf("│                                                       │\n");
	printf("│  > ");
	fflush(stdout);
	scanf("%d,%d", pageSize, pageNum);
	printf("│                                                       │\n");
	printf("│  Parameters accepted:                                │\n");
	printf("│    Page Size:    %-3d                              │\n", *pageSize);
	printf("│    Page Number:  %-3d                              │\n", *pageNum);
	printf("│                                                       │\n");
	printf("└───────────────────────────────────────────────────────┘\n");
}

void showMemberPageConfigScreen(int *gid, int *online, int *pageSize, int *pageNum)
{
	system("cls");

	printf("┌───────────────────────────────────────────────────────┐\n");
	printf("│              Advanced Member Query                     │\n");
	printf("├───────────────────────────────────────────────────────┤\n");
	printf("│                                                       │\n");
	printf("│  Enter query parameters (Format: gid,online,size,num) │\n");
	printf("│  Example: 12,1,20,1 for 12 group online users, page 1 │\n");
	printf("│                                                       │\n");
	printf("│  Parameter details:                                   │\n");
	printf("│    Group ID:      Specific group id                   │\n");
	printf("│    Online Status: 0=Offline, 1=Online,                │\n");
	printf("│    Page Size:     Items per page (1-100)              │\n");
	printf("│    Page Number:   Page index (starts from 1)          │\n");
	printf("│                                                       │\n");
	printf("│                                                       │\n");
	printf("│  Input format: [gid],[online],[pageSize],[pageNum]    │\n");
	printf("│                                                       │\n");
	printf("│  > ");
	fflush(stdout);
	scanf("%d,%d,%d,%d", gid, online, pageSize, pageNum);
	printf("│                                                       │\n");
	printf("│  Parameters accepted:                                 │\n");
	printf("│    Group ID:      %-4d                           │\n",*gid);
	printf("│    Online Status: %-4d (%s)                  │\n", *online, *online == 0 ? "All status" :  "Online only");
	printf("│    Page Size:     %-4d items per page             │\n", *pageSize);
	printf("│    Page Number:   %-4d                            │\n", *pageNum);
	printf("│                                                       │\n");
	printf("└───────────────────────────────────────────────────────┘\n");
}

void showGroupTokenConfigScreen(int *token, int *type)
{
	system("cls");

	printf("┌───────────────────────────────────────────────────────┐\n");
	printf("│              Token Configuration                      │\n");
	printf("├───────────────────────────────────────────────────────┤\n");
	printf("│                                                       │\n");
	printf("│  Enter token parameters (Format: token,type)          │\n");
	printf("│  Example: 123456,1 for token 123456, type 1           │\n");
	printf("│                                                       │\n");
	printf("│  Parameter details:                                   │\n");
	printf("│    Token Value:    Any integer token ID               │\n");
	printf("│    Token Type:     0=Keep, 1=Replace                  │\n");
	printf("│                                                       │\n");
	printf("│                                                       │\n");
	printf("│  Input format: [token],[type]                         │\n");
	printf("│                                                       │\n");
	printf("│  > ");
	fflush(stdout);
	scanf("%d,%d", token, type);
	printf("│                                                       │\n");
	printf("│  Parameters accepted:                                 │\n");
	printf("│    Token: %-10d (", *token);
	switch (*type) {
	case 0: printf("Keep"); break;
	case 1: printf("Replace "); break;
	default: printf("Unknown"); break;
	}
	printf(")                        │\n");
	printf("│    Type:  %-10d                                 │\n", *type);
	printf("│                                                       │\n");
	printf("└───────────────────────────────────────────────────────┘\n");
}

void showGroupConfigScreen(int *gid)
{
	system("cls");

	printf("┌───────────────────────────────────────────────────────┐\n");
	printf("│              Group Selection                          │\n");
	printf("├───────────────────────────────────────────────────────┤\n");
	printf("│                                                       │\n");
	printf("│  Enter Group ID:                                      │\n");
	printf("│                                                       │\n");
	printf("│  >0 = Specific Group                                  │\n");
	printf("│  Examples: 10001, 10002, 20003, 10008                 │\n");
	printf("│                                                       │\n");
	printf("│  > ");
	fflush(stdout);
	scanf("%d", gid);
	printf("│                                                       │\n");
	printf("│  Group selected:                                      │\n");
	printf("│    Group ID: %-4d                              │\n", *gid);
	printf("│                                                       │\n");
	printf("└───────────────────────────────────────────────────────┘\n");
}

int main()
{
	int cmd;
	int pageNum, pageSize;
	int gid, online;
	int token, type;
	char imei[33];
	char dns[256];
	int port;
	showWelcomeScreen();
	showConfigScreen(dns, &port, imei);
	pttClientStart(imei, "111111", dns, port);
	Sleep(1000);
	while (1)
	{
		showMenuScreen(imei, dns, port);
		scanf("%d", &cmd);
		switch (cmd)
		{
		case 1:
			showGroupPageConfigScreen(&pageSize, &pageNum);
			pttClientQueryGroup(pageNum, pageSize);
			break;
		case 2:
			showMemberPageConfigScreen(&gid, &online, &pageSize, &pageNum);
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
			showGroupConfigScreen(&gid);
			pttClientGeneralGroupToken(gid);
			break;
		case 7:
			showGroupTokenConfigScreen(&token, &type);
			pttClientEnterGroup(token, type);
			break;
		case 8:
			showGroupConfigScreen(&gid);
			pttClientExitGroup(gid);
			break;
		case 9:
			showGroupConfigScreen(&gid);
			pttClientDeleteGroup(gid);
			break;
		case 10:
			showGroupConfigScreen(&gid);
			pttClientJoinGroup(gid);
			break;
		case 11:
			pttClientOffline();
			Sleep(5000);
			showGoodbyeScreen();
			ExitProcess(0);
			break;
		default:
			break;
		}
	}
	return 0;
}

