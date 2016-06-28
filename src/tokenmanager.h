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
    peerDisable = 0x01,       //peer禁用
    peerOnlinewithToken,      //peer在线有令牌，主站
    peerOnlinewithoutToken,   //peer在线无令牌，从站
    tokenTakeOutPending,      //该peer主动把令牌给其他peer，但其他peer未确认。该peer具有令牌，主站。
    tokenOrderOutPending,     //其他peer要求该peer的令牌，该peer未确认。该peer具有令牌，主站。
    tokenTakeInPending,       //该peer主动要获得令牌，但令牌持有peer未确认。该peer无令牌，从站。
    tokenOrderInPending,      //令牌持有peer要求把令牌传送给该peer，但该peer未确认。该peer无令牌，从站。
    peerOffline,              //peer离线。此状态不适用于self，用于判断其他peer的掉线状态。

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

    quint32 peerIp;                 //peer的IP地址，类型quint32
    QString peerName;               //peer名称，类型QString，长度小于255bytes
    tmPeerState_e state;            //peer状态，枚举
    tmPeerErrorState_e errorState;  //peer错误状态，枚举
    int tokenGeneratorPriority;     //peer令牌生产优先级，int。全网blkout后，优先级值最大的自动产生令牌
    QTime lastUpdateTime;


} tmPeerInfo_t;






class tmTokenManager : public QObject
{
    Q_OBJECT
public:
    explicit tmTokenManager(QObject *parent = 0);
    ~tmTokenManager();

public:
    //token take out
    int tokenTakeOut(tmPeerInfo_t& target,int overtime=10000);
    int tokenTakeOutCancel();
    //token take in
    int tokenTakeIn(tmPeerInfo_t& source, int overtime=10000);
    int tokenTakeInCancel();
    //token order out
    int tokenOrderOutAck();
    int tokenOrderOutCancel();
    //token order in
    int tokenOrderInAck();
    int tokenOrderInCancel();
    //token take in forced
    int tokenForceTakeIn();
public slots:
    void tokenTakeOutOvertime();
    void tokenTakeInOvertime();
    void tokenOrderOutOvertime();
    void tokenOrderInOvertime();
private:
    int tokenTakeOutAck();
    int tokenTakeInAck();
    int tokenOrderOut(tmPeerInfo_t& source, int overtime=0);
    int tokenOrderIn(tmPeerInfo_t& source, int overtime=0);

public:
    //token manager control
    int start(void);//新建端口，初始化网络连接，自身状态设置为online（启动失败除外）
    int stop(void);//删除端口，终止网络连接，自身状态设为Disable
    int restart(void);//重启

    bool isWithToken();//判断是否有令牌，有=true

//others
private:

    tmPeerInfo_t* selfInfo;//本peer信息
    tmPeerInfo_t* targetInfo;//目标peer信息
    QList<tmPeerInfo_t*> pPeersList;//网络上所有peer信息

    QString masterPeerMessage;
    QUdpSocket* clientSocket;
    QUdpSocket* serverSocket;
    QTimer tokenTakeOutOvertimer, tokenTakeInOvertimer, tokenOrderOutOvertimer, tokenOrderInOvertimer, heartBeatTimer;

    int clearPeerInfo();//清空列表删除内存

private slots:
    void processPendingDatagrams();
    void heartBeatSender();

signals:
    void masterPeerMessageUpdated(QString& msg);
    void selfStateChanged(tmPeerState_e& state);
    void selfErrorStateChanged(tmPeerErrorState_e& state);
    void tmOtherCommandReceived(QString& msg);

//get & set
public:
    //get & set
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

    int setSelfEnable();
    int setSelfDisable();

private:
    //get & set
    void setSelfState(tmPeerState_e newState){
        selfInfo->state = newState;
        emit selfStateChanged(selfInfo->state);
    }
    void setSelfErrorState(tmPeerErrorState_e newErrorState){
        selfInfo->errorState = newErrorState;
        emit selfErrorStateChanged(selfInfo->errorState);
    }


};

#endif // TOKENMANAGER_H
