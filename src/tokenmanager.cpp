#include "tokenmanager.h"

tmTokenManager::tmTokenManager(QObject *parent) : QObject(parent)
{

    serverSocket = new QUdpSocket(this);
    clientSocket = new QUdpSocket(this);
    pPeersList.clear();
    tmPeerInfo_t temp =generateSelfInfo();
    setSelfInfo(&temp);
    //qDebug()<<temp.errorState<<getSelfInfo()->errorState;
    setPartnerInfo(getSelfInfo());

    offlineDelay = 30 * 1000;       //default = 30sec
    offlineCheckInterv = 1* 1000;   //default = 1sec
    heartbeatInterv = 1* 1000;      //default = 1sec
    tokenCheckInterv  = 2*1000;     //default = 2sec
    tokenErrorDelay = 5 * 1000;     //default = 5sec

    tokenTakeOutOvertimer.setSingleShot(true);
    connect(&tokenTakeOutOvertimer,SIGNAL(timeout()),this,SLOT(tokenTakeOutOvertime()));

    tokenTakeInOvertimer.setSingleShot(true);
    connect(&tokenTakeInOvertimer,SIGNAL(timeout()),this,SLOT(tokenTakeInOvertime()));

    tokenOrderOutOvertimer.setSingleShot(true);
    connect(&tokenOrderOutOvertimer,SIGNAL(timeout()),this,SLOT(tokenOrderOutOvertime()));

    tokenOrderInOvertimer.setSingleShot(true);
    connect(&tokenOrderInOvertimer,SIGNAL(timeout()),this,SLOT(tokenOrderInOvertime()));

    heartBeatTimer.setSingleShot(false);
    heartBeatTimer.setInterval(heartbeatInterv);
    connect(&heartBeatTimer,SIGNAL(timeout()),this,SLOT(heartBeatSender()));
    //心跳报文仅当start()调用后开始，可以用stop停止

    offlineCheckTimer.setSingleShot(false);
    offlineCheckTimer.setInterval(offlineCheckInterv);
    connect(&offlineCheckTimer,SIGNAL(timeout()),this,SLOT(offlineCheck()));
    offlineCheckTimer.start();//一直都会检查超时

    tokenCheckTimer.setSingleShot(false);
    tokenCheckTimer.setInterval(tokenCheckInterv);
    connect(&tokenCheckTimer,SIGNAL(timeout()),this,SLOT(tokenCheck()));
    tokenCheckTimer.start();//一直都会检查令牌
    tokenErrorDelayTime.start();//用于检查elapse

    start();
    checkPriority();
}

tmTokenManager::~tmTokenManager(){
    clearPeerInfo();
    if(serverSocket){
        serverSocket->deleteLater();
    }
    if(clientSocket){
        clientSocket->deleteLater();
    }
}

//启动心跳，自身状态设置为online（启动失败除外）
int tmTokenManager::start(void){

    if( !serverSocket->bind( QHostAddress(getSelfInfo()->peerIp),tmPort,QAbstractSocket::ShareAddress) ) {
        setSelfState( tmPeerDisable );
        setSelfErrorState( tmNetworkError );
        emit tmOperationMessage( QString::fromLocal8Bit("网络初始化失败"));
        return -1;
    }
    connect(serverSocket, SIGNAL(readyRead()),this, SLOT(processPendingDatagrams()));
    heartBeatTimer.start();
    if(getSelfInfo()->errorState != tmInnerError){
        setSelfState( tmPeerOnlinewithoutToken );
        emit tmOperationMessage( QString::fromLocal8Bit("启动成功"));
        return 0;
    }
    else{
        setSelfState( tmPeerDisable );
        return -1;
    }
}

//停止心跳，自身状态设为Disable
int tmTokenManager::stop(void){
    serverSocket->close();
    clientSocket->close();
    heartBeatTimer.stop();
    setSelfState( tmPeerDisable );
    emit tmOperationMessage( QString::fromLocal8Bit("停止本peer"));
    return 0;
}

//重启
int tmTokenManager::restart(void){
    stop();
    return(start());
}

//检查收到的报文
//此处主要负责分发，不做条件判断
void tmTokenManager::processPendingDatagrams(){
        QByteArray datagram,head,button;
        int paraBegin;
        bool ok;
        QByteArray source,target;
        qint32 overtime;
        tmPeerInfo_t newOne;
    while (serverSocket->hasPendingDatagrams()) {
        datagram.resize(serverSocket->pendingDatagramSize());
        if( serverSocket->readDatagram(datagram.data(), datagram.size()) <0) continue;//丢弃该报文
        //qDebug()<<datagram;
        button = datagram.right(2);
        //qDebug()<<button;
        if( button != "\r\n") continue;//结尾不对，丢弃该报文
        head = datagram.left(3);
        if (head != "$TM") continue;//协议簇不对，丢弃该报文
        head = datagram.mid(3,3);
        if(head == "SHB") {
//      1、从站心跳报文(slave heart beat)
//      $TMSHB,peer名称(max 255 byte),IP地址(4byte),peer状态(1byte),peer错误状态(1byte),peer令牌优先级(1byte)<CR><LF>
            paraBegin = 7;
            newOne.peerName = QString(  datagramReadParameter( datagram , &paraBegin) );
            if(newOne.peerName == getSelfInfo()->peerName) continue;//本peer自己发的，丢弃该报文
            newOne.peerIp = (quint32)datagramReadParameter( datagram , &paraBegin).toUInt(&ok, 16);
            newOne.state = (tmPeerState_e)datagramReadParameter( datagram , &paraBegin).toHex().toUInt(&ok,16);
            newOne.errorState = (tmPeerErrorState_e)datagramReadParameter( datagram , &paraBegin).toHex().toUInt(&ok,16);
            newOne.priority = datagramReadParameter( datagram , &paraBegin).toHex().toUInt(&ok,16);
            newOne.lastUpdateTime.start();
            setPeerInfo(&newOne);

            //qDebug()<<newOne.state<<newOne.errorState<<newOne.tokenGeneratorPriority;
        }
        else if(head == "MHB") {
//      2、主站心跳报文(master heart beat)
//      $TMMHB,peer名称(max 255 byte),IP地址(4byte),peer状态(1byte),peer错误状态(1byte),peer令牌优先级(1byte),状态消息(max 255 byte)<CR><LF>
            paraBegin = 7;
            newOne.peerName = QString( datagramReadParameter( datagram , &paraBegin) );
            if(newOne.peerName == getSelfInfo()->peerName) continue;//本peer自己发的，丢弃该报文
            newOne.peerIp = (quint32)datagramReadParameter( datagram , &paraBegin).toUInt(&ok, 16);
            newOne.state = (tmPeerState_e)datagramReadParameter( datagram , &paraBegin).toHex().toUInt(&ok,16);
            newOne.errorState = (tmPeerErrorState_e)datagramReadParameter( datagram , &paraBegin).toHex().toUInt(&ok,16);
            newOne.priority = datagramReadParameter( datagram , &paraBegin).toHex().toUInt(&ok,16);
            newOne.lastUpdateTime.start();
            setMasterPeerMessage( QString( datagramReadParameter( datagram , &paraBegin) ) );
            setPeerInfo(&newOne);
        }
        else if(head == "TTR") {
//      3、令牌转移提交(token transfer require)
//      $TMTTR,源peer名称(持有token),目标peer名称,超时时间(秒 4byte)<CR><LF>
            paraBegin = 7;
            source = datagramReadParameter( datagram , &paraBegin);
            target = datagramReadParameter( datagram , &paraBegin);
            overtime = datagramReadParameter( datagram , &paraBegin).toHex().toUInt(&ok,16);
            if (source == getSelfInfo()->peerName ){
                //本peer被动交出令牌
                setPartnerInfo( getPeerInfo( QString(target) ) );
                tokenOrderOut(overtime);
            }
            else if (target == getSelfInfo()->peerName){
                //本peer被动接受令牌
                setPartnerInfo( getPeerInfo( QString(source) ) );
                tokenOrderIn(overtime);
            }
        }
        else if(head == "TTA") {
//      4、令牌转移应答确认(token transfer acknowleadge)
//      $TMTTA,源peer名称,目标peer名称<CR><LF>
            paraBegin = 7;
            source = datagramReadParameter( datagram , &paraBegin);
            target = datagramReadParameter( datagram , &paraBegin);
            if (source == getSelfInfo()->peerName ){
                //本peer主动让出令牌，收到确认
                setPartnerInfo( getPeerInfo( QString(target) ) );
                tokenTakeOutAck();
            }
            if (target == getSelfInfo()->peerName){
                //本peer主动获取令牌，收到确认
                setPartnerInfo( getPeerInfo( QString(source) ) );
                tokenTakeInAck();
            }
        }
        else if(head == "TTC") {
//      5、令牌转移取消(token transfer cancel)
//      $TMTTC,源peer名称,目标peer名称<CR><LF>
            paraBegin = 7;
            source = datagramReadParameter( datagram , &paraBegin);
            target = datagramReadParameter( datagram , &paraBegin);
            if (getSelfInfo()->state ==tmTokenTakeOutPending &&
                    source == getSelfInfo()->peerName ){
                //本peer主动让出交出令牌，收到取消
                setPartnerInfo( getPeerInfo( QString(target) ) );
                tokenTakeOutCancel();
            }
            else if (getSelfInfo()->state ==tmTokenTakeInPending &&
                     target == getSelfInfo()->peerName ){
                //本peer主动获取令牌，收到取消
                setPartnerInfo( getPeerInfo( QString( source ) ) );
                tokenTakeInCancel();
            }
            else if (getSelfInfo()->state ==tmTokenOrderInPending &&
                     target == getSelfInfo()->peerName ){
                //本peer被动获取令牌，收到取消
                setPartnerInfo( getPeerInfo( QString(source) ) );
                tokenOrderInCancel();
            }
            else if (getSelfInfo()->state ==tmTokenOrderOutPending &&
                     source == getSelfInfo()->peerName ){
                //本peer被动让出令牌，收到取消
                setPartnerInfo( getPeerInfo( QString(target) ) );
                tokenOrderOutCancel();
            }
        }
        else if(head == "TTF") {
//      6、令牌强制转移(toekn transfer forced)//用途：1.令牌强制获得。2、令牌丢失后，自动生成令牌
//      $TMTTF,目标peer名称<CR><LF>
            paraBegin = 7;
            target = datagramReadParameter( datagram , &paraBegin);
            if (target != getSelfInfo()->peerName ){
                //强制到其他peer
                setPartnerInfo( getPeerInfo( QString( target) ) );
                tokenForceOrderOut();
            }
        }
        else {
            emit tmOtherCommandReceived( datagram );//无法解析，外送
        }
    }
}
//检查站offline
void tmTokenManager::offlineCheck(){
    tmPeerInfo_t* test;
    int i=2;
    while (i<pPeersList.size()){
        test = pPeersList.at(i);
        if( getPeerInfo(i)->lastUpdateTime.elapsed() >= offlineDelay ){
            //set offline

            if(getPeerInfo(i)->state != tmPeerOffline){
                setPeerState(i,tmPeerOffline);
            }
        }
        i++;
            //will automatically change the state to Online aft heartbeat received
    }
}
//检查token
void tmTokenManager::tokenCheck(){
    int res=findToken();
    //qDebug()<<res<<isFirstPriority()<<tokenErrorDelayTime.elapsed();
    if( res == -1 ){
        //找不到令牌
        if(tokenErrorDelayTime.elapsed() >= tokenErrorDelay){
            setSelfErrorState(tmTokenLost);
            if( isFirstPriority() ){
                //本机有最高优先权则立即获得token
                tokenForceTakeIn();
            }
        }
    }
    else if( res == -2){
        //找到两个或以上令牌
        if(tokenErrorDelayTime.elapsed() >= tokenErrorDelay){
            setSelfErrorState(tmTokenMoreThanOne);
            if ( !isFirstPriority() ){
                //本机没有最高优先权则立即取消token
                tokenForceOrderOut();
            }
        }
    }
    else if( res >= 0){
        //找到了token，复位报警
        if(getSelfInfo()->errorState == tmTokenLost || getSelfInfo()->errorState == tmTokenMoreThanOne){
            setSelfErrorState(tmNoError);
        }
        tokenErrorDelayTime.start();
    }
}

//发送心跳报文
void tmTokenManager::heartBeatSender(){

    if(getSelfInfo()->state == tmPeerDisable) return;

    //qDebug()<<"heartBeat";

    if(isWithToken()){
//      master
//      2、主站心跳报文(master heart beat)
//      $TMMHB,peer名称(max 255 byte),IP地址(4byte),peer状态(1byte),peer错误状态(1byte),peer令牌优先级(1byte),状态消息(max 255 byte)<CR><LF>
//      注意：消息的具体内容由其他程序打包送入和取出解析。
        QByteArray heartBeat;
        heartBeat = "$TMMHB,";
        heartBeat += getSelfInfo()->peerName;
        heartBeat += ",";
        heartBeat += QByteArray::number(getSelfInfo()->peerIp,16);
        heartBeat += ",";
        heartBeat +=  (quint8)getSelfInfo()->state;
        heartBeat += ",";
        heartBeat +=  (quint8)getSelfInfo()->errorState;
        heartBeat += ",";
        heartBeat +=  (quint8)getSelfInfo()->priority;
        heartBeat += ",";
        heartBeat +=  masterPeerMessage;
        heartBeat += 0x0D;
        heartBeat += 0x0A;
        //qDebug()<<heartBeat;
        clientSocket->writeDatagram(heartBeat.data(), heartBeat.size(),QHostAddress::Broadcast, tmPort);
    }
    else{
//      slave
//      1、从站心跳报文(slave heart beat)
//      $TMSHB,peer名称(max 255 byte),IP地址(4byte),peer状态(1byte),peer错误状态(1byte),peer令牌优先级(1byte)<CR><LF>

        QByteArray heartBeat;
        heartBeat = "$TMSHB,";
        heartBeat += getSelfInfo()->peerName;
        heartBeat += ",";
        heartBeat += QByteArray::number(getSelfInfo()->peerIp,16);
        heartBeat += ",";
        heartBeat +=  (quint8)getSelfInfo()->state;
        heartBeat += ",";
        heartBeat +=  (quint8)getSelfInfo()->errorState;
        heartBeat += ",";
        heartBeat +=  (quint8)getSelfInfo()->priority;
        heartBeat += 0x0D;
        heartBeat += 0x0A;
        //qDebug()<<heartBeat;
        clientSocket->writeDatagram(heartBeat.data(), heartBeat.size(),QHostAddress::Broadcast, tmPort);
    }
}


int tmTokenManager::clearPeerInfo(){
    delete pPeersList.at(0);
    for (int i=2; i<pPeersList.size();i++){
        delete pPeersList.at(i);
    }//释放内存
    pPeersList.clear();//清空QList
    return 0;
}


/////////////////////////////////1、本机主动交出令牌
//本机主动交出令牌 请求
int tmTokenManager::tokenTakeOut(QString target, qint32 overtime){
    if( setPartnerInfo( getPeerInfo(target) ) <0 ) return -1;//找不到目标对应的数据
    if(overtime <0) return -1;   //overTime数据不对
    if(getSelfInfo()->state !=tmPeerOnlinewithToken ) return -1;//仅当本peer在线有token时才能进入takeout
    if(getSelfInfo()->errorState == tmInnerError|| getSelfInfo()->errorState == tmNetworkError) return -1;//本peer未准备好
    if(getPartnerInfo()->state != tmPeerOnlinewithoutToken || getPartnerInfo()->errorState == tmInnerError) return -1;//目标peer没有准备好
    setSelfState( tmTokenTakeOutPending );
    if(overtime>0){
        tokenTakeOutOvertimer.setInterval(overtime);
        tokenTakeOutOvertimer.start();
    }
//      3、令牌转移提交(token transfer require)
//      $TMTTR,源peer名称(持有token),目标peer名称,超时时间(秒 4byte)<CR><LF>
    QByteArray require;
    require = "$TMTTR,";
    require += getSelfInfo()->peerName;
    require += ",";
    require += getPartnerInfo()->peerName;
    require += ",";
    require += overtime>>24;
    require += overtime>>16;
    require += overtime>>8;
    require += overtime;
    require += 0x0D;
    require += 0x0A;
    qint64 res=clientSocket->writeDatagram(require.data(), require.size(),QHostAddress::Broadcast, tmPort);
    //qDebug()<<require<<overtime;
    return res;

}
int tmTokenManager::tokenTakeOut(int index, qint32 overtime){
    if(index <2 || index >= getPeerListSize()) return -1;
    QString target = getPeerInfo(index)->peerName;
    tokenTakeOut(target,overtime);
}
//本机主动交出令牌 超时
void tmTokenManager::tokenTakeOutOvertime(){
    if(getSelfInfo()->state == tmTokenTakeOutPending ){
        tokenTakeOutCancel();
        setSelfErrorState( tmTakeOutOverTime );
    }
}
//本机主动交出令牌 确认
int tmTokenManager::tokenTakeOutAck(){
    if(getSelfInfo()->state == tmTokenTakeOutPending ){
        setSelfState( tmPeerOnlinewithoutToken );
        return 0;
    }
    else return -1;
}
//本机主动交出令牌 取消
int tmTokenManager::tokenTakeOutCancel(){
    if(getSelfInfo()->state == tmTokenTakeOutPending ){
//      5、令牌转移取消(token transfer cancel)
//      $TMTTC,源peer名称,目标peer名称<CR><LF>
        QByteArray requrie;
        requrie = "$TMTTC,";
        requrie += getSelfInfo()->peerName;
        requrie += ",";
        requrie += getPartnerInfo()->peerName;
        requrie += 0x0D;
        requrie += 0x0A;
        //qDebug()<<"tokenTakeOutCancel:"<<requrie;
        qint64 res=clientSocket->writeDatagram(requrie.data(), requrie.size(),QHostAddress::Broadcast, tmPort);
        setSelfState( tmPeerOnlinewithToken );
        return res;
    }
    else return -1;
}
/////////////////////////////////2、本机主动获得令牌
//本机主动获得令牌 请求
int tmTokenManager::tokenTakeIn(QString target, qint32 overtime){
    //qDebug()<<target<<overtime;
    if( setPartnerInfo( getPeerInfo(target) ) <0 ) return -1;//找不到目标peer名称对应的数据
    if(overtime <0) return -1;   //overTime数据不对
    if(getSelfInfo()->state != tmPeerOnlinewithoutToken ) return -1;//仅当本peer在线无token时才能进入takein
    if(getSelfInfo()->errorState == tmInnerError|| getSelfInfo()->errorState == tmNetworkError) return -1;//本peer未准备好
    if(getPartnerInfo()->state != tmPeerOnlinewithToken || getPartnerInfo()->errorState == tmInnerError) return -1;//目标peer没有准备好


    setSelfState( tmTokenTakeInPending );
    if(overtime>0){
        tokenTakeInOvertimer.setInterval(overtime);
        tokenTakeInOvertimer.start();
    }

//      3、令牌转移提交(token transfer require)
//      $TMTTR,源peer名称(持有token),目标peer名称,超时时间(秒 4byte)<CR><LF>
    QByteArray require;
    require = "$TMTTR,";
    require += getPartnerInfo()->peerName;
    require += ",";
    require += getSelfInfo()->peerName;
    require += ",";
    require += overtime>>24;
    require += overtime>>16;
    require += overtime>>8;
    require += overtime;
    require += 0x0D;
    require += 0x0A;
    //qDebug()<<requrie;
    qint64 res=clientSocket->writeDatagram(require.data(), require.size(),QHostAddress::Broadcast, tmPort);
    return res;
}
int tmTokenManager::tokenTakeIn(int index,qint32 overtime){
    if(index <2 || index >= getPeerListSize()) return -1;
    QString target = getPeerInfo(index)->peerName;
    tokenTakeIn(target,overtime);
}
//本机主动获得令牌 超时
void tmTokenManager::tokenTakeInOvertime(){
    if(getSelfInfo()->state == tmTokenTakeInPending ){
        tokenTakeInCancel();
        setSelfErrorState( tmTakeInOverTime );
    }
}
//本机主动获得令牌 确认
int tmTokenManager::tokenTakeInAck(){
    if(getSelfInfo()->state == tmTokenTakeInPending ){
        setSelfState( tmPeerOnlinewithToken );
        return 0;
    }
    else return -1;
}
//本机主动获得令牌 取消
int tmTokenManager::tokenTakeInCancel(){
    if(getSelfInfo()->state == tmTokenTakeInPending ){
//      5、令牌转移取消(token transfer cancel)
//      $TMTTC,源peer名称,目标peer名称<CR><LF>
        QByteArray requrie;
        requrie = "$TMTTC,";
        requrie += getPartnerInfo()->peerName;
        requrie += ",";
        requrie += getSelfInfo()->peerName;
        requrie += 0x0D;
        requrie += 0x0A;
        qint64 res=clientSocket->writeDatagram(requrie.data(), requrie.size(),QHostAddress::Broadcast, tmPort);
        //qDebug()<<"tokenTakeInCancel:"<<requrie;
        setSelfState( tmPeerOnlinewithoutToken );
        return res;
    }
    else return -1;
}

/////////////////////////////////3、本机被动交出令牌
//本机被动将令牌交出 请求
int tmTokenManager::tokenOrderOut(qint32 overtime){
    if( pPeersList.at(1) <0 ) return -1;//找不到目标peer数据
    if(overtime <0) return -1;   //overTime数据不对
    if(getSelfInfo()->state !=tmPeerOnlinewithToken ) return -1;//本peer无token则返回-1
    if(getSelfInfo()->errorState == tmInnerError|| getSelfInfo()->errorState == tmNetworkError) return -1;//本peer未准备好
    if(getPartnerInfo()->state == tmPeerOnlinewithToken || getPartnerInfo()->errorState == tmInnerError) return -1;//目标peer没有准备好

    setSelfState( tmTokenOrderOutPending );

    if(overtime>0){
        tokenOrderOutOvertimer.setInterval(overtime);
        tokenOrderOutOvertimer.start();
    }
    return 0;

}
//本机被动交出令牌 超时
void tmTokenManager::tokenOrderOutOvertime(){
    if(getSelfInfo()->state == tmTokenOrderOutPending ){
        setSelfErrorState( tmOrderOutOverTime );
        tokenOrderOutCancel();
    }
}
//本机被动交出令牌 确认
int tmTokenManager::tokenOrderOutAck(){
    if(getSelfInfo()->state == tmTokenOrderOutPending ){
        setSelfState( tmPeerOnlinewithoutToken );
//      4、令牌转移应答确认(token transfer acknowleadge)
//      $TMTTA,源peer名称,目标peer名称<CR><LF>
        QByteArray ack;
        ack = "$TMTTA,";
        ack += getSelfInfo()->peerName;
        ack += ",";
        ack += getPartnerInfo()->peerName;
        ack += 0x0D;
        ack += 0x0A;
        qint64 res=clientSocket->writeDatagram(ack.data(), ack.size(),QHostAddress::Broadcast, tmPort);
        return res;
    }
    else return -1;
}
//本机被动交出令牌 取消
int tmTokenManager::tokenOrderOutCancel(){
    if(getSelfInfo()->state == tmTokenOrderOutPending ){
//      5、令牌转移取消(token transfer cancel)
//      $TMTTC,源peer名称,目标peer名称<CR><LF>
        QByteArray requrie;
        requrie = "$TMTTC,";
        requrie += getSelfInfo()->peerName;
        requrie += ",";
        requrie += getPartnerInfo()->peerName;
        requrie += 0x0D;
        requrie += 0x0A;
        qint64 res=clientSocket->writeDatagram(requrie.data(), requrie.size(),QHostAddress::Broadcast, tmPort);
        setSelfState( tmPeerOnlinewithToken );
        return res;
    }
    else return -1;
}

/////////////////////////////////4、本机被动获得令牌
//本机被动获得令牌 请求
int tmTokenManager::tokenOrderIn( qint32 overtime){
    if( pPeersList.at(1) <0 ) return -1;//找不到目标peer数据
    if(overtime <0) return -1;   //overTime数据不对
    if(getSelfInfo()->state !=tmPeerOnlinewithoutToken ) return -1;//本peer有token则返回-1
    if(getSelfInfo()->state == tmPeerDisable || getSelfInfo()->errorState == tmInnerError|| getSelfInfo()->errorState == tmNetworkError) return -1;//本peer未准备好
    if(getPartnerInfo()->state == tmPeerOnlinewithoutToken && getPartnerInfo()->errorState == tmInnerError) return -1;//目标peer没有准备好

    setSelfState( tmTokenOrderInPending );

    if(overtime>0){
        tokenOrderInOvertimer.setInterval(overtime);
        tokenOrderInOvertimer.start();
    }
    //qDebug()<<"tokenOrderIn"<<overtime<<tokenOrderInOvertimer.isActive();
    return 0;
}
//本机被动获得令牌 超时
void tmTokenManager::tokenOrderInOvertime(){
    if(getSelfInfo()->state == tmTokenOrderInPending ){
        setSelfErrorState( tmOrderInOverTime );
        tokenOrderInCancel();
        qDebug()<<"tokenOrderInOvertime";
    }
}
//本机被动获得令牌 确认
int tmTokenManager::tokenOrderInAck(){
    if(getSelfInfo()->state == tmTokenOrderInPending ){
        setSelfState( tmPeerOnlinewithToken );
//      4、令牌转移应答确认(token transfer acknowleadge)
//      $TMTTA,源peer名称,目标peer名称<CR><LF>
        QByteArray ack;
        ack = "$TMTTA,";
        ack += getPartnerInfo()->peerName;
        ack += ",";
        ack += getSelfInfo()->peerName;
        ack += 0x0D;
        ack += 0x0A;
        qint64 res=clientSocket->writeDatagram(ack.data(), ack.size(),QHostAddress::Broadcast, tmPort);
        return res;
    }
    else return -1;
}
//本机被动获得令牌 取消
int tmTokenManager::tokenOrderInCancel(){
    if(getSelfInfo()->state == tmTokenOrderInPending ){
//      5、令牌转移取消(token transfer cancel)
//      $TMTTC,源peer名称,目标peer名称<CR><LF>
        QByteArray requrie;
        requrie = "$TMTTC,";
        requrie += getPartnerInfo()->peerName;
        requrie += ",";
        requrie += getSelfInfo()->peerName;
        requrie += 0x0D;
        requrie += 0x0A;
        qint64 res=clientSocket->writeDatagram(requrie.data(), requrie.size(),QHostAddress::Broadcast, tmPort);
        setSelfState( tmPeerOnlinewithoutToken );
        return res;
    }
    else return -1;
}

//强制切出
int tmTokenManager::tokenForceOrderOut(){
    if(getSelfInfo()->state != tmPeerDisable){
        setSelfState( tmPeerOnlinewithoutToken );
        return 0;
    }
    else return -1;
}
//强制切入
int tmTokenManager::tokenForceTakeIn(){
    if(getSelfInfo()->state == tmPeerDisable || getSelfInfo()->errorState == tmInnerError|| getSelfInfo()->errorState == tmNetworkError) return -1;//本peer未准备好
//      6、令牌强制转移(toekn transfer forced)//用途：1.令牌强制获得。2、令牌丢失后，自动生成令牌
//      $TMTTF,目标peer名称<CR><LF>
    int index=findToken();
    if(index ==-2 || index ==0 ){
        //网上token过多or本peer有token 则返回-1
        return -1;
    }
    if (index >0){
        if( getPeerInfo(index)->priority > getSelfInfo()->priority ){
            //若现在的主站优先级比本peer高，则返回-1
            //qDebug()<<"here";
            return -1;
        }
    }
    QByteArray requrie;
    requrie = "$TMTTF,";
    requrie += getSelfInfo()->peerName;
    requrie += 0x0D;
    requrie += 0x0A;
    qint64 res=clientSocket->writeDatagram(requrie.data(), requrie.size(),QHostAddress::Broadcast, tmPort);
    setSelfState( tmPeerOnlinewithToken );
    return res;
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
bool tmTokenManager::isWithToken(int index){
    if ( pPeersList.at(index)->state == tmPeerOnlinewithToken ||
            pPeersList.at(index)->state == tmTokenTakeOutPending ||
                pPeersList.at(index)->state == tmTokenOrderOutPending   )
        return true;
    else return false;
}
//返回本机状态
bool tmTokenManager::isStarted(){
    return heartBeatTimer.isActive();
}

//判断本peer是否最高优先级。
bool tmTokenManager::isFirstPriority(){
    int self= getSelfInfo()->priority;
    int index=2;
    while(index<pPeersList.size() ){
        if(  pPeersList.at(index)->state != tmPeerDisable &&
             pPeersList.at(index)->state != tmPeerOffline &&
                self < pPeersList.at(index)->priority    )
            return false;
        index++;
    }

    return true;
}

//检查优先等级，发现相同优先级则ip地址最后一段较小的peer的优先级++
void tmTokenManager::checkPriority(){
    int self= getSelfInfo()->priority;
    qint8 aux1 = (qint8)getSelfInfo()->peerIp;
    qint8 aux2 = (qint8)getSelfInfo()->peerName.right(1).toLocal8Bit().toInt();
    //qDebug()<<"checkPriority";
    for (int index = 2;index<pPeersList.size();index++){
        if(self == pPeersList.at(index)->priority &&
                aux1 < (qint8)pPeersList.at(index)->peerIp){
            self++;
            getSelfInfo()->priority = self;
            emit peersListChanged(0);
            return;
        }
        if(self == pPeersList.at(index)->priority &&
                aux2 < (qint8)pPeersList.at(index)->peerName.right(1).toLocal8Bit().toInt()){
            self++;
            getSelfInfo()->priority = self;
            emit peersListChanged(0);
            return;
        }
    }
}

//查找报文中以逗号分割的参数
QByteArray tmTokenManager::datagramReadParameter(QByteArray & data, int *begin){
    QByteArray comma(",");
    QByteArray end;
    end.append(0x0D).append(0x0A);
    int len = data.indexOf(comma,*begin) - *begin;  //报文指定起始点开始找第一个comma，算长度

    QByteArray parameter;
    parameter.resize(len);
    parameter= data.mid(*begin,len);
    parameter.replace(end,QByteArray(""));
    *begin += (len +1);  //begine 迭代
    return parameter;
}

//找到令牌所在peer的index，找不到则返回-1，多于1个则返回-2，同时修改报警状态
int tmTokenManager::findToken(){
    int count=0;
    int ret =-1;
    if(isWithToken(0)){
        //qDebug()<<"here";
        count++;
        ret = 0;
    }
    int index=2;
    while(index<pPeersList.size()){
        if(isWithToken(index)){
            count++;
            //qDebug()<<count;
            ret = index;
        }
        index++;
    }
    if(count ==0) return -1;
    if(count ==1) return ret;
    if(count >1) return -2;
}

//set & get
QString tmTokenManager::getSelfStateString() const{
    return getStateString(0);
}
QString tmTokenManager::getSelfErrorStateString() const{
    return getErrorStateString(0);
}

QString tmTokenManager::getStateString(int index) const{
    if(index<0 || index>= pPeersList.size()) return QString::fromLocal8Bit("序号错误");
    switch(pPeersList.at(index)->state){
        case tmPeerDisable:
            return QString::fromLocal8Bit("停用");
        case tmPeerOnlinewithToken:
            return QString::fromLocal8Bit("在线有令牌");
        case tmPeerOnlinewithoutToken:
            return QString::fromLocal8Bit("在线无令牌");
        case tmTokenTakeOutPending:
            return QString::fromLocal8Bit("主动让出等待确认");
        case tmTokenOrderOutPending:
            return QString::fromLocal8Bit("被动让出等待确认");
        case tmTokenTakeInPending:
            return QString::fromLocal8Bit("主动获取等待确认");
        case tmTokenOrderInPending:
            return QString::fromLocal8Bit("被动获取等待确认");
        case tmPeerOffline:
            return QString::fromLocal8Bit("掉线");
        default:
            return QString::fromLocal8Bit("数据错误");
    }
}
QString tmTokenManager::getErrorStateString(int index) const{
    if(index<0 || index>= pPeersList.size()) return QString::fromLocal8Bit("序号错误");
    switch(pPeersList.at(index)->errorState){
        case tmNoError:
            return QString::fromLocal8Bit("无错误");
        case tmTokenLost:
            return QString::fromLocal8Bit("令牌丢失");
        case tmTokenMoreThanOne:
            return QString::fromLocal8Bit("令牌超过两个");
        case tmInnerError:
            return QString::fromLocal8Bit("内部错误");
        case tmTakeOutOverTime:
            return QString::fromLocal8Bit("主动让出等待超时");
        case tmTakeInOverTime:
            return QString::fromLocal8Bit("主动获取等待超时");
        case tmOrderOutOverTime:
            return QString::fromLocal8Bit("被动让出等待超时");
        case tmOrderInOverTime:
            return QString::fromLocal8Bit("被动获取等待超时");
        case tmNetworkError:
            return QString::fromLocal8Bit("网络错误，需要重启");
        default:
            return QString::fromLocal8Bit("数据错误");;
    }
}

QString tmTokenManager::getMasterPeerMessage() const{
    return masterPeerMessage;
}
void tmTokenManager::setMasterPeerMessage(QString& msg){
    if(masterPeerMessage == msg) return; //无改变
    masterPeerMessage = msg;
    emit masterPeerMessageUpdated();
}
QList<tmPeerInfo_t*> tmTokenManager::getPeersInfo() const{
    return pPeersList;
}

tmPeerInfo_t* tmTokenManager::getSelfInfo() const{
    return pPeersList[0];
}

tmPeerInfo_t* tmTokenManager::getPartnerInfo() const{
    return pPeersList[1];
}
int tmTokenManager::getPeerListSize(){
    return pPeersList.size();
}
int tmTokenManager::setSelfState(tmPeerState_e newState){
    if(getSelfInfo()->state == newState) return -1; //无状态改变
    getSelfInfo()->state = newState;
    //qDebug()<<"setSelfState"<<newState;
    emit selfStateChanged();
    emit peersListChanged(0);
    return 0;
}
int tmTokenManager::setSelfErrorState(tmPeerErrorState_e newErrorState){
    if(getSelfInfo()->errorState == newErrorState) return -1; //无状态改变
    getSelfInfo()->errorState = newErrorState;
    emit selfErrorStateChanged();
    emit peersListChanged(0);
    return 0;
}
int tmTokenManager::setPeersInfo(QList<tmPeerInfo_t*>& newlist){
    if(newlist.size()>1){
        clearPeerInfo();//释放内存
        for (int i=0; i<newlist.size();i++){
            setPeerInfo( newlist.at(i) );
        }
        checkPriority();
        emit selfErrorStateChanged();
        emit selfStateChanged();

        return 0;
    }
    else return -1;
}

int tmTokenManager::setSelfInfo(tmPeerInfo_t* self){
    //qDebug()<<"setSelfInfo";
    switch(pPeersList.size()){
        case 0:
            pPeersList.append(new tmPeerInfo_t());

        default:

            if(self->peerName != "" && self->peerIp != 0){

                *(pPeersList.at(0)) = *self;
                checkPriority();
                if(!isStarted()){
                    getSelfInfo()->state = tmPeerDisable;
                }
                emit selfErrorStateChanged();
                emit selfStateChanged();
                emit peersListChanged(0);
                return 0;
            }
            else return -1;
    }
}


int tmTokenManager::setPeerInfo(tmPeerInfo_t *newone){
    //qDebug()<<"setPeerInfo";
    if(newone->peerName != "" && newone->peerIp != 0x0){
        tmPeerInfo_t* it =0;
        int i;
        for (i=2; i<pPeersList.size();i++){
            it = pPeersList.at(i);
            if( it->peerName == newone->peerName){
                if( it->state == newone->state &&
                        it->errorState  ==  newone->errorState &&
                        it->peerIp      ==  newone->peerIp &&
                        it->priority    ==  newone->priority){
                    //完全相同，只要更新时间
                    it->lastUpdateTime.start();
                    return -2;
                }

                *it = *newone;//发现同名节点，替换
                checkPriority();
                emit peersListChanged(i);
                return 0;
            }
        }
        it = new tmPeerInfo_t();
        *it = *newone;
        pPeersList.append(it);//未发现同名节点，新建
        checkPriority();
        emit peersListChanged(i);
        return 0;
    }
    else return -1;
}
tmPeerInfo_t tmTokenManager::generateSelfInfo(){
    tmPeerInfo_t ret;
    QTime time = QTime::currentTime();
    qsrand(time.msec());
    ret.peerName = tr("default_station_%1").arg(qrand());

    foreach( QHostAddress t, QNetworkInterface::allAddresses()){
        if (t.protocol() == QAbstractSocket::IPv4Protocol && !t.isLoopback()){
            ret.peerIp = t.toIPv4Address();
            //qDebug()<<t;
        }
    }

    ret.priority = 0 ;
    ret.lastUpdateTime = QTime::currentTime();
    ret.errorState = tmNoError;
    ret.state = tmPeerDisable;
    return ret;
}

int tmTokenManager::setPartnerInfo(tmPeerInfo_t *partner){
    if(partner <=0) return -1;//指针不对
    checkPriority();
    switch(pPeersList.size()){
        case 0:
            return -1;
        case 1:
            pPeersList.append(pPeersList[0]);
            emit peersListChanged(1);
            return 0;
        default:
            if(partner > 0){
                pPeersList[1] = partner;
                emit peersListChanged(1);
                return 0;
            }
            else return -1;
    }
}

tmPeerInfo_t* tmTokenManager::getPeerInfo(QString& name) const{
    tmPeerInfo_t* it =0;
    for (int i=2; i<pPeersList.size();i++){
        it = pPeersList.at(i);
        if( it->peerName == name){
            return it;
        }
    }
    return NULL;
}

tmPeerInfo_t* tmTokenManager::getPeerInfo(int index) const{
    if(index<0 || index>= pPeersList.size()) return NULL;
    return pPeersList.at(index);
}

int tmTokenManager::setPeerState(int index, tmPeerState_e newState){
    if(index<0 || index>= pPeersList.size()) return -1;
    if(newState<0) return -1;
    pPeersList.at(index)->state = newState;
    emit peersListChanged(index);
    return 0;
}

int tmTokenManager::setPeerErrorState(int index, tmPeerErrorState_e newErrorState){
    if(index<0 || index>= pPeersList.size()) return -1;
    if(newErrorState<0) return -1;
    pPeersList.at(index)->errorState = newErrorState;
    emit peersListChanged(index);
    return 0;
}
