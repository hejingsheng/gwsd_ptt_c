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

__declspec(dllexport) int pttSetCustomMalloc(memory_allocate alloc);

__declspec(dllexport) int pttInit(GWPttEvent cb, GWMsgEvent cb1, GWPttAudioModule *audioModule, char externalCodec, int encodeLevel, int framesize);

__declspec(dllexport) int pttSetLogCallback(print_log_cb cb);

__declspec(dllexport) int pttSetQueuePath(const char *path);

__declspec(dllexport) char *pttGetVersion(void);

__declspec(dllexport) unsigned int pttGetTime(void);

__declspec(dllexport) int pttConfigServer(int type, char *host, int port);

__declspec(dllexport) int pttSaveVoice(int open, char *path);

__declspec(dllexport) int pttNetCheck(int type, char *host, int port);

__declspec(dllexport) int pttLogin(const char *account, const char *pass, const char *imei, const char *iccid);

__declspec(dllexport) int pttQueryGroup(void);

__declspec(dllexport) int pttQueryGroupByPage(int pageSize, int pageNum);

__declspec(dllexport) int pttJoinGroup(int gid, int type);

__declspec(dllexport) int pttQueryMember(int gid, int type);

__declspec(dllexport) int pttQueryMemberByPage(int gid, int type, int online, int pageSize, int pageNum);

__declspec(dllexport) int pttQueryLastMsg(int gid);

__declspec(dllexport) int pttQueryDispatcher(int gid, int type);

__declspec(dllexport) int pttTempGroup(int *uids, int num);

__declspec(dllexport) int pttDuplexCall(int uid, int action);

__declspec(dllexport) int pttGroupCreate(const char *gname);

__declspec(dllexport) int pttGroupCreateWithParam(const char *prefix,const char *gname,char inviteMg);

__declspec(dllexport) int pttGroupAddUser(int gid, int num, int *uids);

__declspec(dllexport) int pttGroupDelUser(int gid, int num, int *uids);

__declspec(dllexport) int pttGroupDelete(int gid);

__declspec(dllexport) int pttGroupRename(int gid, const char *gname);

__declspec(dllexport) int pttGroupExit(int gid);

__declspec(dllexport) int pttGroupEnter(int gid);

__declspec(dllexport) int pttGroupGeneralToken(int *gids, int num);

__declspec(dllexport) int pttGroupEnterByToken(int token, int type);

__declspec(dllexport) int pttGetManagerInfo(void);

__declspec(dllexport) int pttUnbind(void);

__declspec(dllexport) int pttSpeakStartDmr(char type, int id, long long ms);

__declspec(dllexport) int pttSpeak(int action, long long ms);

__declspec(dllexport) int pttRegOfflineMsg(int groups[], int types[], int num, char security);

__declspec(dllexport) int pttRegOnlineMsg(int groups[], int types[], int num, char security);

__declspec(dllexport) int pttSendMsg(int sid, const char *snm, int type, int id, const char *rnm, int msgType, const char *content, const char *thumburl, int time);

__declspec(dllexport) int pttReportLocationGps(double lat, double lon,int type,int uid);

__declspec(dllexport) int pttReportLocationCell(int cellid, int lac, int mode, char *mcc, char *mnc,int type,int uid);

__declspec(dllexport) int pttReportLocationWifi(char *bssid, int signal, char *ssid);

__declspec(dllexport) int pttGetWeather(int cellid, int lac, int mode, char *mcc, char *mnc);

__declspec(dllexport) int pttGetLocation(int cellid, int lac, int mode, char *mcc, char *mnc);

__declspec(dllexport) int pttPatrol(int id, char *tag, int result, char *desc, int taskId, char *taskNm, char *attach, int uid);

__declspec(dllexport) int pttFileUpload(int filetype, char *name, int uid, char *url, char *thumburl, char *desc);

__declspec(dllexport) int pttQueryTmpGrp(int type);

__declspec(dllexport) int pttHeart(int battery, const char *net);

__declspec(dllexport) int pttSendSos(int sid, const char *snm, int id, long long ms, double lat, double lon, char hasLoc);

__declspec(dllexport) int pttSendSelfMsg(int id, int type, char *selfmsg, char offline);

__declspec(dllexport) int pttVoiceToggle(int dir, int open);

__declspec(dllexport) int pttLogout(void);

__declspec(dllexport) int pttOnPcmData(char *pcm, int len);

__declspec(dllexport) int pttOnEncodeData(char *enc, int len);

__declspec(dllexport) int pttUpdateLeftVoicePacket(int num);

__declspec(dllexport) int pttAddFriend(int uid);

__declspec(dllexport) int pttQueryFriends(int type);

__declspec(dllexport) int pttDelFriend(int uid);

__declspec(dllexport) int pttAcceptAddFriend(int uid);

__declspec(dllexport) int pttRejectAddFriend(int uid);

__declspec(dllexport) int pttQueryChatGroups(int uid);

__declspec(dllexport) int pttQueryGroupDetail(int gid, int type);

__declspec(dllexport) int pttActiveSession(int sid, int type);

__declspec(dllexport) int pttDeactiveSession(int sid, int type);

__declspec(dllexport) int pttDeleteSession(int sid, int type);

__declspec(dllexport) int pttQueryMsgDetail(int gid, int type, char *msgid);

__declspec(dllexport) int pttEnterAI(int role, char hasloc, double lat, double lon);

__declspec(dllexport) int pttConfigAI(int role, char hasloc, double lat, double lon);

__declspec(dllexport) int pttAIPriority(int pri);

__declspec(dllexport) int pttAISpeak(int action);

__declspec(dllexport) int pttAICancel(void);

__declspec(dllexport) int pttExitAI(void);

__declspec(dllexport) int pttConfigAgeGain(int db);

__declspec(dllexport) int pttConfigAgeBalance(int balance);

#ifdef __cplusplus
}
#endif

#endif /* GWSD_GWPTTENGINE_H */
