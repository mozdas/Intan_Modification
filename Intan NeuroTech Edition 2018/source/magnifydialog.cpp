#include "magnifydialog.h"
//  ------------------------------------------------------------------------
//
//  This file is part of the Intan Technologies RHD2000 Interface
//  Version 1.5.1
//  Copyright (C) 2013-2017 Intan Technologies
//
//  ------------------------------------------------------------------------
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif
#include <iostream>

#include "globalconstants.h"

#include "signalchannel.h"
#include "signalgroup.h"
#include "magnifyplot.h"

// Magnify dialog.


MagnifyDialog::MagnifyDialog(SignalProcessor *inSignalProcessor, SignalSources *inSignalSources,
                                   SignalChannel *initialChannel, QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Magnify"));

    signalProcessor = inSignalProcessor;
    signalSources = inSignalSources;

    magnifyPlot = new MagnifyPlot(signalProcessor, initialChannel, this, this);
    currentChannel = initialChannel;


    clearScopeButton = new QPushButton(tr("Clear Scope"));


    connect(clearScopeButton, SIGNAL(clicked()),
            this, SLOT(clearScope()));
    tScaleList.append(200);
    tScaleList.append(500);
    tScaleList.append(1000);
    tScaleList.append(2000);

    tScaleComboBox=new QComboBox();
    for(int i=0;i<tScaleList.size();i++){
        tScaleComboBox->addItem(QString::number(tScaleList[i])+"ms");
    }
    tScaleComboBox->setCurrentIndex(0);

    connect(tScaleComboBox,SIGNAL(currentIndexChanged(int)),
            this,SLOT(changeTScale(int)));


    yScaleList.append(50);
    yScaleList.append(100);
    yScaleList.append(200);
    yScaleList.append(500);
    yScaleList.append(1000);
    yScaleList.append(2000);
    yScaleList.append(5000);

    yScaleComboBox = new QComboBox();
    for (int i = 0; i < yScaleList.size(); ++i) {
        yScaleComboBox->addItem("+/-" + QString::number(yScaleList[i]) +
                                " " + QSTRING_MU_SYMBOL + "V");
    }
    yScaleComboBox->setCurrentIndex(3);

    connect(yScaleComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeYScale(int)));



    QHBoxLayout *displayLayout = new QHBoxLayout;
    displayLayout->addWidget(new QLabel(tr("Voltage Scale:")));
    displayLayout->addWidget(yScaleComboBox);
    displayLayout->addWidget(new QLabel(tr("Time Scale:")));
    displayLayout->addWidget(tScaleComboBox);

    displayLayout->addWidget(clearScopeButton);



    QGroupBox *displayGroupBox = new QGroupBox(tr("Display Settings"));
    displayGroupBox->setLayout(displayLayout);



    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(displayGroupBox);
    mainLayout->addWidget(magnifyPlot);
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);

    setLayout(mainLayout);


}

void MagnifyDialog::changeYScale(int index)
{
    magnifyPlot->setYScale(yScaleList[index]);
}

void MagnifyDialog::setYScale(int index)
{
    yScaleComboBox->setCurrentIndex(index);
    magnifyPlot->setYScale(yScaleList[index]);
}
void MagnifyDialog::changeTScale(int index)
{
    magnifyPlot->setTScale(tScaleList[index]);
}

void MagnifyDialog::setTScale(int index)
{
    tScaleComboBox->setCurrentIndex(index);
    magnifyPlot->setTScale(tScaleList[index]);
}

void MagnifyDialog::setSampleRate(double newSampleRate)
{
    magnifyPlot->setSampleRate(newSampleRate);
}

// Select a voltage trigger if index == 0.
// Select a digital input trigger if index == 1.



void MagnifyDialog::updateWaveform(int numBlocks)
{
    magnifyPlot->updateWaveform(numBlocks);
}




void MagnifyDialog::clearScope()
{
    magnifyPlot->clearScope();
}







// Set Spike Scope to a new signal channel source.
void MagnifyDialog::setNewChannel(SignalChannel* newChannel)
{
    magnifyPlot->setNewChannel(newChannel);
    currentChannel = newChannel;



}

void MagnifyDialog::expandYScale()
{
    if (yScaleComboBox->currentIndex() > 0) {
        yScaleComboBox->setCurrentIndex(yScaleComboBox->currentIndex() - 1);
        changeYScale(yScaleComboBox->currentIndex());
    }
}

void MagnifyDialog::contractYScale()
{
    if (yScaleComboBox->currentIndex() < yScaleList.size() - 1) {
        yScaleComboBox->setCurrentIndex(yScaleComboBox->currentIndex() + 1);
        changeYScale(yScaleComboBox->currentIndex());
    }
}

// Apply trigger settings to all channels on selected port.
/*
void MagnifyDialog::applyToAll()
{
    QMessageBox::StandardButton r;
    r = QMessageBox::question(this, tr("Trigger Settings"),
                                 tr("Do you really want to copy the current channel's trigger "
                                    "settings to <b>all</b> amplifier channels on this port?"),
                                 QMessageBox::Yes | QMessageBox::No);
    if (r == QMessageBox::Yes) {
        for (int i = 0; i < currentChannel->signalGroup->numChannels(); ++i) {
            currentChannel->signalGroup->channel[i].voltageTriggerMode = currentChannel->voltageTriggerMode;
            currentChannel->signalGroup->channel[i].voltageThreshold = currentChannel->voltageThreshold;
            currentChannel->signalGroup->channel[i].digitalTriggerChannel = currentChannel->digitalTriggerChannel;
            currentChannel->signalGroup->channel[i].digitalEdgePolarity = currentChannel->digitalEdgePolarity;
        }
    }
}
*/
