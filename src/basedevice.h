#ifndef BASEDEVICE_H
#define BASEDEVICE_H

#include <QObject>
#include <QDebug>
#include "sierrmsg.h"
#include "sistatemsg.h"

class baseDevice: public QObject, public siErrMsg, public siStateMsg
{
    Q_OBJECT
public:
    baseDevice(QObject *parent = 0);
    //State
    virtual inline quint64 getState() const;

    //error
    virtual QStringList getErrorStringList(int lang=0) const;
    virtual bool getError(quint64 errorCode) const;
    virtual quint64 getError() const;

    //errorMask,对应位=1则不能被外部复位,（必须使用protected:int resetErrorState(quint64 errorCode)复位）
    //默认都可以被外部复位
    virtual quint64 getErrorMask() const;

    //外部复位
    virtual void resetAll();//外部复位，全复位
    virtual void reset(quint64 resetCode);//外部复位，

    //state修改
    virtual inline void setState(quint64 newState);
    //error修改
    virtual int setError(quint64 errorCode);
    virtual int resetError(quint64 resetCode);
    //errorMask修改
    virtual int setErrorMask(quint64 errorCode);


signals:
    void msgStateChanged(quint64 state);
    void msgErrorSet    (quint64 errorCode);
    void msgErrorReset  (quint64 errorCode);

private:
    quint64 error;
    quint64 errorMask;
    quint64 state;
};

#endif // BASEDEVICE_H
