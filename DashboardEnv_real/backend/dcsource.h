#ifndef DCSOURCE_H
#define DCSOURCE_H
#include <backend/basepowersource.h>
class DCsource :  public BasePowerSource
{
    Q_OBJECT
public:
    explicit DCsource(QObject *parent = nullptr): BasePowerSource(parent){}
};

#endif // DCSOURCE_H
