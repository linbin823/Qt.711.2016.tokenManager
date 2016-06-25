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

typedef enum {
    peerDisable = 0x01,
    tokenAvailable,
    tokenNOTAvailable,
    tokenTakeOutPending,//本机主动把令牌给其他peer，但其他peer未确认
    tokenOrderOutPending,//其他peer要求本机令牌，本机未确认
    tokenTakeInPending,//本机主动要获得令牌，但令牌持有peer未确认
    tokenOrderInPending //令牌持有peer要求把令牌传送给本机，但本机未确认

} peerState_e;

typedef enum {
    noError  = 0x01,
    tokenLost, //网络上没有令牌
    tokenMoreThanOne,
    clientError,
    serverError,
    takeOutOverTime,
    takeInOverTime,
    OrderOutOverTime,
    OrderInOverTime

} peerErrorState_e;

typedef struct {

    bool tokenGenerator;//peer是令牌产生者，整个内网上必须有且唯一
    QString peerIp;
    QString peerName;
    peerState_e state;
    peerErrorState_e errorState;

} peerInfo_t;

class tokenManager : public QObject
{
    Q_OBJECT
public:
    explicit tokenManager(QObject *parent = 0);
    ~tokenManager();

//get & set
    peerState_e getPeerState() const{
        return selfInfo->state;
    }
    peerErrorState_e getPeerErrorState() const{
        return selfInfo->errorState;
    }

    QString getMasterPeerMessage() const{
        return masterPeerMessage;
    }
    void setMasterPeerMessage(QString& msg){
        masterPeerMessage = msg;
        emit masterPeerMessageUpdated(msg);
    }

    int setPeerInfo(peerInfo_t& newone);
    int setPeersInfo(QList<peerInfo_t*>& newlist);

    QList<peerInfo_t*> getPeersInfo() const{
        return pPeersList;
    }

    int setSelfInfo(peerInfo_t& self);
    peerInfo_t getSelfInfo() const{
        return *selfInfo;
    }
    peerInfo_t generateSelfInfo();

    int setTargetInfo(peerInfo_t& target);
    peerInfo_t getTargetInfo() const{
        return *targetInfo;
    }

    int selfEnable();
    int selfDisable();

//token take out
public:
    int tokenTakeOut(peerInfo_t& target,int overtime=10000);
    int tokenTakeOutCancel();
public slots:
    void tokenTakeOutOvertime();
private:
    int tokenTakeOutAck();

//token take in
public:
    int tokenTakeIn(peerInfo_t& source, int overtime=10000);
    int tokenTakeInCancel();
public slots:
    void tokenTakeInOvertime();
private:
    int tokenTakeInAck();

//token order out
public:
    int tokenOrderOutAck();
    int tokenOrderOutCancel();
public slots:
    void tokenOrderOutOvertime();
private:
    int tokenOrderOut(peerInfo_t& source, int overtime=0);

//token order in
public:
    int tokenOrderInAck();
    int tokenOrderInCancel();
public slots:
    void tokenOrderInOvertime();
private:
    int tokenOrderIn(peerInfo_t& source, int overtime=0);


signals:
    void masterPeerMessageUpdated(QString msg);
    void peerStateChanged(peerState_e& state);
    void peerErrorStateChanged(peerErrorState_e& state);

private:
    peerInfo_t* selfInfo;//本peer信息
    peerInfo_t* targetInfo;//目标peer信息
    QList<peerInfo_t*> pPeersList;//网络上所有peer信息

    QString masterPeerMessage;
    QUdpSocket* clientSocket;
    QUdpSocket* serverSocket;
    QTimer tokenTakeOutOvertimer, tokenTakeInOvertimer, tokenOrderOutOvertimer, tokenOrderInOvertimer;

    int clearPeerInfo();
    int port = 10452;
};

#endif // TOKENMANAGER_H
