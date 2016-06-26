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

#define tmPort 10452

typedef enum {
    peerDisable = 0x01,
    peerOnlinewithToken,
    peerOnlinewithoutToken,
    peerOffline,
    tokenTakeOutPending,//本机主动把令牌给其他peer，但其他peer未确认
    tokenOrderOutPending,//其他peer要求本机令牌，本机未确认
    tokenTakeInPending,//本机主动要获得令牌，但令牌持有peer未确认
    tokenOrderInPending, //令牌持有peer要求把令牌传送给本机，但本机未确认


} tmPeerState_e;

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

} tmPeerErrorState_e;

typedef struct {

    bool tokenGenerator;//peer是令牌产生者，整个内网上必须有且唯一
    QString peerIp;
    QString peerName;
    tmPeerState_e state;
    tmPeerErrorState_e errorState;

} tmPeerInfo_t;

class tmTokenManager : public QObject
{
    Q_OBJECT
public:
    explicit tmTokenManager(QObject *parent = 0);
    ~tmTokenManager();

//get & set
    tmPeerState_e getPeerState() const{
        return selfInfo->state;
    }
    tmPeerErrorState_e getPeerErrorState() const{
        return selfInfo->errorState;
    }

    QString getMasterPeerMessage() const{
        return masterPeerMessage;
    }
    void setMasterPeerMessage(QString& msg){
        masterPeerMessage = msg;
        emit masterPeerMessageUpdated(msg);
    }

    int setPeerInfo(tmPeerInfo_t& newone);
    int setPeersInfo(QList<tmPeerInfo_t*>& newlist);

    QList<tmPeerInfo_t*> getPeersInfo() const{
        return pPeersList;
    }

    int setSelfInfo(tmPeerInfo_t& self);
    tmPeerInfo_t getSelfInfo() const{
        return *selfInfo;
    }
    tmPeerInfo_t generateSelfInfo();

    int setTargetInfo(tmPeerInfo_t& target);
    tmPeerInfo_t getTargetInfo() const{
        return *targetInfo;
    }

    int selfEnable();
    int selfDisable();

//token take out
public:
    int tokenTakeOut(tmPeerInfo_t& target,int overtime=10000);
    int tokenTakeOutCancel();
public slots:
    void tokenTakeOutOvertime();
private:
    int tokenTakeOutAck();

//token take in
public:
    int tokenTakeIn(tmPeerInfo_t& source, int overtime=10000);
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
    int tokenOrderOut(tmPeerInfo_t& source, int overtime=0);

//token order in
public:
    int tokenOrderInAck();
    int tokenOrderInCancel();
public slots:
    void tokenOrderInOvertime();
private:
    int tokenOrderIn(tmPeerInfo_t& source, int overtime=0);


signals:
    void masterPeerMessageUpdated(QString msg);
    void peerStateChanged(tmPeerState_e& state);
    void peerErrorStateChanged(tmPeerErrorState_e& state);

private:
    tmPeerInfo_t* selfInfo;//本peer信息
    tmPeerInfo_t* targetInfo;//目标peer信息
    QList<tmPeerInfo_t*> pPeersList;//网络上所有peer信息

    QString masterPeerMessage;
    QUdpSocket* clientSocket;
    QUdpSocket* serverSocket;
    QTimer tokenTakeOutOvertimer, tokenTakeInOvertimer, tokenOrderOutOvertimer, tokenOrderInOvertimer;

    int clearPeerInfo();

};

#endif // TOKENMANAGER_H
