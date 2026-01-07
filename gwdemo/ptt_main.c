/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "pt.h"
#include "cJSON.h"
#include "GWLog.h"
#include "GWPttEngine.h"

#pragma comment(lib, "ws2_32.lib")

#define WM_USER_MESSAGE (WM_USER + 100)

#define TIME_1_MICROSECOND  (1)
#define TIME_1_MILLSECOND   (1000 * TIME_1_MICROSECOND)
#define TIME_1_SECOND       (1000 * TIME_1_MILLSECOND)

#define PTT_CLIENT_DEFAULT_HEART_PERIOD   10

#define PTT_THREAD_TIMER_EVENT_TYPE         1
#define PTT_THREAD_NET_READY_EVENT_TYPE     2
#define PTT_THREAD_LOGIN_EVENT_TYPE         3
#define PTT_THREAD_REG_MSG_EVENT_TYPE       4
#define PTT_THREAD_IDLE_EVENT_TYPE          5
#define PTT_THREAD_OFFLINE_EVENT_TYPE       6

typedef enum {
	PTT_CLIENT_STATUS_NONE      = -1,
	PTT_CLIENT_STATUS_NETSCAN  = 0,
	PTT_CLIENT_STATUS_NETCONN  = 1,
	PTT_CLIENT_STATUS_LOGIN     = 2,
	PTT_CLIENT_STATUS_REGMSG    = 3,
	PTT_CLIENT_STATUS_JOINGRP   = 4,
	PTT_CLIENT_STATUS_IDLE      = 5,
	PTT_CLIENT_STATUS_OFFLINE   = 6,
}PTT_CLIENT_STATUS_ENUM;

typedef struct
{
	struct pt ptt_pt;
	PTT_CLIENT_STATUS_ENUM status; //-1 none 0 start net scan 1 start net connect 2 start login 3 start join group  4 enter idle 5 offline
	int net_search_timeout;
	int net_ready_timeout;
	int login_timeout;
	int regmsg_timeout;
	int joingrp_timeout;
	int heart_timeout;
	int heart_period;
	char account[33];
	char password[33];
	char newAccount[33];
	char newPassword[33];
	char agentPassword[33];
	char name[64];
	uint32_t uid;
	int defaultGrpId;
	int currentGrpId;
	int currentGrpPri;
	int currentGrpType;
	int gps;
	int gps_period;
	int message;
	int call;
	int video;
	int silent;
	char msgStart;
	char currentGrpName[64];
	char serverIp[33];
	char platformDns[64];
	int platformPort;
	signed char local_zone;
	char local_zone_dec;
	int mode;
	char exitai;
	char binding;
	char locinfo[256];
	int reportLoc;
	char weatherinfo[256];
	char battery;
	char net[10];
}PttClient;

struct Group
{
    char name[64];
    uint32_t gid;
    int type;
	uint32_t author;
};

struct Member
{
    char *name;
    uint32_t gid;
    uint32_t uid;
};

struct Wifi
{
	char ssid[33];
	char pass[33];
	unsigned char auth;
	unsigned char rssi;
	unsigned char available;
};

typedef struct
{
	int type;
	void *data;
	int len;
}PttThreadMsg;

static PttClient *pttClient = NULL;
static DWORD ptt_thread_id = 0;
static void sendMsgToPttMainThread(int type, int len)
{
	PostThreadMessage(ptt_thread_id, WM_USER_MESSAGE, type, len);
}

static void offline()
{
	pttClient->uid = 0;
	pttClient->defaultGrpId = 0;
	pttClient->currentGrpId = 0;
	pttClient->msgStart = 0;
    sendMsgToPttMainThread(PTT_THREAD_OFFLINE_EVENT_TYPE, 0);
}

static void onGWPttEvent(int event, char *data, int data1)
{
    cJSON *root = NULL;
    int ret;

    if (event == GW_PTT_EVENT_LOGIN)
    {
    	cJSON *item;
        root = cJSON_Parse(data);
        ret = cJSON_GetObjectItem(root, "result")->valueint;
        if (ret == 0)
        {
            pttClient->uid = cJSON_GetObjectItem(root, "uid")->valueint;
            pttClient->defaultGrpId = cJSON_GetObjectItem(root, "defaultGid")->valueint;
            item = cJSON_GetObjectItem(root, "gps");
            if (item != NULL)
            {
            	pttClient->gps = item->valueint;
            	if (pttClient->gps)
            	{
            		item = cJSON_GetObjectItem(root, "gps_period");
            		pttClient->gps_period = item->valueint;
            	}
            }
            else
            {
            	pttClient->gps = 0;
            }
            item = cJSON_GetObjectItem(root, "message");
            if (item != NULL)
            {
            	pttClient->message = item->valueint;
            }
            else
            {
            	pttClient->message = 0;
            }
            item = cJSON_GetObjectItem(root, "call");
            if (item != NULL)
            {
            	pttClient->call = item->valueint;
            }
            else
            {
            	pttClient->call = 0;
            }
            item = cJSON_GetObjectItem(root, "video");
            if (item != NULL)
            {
            	pttClient->video = item->valueint;
            }
            else
            {
            	pttClient->video = 0;
            }
            strcpy(pttClient->name, cJSON_GetObjectItem(root, "name")->valuestring);
            sendMsgToPttMainThread(PTT_THREAD_LOGIN_EVENT_TYPE, 0);
        }
        else if (ret == 1)
        {
        }
        else
        {
        	pttClient->uid = 0;
            memset(pttClient->name, 0, sizeof(pttClient->name));
        }
    }
    else if (event == GW_PTT_EVENT_JOIN_GROUP)
    {
		uint32_t creater;
        root = cJSON_Parse(data);
        ret = cJSON_GetObjectItem(root, "result")->valueint;
        if (ret == 0)
        {
        	pttClient->currentGrpPri = cJSON_GetObjectItem(root, "priority")->valueint;
        	pttClient->currentGrpId = cJSON_GetObjectItem(root, "gid")->valueint;
			creater = cJSON_GetObjectItem(root, "creater")->valueint;
			if (cJSON_GetObjectItem(root, "gname") != NULL)
			{
				strcpy(pttClient->currentGrpName, cJSON_GetObjectItem(root, "gname")->valuestring);
			}
			pttClient->currentGrpType = 16;
            sendMsgToPttMainThread(PTT_THREAD_IDLE_EVENT_TYPE, 0);
        }
        else
        {
        	GWLOG_PRINT(GW_LOG_LEVEL_INFO, "join group fail");
        	sendMsgToPttMainThread(PTT_THREAD_IDLE_EVENT_TYPE, 0);
        }
    }
    else if (event == GW_PTT_EVENT_SPEAK)
    {
        int speakerid = 0;
        int speakerpri = 0;
        char *speakernm = NULL;
        int status = 0;
	 	cJSON *item = NULL;
        
        root = cJSON_Parse(data);
        speakerid = cJSON_GetObjectItem(root, "uid")->valueint;
        if (speakerid > 0)
        {
            speakernm = cJSON_GetObjectItem(root, "name")->valuestring;
	     	item = cJSON_GetObjectItem(root, "priority");
            if (item != NULL && item->type == cJSON_Number)
            {
                speakerpri = item->valueint;
            }
            if (speakerpri >= pttClient->currentGrpPri)
            {
                status = 0;
            }
            else
            {
                status = 1;
            }
        }
        else
        {
            status = 1;
        }
    }
	else if (event == GW_PTT_EVENT_CURRENT_GROUP)
    {
    	cJSON *item;
		uint32_t creater;
    	root = cJSON_Parse(data);
    	creater = cJSON_GetObjectItem(root, "creater")->valueint;
    	pttClient->currentGrpId = cJSON_GetObjectItem(root, "gid")->valueint;
    	item = cJSON_GetObjectItem(root, "name");
    	if (item != NULL)
    	{
    		strcpy(pttClient->currentGrpName, item->valuestring);
    	}
		pttClient->currentGrpType = 16;
    	GWLOG_PRINT(GW_LOG_LEVEL_INFO, "join group %d, %s(%d)", pttClient->currentGrpId, pttClient->currentGrpName, creater);
    }
	else if (event == GW_PTT_EVENT_QUERY_TMPGRP)
	{
        
	}
	else if (event == GW_PTT_EVENT_AI_STATUS)
    {
    	int status;
    	root = cJSON_Parse(data);
    	ret = cJSON_GetObjectItem(root, "result")->valueint;
    	if (ret == 0)
    	{
    		status = cJSON_GetObjectItem(root, "status")->valueint;
    		if (status == 0)
    		{
    			//updateUI(EXIT_AI, NULL);
    			pttClient->exitai = 0;
    		}
    		else
    		{
    			//updateUI(ENTER_AI, NULL);
    		}
    	}
    	else
    	{
    		//updateUI(EXIT_AI, NULL);
    	}
    }
    else if (event == GW_PTT_EVENT_LOGOUT || event == GW_PTT_EVENT_KICKOUT || event == GW_PTT_EVENT_ERROR || event == GW_PTT_EVENT_UNBIND)
    {
        offline();
    }
    if (root)
    {
        cJSON_Delete(root);
    }
}

static void processSignal(cJSON *signal)
{
	int action;
	if (signal == NULL)
	{
		return;
	}
	action = cJSON_GetObjectItem(signal, "action")->valueint;
	if (action == GW_PTT_INVITE_TO_GROUP)
	{
		struct Group group;
		group.gid = cJSON_GetObjectItem(signal, "gid")->valueint;
		group.author = cJSON_GetObjectItem(signal, "authorUid")->valueint;
		strcpy(group.name, cJSON_GetObjectItem(signal, "gname")->valuestring);
		GWLOG_PRINT(GW_LOG_LEVEL_INFO, "invite to join group %s(%d)", group.name, group.gid);
	}
	else if (action == GW_PTT_KICK_FROM_GROUP)
	{
		int gid = cJSON_GetObjectItem(signal, "gid")->valueint;
		GWLOG_PRINT(GW_LOG_LEVEL_INFO, "kick out from group %d", gid);
	}
	else if (action == GW_PTT_USER_LEAVE_GROUP)
	{

	}
	else if (action == GW_PTT_USER_ENTER_GROUP)
	{

	}
}

static void onGWMsgEvent(int status, char *msg, int length)
{
	cJSON *root = NULL;
	cJSON *data = NULL;
	cJSON *item = NULL;
	int sta;
	GWLOG_PRINT(GW_LOG_LEVEL_INFO, "msg status %d", status);
	if (status == GW_MSG_STATUS_ERROR)
	{
		pttClient->msgStart = 0;
		sendMsgToPttMainThread(PTT_THREAD_REG_MSG_EVENT_TYPE, 0);
	}
	if (status == GW_MSG_STATUS_SUCC)
	{
		pttClient->msgStart = 1;
		sendMsgToPttMainThread(PTT_THREAD_REG_MSG_EVENT_TYPE, 0);
	}
	if (status == GW_MSG_STATUS_DATA)
	{
		int msgtype;
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		msgtype = cJSON_GetObjectItem(root, "msgtype")->valueint;
		if (msgtype == GW_PTT_MSG_TYPE_USER_SIGNAL)
		{
			cJSON *signal = cJSON_GetObjectItem(root, "signal");
			processSignal(signal);
		}
	}
	else if (status == GW_MSG_STATUS_ADDRESS)
	{
		
	}
	else if (status == GW_MSG_STATUS_WEATHER)
	{
		
	}
	else if (status == GW_MSG_STATUS_QUERY_GROUP)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			int size;
			int index;
			int pages;
			int totals;
			cJSON *groups;
			cJSON *tmp;
			data = cJSON_GetObjectItem(root, "data");
			if (data == NULL)
			{
				goto error;
			}
			tmp = cJSON_GetObjectItem(root, "pages");
			if (tmp == NULL || tmp->valueint == 0)
			{
				// normal query
				pages = 0;
			}
			else
			{
				pages = tmp->valueint;
				totals = cJSON_GetObjectItem(root, "total")->valueint;
			}
			groups = cJSON_GetObjectItem(data, "groups");
			if (groups != NULL)
			{
				size = cJSON_GetArraySize(groups);
				if (size > 0)
				{
					for (index = 0; index < size; index++)
					{
						struct Group group;
						cJSON *item = cJSON_GetArrayItem(groups, index);
						strcpy(group.name, cJSON_GetObjectItem(item, "name")->valuestring);
						group.gid = cJSON_GetObjectItem(item, "gid")->valueint;
						group.author = cJSON_GetObjectItem(item, "authorUid")->valueint;
						GWLOG_PRINT(GW_LOG_LEVEL_INFO, "group id %d, name %s, creater %d", group.gid, group.name, group.author);
					}
					if (pages == 0)
					{
						GWLOG_PRINT(GW_LOG_LEVEL_INFO, "group count %d", size);
					}
					else
					{
						GWLOG_PRINT(GW_LOG_LEVEL_INFO, "page group count %d,%d", totals, size);
					}
				}
				else
				{
					GWLOG_PRINT(GW_LOG_LEVEL_INFO, "no group");
				}
			}
			else
			{
				GWLOG_PRINT(GW_LOG_LEVEL_INFO, "no group")
			}
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "query group fail %d", sta);
		}
	}
	else if (status == GW_MSG_STATUS_GROUP_OPERATE_CREATE)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			struct Group group;
			int token;
			int ttl;
			data = cJSON_GetObjectItem(root, "data");
			strcpy(group.name, cJSON_GetObjectItem(data, "name")->valuestring);
			group.gid = cJSON_GetObjectItem(data, "gid")->valueint;
			token = cJSON_GetObjectItem(data, "code")->valueint;
			ttl = cJSON_GetObjectItem(data, "ttl")->valueint;
			group.author = pttClient->uid;
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "create group %s(%d) success", group.name, group.gid);
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "create group fail");
		}
	}
	else if (status == GW_MSG_STATUS_GROUP_OPERATE_ADDUSER)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "add user success");
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "add user fail");
		}
	}
	else if (status == GW_MSG_STATUS_GROUP_OPERATE_DELUSER)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "delete user success");
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "delete user fail");
		}
	}
	else if (status == GW_MSG_STATUS_GROUP_OPERATE_ENTER)
	{
		
	}
	else if (status == GW_MSG_STATUS_GROUP_OPERATE_EXIT)
	{
		cJSON *data;
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			int gid;
			data = cJSON_GetObjectItem(root, "data");
			gid = cJSON_GetObjectItem(data, "gid")->valueint;
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "exit group %d success", gid);
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "exit group fail");
		}
	}
	else if (status == GW_MSG_STATUS_GROUP_OPERATE_RENAME)
	{
		// not process
	}
	else if (status == GW_MSG_STATUS_GROUP_OPERATE_DELETE)
	{
		cJSON *data;
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			int gid;
			data = cJSON_GetObjectItem(root, "data");
			gid = cJSON_GetObjectItem(data, "gid")->valueint;
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "delete group %d success", gid);
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "delete group fail")
		}
	}
	else if (status == GW_MSG_STATUS_GENERAL_GROUP_TOKEN)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			cJSON *code;
			int token;
			int ttl;
			data = cJSON_GetObjectItem(root, "data");
			code = cJSON_GetObjectItem(data, "grpCode");
			token = cJSON_GetObjectItem(code, "code")->valueint;
			ttl = cJSON_GetObjectItem(code, "ttl")->valueint;
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "general tokne %d %d", token, ttl);
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "general token fail")
		}
	}
	else if (status == GW_MSG_STATUS_GROUP_TOKEN_ENTER)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "enter group success");
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "enter group fail");
		}
	}
	else if (status == GW_MSG_STATUS_UNBIND)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "unbind success");
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "unbind fail");
		}
	}
	else if (status == GW_MSG_STATUS_QUERY_MEMBER)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			cJSON *members;
			cJSON *tmp;
			int size;
			int idx;
			int pages;
			int totals;
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "query member success");
			data = cJSON_GetObjectItem(root, "data");
			if (data == NULL)
			{
				goto error;
			}
			tmp = cJSON_GetObjectItem(root, "pages");
			if (tmp == NULL || tmp->valueint == 0)
			{
				// normal query
				pages = 0;
			}
			else
			{
				pages = tmp->valueint;
				totals = cJSON_GetObjectItem(root, "total")->valueint;
			}
			members = data;
			if (members != NULL)
			{
				size = cJSON_GetArraySize(members);
				if (size > 0)
				{
					for (idx = 0; idx < size; idx++)
					{
						cJSON *item = cJSON_GetArrayItem(members, idx);
						int uid = cJSON_GetObjectItem(item, "uid")->valueint;
						char *uname = cJSON_GetObjectItem(item, "name")->valuestring;
						int online = cJSON_GetObjectItem(item, "online")->valueint;
						GWLOG_PRINT(GW_LOG_LEVEL_INFO, "user id %d, name %s, online %d", uid, uname, online);
					}
					if (pages == 0)
					{
						GWLOG_PRINT(GW_LOG_LEVEL_INFO, "user count %d", size);
					}
					else
					{
						GWLOG_PRINT(GW_LOG_LEVEL_INFO, "page user count %d,%d", totals, size);
					}
				}
				else
				{
					GWLOG_PRINT(GW_LOG_LEVEL_INFO, "no user");
				}
			}
			else
			{
				GWLOG_PRINT(GW_LOG_LEVEL_INFO, "no user")
			}
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "query member fail %d", sta);
		}
	}
error:
	if (root != NULL)
	{
		cJSON_Delete(root);
	}
}

static int configAddress(PttClient *client)
{
	char serverIp[33];
	const char* hostname = client->platformDns;
	printf("Resolving %s using gethostbyname()...\n", hostname);
	struct hostent* host = gethostbyname(hostname);
	if (host == NULL) {
		printf("gethostbyname failed: %d\n", WSAGetLastError());
		return -1;
	}
	printf("Official name: %s\n", host->h_name);
	printf("Address type: %s\n",host->h_addrtype == AF_INET ? "IPv4" : "IPv6");
	printf("Address length: %d bytes\n", host->h_length);

	// 打印所有IP地址
	char ip[64];
	for (int i = 0; host->h_addr_list[i] != NULL; i++) {
		struct in_addr addr;
		memcpy(&addr, host->h_addr_list[i], sizeof(addr));
		inet_ntop(AF_INET, &addr, ip, sizeof(ip));
		printf("IP %d: %s\n", i + 1, ip);
		strcpy(serverIp, ip);
	}

	GWLOG_PRINT(GW_LOG_LEVEL_INFO, "server ip %s", serverIp);
	pttConfigServer(0, serverIp, client->platformPort);
	return 0;
}

static int netIsConnected()
{
	// check current network status 
	// net ready return 1
	// net error retrun 0
	return 1;
}

static char pttClientMainLoop(PttClient *client, int ms)
{
	int ret;
	struct pt *pt = &client->ptt_pt;

	if (ms != 0)
	{
		if (client->net_search_timeout > ms)
		{
			client->net_search_timeout -= ms;
		}
		else
		{
			client->net_search_timeout = 0;
		}
		if (client->net_ready_timeout > ms)
		{
			client->net_ready_timeout -= ms;
		}
		else
		{
			client->net_ready_timeout = 0;
		}
		if (client->login_timeout > ms)
		{
			client->login_timeout -= ms;
		}
		else
		{
			client->login_timeout = 0;
		}
		if (client->regmsg_timeout > ms)
		{
			client->regmsg_timeout -= ms;
		}
		else
		{
			client->regmsg_timeout = 0;
		}
		if (client->joingrp_timeout > ms)
		{
			client->joingrp_timeout -= ms;
		}
		else
		{
			client->joingrp_timeout = 0;
		}
		if (client->heart_timeout > ms)
		{
			client->heart_timeout -= ms;
		}
		else
		{
			client->heart_timeout = 0;
		}
	}

	if (client->status == PTT_CLIENT_STATUS_OFFLINE)
	{
		GWLOG_PRINT(GW_LOG_LEVEL_INFO, "offline...");
		PT_INIT(pt);
	}

	PT_BEGIN(pt);

	PT_WAIT_UNTIL(pt, (client->status == PTT_CLIENT_STATUS_LOGIN));

LOGIN:
	GWLOG_PRINT(GW_LOG_LEVEL_INFO, "net connect success start logining...");
	ret = configAddress(client);
	if (ret < 0)
	{
		GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "domain or ip is error");
		PT_EXIT(pt);
	}
	pttLogin(pttClient->account, pttClient->password, "12345", "54321");

	client->login_timeout = 30 * TIME_1_SECOND;
	PT_WAIT_UNTIL(pt, (client->status == PTT_CLIENT_STATUS_REGMSG) || (client->login_timeout == 0));
	if (client->login_timeout == 0)
	{
		GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "login fail...");
		goto LOGIN;
	}

	GWLOG_PRINT(GW_LOG_LEVEL_INFO, "login success start register msg...");
REGMSG:
	//pttQueryGroup();
	if (pttClient->message == 1)
	{
		pttRegOfflineMsg(NULL, NULL, 0, 0);
	}
	else
	{
		client->status = PTT_CLIENT_STATUS_JOINGRP;
	}
	client->regmsg_timeout = 10 * TIME_1_SECOND;
	PT_WAIT_UNTIL(pt, (client->status == PTT_CLIENT_STATUS_JOINGRP) || (client->regmsg_timeout == 0));
	if ((client->msgStart == 0 && pttClient->message == 1) || (client->regmsg_timeout == 0))
	{
		GWLOG_PRINT(GW_LOG_LEVEL_WARN, "register msg error");
		client->status = PTT_CLIENT_STATUS_REGMSG;
		goto REGMSG;
	}
	GWLOG_PRINT(GW_LOG_LEVEL_INFO, "register msg success start join group...");
JOINGRP:
	if (client->defaultGrpId != 0)
	{
		pttJoinGroup(client->defaultGrpId, 16);
	}
	else
	{
		GWLOG_PRINT(GW_LOG_LEVEL_WARN, "not default group enter idle");
		client->status = PTT_CLIENT_STATUS_IDLE;
	}
	client->joingrp_timeout = 30 * TIME_1_SECOND;
	PT_WAIT_UNTIL(pt, (client->status == PTT_CLIENT_STATUS_IDLE) || (client->joingrp_timeout == 0));
	if (client->joingrp_timeout == 0)
	{
		GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "join group fail...");
		goto JOINGRP;
	}
	GWLOG_PRINT(GW_LOG_LEVEL_INFO, "join group success...");
	while(1)
	{
		pttHeart(client->battery, client->net);
		GWLOG_PRINT(GW_LOG_LEVEL_INFO, "send heart...");
		client->heart_timeout = client->heart_period * TIME_1_SECOND;
		PT_WAIT_UNTIL(pt, (client->heart_timeout == 0));
	}
	PT_END(pt);
}

static VOID CALLBACK timerCallback(PVOID lpParam, BOOLEAN timerOrWaitFired)
{
	// timer callback
	sendMsgToPttMainThread(PTT_THREAD_TIMER_EVENT_TYPE, 0);
}

static void pttClientMainThread(void *param)
{
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	while(1)
	{
		if (GetMessage(&msg, NULL, 0, 0))
		{
			if (msg.message == WM_USER_MESSAGE)
			{
				switch (msg.wParam)
				{
				case PTT_THREAD_TIMER_EVENT_TYPE:
					pttClientMainLoop(pttClient, TIME_1_SECOND);
					break;
				case PTT_THREAD_NET_READY_EVENT_TYPE:
					pttClient->status = PTT_CLIENT_STATUS_LOGIN;
					pttClientMainLoop(pttClient, 0);
					break;
				case PTT_THREAD_LOGIN_EVENT_TYPE:
					pttClient->status = PTT_CLIENT_STATUS_REGMSG;
					pttClientMainLoop(pttClient, 0);
					break;
				case PTT_THREAD_REG_MSG_EVENT_TYPE:
					pttClient->status = PTT_CLIENT_STATUS_JOINGRP;
					pttClientMainLoop(pttClient, 0);
					break;
				case PTT_THREAD_IDLE_EVENT_TYPE:
					pttClient->status = PTT_CLIENT_STATUS_IDLE;
					pttClientMainLoop(pttClient, 0);
					//updatePeriod(pttClient->heart_period*TIME_1_SECOND);
					break;
				case PTT_THREAD_OFFLINE_EVENT_TYPE:
					pttClient->status = PTT_CLIENT_STATUS_OFFLINE;
					pttClientMainLoop(pttClient, 0);
					//updatePeriod(100*TIME_1_MILLSECOND);
					break;
				default:
					break;
				}
			}
		}
		else
		{
			continue;
		}
	}
}

static void windows_init_winsock(void)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSAStartup failed\n");
		return;
	}
	sendMsgToPttMainThread(PTT_THREAD_NET_READY_EVENT_TYPE, 0);
}

static void initTimer()
{
	HANDLE g_hTimerQueue = NULL;
	HANDLE g_hTimer = NULL;
	g_hTimerQueue = CreateTimerQueue();
	if (!g_hTimerQueue) {
		printf("创建定时器队列失败: %lu\n", GetLastError());
		return;
	}

	// 创建定时器
	if (!CreateTimerQueueTimer(
		&g_hTimer,          // 定时器句柄
		g_hTimerQueue,      // 定时器队列
		timerCallback,      // 回调函数
		NULL,               // 回调参数
		TIME_1_SECOND/1000,               // 延迟时间（毫秒）
		TIME_1_SECOND/1000,               // 间隔时间（毫秒）
		WT_EXECUTEDEFAULT   // 执行标志
	)) {
		printf("创建定时器失败: %lu\n", GetLastError());
		DeleteTimerQueue(g_hTimerQueue);
		return;
	}
}

// simulate audio device
static DWORD record_thread_id = 0;
static char speaking = 0;
void *record_thread(void *param)
{
	FILE *fp = fopen("speaker_16k.pcm", "rb");
	if (fp == NULL)
	{
		return NULL;
	}
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	while (speaking)
	{
		char pcm[640];
		int size;
		size = fread(pcm, 1, 640, fp);
		if (size <= 0)
		{
			break;
		}
		pttOnPcmData(pcm, 640);
		Sleep(10);
	}
	fclose(fp);
	return NULL;
}

static void startRecord()
{
	// start record non block
	speaking = 1;
	CreateThread(
		NULL,                   // 默认安全属性
		3072,             // 堆栈大小
		(LPTHREAD_START_ROUTINE)record_thread,   // 线程入口函数
		NULL,                   // 参数
		0,                      // 创建标志，0表示立即运行
		&record_thread_id);
}

static void stopRecord()
{
	// stop record non block
	speaking = 0;
}

static void startPlay()
{
	// start play non block
}

static void stopPlay()
{
	// stop play non block
}

void write_to_file(char *data, int len)
{
	FILE *fp = fopen("play.pcm", "ab");

	fwrite(data, 1, len, fp);

	fclose(fp);
}

static void playbackPlayData(char *pcm, int len)
{
	// write wav data to play device
	printf("recv play data %d\n", len);
	write_to_file(pcm, len);

}

static void muteSpeaker(char mute)
{

}

static void muteRecorder(char mute)
{
}

static GWPttAudioModule pttAudioDevice = {
		.startPlay = startPlay,
		.stopPlay = stopPlay,
		.playData = playbackPlayData,
		.startRecord = startRecord,
		.stopRecord = stopRecord,
		.muteSpeaker = muteSpeaker,
		.muteRecorder = muteRecorder,
};

// ptt client api
void pttClientStart(const char *account, const char *password, const char *dns, int port)
{
	HANDLE th = NULL;

	printf("GWPttClient Start...\n");

	initTimer();

	char *version = NULL;
	version = pttGetVersion();
	printf("gw ptt version %s\n", version);

	pttInit(onGWPttEvent, onGWMsgEvent, &pttAudioDevice, 0, GW_PTT_ENCODE_LEVEL_HIGH, 0);

	pttClient = (PttClient*)malloc(sizeof(PttClient));
	assert(pttClient != NULL);
	memset(pttClient, 0, sizeof(PttClient));
	pttClient->status = PTT_CLIENT_STATUS_NONE;
	pttClient->heart_period = PTT_CLIENT_DEFAULT_HEART_PERIOD;
	pttClient->local_zone = 8; // beijing time
	pttClient->local_zone_dec = 0;
	strcpy(pttClient->account, account);
	strcpy(pttClient->password, password);
	strcpy(pttClient->platformDns, dns);
	pttClient->platformPort = port;
	pttClient->battery = 100;
	strcpy(pttClient->net, "net");

	th = CreateThread(
		NULL,                   // 默认安全属性
		5*1024,             // 堆栈大小
		(LPTHREAD_START_ROUTINE)pttClientMainThread,   // 线程入口函数
		NULL,                   // 参数
		0,                      // 创建标志，0表示立即运行
		&ptt_thread_id              // 线程ID
	);
	assert(th != NULL);

	Sleep(1000);
	windows_init_winsock();
}

void pttClientQueryGroup(int pageNum, int pageSize)
{
	if (pageNum != 0 && pageSize != 0)
	{
		pttQueryGroupByPage(pageSize, pageNum);
	}
	else
	{
		pttQueryGroup();
	}
}

void pttClientQueryMember(int gid, int online, int pageNum, int pageSize)
{
	if (pageNum != 0 && pageSize != 0)
	{
		pttQueryMemberByPage(gid, 16, online, pageSize, pageNum);
	}
	else
	{
		pttQueryMember(gid, 16);
	}
}

void pttClientSpeakStart()
{
	pttSpeak(GW_PTT_SPEAK_START, 0);
}

void pttClientSpeakEnd()
{
	pttSpeak(GW_PTT_SPEAK_END, 0);
}

void pttClientCreateGroup()
{
	pttGroupCreate("");
}

void pttClientGeneralGroupToken(int gid)
{
	pttGroupGeneralToken(&gid, 1);
}

void pttClientEnterGroup(int token, int type)
{
	pttGroupEnterByToken(token, type);
}

void pttClientExitGroup(int gid)
{
	pttGroupExit(gid);
}

void pttClientDeleteGroup(int gid)
{
	pttGroupDelete(gid);
}

void pttClientJoinGroup(int gid)
{
	pttJoinGroup(gid, 16);
}

