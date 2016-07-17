#include "tmeasyapi.h"

tmEasyAPI::tmEasyAPI(QObject *parent):tmTokenManager(parent)
{
    connect(getPeers().at(0),SIGNAL(msgStateChanged(quint64)),this,SLOT(tmEasySelfStateChanged(quint64)));
}
tmEasyAPI::~tmEasyAPI(){

}

void tmEasyAPI::tmEasyAck(){
    if(getSelfPeer()->getState() == tmPeer::stateOnlinewithoutToken){
        tokenForceTakeIn();
    }
    else if(getSelfPeer()->getState() == tmPeer::stateTokenTakeInPending){
        tokenForceTakeIn();
    }
    else if(getSelfPeer()->getState() == tmPeer::stateTokenOrderOutPending){
        tokenOrderOutAck();
    }
    else if(getSelfPeer()->getState() == tmPeer::stateTokenOrderInPending){
        tokenOrderInAck();
    }
    else {
    }
}

void tmEasyAPI::tmEasyCancel(){
    if(getSelfPeer()->getState() == tmPeer::stateTokenTakeOutPending){
        tokenTakeOutCancel();
    }
    else if(getSelfPeer()->getState() == tmPeer::stateTokenTakeInPending){
        tokenTakeInCancel();
    }
    else if(getSelfPeer()->getState() == tmPeer::stateTokenOrderOutPending){
        tokenOrderOutCancel();
    }
    else if(getSelfPeer()->getState() == tmPeer::stateTokenOrderInPending){
        tokenOrderInCancel();
    }
    else {
    }
}

void tmEasyAPI::tmEasyTakeIn(int index, qint32 overtime){
    tokenTakeIn(index, overtime);
}

void tmEasyAPI::tmEasyTakeOut(int index, qint32 overtime){
    tokenTakeOut(index, overtime);
}

void tmEasyAPI::tmEasyListClear(){
    clearPeerInfo();
    generateSelfInfo();
    //qDebug()<<temp.errorState<<getSelfInfo()->errorState;
    restart();
}

void tmEasyAPI::tmEasySelfStateChanged(quint64 state){
    //qDebug()<<state;
    if( state == tmPeer::stateOnlinewithToken){
        emit tmEasyTakeOutEnableChange(true);
        emit tmEasyTakeInEnableChange(false);
        emit tmEasyCancelEnableChange(false);
        emit tmEasyAckEnableChange(false);
        emit tmEasyAckStringChange(QString::fromLocal8Bit("确认"));
    }
    else if(state == tmPeer::stateOnlinewithoutToken){
        emit tmEasyTakeOutEnableChange(false);
        emit tmEasyTakeInEnableChange(true);
        emit tmEasyCancelEnableChange(false);
        emit tmEasyAckEnableChange(true);
        emit tmEasyAckStringChange(QString::fromLocal8Bit("强切"));
    }
    else if(state == tmPeer::stateTokenTakeOutPending){
        emit tmEasyTakeOutEnableChange(false);
        emit tmEasyTakeInEnableChange(false);
        emit tmEasyCancelEnableChange(true);
        emit tmEasyAckEnableChange(false);
        emit tmEasyAckStringChange(QString::fromLocal8Bit("确认"));
    }
    else if(state == tmPeer::stateTokenTakeInPending){
        emit tmEasyTakeOutEnableChange(false);
        emit tmEasyTakeInEnableChange(false);
        emit tmEasyCancelEnableChange(true);
        emit tmEasyAckEnableChange(true);
        emit tmEasyAckStringChange(QString::fromLocal8Bit("强切"));
    }
    else if(state == tmPeer::stateTokenOrderOutPending){
        emit tmEasyTakeOutEnableChange(false);
        emit tmEasyTakeInEnableChange(false);
        emit tmEasyCancelEnableChange(true);
        emit tmEasyAckEnableChange(true);
        emit tmEasyAckStringChange(QString::fromLocal8Bit("确认"));
    }
    else if(state == tmPeer::stateTokenOrderInPending){
        emit tmEasyTakeOutEnableChange(false);
        emit tmEasyTakeInEnableChange(false);
        emit tmEasyCancelEnableChange(true);
        emit tmEasyAckEnableChange(true);
        emit tmEasyAckStringChange(QString::fromLocal8Bit("确认"));
    }
    else if(state == tmPeer::stateDisable){
        emit tmEasyTakeOutEnableChange(false);
        emit tmEasyTakeInEnableChange(false);
        emit tmEasyCancelEnableChange(false);
        emit tmEasyAckEnableChange(false);
        emit tmEasyAckStringChange(QString::fromLocal8Bit("确认"));
    }
    else {

    }
}
QString tmEasyAPI::getErrorString(quint64 errorCode, int lang) const{
    //qDebug()<<"tmEasyAPI::getErrorString 1#"<<errorCode;
    return tmTokenManager::getErrorString(errorCode,lang);
}

