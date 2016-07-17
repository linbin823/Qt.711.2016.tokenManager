/**************************************************************************************************
 * standard interface
 * load & save
 * 状态保存&读取的标准接口
 *
 **************************************************************************************************/
#ifndef SILOADSAVE_H
#define SILOADSAVE_H

#include <QObject>
#include "sierrmsg.h"

class siLoadSave : public siErrMsg
{
public:
    explicit siLoadSave(){}

    virtual int saveParameters()=0;
    virtual int loadParameters()=0;
};

#endif // SILOADSAVE_H
