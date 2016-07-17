#include "tmpeer.h"

tmPeer::tmPeer(QObject *parent) : baseDevice(parent)
{
    QTime time = QTime::currentTime();
    qsrand(time.msec());
    peerName = QString::fromLocal8Bit("默认peer名称_%1").arg(qrand());
    peerIp = 0;
    priority = 0;
    lastUpdateTime.start();
    resetError(0xFFFFFFFFFFFFFFFF);
    setState(stateDisable);

    offlineDelay = 30 * 1000;       //default = 30sec
    offlineCheckInterv = 1* 1000;   //default = 1sec

    offlineCheckTimer.setSingleShot(false);
    offlineCheckTimer.setInterval(offlineCheckInterv);
    connect(&offlineCheckTimer,SIGNAL(timeout()),this,SLOT(offlineCheck()));
    offlineCheckTimer.start();//一直都会检查超时

    connect(this,SIGNAL(msgStateChanged(quint64)),this,SLOT(peerChanged()));
    connect(this,SIGNAL(msgErrorSet(quint64)),this,SLOT(peerChanged()));
    connect(this,SIGNAL(msgErrorReset(quint64)),this,SLOT(peerChanged()));
    connect(this,SIGNAL(msgNameChanged(QString)),this,SLOT(peerChanged()));
    connect(this,SIGNAL(msgIpChanged(qint32)),this,SLOT(peerChanged()));
    connect(this,SIGNAL(msgPriorityChanged(qint32)),this,SLOT(peerChanged()));
}

void tmPeer::setPeerName(QString& name){
    if(name.isEmpty()) return;
    if(peerName != name){
        peerName = name;
        emit msgNameChanged(name);
        //qDebug()<<"newName"<<peerName;
    }
}

void tmPeer::setPeerPriority(quint32 pri){
    if(pri<=0 ) return;
    if(priority != pri){
        priority = pri;
        emit msgPriorityChanged(pri);
        //qDebug()<<"newPriority"<<peerName<<pri;
    }
}

void tmPeer::setPeerIp(quint32 ip){
    if(ip<=0 ) return;
    if(peerIp != ip){
        peerIp = ip;
        emit msgIpChanged(ip);
        //qDebug()<<"newIp"<<peerName<<ip;
    }
}

QString tmPeer::getPeerName() const{
    return peerName;
}
quint32 tmPeer::getPeerPriority() const{
    return priority;
}
quint32 tmPeer::getPeerIp() const{
    return peerIp;
}


QString tmPeer::getErrorString(quint64 errorCode, int lang) const{
    //qDebug()<<"tmPeer::getErrorString 1#"<<errorCode;
    switch(errorCode){
        case errorInnerError:
            //qDebug()<<"tmPeer::getErrorString 2#";
            return QString::fromLocal8Bit("内部错误");
        case errorTakeOutOverTime:
            return QString::fromLocal8Bit("主动让出等待超时");
        case errorTakeInOverTime:
            return QString::fromLocal8Bit("主动获取等待超时");
        case errorOrderOutOverTime:
            return QString::fromLocal8Bit("被动让出等待超时");
        case errorOrderInOverTime:
            return QString::fromLocal8Bit("被动获取等待超时");
        default:
            return QString::fromLocal8Bit("数据错误");
    }
}


QString tmPeer::getStateString(int lang) const{
    switch(getState()){
    case stateDisable:
        return QString::fromLocal8Bit("停用");
    case stateOnlinewithToken:
        return QString::fromLocal8Bit("在线有令牌");
    case stateOnlinewithoutToken:
        return QString::fromLocal8Bit("在线无令牌");
    case stateTokenTakeOutPending:
        return QString::fromLocal8Bit("主动让出等待确认");
    case stateTokenOrderOutPending:
        return QString::fromLocal8Bit("被动让出等待确认");
    case stateTokenTakeInPending:
        return QString::fromLocal8Bit("主动获取等待确认");
    case stateTokenOrderInPending:
        return QString::fromLocal8Bit("被动获取等待确认");
    case stateOffline:
        return QString::fromLocal8Bit("掉线");
    default:
        return QString::fromLocal8Bit("数据错误");
    }
    //qDebug()<<getState()<<"state";
}


void tmPeer::update(QString& name, quint64 state, quint64 error, quint32 pri, quint32 ip){
    //qDebug()<<name<<state<<error<<pri<<ip;
    setPeerName(name);
    setState(state);
    setPeerPriority(pri);
    setPeerIp(ip);
    setError(error);
    lastUpdateTime.start();
}


void tmPeer::update(){
    lastUpdateTime.start();
}

//检查站offline
void tmPeer::offlineCheck(){
    if( lastUpdateTime.elapsed() >= offlineDelay ){
        //set offline
        if(getState() != stateOffline){
            //qDebug()<<"setOffline";
            setState(stateOffline);
        }
    }
    //will automatically change the state to Online aft heartbeat received
}

//判断是否有令牌，有=true
//disable                   = 0x0000000000000001ULL,      //peer禁用
//onlinewithToken           = 0x0000000000000002ULL,      //peer在线有令牌，主站
//onlinewithoutToken        = 0x0000000000000004ULL,      //peer在线无令牌，从站
//tokenTakeOutPending       = 0x0000000000000008ULL,      //该peer主动把令牌给其他peer，但其他peer还未确认。该peer具有令牌，主站。
//tokenOrderOutPending      = 0x0000000000000010ULL,      //其他peer要求该peer的令牌，该peer还未确认。该peer具有令牌，主站。
//tokenTakeInPending        = 0x0000000000000020ULL,      //该peer主动要获得令牌，但令牌持有peer还未确认。该peer无令牌，从站。
//tokenOrderInPending       = 0x0000000000000040ULL,      //令牌持有peer要求把令牌传送给该peer，但该peer还未确认。该peer无令牌，从站。
//offline                   = 0x0000000000000080ULL,      //peer离线。此状态不适用于本站，用于判断其他peer的掉线状态。
bool tmPeer::isWithToken(){
    if ( getState() == stateOnlinewithToken ||
            getState() == stateTokenTakeOutPending ||
                getState() == stateTokenOrderOutPending   )
        return true;
    else return false;
}
bool tmPeer::stopCheckOffline(){
    offlineCheckTimer.stop();
    return false;
}

void tmPeer::peerChanged(){
    //qDebug()<<"tmPeer::peerChanged";
    emit msgPeerChanged( getPeerName() );
}
