#ifndef TMPEER_H
#define TMPEER_H

#include <QObject>
#include <QTime>
#include <QTimer>
#include <QHostAddress>
#include <QNetworkInterface>
#include "basedevice.h"
#include "iloadsaveprocessor.h"

class tmPeer : public baseDevice
{
    Q_OBJECT
public:
    explicit tmPeer(QObject *parent = 0);

    enum state: quint64{
        stateDisable                   = 0x0000000000000001ULL,      //peer禁用
        stateOnlinewithToken           = 0x0000000000000002ULL,      //peer在线有令牌，主站
        stateOnlinewithoutToken        = 0x0000000000000004ULL,      //peer在线无令牌，从站
        stateTokenTakeOutPending       = 0x0000000000000008ULL,      //该peer主动把令牌给其他peer，但其他peer还未确认。该peer具有令牌，主站。
        stateTokenOrderOutPending      = 0x0000000000000010ULL,      //其他peer要求该peer的令牌，该peer还未确认。该peer具有令牌，主站。
        stateTokenTakeInPending        = 0x0000000000000020ULL,      //该peer主动要获得令牌，但令牌持有peer还未确认。该peer无令牌，从站。
        stateTokenOrderInPending       = 0x0000000000000040ULL,      //令牌持有peer要求把令牌传送给该peer，但该peer还未确认。该peer无令牌，从站。
        stateOffline                   = 0x0000000000000080ULL,      //peer离线。此状态不适用于本站，用于判断其他peer的掉线状态。
    };
    virtual  QString getStateString(int lang = 0 ) const{
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
    enum errorType : quint64 {
        errorInnerError                = 0x0000000000000001ULL,      //peer内部故障，致命必须重启
        errorTakeOutOverTime           = 0x0000000000000002ULL,
        errorTakeInOverTime            = 0x0000000000000004ULL,
        errorOrderOutOverTime          = 0x0000000000000008ULL,
        errorOrderInOverTime           = 0x0000000000000010ULL,
    };
    virtual QString getErrorString(quint64 errorCode, int lang = 0 ) const{
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

    virtual int save(iLoadSaveProcessor* processor);
    virtual int load(iLoadSaveProcessor* processor);


public:
    void setPeerPriority(quint32 pri);
    void setPeerIp(quint32 ip);

    quint32 getPeerPriority() const;
    quint32 getPeerIp() const;

    void update(QString& name, quint64 state, quint64 error, quint32 pri, quint32 ip);
    void update();
    bool isWithToken();  //判断是否有令牌，有=true

    bool stopCheckOffline();//对于本peer，不用检查掉线

signals:
    void msgPriorityChanged(qint32 pri);
    void msgIpChanged(qint32 ip);

private slots:
    void offlineCheck();
//    void peerChanged();

private:
    quint32 peerIp;
    quint32 priority;
    QTime lastUpdateTime;

    QTimer offlineCheckTimer;
    int offlineDelay;
    int offlineCheckInterv;

};

#endif // TMPEER_H
