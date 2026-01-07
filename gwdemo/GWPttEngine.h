/*
 * GWEngine.h
 *
 *  Created on: 2024年5月24日
 *      Author: hejingsheng
 */

#ifndef GWSD_GWPTTENGINE_H
#define GWSD_GWPTTENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "GWPttAudioDevice.h"
#include "gwsd_ptt.h"

typedef void (*print_log_cb)(char *log);

int pttInit(GWPttEvent cb, GWMsgEvent cb1, GWPttAudioModule *audioModule, char externalCodec, int encodeLevel, int framesize);

int pttSetLogCallback(print_log_cb cb);

int pttSetQueuePath(const char *path);

char *pttGetVersion(void);

unsigned int pttGetTime(void);

int pttConfigServer(int type, char *host, int port);

int pttSaveVoice(int open, char *path);

int pttNetCheck(int type, char *host, int port);

int pttLogin(const char *account, const char *pass, const char *imei, const char *iccid);

int pttQueryGroup(void);

int pttQueryGroupByPage(int pageSize, int pageNum);

int pttJoinGroup(int gid, int type);

int pttQueryMember(int gid, int type);

int pttQueryMemberByPage(int gid, int type, int online, int pageSize, int pageNum);

int pttQueryLastMsg(int gid);

int pttQueryDispatcher(int gid, int type);

int pttTempGroup(int *uids, int num);

int pttDuplexCall(int uid, int action);

int pttGroupCreate(const char *gname);

int pttGroupCreateWithParam(const char *prefix,const char *gname,char inviteMg);

int pttGroupAddUser(int gid, int num, int *uids);

int pttGroupDelUser(int gid, int num, int *uids);

int pttGroupDelete(int gid);

int pttGroupRename(int gid, const char *gname);

int pttGroupExit(int gid);

int pttGroupEnter(int gid);

int pttGroupGeneralToken(int *gids, int num);

int pttGroupEnterByToken(int token, int type);

int pttGetManagerInfo(void);

int pttUnbind(void);

int pttSpeakStartDmr(char type, int id, long long ms);

int pttSpeak(int action, long long ms);

int pttRegOfflineMsg(int groups[], int types[], int num, char security);

int pttRegOnlineMsg(int groups[], int types[], int num, char security);

int pttSendMsg(int sid, const char *snm, int type, int id, const char *rnm, int msgType, const char *content, const char *thumburl, int time);

int pttReportLocationGps(double lat, double lon,int type,int uid);

int pttReportLocationCell(int cellid, int lac, int mode, char *mcc, char *mnc,int type,int uid);

int pttReportLocationWifi(char *bssid, int signal, char *ssid);

int pttGetWeather(int cellid, int lac, int mode, char *mcc, char *mnc);

int pttGetLocation(int cellid, int lac, int mode, char *mcc, char *mnc);

int pttPatrol(int id, char *tag, int result, char *desc, int taskId, char *taskNm, char *attach, int uid);

int pttFileUpload(int filetype, char *name, int uid, char *url, char *thumburl, char *desc);

int pttQueryTmpGrp(int type);

int pttHeart(int battery, const char *net);

int pttSendSos(int sid, const char *snm, int id, long long ms, double lat, double lon, char hasLoc);

int pttSendSelfMsg(int id, int type, char *selfmsg, char offline);

int pttVoiceToggle(int dir, int open);

int pttLogout(void);

int pttOnPcmData(char *pcm, int len);

int pttOnEncodeData(char *enc, int len);

int pttUpdateLeftVoicePacket(int num);

int pttAddFriend(int uid);

int pttQueryFriends(int type);

int pttDelFriend(int uid);

int pttAcceptAddFriend(int uid);

int pttRejectAddFriend(int uid);

int pttQueryChatGroups(int uid);

int pttQueryGroupDetail(int gid, int type);

int pttActiveSession(int sid, int type);

int pttDeactiveSession(int sid, int type);

int pttDeleteSession(int sid, int type);

int pttQueryMsgDetail(int gid, int type, char *msgid);

int pttEnterAI(int role, char hasloc, double lat, double lon);

int pttConfigAI(int role, char hasloc, double lat, double lon);

int pttAIPriority(int pri);

int pttAISpeak(int action);

int pttAICancel(void);

int pttExitAI(void);

#ifdef __cplusplus
}
#endif

#endif /* GWSD_GWPTTENGINE_H */
