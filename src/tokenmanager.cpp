#include "tokenmanager.h"

tmTokenManager::tmTokenManager(QObject *parent) : QObject(parent)
{
    pPeersList.clear();
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

    heartBeatTimer.setSingleShot(false);
    connect(&heartBeatTimer,SIGNAL(timeout()),this,SLOT(heartBeatSender()));

    start();

    connect(serverSocket, SIGNAL(readyRead()),this, SLOT(processPendingDatagrams()));
}

tmTokenManager::~tmTokenManager(){
    clearPeerInfo();
}

//新建端口，初始化网络连接，自身状态设置为online（启动失败除外）
int tmTokenManager::start(void){
    if(serverSocket->isValid()){
        serverSocket->deleteLater();
    }
    serverSocket = new QUdpSocket(this);

    if( !serverSocket->bind(QHostAddress::Any,tmPort,QAbstractSocket::ShareAddress) ) {
        setSelfState( peerDisable );
        setSelfErrorState( innerError );
        return -1;
    }

    if(clientSocket->isValid()){
        clientSocket->deleteLater();
    }
    clientSocket = new QUdpSocket(this);

    heartBeatTimer.start();

    if(selfInfo->errorState != innerError){
        setSelfState( peerOnlinewithoutToken );
        return 0;
    }
    else{
        setSelfState( peerDisable );
        return -1;
    }

}

//删除端口，终止网络连接，自身状态设为Disable
int tmTokenManager::stop(void){
    if(serverSocket->isValid()){
        serverSocket->deleteLater();
    }

    if(clientSocket->isValid()){
        clientSocket->deleteLater();
    }

    heartBeatTimer.stop();

    setSelfState( peerDisable );

    return 0;
}

//重启
int tmTokenManager::restart(void){
    stop();
    return(start());
}

//检查收到的报文
void tmTokenManager::processPendingDatagrams(){

}

//发送心跳报文
void tmTokenManager::heartBeatSender(){
    if(selfInfo->state == peerDisable) return;

    if(isWithToken()){
//      master
//      2、主站心跳报文(master heart beat)
//      $TMMHB,peer名称(max 255 byte),IP地址(4byte),peer状态(1byte),peer错误状态(1byte),peer令牌优先级(1byte),状态消息(max 255 byte)<CR><LF>
//      注意：消息的具体内容由其他程序打包送入和取出解析。
        QByteArray heartBeat;
        heartBeat = "$TMMHB,";
        heartBeat += selfInfo->peerName;
        heartBeat += ",";
        heartBeat += QByteArray::number(selfInfo->peerIp,16);
        heartBeat += ",";
        heartBeat +=  (char)selfInfo->state;
        heartBeat += ",";
        heartBeat +=  (char)selfInfo->errorState;
        heartBeat += ",";
        heartBeat +=  (char)selfInfo->tokenGeneratorPriority;
        heartBeat += ",";
        heartBeat +=  masterPeerMessage;
        heartBeat += 0x0D;
        heartBeat += 0x0A;

        clientSocket->writeDatagram(heartBeat.data(), heartBeat.size(),QHostAddress::Broadcast, tmPort);
        return;

    }
    else{
//      slave
//      1、从站心跳报文(slave heart beat)
//      $TMSHB,peer名称(max 255 byte),IP地址(4byte),peer状态(1byte),peer错误状态(1byte),peer令牌优先级(1byte)<CR><LF>

        QByteArray heartBeat;
        heartBeat = "$TMSHB,";
        heartBeat += selfInfo->peerName;
        heartBeat += ",";
        heartBeat += QByteArray::number(selfInfo->peerIp,16);
        heartBeat += ",";
        heartBeat +=  (char)selfInfo->state;
        heartBeat += ",";
        heartBeat +=  (char)selfInfo->errorState;
        heartBeat += ",";
        heartBeat +=  (char)selfInfo->tokenGeneratorPriority;
        heartBeat += 0x0D;
        heartBeat += 0x0A;

        clientSocket->writeDatagram(heartBeat.data(), heartBeat.size(),QHostAddress::Broadcast, tmPort);

        return;
    }
}


int tmTokenManager::setPeerInfo(tmPeerInfo_t& newone){
    if(newone.peerName != "" && newone.peerIp != 0x0){
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
        emit selfErrorStateChanged(selfInfo->errorState);
        emit selfStateChanged(selfInfo->state);
        return 0;
    }
    else return -1;
}

int tmTokenManager::setSelfInfo(tmPeerInfo_t& self){
    switch(pPeersList.size()){
        case 0:
            pPeersList.append(new tmPeerInfo_t());
        default:
            if(self.peerName != "" && self.peerIp != 0x0){
                *(pPeersList.at(0)) = self;
                selfInfo = pPeersList.at(0);
                emit selfErrorStateChanged(selfInfo->errorState);
                emit selfStateChanged(selfInfo->state);
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
    ret.peerIp = QNetworkInterface::allAddresses().first().toIPv4Address();
    ret.tokenGeneratorPriority = 0 ;
    ret.lastUpdateTime = QTime::currentTime();
    ret.errorState = noError;
    ret.state = peerDisable;
    return ret;
}

int tmTokenManager::setTargetInfo(tmPeerInfo_t& target){
    switch(pPeersList.size()){
        case 0:
            return -1;
        case 1:
            pPeersList.append(new tmPeerInfo_t());
        default:
            if(target.peerName != "" && target.peerIp != 0x0){
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


/////////////////////////////////1、本机主动交出令牌
//本机主动交出令牌 请求
int tmTokenManager::tokenTakeOut(tmPeerInfo_t& target, int overtime){
    if(selfInfo->state == peerOnlinewithToken ){
        if(setTargetInfo(target))
            return -1;
        setSelfState( tokenTakeOutPending );

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
        setSelfState( peerOnlinewithToken );
        setSelfErrorState( takeOutOverTime );
    }
}
//本机主动交出令牌 确认
int tmTokenManager::tokenTakeOutAck(){
    if(selfInfo->state == tokenTakeOutPending ){
        setSelfState( peerOnlinewithoutToken );
        return 0;
    }
    else return -1;
}
//本机主动交出令牌 取消
int tmTokenManager::tokenTakeOutCancel(){
    if(selfInfo->state == tokenTakeOutPending ){
        setSelfState( peerOnlinewithToken );
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
        setSelfState( tokenTakeInPending );

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
        setSelfState( peerOnlinewithoutToken );
        setSelfErrorState( takeInOverTime );
    }
}
//本机主动获得令牌 确认
int tmTokenManager::tokenTakeInAck(){
    if(selfInfo->state == tokenTakeInPending ){
        setSelfState( peerOnlinewithToken );
        return 0;
    }
    else return -1;
}
//本机主动获得令牌 取消
int tmTokenManager::tokenTakeInCancel(){
    if(selfInfo->state == tokenTakeInPending ){
        setSelfState( peerOnlinewithoutToken );
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
        setSelfState( tokenOrderOutPending );

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
        setSelfState( peerOnlinewithToken );
        setSelfErrorState( OrderOutOverTime );
    }
}
//本机被动交出令牌 确认
int tmTokenManager::tokenOrderOutAck(){
    if(selfInfo->state == tokenOrderOutPending ){
        setSelfState( peerOnlinewithoutToken );
        return 0;
    }
    else return -1;
}
//本机被动交出令牌 取消
int tmTokenManager::tokenOrderOutCancel(){
    if(selfInfo->state == tokenOrderOutPending ){
        setSelfState( peerOnlinewithToken );
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
        setSelfState( tokenOrderInPending );

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
        setSelfState( peerOnlinewithoutToken );
        setSelfErrorState( OrderInOverTime );
    }
}
//本机被动获得令牌 确认
int tmTokenManager::tokenOrderInAck(){
    if(selfInfo->state == tokenOrderInPending ){
        setSelfState( peerOnlinewithToken );
        return 0;
    }
    else return -1;
}
//本机被动获得令牌 取消
int tmTokenManager::tokenOrderInCancel(){
    if(selfInfo->state == tokenOrderInPending ){
        setSelfState( peerOnlinewithoutToken );
        return 0;
    }
    else return -1;
}

//判断是否有令牌，有=true
//a、peerDisable               peer禁用
//b、peerOnlinewithToken,      peer在线有令牌，主站
//c、peerOnlinewithoutToken,   peer在线无令牌，从站
//d、tokenTakeOutPending,      该peer主动把令牌给其他peer，但其他peer未确认。该peer具有令牌，主站。
//e、tokenOrderOutPending,     其他peer要求该peer的令牌，该peer未确认。该peer具有令牌，主站。
//f、tokenTakeInPending,       该peer主动要获得令牌，但令牌持有peer未确认。该peer无令牌，从站。
//g、tokenOrderInPending,      令牌持有peer要求把令牌传送给该peer，但该peer未确认。该peer无令牌，从站。
//h、peerOffline,              peer离线。此状态不适用于self，用于判断其他peer的掉线状态。
bool tmTokenManager::isWithToken(){
    if ( selfInfo->state == peerOnlinewithToken ||
            selfInfo->state == tokenTakeOutPending ||
                selfInfo->state == tokenOrderOutPending   )
        return true;
    else return false;
}
