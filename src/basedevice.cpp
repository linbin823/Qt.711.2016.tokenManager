#include "basedevice.h"

baseDevice::baseDevice(QObject *parent):QObject(parent),siErrMsg()
{
    state = 0;
    error = 0;
    errorMask = 0;
    //errorMask,对应位=1则不能被外部复位,（必须使用protected:int resetErrorState(quint64 errorCode)复位）
    //默认都可以被外部复位
}

//State
inline quint64 baseDevice::getState() const{
    return state;
}

//error
QStringList baseDevice::getErrorStringList(int lang) const{
    QStringList ret;
    quint64 test=1;
    //qDebug()<<"baseDevice::getErrorStringList 1#"<<getError();
    if(getError() == 0){
            ret<<QString::fromLocal8Bit("无错误");
            return ret;
    }
    //qDebug()<<"baseDevice::getErrorStringList 2#";
    for(int i=0; i<=63; i++){
       if(test & getError()){
           //qDebug()<<"baseDevice::getErrorStringList 3#"<<test;
           ret<<getErrorString(test);
           //qDebug()<<"baseDevice::getErrorStringList 4#"<<ret;
       }
       test = test<<1;
    }
    //qDebug()<<"baseDevice::getErrorStringList 5#"<<ret;
    return ret;
}
bool baseDevice::getError(quint64 errorCode) const{
    if(error & errorCode) {
        return true;
    }
    return false;
}
quint64 baseDevice::getError() const{
    return error;
}

//errorMask,对应位=1则不能被外部复位,（必须使用protected:int resetErrorState(quint64 errorCode)复位）
//默认都可以被外部复位
quint64 baseDevice::getErrorMask() const{
    return errorMask;
}

//外部复位，全复位
void baseDevice::resetAll(){
    reset(0xFFFFFFFFFFFFFFFF);
}
//外部复位，
void baseDevice::reset(quint64 resetCode){
    //外部输入 restCode=  0x0000 0010
    //       errorMask=  0x0000 0110
    //实际允许外部复位的 restCode = 0x0000 0000
    resetCode = resetCode & (~errorMask);
    resetError( resetCode);
}


//State修改
inline void baseDevice::setState(quint64 newState){
    if(state != newState){
        state = newState;
        emit msgStateChanged(state);
        //qDebug()<<"newState";
    }
}
//error修改
int baseDevice::setError(quint64 errorCode){
    int qty=0;
    quint64 test = 1;
    for(int i=0;i<=63;i++){
        //errorCode  = 0x0000 0110 初始
        //test       = 0x0000 0001 初始
        //error      = 0x0000 1100 初始
        //~error     = 0x1111 0011 取反
        //新报警 errorCode & test & ~errorState
        if(errorCode & test & (~error) ){
            //test       = 0x0000 0001 初始
            //error      = 0x0000 1100 初始
            //error      = 0x0000 1101 结果
            //qDebug()<<"baseDevice::setError1#"<<error<<i<<test<<errorCode;
            error = error | test;
            //qDebug()<<"baseDevice::setError2#";
            emit msgErrorSet(test);
            //qDebug()<<"baseDevice::setError3#";
            qty++;
        }
        //errorCode  = 0x0000 0110 初始
        //~errorCode = 0x1111 1001 取反
        //test       = 0x0000 1000 初始
        //error      = 0x0000 1100 初始
        //去报警 (~errorCode) & test & error
        if( (~errorCode) & test & error ){
            //test       = 0x0000 1000 初始
            //~test      = 0x1111 0111 取反
            //error      = 0x0000 1100 初始
            //error      = 0x0000 0100 结果
            //qDebug()<<"baseDevice::setError4#"<<error<<i<<test<<errorCode;
            error = error & (~test);
            //qDebug()<<"baseDevice::setError5#";
            emit msgErrorReset(test);
            //qDebug()<<"baseDevice::setError6#";
            qty++;
        }
        test = test<<1;
    }
    return qty;
}

int baseDevice::resetError(quint64 resetCode){
    //resetCode  = 0x0000 0110 初始
    //test       = 0x0000 0001 初始
    //error      = 0x0000 1100 初始
    //发射 resetCode & test & error
    int qty = 0;
    quint64 test = 1;
    for(int i=0;i<=63;i++){
        if( resetCode & error & test ){
        //test       = 0x0000 0100 初始
        //~test      = 0x1111 1011 取反
        //error      = 0x0000 1100 初始
        //error      = 0x0000 1000 结果
            error &= (~test);
            emit msgErrorReset(test);
            //qDebug()<<"goneError"<<error;
            qty++;
        }
        test = test<<1;
    }
    return qty;
}

//errorMask修改
int baseDevice::setErrorMask(quint64 errorCode){
    errorMask = errorCode;
    return 0;
}
