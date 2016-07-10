//p2p形式，每个peer都是server和client的结合
//通过udp广播，在内网上管理和传递唯一的令牌

#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include <QObject>
#include <QList>
#include <QUdpSocket>
#include <QTime>
#include <QTimer>
#include <QNetworkInterface>
#include "sierrmsg.h"

#define tmPort 10452

typedef enum {
    tmPeerDisable = 1,       //peer禁用
    tmPeerOnlinewithToken,      //peer在线有令牌，主站
    tmPeerOnlinewithoutToken,   //peer在线无令牌，从站
    tmTokenTakeOutPending,      //该peer主动把令牌给其他peer，但其他peer未确认。该peer具有令牌，主站。
    tmTokenOrderOutPending,     //其他peer要求该peer的令牌，该peer未确认。该peer具有令牌，主站。
    tmTokenTakeInPending,       //该peer主动要获得令牌，但令牌持有peer未确认。该peer无令牌，从站。
    tmTokenOrderInPending,      //令牌持有peer要求把令牌传送给该peer，但该peer未确认。该peer无令牌，从站。
    tmPeerOffline,              //peer离线。此状态不适用于self，用于判断其他peer的掉线状态。

} tmPeerState_e;

typedef enum {
    tmNoError  = 1,
    tmTokenLost,                      //网络上没有令牌
    tmTokenMoreThanOne,
    tmInnerError,
    tmTakeOutOverTime,
    tmTakeInOverTime,
    tmOrderOutOverTime,
    tmOrderInOverTime,
    tmNetworkError

} tmPeerErrorState_e;

typedef struct {

    quint32 peerIp;                 //peer的IP地址，类型quint32
    QString peerName;               //peer名称，类型QString，长度小于255bytes
    tmPeerState_e state;            //peer状态，枚举
    tmPeerErrorState_e errorState;  //peer错误状态，枚举
    int priority;                   //peer优先级，int。全网blkout后，优先级值最大的自动产生令牌。
    QTime lastUpdateTime;


} tmPeerInfo_t;






class tmTokenManager : public QObject/*, public siErrMsg*/
{
    Q_OBJECT
public:
    explicit tmTokenManager(QObject *parent = 0);
    ~tmTokenManager();
//    enum errorType : quint64 {
//        tmNoError           = 0x0000000000000001ULL,
//        tmTokenLost         = 0x0000000000000002ULL,                      //网络上没有令牌
//        tmTokenMoreThanOne  = 0x0000000000000004ULL,
//        tmInnerError        = 0x0000000000000008ULL,
//        tmTakeOutOverTime   = 0x0000000000000010ULL,
//        tmTakeInOverTime    = 0x0000000000000020ULL,
//        tmOrderOutOverTime  = 0x0000000000000040ULL,
//        tmOrderInOverTime   = 0x0000000000000080ULL,
//        tmNetworkError      = 0x0000000000000100ULL,
//    };

public:
    //token take out
    Q_INVOKABLE int tokenTakeOut(QString target, qint32 overtime=10000);
    Q_INVOKABLE int tokenTakeOut(int index, qint32 overtime=10000);
    Q_INVOKABLE int tokenTakeOutCancel();
    //token take in
    Q_INVOKABLE int tokenTakeIn(QString target,qint32 overtime=10000);
    Q_INVOKABLE int tokenTakeIn(int index,qint32 overtime=10000);
    Q_INVOKABLE int tokenTakeInCancel();
    //token order out
    Q_INVOKABLE int tokenOrderOutAck();
    Q_INVOKABLE int tokenOrderOutCancel();
    //token order in
    Q_INVOKABLE int tokenOrderInAck();
    Q_INVOKABLE int tokenOrderInCancel();
    //token take in forced
    Q_INVOKABLE int tokenForceTakeIn();
public slots:
    Q_INVOKABLE void tokenTakeOutOvertime();
    Q_INVOKABLE void tokenTakeInOvertime();
    Q_INVOKABLE void tokenOrderOutOvertime();
    Q_INVOKABLE void tokenOrderInOvertime();
private:
    int tokenTakeOutAck();
    int tokenTakeInAck();
    int tokenOrderOut(qint32 overtime=0);
    int tokenOrderIn(qint32 overtime=0);
    //token Order out forced
    int tokenForceOrderOut();

public:
    //token manager control
    Q_INVOKABLE int start(void);//新建端口，初始化网络连接，自身状态设置为online（启动失败除外）
    Q_INVOKABLE int stop(void);//删除端口，终止网络连接，自身状态设为Disable
    Q_INVOKABLE int restart(void);//重启

    Q_INVOKABLE bool isWithToken(int index=0);//判断是否有令牌，有=true
    Q_INVOKABLE bool isStarted();//判断本peer是否启动
    Q_INVOKABLE bool isFirstPriority();//判断本peer是否最高优先级。
    Q_INVOKABLE void checkPriority();//检查优先等级，发现相同优先级则ip地址最后一段较小的peer的优先级++

//others
private:

    QList<tmPeerInfo_t*> pPeersList;//网络上所有peer信息

    QString masterPeerMessage;
    QUdpSocket* clientSocket;
    QUdpSocket* serverSocket;
    QTimer tokenTakeOutOvertimer, tokenTakeInOvertimer, tokenOrderOutOvertimer, tokenOrderInOvertimer;
    QTimer heartBeatTimer,offlineCheckTimer,tokenCheckTimer;
    QTime tokenErrorDelayTime;


    QByteArray datagramReadParameter(QByteArray & data, int *begin); //读取报文
    int offlineDelay;
    int offlineCheckInterv;
    int heartbeatInterv;
    int tokenErrorDelay;
    int tokenCheckInterv;

    int findToken();//找到令牌所在peer的index，找不到则返回-1，多于1个则返回-2，同时修改报警状态
protected:
    int clearPeerInfo();//清空列表删除内存

private slots:
    void processPendingDatagrams();
    void heartBeatSender();
    void offlineCheck();
    void tokenCheck();

signals:
    void masterPeerMessageUpdated();
    void selfStateChanged();
    void selfErrorStateChanged();
    void peersListChanged(int index);
    void tmOtherCommandReceived(QByteArray msg);
    void tmOperationMessage(QString msg);

//get & set
public:

    Q_INVOKABLE int setSelfInfo(tmPeerInfo_t* self);
    Q_INVOKABLE inline tmPeerInfo_t* getSelfInfo() const;

    Q_INVOKABLE QString getSelfStateString() const;
    Q_INVOKABLE QString getSelfErrorStateString() const;

    Q_INVOKABLE QString getMasterPeerMessage() const;
    Q_INVOKABLE void setMasterPeerMessage(QString& msg);

    Q_INVOKABLE tmPeerInfo_t* getPeerInfo(QString& name) const;
    Q_INVOKABLE tmPeerInfo_t* getPeerInfo(int index) const;

    Q_INVOKABLE tmPeerInfo_t generateSelfInfo();

    Q_INVOKABLE QString getStateString(int index) const;
    Q_INVOKABLE QString getErrorStateString(int index) const;

    Q_INVOKABLE int setPeersInfo(QList<tmPeerInfo_t*>& newlist);//for save
    Q_INVOKABLE QList<tmPeerInfo_t*> getPeersInfo() const;//for load

private:
    int setSelfState(tmPeerState_e newState);
    int setSelfErrorState(tmPeerErrorState_e newErrorState);

    int setPeerInfo(tmPeerInfo_t* newone);

    int setPeerState(int index, tmPeerState_e newState);
    int setPeerErrorState(int index, tmPeerErrorState_e newErrorState);
protected:
    int setPartnerInfo(tmPeerInfo_t* target);
    inline tmPeerInfo_t* getPartnerInfo() const;

    inline int getPeerListSize();
};

#endif // TOKENMANAGER_H
