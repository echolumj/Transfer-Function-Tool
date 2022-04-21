#include "TempTFDesigner.h"
#include "ui_tempTFDesigner.h"
#include "TFInteraction.h"

TempTFDesigner::TempTFDesigner(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::TempTFDesigner)
{
    ui->setupUi(this);

}

TempTFDesigner::~TempTFDesigner()
{
    delete ui;
}
