#include "tokenmanager.h"

tmTokenManager::tmTokenManager(QObject *parent) : QObject(parent)
{
    clearPeerInfo();
    tmPeerInfo_t temp =generateSelfInfo();
    setSelfInfo(temp);
    setTargetInfo(temp);


    tokenTakeOutOvertimer.setSingleShot(true);
    connect(&tokenTakeOutOvertimer,SIGNAL(timeout()),this,SLOT(tokenTakeOutOvertime()));

    tokenTakeInOvertimer.setSingleShot(true);
    connect(&tokenTakeInOvertimer,SIGNAL(timeout()),this,SLOT(tokenTakeInOvertime()));

    tokenOrderOutOvertimer.setSingleShot(true);
    connect(&tokenOrderOutOvertimer,SIGNAL(timeout()),this,SLOT(tokenOrderOutOvertime()));

    tokenOrderInOvertimer.setSingleShot(true);
    connect(&tokenOrderInOvertimer,SIGNAL(timeout()),this,SLOT(tokenOrderInOvertime()));

    clientSocket = new QUdpSocket(this);
    if( clientSocket->bind(tmPort) ) {
        selfInfo->errorState = OrderInOverTime;
        emit peerErrorStateChanged(selfInfo->errorState);
    }

}

tmTokenManager::~tmTokenManager(){
    clearPeerInfo();
}

int tmTokenManager::setPeerInfo(tmPeerInfo_t& newone){
    if(newone.peerName != "" && newone.peerIp != ""){
        tmPeerInfo_t* test;
        for (int i=0; i<pPeersList.size();i++){
            test = pPeersList.at(i);
            if(test->peerName == newone.peerName){
                *test = newone;//发现同名节点，替换
                return 0;
            }
        }
        test = new tmPeerInfo_t();
        *test = newone;
        pPeersList.append(test);//未发现同名节点，新建
        return 0;
    }
    else return -1;
}

int tmTokenManager::setPeersInfo(QList<tmPeerInfo_t*>& newlist){
    if(newlist.size()>1){
        clearPeerInfo();//释放内存

        for (int i=0; i<newlist.size();i++){
            setPeerInfo( *newlist.at(i) );
        }

        selfInfo = pPeersList.at(0);
        targetInfo = pPeersList.at(1);
        emit peerErrorStateChanged(selfInfo->errorState);
        emit peerStateChanged(selfInfo->state);
        return 0;
    }
    else return -1;
}

int tmTokenManager::setSelfInfo(tmPeerInfo_t& self){
    switch(pPeersList.size()){
        case 0:
            pPeersList.append(new tmPeerInfo_t());
        default:
            if(self.peerName != "" && self.peerIp != ""){
                *(pPeersList.at(0)) = self;

                selfInfo = pPeersList.at(0);
                emit peerErrorStateChanged(selfInfo->errorState);
                emit peerStateChanged(selfInfo->state);
                return 0;
            }
            else return -1;
    }
}

tmPeerInfo_t tmTokenManager::generateSelfInfo(){
    tmPeerInfo_t ret;
    QTime time = QTime::currentTime();
    qsrand(time.msec());
    ret.peerName = tr("default_station_%1").arg(qrand());
    ret.peerIp = QNetworkInterface::allAddresses().first().toString();
    ret.tokenGenerator =false;
    ret.errorState = noError;
    ret.state = peerOnlinewithoutToken;
    return ret;
}

int tmTokenManager::setTargetInfo(tmPeerInfo_t& target){
    switch(pPeersList.size()){
        case 0:
            return -1;
        case 1:
            pPeersList.append(new tmPeerInfo_t());
        default:
            if(target.peerName != "" && target.peerIp != ""){
                *(pPeersList.at(1)) = target;

                targetInfo = pPeersList.at(1);
                return 0;
            }
            else return -1;
    }
}

int tmTokenManager::clearPeerInfo(){
    for (int i=0; i<pPeersList.size();i++){
        delete pPeersList.at(i);
    }//释放内存
    pPeersList.clear();//清空QList
    return 0;
}

int tmTokenManager::selfEnable(){
    selfInfo->state = peerOnlinewithoutToken;
    emit peerStateChanged(selfInfo->state);
    return 0;
}

int tmTokenManager::selfDisable(){
    selfInfo->state = peerDisable;
    emit peerStateChanged(selfInfo->state);
    return 0;
}

/////////////////////////////////1、本机主动交出令牌
//本机主动交出令牌 请求
int tmTokenManager::tokenTakeOut(tmPeerInfo_t& target, int overtime){
    if(selfInfo->state == peerOnlinewithToken ){
        if(setTargetInfo(target))
            return -1;
        selfInfo->state = tokenTakeOutPending;
        emit peerStateChanged(selfInfo->state);

        if(overtime>0){
            tokenTakeOutOvertimer.setInterval(overtime);
            tokenTakeOutOvertimer.start();
        }
        return 0;
    }
    else return -1;
}
//本机主动交出令牌 超时
void tmTokenManager::tokenTakeOutOvertime(){
    if(selfInfo->state == tokenTakeOutPending ){
        selfInfo->state = peerOnlinewithToken;
        emit peerStateChanged(selfInfo->state);
        selfInfo->errorState = takeOutOverTime;
        emit peerErrorStateChanged(selfInfo->errorState);
    }
}
//本机主动交出令牌 确认
int tmTokenManager::tokenTakeOutAck(){
    if(selfInfo->state == tokenTakeOutPending ){
        selfInfo->state = peerOnlinewithoutToken;
        emit peerStateChanged(selfInfo->state);
        return 0;
    }
    else return -1;
}
//本机主动交出令牌 取消
int tmTokenManager::tokenTakeOutCancel(){
    if(selfInfo->state == tokenTakeOutPending ){
        selfInfo->state = peerOnlinewithToken;
        emit peerStateChanged(selfInfo->state);
        return 0;
    }
    else return -1;
}
/////////////////////////////////2、本机主动获得令牌
//本机主动获得令牌 请求
int tmTokenManager::tokenTakeIn(tmPeerInfo_t& source, int overtime){
    if(selfInfo->state == peerOnlinewithoutToken ){
        if(setTargetInfo(source))
            return -1;
        selfInfo->state = tokenTakeInPending;
        emit peerStateChanged(selfInfo->state);

        if(overtime>0){
            tokenTakeInOvertimer.setInterval(overtime);
            tokenTakeInOvertimer.start();
        }
        return 0;
    }
    else return -1;
}
//本机主动获得令牌 超时
void tmTokenManager::tokenTakeInOvertime(){
    if(selfInfo->state == tokenTakeInPending ){
        selfInfo->state = peerOnlinewithoutToken;
        emit peerStateChanged(selfInfo->state);
        selfInfo->errorState = takeInOverTime;
        emit peerErrorStateChanged(selfInfo->errorState);
    }
}
//本机主动获得令牌 确认
int tmTokenManager::tokenTakeInAck(){
    if(selfInfo->state == tokenTakeInPending ){
        selfInfo->state = peerOnlinewithToken;
        emit peerStateChanged(selfInfo->state);
        return 0;
    }
    else return -1;
}
//本机主动获得令牌 取消
int tmTokenManager::tokenTakeInCancel(){
    if(selfInfo->state == tokenTakeInPending ){
        selfInfo->state = peerOnlinewithoutToken;
        emit peerStateChanged(selfInfo->state);
        return 0;
    }
    else return -1;
}

/////////////////////////////////3、本机被动交出令牌
//本机主动将令牌交出 请求
int tmTokenManager::tokenOrderOut(tmPeerInfo_t& target, int overtime){
    if(selfInfo->state == peerOnlinewithToken ){
        if(setTargetInfo(target))
            return -1;
        selfInfo->state = tokenOrderOutPending;
        emit peerStateChanged(selfInfo->state);

        if(overtime>0){
            tokenOrderOutOvertimer.setInterval(overtime);
            tokenOrderOutOvertimer.start();
        }
        return 0;
    }
    else return -1;
}
//本机被动交出令牌 超时
void tmTokenManager::tokenOrderOutOvertime(){
    if(selfInfo->state == tokenOrderOutPending ){
        selfInfo->state = peerOnlinewithToken;
        emit peerStateChanged(selfInfo->state);
        selfInfo->errorState = OrderOutOverTime;
        emit peerErrorStateChanged(selfInfo->errorState);
    }
}
//本机被动交出令牌 确认
int tmTokenManager::tokenOrderOutAck(){
    if(selfInfo->state == tokenOrderOutPending ){
        selfInfo->state = peerOnlinewithoutToken;
        emit peerStateChanged(selfInfo->state);
        return 0;
    }
    else return -1;
}
//本机被动交出令牌 取消
int tmTokenManager::tokenOrderOutCancel(){
    if(selfInfo->state == tokenOrderOutPending ){
        selfInfo->state = peerOnlinewithToken;
        emit peerStateChanged(selfInfo->state);
        return 0;
    }
    else return -1;
}

/////////////////////////////////4、本机被动获得令牌
//本机被动获得令牌 请求
int tmTokenManager::tokenOrderIn(tmPeerInfo_t& source, int overtime){
    if(selfInfo->state == peerOnlinewithoutToken ){
        if(setTargetInfo(source))
            return -1;
        selfInfo->state = tokenOrderInPending;
        emit peerStateChanged(selfInfo->state);

        if(overtime>0){
            tokenOrderInOvertimer.setInterval(overtime);
            tokenOrderInOvertimer.start();
        }
        return 0;
    }
    else return -1;
}
//本机被动获得令牌 超时
void tmTokenManager::tokenOrderInOvertime(){
    if(selfInfo->state == tokenOrderInPending ){
        selfInfo->state = peerOnlinewithoutToken;
        emit peerStateChanged(selfInfo->state);
        selfInfo->errorState = OrderInOverTime;
        emit peerErrorStateChanged(selfInfo->errorState);
    }
}
//本机被动获得令牌 确认
int tmTokenManager::tokenOrderInAck(){
    if(selfInfo->state == tokenOrderInPending ){
        selfInfo->state = peerOnlinewithToken;
        emit peerStateChanged(selfInfo->state);
        return 0;
    }
    else return -1;
}
//本机被动获得令牌 取消
int tmTokenManager::tokenOrderInCancel(){
    if(selfInfo->state == tokenOrderInPending ){
        selfInfo->state = peerOnlinewithoutToken;
        emit peerStateChanged(selfInfo->state);
        return 0;
    }
    else return -1;
}

