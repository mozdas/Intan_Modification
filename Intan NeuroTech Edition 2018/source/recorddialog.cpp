#include "recorddialog.h"

#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif
#include <iostream>

#include "globalconstants.h"
#include "spikescopedialog.h"
#include "signalchannel.h"
#include "signalgroup.h"
#include "spikeplot.h"
#include "recordplot.h"

//record dialog is for recording psth data in another dialog box.
RecordDialog::RecordDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Recorded Psth Data"));

    recordPlot=new RecordPlot(this,this);







    clearScopeButton = new QPushButton(tr("Clear Scope"));

    connect(clearScopeButton, SIGNAL(clicked()),
            this, SLOT(clearScope()));







    yScaleList.append(0.1);
    yScaleList.append(0.2);
    yScaleList.append(0.5);
    yScaleList.append(1);


    yScaleComboBox = new QComboBox();
    for (int i = 0; i < yScaleList.size(); ++i) {
        yScaleComboBox->addItem( QString::number(yScaleList[i]) +
                                " ");
    }

    yScaleComboBox->setCurrentIndex(3);

    selectedFrameComboBox=new QComboBox();
    for (int i = 0; i < 20; ++i) {
        selectedFrameComboBox->addItem(QString::number(i)
                                );
    }
    connect(selectedFrameComboBox,SIGNAL(currentIndexChanged(int)),
            this,SLOT(changeSelectedFrame(int)));


    connect(yScaleComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeYScale(int)));



    QHBoxLayout *displayLayout = new QHBoxLayout;
    displayLayout->addWidget(new QLabel(tr("Selected Frame: ")));
    displayLayout->addWidget(selectedFrameComboBox);
    displayLayout->addWidget(new QLabel(tr("Y Scale:")));
    displayLayout->addWidget(yScaleComboBox);

    displayLayout->addWidget(clearScopeButton);
    displayLayout->addStretch(1);



    QGroupBox *displayGroupBox = new QGroupBox(tr("Display Settings"));
    displayGroupBox->setLayout(displayLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(displayGroupBox);
    mainLayout->addWidget(recordPlot);

    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);

    setLayout(mainLayout);


}
void RecordDialog::changeSelectedFrame(int frame){
    recordPlot->changeSelectedFrame(frame);
}

void RecordDialog::changeYScale(int index)
{
    recordPlot->setYScale(yScaleList[index]);
}

void RecordDialog::setYScale(int index)
{
    yScaleComboBox->setCurrentIndex(index);
    recordPlot->setYScale(yScaleList[index]);
}


void RecordDialog::updateRecordPlot(){
    recordPlot->updateRecordPlot();
}

void RecordDialog::updateWaveform(QVector<double> waveform,double tScale)
{
    recordPlot->updateWaveform(waveform,tScale);
}



void RecordDialog::clearScope()
{
    recordPlot->clearScope();
}

void RecordDialog::expandYScale()
{
    if (yScaleComboBox->currentIndex() > 0) {
        yScaleComboBox->setCurrentIndex(yScaleComboBox->currentIndex() - 1);
        changeYScale(yScaleComboBox->currentIndex());
    }
}

void RecordDialog::contractYScale()
{
    if (yScaleComboBox->currentIndex() < yScaleList.size() - 1) {
        yScaleComboBox->setCurrentIndex(yScaleComboBox->currentIndex() + 1);
        changeYScale(yScaleComboBox->currentIndex());
    }
}
