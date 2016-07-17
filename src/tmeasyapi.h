#ifndef TMEASYAPI_H
#define TMEASYAPI_H

#include <QObject>
#include <tmtokenmanager.h>


class tmEasyAPI :  public tmTokenManager
{
    Q_OBJECT
public:

    tmEasyAPI(QObject *parent = 0);
    ~tmEasyAPI();
//controls
public slots:
    Q_INVOKABLE void tmEasyAck();
    Q_INVOKABLE void tmEasyCancel();
    Q_INVOKABLE void tmEasyTakeIn(int index, qint32 overtime=10000);
    Q_INVOKABLE void tmEasyTakeOut(int index, qint32 overtime=10000);
    Q_INVOKABLE void tmEasyListClear();
private slots:

    void tmEasySelfStateChanged(quint64 state);
//    void tmEasySelfErrorChanged(quint64 error);

signals:
    void tmEasyAckEnableChange(bool enable);
    void tmEasyCancelEnableChange(bool enable);
    void tmEasyTakeInEnableChange(bool enable);
    void tmEasyTakeOutEnableChange(bool enable);
    void tmEasyAckStringChange(QString str);
protected:
    virtual QString getErrorString(quint64 errorCode, int lang=0) const;
};

#endif // TMEASYAPI_H
