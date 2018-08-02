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
#include <spikeplot.h>
#include "globalconstants.h"
#include "psthdialog.h"
#include "signalchannel.h"
#include "signalgroup.h"
#include "psthplot.h"
#include "triggerplot.h"
#include "recorddialog.h"
#include "psthanalysisdialog.h"

//psth dialog is for the psth analysis tool
PsthDialog::PsthDialog(SignalProcessor *inSignalProcessor, SignalSources *inSignalSources,
                                   SignalChannel *initialChannel, QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("PSTH Dialog"));

    signalProcessor = inSignalProcessor;
    signalSources = inSignalSources;

    psthPlot = new PsthPlot(signalProcessor, initialChannel, this, this);
    spikePlot = new SpikePlot(signalProcessor,initialChannel,0,this,false,this);
     triggerPlot = new TriggerPlot(signalProcessor,initialChannel,this,this);
     recordDialog=new RecordDialog(this);

    currentChannel = initialChannel;

    //analysis dialog is not generated until the button is pressed

    analysisDialog=0;

    resetToZeroButton = new QPushButton(tr("Zero"));
    clearScopeButton = new QPushButton(tr("Clear Scope"));
    applyToAllButton = new QPushButton(tr("Apply to Entire Port"));
    psthAnalysisButton = new QPushButton("Enable Psth Analysis");

   resetPsthButton =new QPushButton(tr("Reset PSTH"));
   recordedDataButton=new QPushButton(tr("Recordings"));

   connect(recordedDataButton,SIGNAL(clicked()),
           this,SLOT(raiseRecordedWindow()));

   connect(psthAnalysisButton,SIGNAL(clicked()),
           this,SLOT(psthAnalysis()));


    connect(resetToZeroButton, SIGNAL(clicked()),
            this, SLOT(resetThresholdToZero()));
    connect(clearScopeButton, SIGNAL(clicked()),
            this, SLOT(clearScope()));
    connect(applyToAllButton, SIGNAL(clicked()),
            this, SLOT(applyToAll()));

    connect(resetPsthButton,SIGNAL(clicked()),
            this,SLOT(resetPsth()));



    triggerTypeComboBox = new QComboBox();
    triggerTypeComboBox->addItem(tr("Voltage Threshold"));
    triggerTypeComboBox->addItem(tr("Digital Input"));
    triggerTypeComboBox->setCurrentIndex(0);

    connect(triggerTypeComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setTriggerType(int)));

    thresholdSpinBox = new QSpinBox();
    thresholdSpinBox->setRange(-5000, 0);
    thresholdSpinBox->setSingleStep(5);
    thresholdSpinBox->setValue(0);

    connect(thresholdSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(setVoltageThreshold(int)));

    recordButton = new QRadioButton(tr("Record Data WHEN Reset is Pressed"));

    multiplierSpinBox=new QSpinBox();
    multiplierSpinBox->setRange(0,10);
    multiplierSpinBox->setSingleStep(1);
    multiplierSpinBox->setValue(4);

    connect(multiplierSpinBox,SIGNAL(valueChanged(int)),
            this,SLOT(setMultiplier(int) ));

    QHBoxLayout *thresholdSpinBoxLayout = new QHBoxLayout;
    thresholdSpinBoxLayout->addWidget(resetToZeroButton);
    thresholdSpinBoxLayout->addWidget(thresholdSpinBox);
    thresholdSpinBoxLayout->addWidget(new QLabel(QSTRING_MU_SYMBOL + "V"));
    // thresholdSpinBoxLayout->addStretch(1);

    digitalInputComboBox = new QComboBox();
    digitalInputComboBox->addItem(tr("Digital Input 0"));
    digitalInputComboBox->addItem(tr("Digital Input 1"));
    digitalInputComboBox->addItem(tr("Digital Input 2"));
    digitalInputComboBox->addItem(tr("Digital Input 3"));
    digitalInputComboBox->addItem(tr("Digital Input 4"));
    digitalInputComboBox->addItem(tr("Digital Input 5"));
    digitalInputComboBox->addItem(tr("Digital Input 6"));
    digitalInputComboBox->addItem(tr("Digital Input 7"));
    digitalInputComboBox->addItem(tr("Digital Input 8"));
    digitalInputComboBox->addItem(tr("Digital Input 9"));
    digitalInputComboBox->addItem(tr("Digital Input 10"));
    digitalInputComboBox->addItem(tr("Digital Input 11"));
    digitalInputComboBox->addItem(tr("Digital Input 12"));
    digitalInputComboBox->addItem(tr("Digital Input 13"));
    digitalInputComboBox->addItem(tr("Digital Input 14"));
    digitalInputComboBox->addItem(tr("Digital Input 15"));
    digitalInputComboBox->setCurrentIndex(1);

    connect(digitalInputComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setDigitalInput(int)));

    edgePolarityComboBox = new QComboBox();
    edgePolarityComboBox->addItem(tr("Rising Edge"));
    edgePolarityComboBox->addItem(tr("Falling Edge"));
    edgePolarityComboBox->setCurrentIndex(0);

    connect(edgePolarityComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setEdgePolarity(int)));

    thresholdTypeComboBox=new QComboBox();
    thresholdTypeComboBox->addItem(tr("With Rms Multiplier"));
    thresholdTypeComboBox->addItem(tr("From Spike Plot"));
    thresholdTypeComboBox->setCurrentIndex(1);



    connect(thresholdTypeComboBox,SIGNAL(currentIndexChanged(int)),
            this,SLOT(setThresholdType(int)));

    numSpikesComboBox = new QComboBox();
    numSpikesComboBox->addItem(tr("Show 10 Spikes"));
    numSpikesComboBox->addItem(tr("Show 20 Spikes"));
    numSpikesComboBox->addItem(tr("Show 30 Spikes"));
    numSpikesComboBox->setCurrentIndex(1);

    connect(numSpikesComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setNumSpikes(int)));

    yScaleList.append(50); psthYScaleList.append(1);
    yScaleList.append(100); psthYScaleList.append(0.5);
    yScaleList.append(200); psthYScaleList.append(0.2);
    yScaleList.append(500);  psthYScaleList.append(0.1);
    yScaleList.append(1000);
    yScaleList.append(2000);
    yScaleList.append(5000);


    psthTScaleList.append(75);
    psthTScaleList.append(150);

    psthTScaleComboBox=new QComboBox();
    for(int i=0;i<psthTScaleList.size();i++){
        psthTScaleComboBox->addItem(QString::number(psthTScaleList[i])+" ms");
    }
    psthTScaleComboBox->setCurrentIndex(0);

    connect(psthTScaleComboBox,SIGNAL(currentIndexChanged(int)),
            this,SLOT(changePsthTScale(int)));




    yScaleComboBox = new QComboBox();
    for (int i = 0; i < yScaleList.size(); ++i) {
        yScaleComboBox->addItem("+/-" + QString::number(yScaleList[i]) +
                                " " + QSTRING_MU_SYMBOL + "V");
    }
    yScaleComboBox->setCurrentIndex(3);

    connect(yScaleComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeYScale(int)));

    psthYScaleComboBox = new QComboBox();
    for (int i = 0; i < psthYScaleList.size(); ++i) {
        psthYScaleComboBox->addItem(QString::number(psthYScaleList[i]) );
    }
    psthYScaleComboBox->setCurrentIndex(0);

    connect(psthYScaleComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(psthChangeYScale(int)));





    setTriggerType(triggerTypeComboBox->currentIndex());
    setNumSpikes(numSpikesComboBox->currentIndex());
    setVoltageThreshold(thresholdSpinBox->value());
    setDigitalInput(digitalInputComboBox->currentIndex());
    setEdgePolarity(edgePolarityComboBox->currentIndex());
    setThresholdType(thresholdTypeComboBox->currentIndex());
    setMultiplier(multiplierSpinBox->value());


    QVBoxLayout *digitalTriggerLayout = new QVBoxLayout;
    digitalTriggerLayout->addWidget(new QLabel(tr("Digital Source:")));
    digitalTriggerLayout->addWidget(digitalInputComboBox);
    digitalTriggerLayout->addWidget(new QLabel(tr("Edge Polarity:")));
    digitalTriggerLayout->addWidget(edgePolarityComboBox);
    digitalTriggerLayout->addStretch(1);

    QGroupBox *digitalInputGroupBox=new QGroupBox(tr("Digital Input Settings"));
    digitalInputGroupBox->setLayout(digitalTriggerLayout);


    QVBoxLayout *spikePlotLayout=new QVBoxLayout;
    spikePlotLayout->addWidget(new QLabel(tr("Voltage Scale:")));
    spikePlotLayout->addWidget(yScaleComboBox);
    spikePlotLayout->addWidget(new QLabel(tr("Voltage Threshold:")));
    spikePlotLayout->addLayout(thresholdSpinBoxLayout);
    spikePlotLayout->addWidget(new QLabel(tr("Set Num. Waveforms:")));
    spikePlotLayout->addWidget(numSpikesComboBox);
    spikePlotLayout->addStretch(1);


    QGroupBox *spikePlotGroupBox = new QGroupBox(tr("Spike Plot Settings"));
    spikePlotGroupBox->setLayout(spikePlotLayout);


    QVBoxLayout *psthPlotLayout =new QVBoxLayout;
    psthPlotLayout->addWidget(new QLabel(tr("Threshold Type")));
    psthPlotLayout->addWidget(thresholdTypeComboBox);
    psthPlotLayout->addWidget(new QLabel(tr("Multiplier")));
    psthPlotLayout->addWidget(multiplierSpinBox);
    psthPlotLayout->addWidget(new QLabel(tr("Psth Y Scale:")));
    psthPlotLayout->addWidget(psthYScaleComboBox);
    psthPlotLayout->addWidget(new QLabel(tr("Psth T Scale:")));
    psthPlotLayout->addWidget(psthTScaleComboBox);
    psthPlotLayout->addWidget(resetPsthButton);
    psthPlotLayout->addWidget(recordButton);
    psthPlotLayout->addWidget(recordedDataButton);
    psthPlotLayout->addWidget(psthAnalysisButton);

    psthPlotLayout->addStretch(1);

    QGroupBox *psthGroupBox= new QGroupBox(tr("PSTH Settings"));
    psthGroupBox->setLayout(psthPlotLayout);

    QVBoxLayout *settingsLeftLayout =new QVBoxLayout;
    settingsLeftLayout->addWidget(digitalInputGroupBox);
    settingsLeftLayout->addWidget(spikePlotGroupBox);




    QHBoxLayout *settingsLayout= new QHBoxLayout;
    settingsLayout->addLayout(settingsLeftLayout);
    settingsLayout->addWidget(psthGroupBox);
    settingsLayout->addStretch(1);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addLayout(settingsLayout);
    leftLayout->addWidget(clearScopeButton);
    leftLayout->addWidget(spikePlot);

    QVBoxLayout *rightLayout = new QVBoxLayout;

    rightLayout->addWidget(triggerPlot);
    rightLayout->addWidget(psthPlot);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);

    setLayout(mainLayout);






    //QVBoxLayout *triggerLayout = new QVBoxLayout;
    //triggerLayout->addWidget(new QLabel(tr("Type:")));
    //triggerLayout->addWidget(triggerTypeComboBox);
    //triggerLayout->addWidget(new QLabel(tr("Voltage Threshold:")));
    //triggerLayout->addLayout(thresholdSpinBoxLayout);
    //triggerLayout->addWidget(new QLabel(tr("(or click in scope to set)")));
    //triggerLayout->addWidget(new QLabel(tr("Digital Source:")));
   // triggerLayout->addWidget(digitalInputComboBox);
   // triggerLayout->addWidget(edgePolarityComboBox);

    //QVBoxLayout *displayLayout = new QVBoxLayout;
    //displayLayout->addWidget(new QLabel(tr("Voltage Scale:")));
    //displayLayout->addWidget(yScaleComboBox);
    //displayLayout->addWidget(numSpikesComboBox);
    //displayLayout->addWidget(clearScopeButton);
    //displayLayout->addWidget(new QLabel(tr("Threshold Type")));
    //displayLayout->addWidget(thresholdTypeComboBox);
    //displayLayout->addWidget(new QLabel(tr("Multiplier")));
    //displayLayout->addWidget(multiplierSpinBox);
    //displayLayout->addWidget(new QLabel(tr("Psth YScale")));
    //displayLayout->addWidget(psthYScaleComboBox);

    //QGroupBox *triggerGroupBox = new QGroupBox(tr("Trigger Settings"));
    //triggerGroupBox->setLayout(triggerLayout);

    //QGroupBox *displayGroupBox = new QGroupBox(tr("Display Settings"));
    //displayGroupBox->setLayout(displayLayout);

    //QVBoxLayout *leftLayout = new QVBoxLayout;
    //leftLayout->addWidget(triggerGroupBox);
    //leftLayout->addWidget(applyToAllButton);
    //leftLayout->addWidget(displayGroupBox);
    //leftLayout->addStretch(1);



   // rightLayout->addStretch(1);




}
void PsthDialog::increaseAnalysisStimulus(){
    if(analysisDialog){
        analysisDialog->increaseAnalysisStimulus();
    }
}
void PsthDialog::increaseAnalysisSpikes(){
    if(analysisDialog){
        analysisDialog->increaseAnalysisSpikes();
    }

}
void PsthDialog::raiseRecordedWindow(){
    recordDialog->show();
    recordDialog->raise();
    recordDialog->activateWindow();
    recordDialog->updateRecordPlot();
}
void PsthDialog::changePsthTScale(int index){
    psthPlot->changeTScale(psthTScaleList[index]);
}
void PsthDialog::changeYScale(int index)
{
    spikePlot->setYScale(yScaleList[index]);
}

void PsthDialog::psthChangeYScale(int index){
    psthPlot->setYScale(psthYScaleList[index]);
}

void PsthDialog::setYScale(int index)
{
    yScaleComboBox->setCurrentIndex(index);
    spikePlot->setYScale(yScaleList[index]);
}
void PsthDialog::psthSetYScale(int index){
    psthYScaleComboBox->setCurrentIndex(index);
    psthPlot->setYScale(psthYScaleList[index]);
}

void PsthDialog::setSampleRate(double newSampleRate)
{
    psthPlot->setSampleRate(newSampleRate);
    spikePlot->setSampleRate(newSampleRate);
    triggerPlot->setSampleRate(newSampleRate);

}

// Select a voltage trigger if index == 0.
// Select a digital input trigger if index == 1.
void PsthDialog::setTriggerType(int index)
{



   // spikePlot->setVoltageTriggerMode(index == 0);
}

void PsthDialog::resetThresholdToZero()
{
    thresholdSpinBox->setValue(0);
    setVoltageThreshold(0);
}

void PsthDialog::updateWaveform(int numBlocks)
{

    spikePlot->updateWaveform(numBlocks);
    triggerPlot->updateWaveform(numBlocks);

}
void PsthDialog::updatePsthArray(int numBlocks){
    psthPlot->updatePsthArray(numBlocks);
}
// Set number of spikes plotted superimposed.
void PsthDialog::setNumSpikes(int index)
{
    int num;

    switch (index) {
    case 0: num = 10; break;
    case 1: num = 20; break;
    case 2: num = 30; break;
    }

    spikePlot->setMaxNumSpikeWaveforms(num);

}

void PsthDialog::clearScope()
{
    spikePlot->clearScope();
    psthPlot->clearScope();
   triggerPlot->clearScope();

}

void PsthDialog::setDigitalInput(int index)
{
    psthPlot->setDigitalTriggerChannel(index);
    triggerPlot->setDigitalTriggerChannel(index);

}

void PsthDialog::setVoltageThreshold(int value)
{
    spikePlot->setVoltageThreshold(value);
    if(!(getThresholdType())){
        psthPlot->setVoltageThreshold(value);
    }
}
void PsthDialog::setMultiplier(int value){
    if(getThresholdType()){
        psthPlot->setRmsMultiplier(value);

    }

}
void PsthDialog::setVoltageThresholdDisplay(int value)
{
    thresholdSpinBox->setValue(value);
}

void PsthDialog::setEdgePolarity(int index)
{
    spikePlot->setDigitalEdgePolarity(index == 0);

    psthPlot->setDigitalEdgePolarity(index==0);
    triggerPlot->setDigitalEdgePolarity(index==0);
}
void PsthDialog::setThresholdType(int index){
    psthPlot->setThresholdType(index == 0 );
    multiplierSpinBox->setEnabled(index==0);
    multiplierSpinBox->setDisabled(index==1);
    setVoltageThreshold(0);
    resetToZeroButton->setEnabled(index==1);
    resetToZeroButton->setDisabled(index==0);
    thresholdSpinBox->setEnabled(index==1);
    thresholdSpinBox->setDisabled(index==0);
}
bool PsthDialog::getThresholdType(){
    if(thresholdTypeComboBox->currentIndex()==0){
        return true;
      }
    else return false;

}

// Set Spike Scope to a new signal channel source.
void PsthDialog::setNewChannel(SignalChannel* newChannel)
{
    psthPlot->setNewChannel(newChannel);
    spikePlot->setNewChannel(newChannel);
    triggerPlot->setNewChannel(newChannel);


    currentChannel = newChannel;
    //if (newChannel->voltageTriggerMode) {
      //  triggerTypeComboBox->setCurrentIndex(0);
    //} else {
      //  triggerTypeComboBox->setCurrentIndex(1);
    //}
    //thresholdSpinBox->setValue(newChannel->voltageThreshold);
    //digitalInputComboBox->setCurrentIndex(newChannel->digitalTriggerChannel);
    //if (newChannel->digitalEdgePolarity) {
        //edgePolarityComboBox->setCurrentIndex(0);
    //} else {
      //  edgePolarityComboBox->setCurrentIndex(1);
    //}
}

void PsthDialog::resetPsth(){
    psthPlot->resetPsth();

    if(analysisDialog){
        analysisDialog->setNewChannel(psthPlot->selectedChannel);
    }



    //recordDialog->updateRecordPlot();
}

void PsthDialog::expandYScale()
{
    if (yScaleComboBox->currentIndex() > 0) {
        yScaleComboBox->setCurrentIndex(yScaleComboBox->currentIndex() - 1);
        changeYScale(yScaleComboBox->currentIndex());
    }
}

void PsthDialog::contractYScale()
{
    if (yScaleComboBox->currentIndex() < yScaleList.size() - 1) {
        yScaleComboBox->setCurrentIndex(yScaleComboBox->currentIndex() + 1);
        changeYScale(yScaleComboBox->currentIndex());
    }
}
void PsthDialog::contractpsthYScale(){
    if (psthYScaleComboBox->currentIndex() < psthYScaleList.size() - 1) {
        psthYScaleComboBox->setCurrentIndex(psthYScaleComboBox->currentIndex() + 1);
        psthChangeYScale(psthYScaleComboBox->currentIndex());
    }
}
void PsthDialog::expandpsthYScale(){
    if (psthYScaleComboBox->currentIndex() > 0) {
        psthYScaleComboBox->setCurrentIndex(psthYScaleComboBox->currentIndex() - 1);
        psthChangeYScale(psthYScaleComboBox->currentIndex());
    }
}

// Apply trigger settings to all channels on selected port.
void PsthDialog::applyToAll()
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

void PsthDialog::psthAnalysis(){
    if (!analysisDialog) {
        analysisDialog = new PsthAnalysisDialog( psthPlot, signalProcessor, currentChannel, this);
        // add any 'connect' statements here
    }

    analysisDialog->show();
    analysisDialog->raise();
    analysisDialog->activateWindow();
    //analysisDialog->setYScale(yScaleComboBox->currentIndex());

}
