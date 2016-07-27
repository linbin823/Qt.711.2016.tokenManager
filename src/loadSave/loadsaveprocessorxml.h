#ifndef LOADSAVEPROCESSORXML_H
#define LOADSAVEPROCESSORXML_H

#include <QDebug>
#include <QList>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QCoreApplication>
#include <QDir>
#include <QtXmlPatterns>
#include <QFileInfo>
#include "siloadsaveprocessor.h"
#include "basedevice.h"

class loadSaveProcessorXml :  public baseDevice, public siLoadSaveProcessor
{
    Q_OBJECT
public:
    loadSaveProcessorXml(QObject *parent = 0);
    ~loadSaveProcessorXml();

    Q_PROPERTY(QString resXmlFilePath READ getResXmlFilePath WRITE setResXmlFilePath NOTIFY resXmlFilePathChanged)

    enum state: quint64{
        stateNotReady        = 0x0000000000000001ULL,      //Save&Load处理器未就绪
        stateReady           = 0x0000000000000002ULL,      //Save&Load处理器就绪
        stateOccupied        = 0x0000000000000004ULL,      //Save&Load处理器占用中
        stateReadFile        = 0x0000000000000008ULL,      //Save&Load处理器正在读文件
        stateWriteFile       = 0x0000000000000010ULL,      //Save&Load处理器正在写文件
    };
    virtual QString getStateString(int lang = 0 ) const{
         switch(getState()){
         case stateNotReady:
             return QString::fromLocal8Bit("Save&Load处理器未就绪");
         case stateReady:
             return QString::fromLocal8Bit("Save&Load处理器就绪");
         case stateOccupied:
             return QString::fromLocal8Bit("Save&Load处理器占用中");
         case stateReadFile:
             return QString::fromLocal8Bit("Save&Load处理器正在读文件");
         case stateWriteFile:
             return QString::fromLocal8Bit("Save&Load处理器正在写文件");
         default:
             return QString::fromLocal8Bit("数据错误");
         }
     };
    enum errorType : quint64 {
        errorFlieFomatWrong              = 0x0000000000000001ULL,       //xml文件格式错误
        errorFileNotFound                = 0x0000000000000002ULL,       //xml文件找不到，新建
        errorFileOpenFail                = 0x0000000000000004ULL,       //xml文件打开失败
        errorFileWriteFail               = 0x0000000000000008ULL,       //xml文件写入失败
        errorLoadFail                    = 0x0000000000000010ULL,       //读取时遇到错误
        errorSaveFail                    = 0x0000000000000020ULL,       //保存时遇到错误
        errorFileNameError               = 0x0000000000000040ULL,       //xml文件名称错误
    };
protected:
    virtual QString getErrorString(quint64 errorCode, int lang =0 ) const{
        switch(errorCode){
            case errorFlieFomatWrong:
                return QString::fromLocal8Bit("xml文件格式错误");
            case errorFileNotFound:
                return QString::fromLocal8Bit("xml文件找不到，新建");
            case errorFileOpenFail:
                return QString::fromLocal8Bit("xml文件打开失败");
            case errorFileWriteFail:
                return QString::fromLocal8Bit("xml文件写入失败");
            case errorLoadFail:
                return QString::fromLocal8Bit("读取时遇到错误");
            case errorSaveFail:
                return QString::fromLocal8Bit("保存时遇到错误");
            case errorFileNameError:
                return QString::fromLocal8Bit("xml文件名称错误");
            default:
                return QString::fromLocal8Bit("数据错误");
        }
    }

public:

    virtual int loadParameters(QString & paraName, QString *paraValue);
    virtual int saveParameters(QString & paraName, QString & paraValue);
    virtual int moveToInstance(QString & ObjType, QString & index);
    virtual int createNewInstance(QString & ObjType, QString & InstID);
    virtual int moveBackToParent();

    void    setResXmlFilePath(const QString &name);
    QString getResXmlFilePath(void);

    //不实现siLoadSave
    virtual int load(siLoadSaveProcessor *processor){return 0;}
    virtual int save(siLoadSaveProcessor *processor){return 0;}

    virtual int transactionStart();
    virtual int transactionEnd();
    virtual int loadFile(QString fileName = NULL);
    virtual int saveFile(QString fileName = NULL);
private:

    QString resXmlFilePath;//给QML用的
    QString resXmlFilePathWithoutProtocol;//自己用的
    QDomDocument resXml;//本实例唯一的DomDoc
    QList<QDomElement> domElementParentList;//element父目录清单

    int initXmlFile();
    int readXmlFile();
    int writeXmlFile();

    int getElement(QDomElement& res, QString tagName, QString id = QString::null);
    int setElement(QDomElement& res, QString tagName, QString id = QString::null);

    inline QDomElement getParent(){
        if (domElementParentList.size() ==0 )
            return resXml.childNodes().at(0).toElement();
        return domElementParentList.last();
    }
    inline int popParent(){
        if (domElementParentList.size() ==1 )
            return -1;
        domElementParentList.removeLast();
        return 0;
    }
    inline int pushParent(QDomElement newone){
        domElementParentList.append(newone);
        return 0;
    }


signals:
    void resXmlRefresh   (void);
    void resXmlFilePathChanged (void);

};

#endif // loadSaveProcessorXml_H
