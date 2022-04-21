#ifndef TempTFDesigner_H
#define TempTFDesigner_H

#include <QDockWidget>
#include "TransferFunction.h"

namespace Ui {
class TempTFDesigner;
}

class TempTFDesigner : public QDockWidget
{
    Q_OBJECT

public:
    explicit TempTFDesigner(QWidget *parent = 0);
    ~TempTFDesigner();
    TransferFunction * getTF(){return myTF;}


signals:
    void updateVRView();

private:
    Ui::TempTFDesigner *ui;
    VolumeDataInterface* data;
    bool updateAutoFlag;
    TransferFunction *myTF;  //this is the really owner of this object
};

#endif // TempTFDesigner_H
