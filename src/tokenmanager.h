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
    tokenTakeOutPending,//本机主动把令牌给其他peer，但其他peer未确认
    tokenOrderOutPending,//其他peer要求本机令牌，本机未确认
    tokenTakeInPending,//本机主动要获得令牌，但令牌持有peer未确认
    tokenOrderInPending, //令牌持有peer要求把令牌传送给本机，但本机未确认

    peerOffline,//本状态不适用于self peer，掉线状态


} tmPeerState_e;

typedef enum {
    noError  = 0x01,
    tokenLost, //网络上没有令牌
    tokenMoreThanOne,
    innerError,
    takeOutOverTime,
    takeInOverTime,
    OrderOutOverTime,
    OrderInOverTime

} tmPeerErrorState_e;

typedef struct {

    QString peerIp;
    QString peerName;
    tmPeerState_e state;
    tmPeerErrorState_e errorState;
    int tokenGeneratorPriority;//本peer令牌产生的优先级。全网blkout后，优先级值最大的自动产生令牌

} tmPeerInfo_t;






class tmTokenManager : public QObject
{
    Q_OBJECT
public:
    explicit tmTokenManager(QObject *parent = 0);
    ~tmTokenManager();

signals:
    void masterPeerMessageUpdated(QString msg);
    void selfStateChanged(tmPeerState_e& state);
    void selfErrorStateChanged(tmPeerErrorState_e& state);

//get & set
private:
    void setSelfState(tmPeerState_e newState){
        selfInfo->state = newState;
        emit selfStateChanged(selfInfo->state);
    }
    void setSelfErrorState(tmPeerErrorState_e newErrorState){
        selfInfo->errorState = newErrorState;
        emit selfErrorStateChanged(selfInfo->errorState);
    }
public:
    tmPeerState_e getSelfState() const{
        return selfInfo->state;
    }
    tmPeerErrorState_e getSelfErrorState() const{
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

    int tmTokenManager::setSelfEnable(){
        if(selfInfo->errorState != innerError){
            setSelfState( peerOnlinewithoutToken );
            return -1;
        }
        else setSelfState( peerDisable );
        return 0;
    }

    int tmTokenManager::setSelfDisable(){
        setSelfState( peerDisable );
        return 0;
    }


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


//others
public:
    int start(void);//新建端口，初始化网络连接，自身状态设置为online（启动失败除外）
    int stop(void);//删除端口，终止网络连接，自身状态设为Disable
    int restart(void);//重启

private:
    tmPeerInfo_t* selfInfo;//本peer信息
    tmPeerInfo_t* targetInfo;//目标peer信息
    QList<tmPeerInfo_t*> pPeersList;//网络上所有peer信息

    QString masterPeerMessage;
    QUdpSocket* clientSocket;
    QUdpSocket* serverSocket;
    QTimer tokenTakeOutOvertimer, tokenTakeInOvertimer, tokenOrderOutOvertimer, tokenOrderInOvertimer;

    int clearPeerInfo();

private slots:
    void processPendingDatagrams();
    void broadcastDatagram();
};

#endif // TOKENMANAGER_H
