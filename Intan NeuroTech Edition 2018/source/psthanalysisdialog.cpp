#include "psthanalysisdialog.h"
#include <QtGui>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif
#include <iostream>

#include "globalconstants.h"
#include "lfpscopedialog.h"
#include "lfpanalysisdialog.h"
#include "signalchannel.h"
#include "signalgroup.h"
#include "lfpplot.h"
#include "diginplot.h"
#include "psthanalysisplot.h"



PsthAnalysisDialog::PsthAnalysisDialog(PsthPlot *inPsthPlot, SignalProcessor *inSignalProcessor,
                                     SignalChannel *initialChannel, QWidget *parent) :
    QDialog(parent)
{

    //errorbarState = false;
    setWindowTitle(tr("PSTH Analysis Scope"));

    currentChannel = initialChannel;
    signalProcessor = inSignalProcessor;
    psthPlot = inPsthPlot;

    analysisPlot = new PsthAnalysisPlot(psthPlot,signalProcessor,currentChannel,this,this);

    isRunning =false;

    // Y Scale adjustment;
    yScaleList.append(2);
    yScaleList.append(5);
    yScaleList.append(10);
    yScaleList.append(20);
    yScaleList.append(50);


    yScaleComboBox = new QComboBox();
    for (int i = 0; i < yScaleList.size(); ++i) {
        yScaleComboBox->addItem( QString::number(yScaleList[i]) );
    }
    yScaleComboBox->setCurrentIndex(2);


    connect(yScaleComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeYScale(int)));

    //total time scale


    tScaleList.append(10*60);
    tScaleList.append(20*60);
    tScaleList.append(30*60);
    tScaleList.append(45*60);
    tScaleList.append(60*60);
    tScaleList.append(90*60);
    tScaleList.append(120*60);
    tScaleList.append(150*60);
    tScaleList.append(180*60);

    tScaleComboBox = new QComboBox();
    tScaleComboBox->addItem(tr("10 minutes"));
    tScaleComboBox->addItem(tr("20 minutes"));
    tScaleComboBox->addItem(tr("30 minutes"));
    tScaleComboBox->addItem(tr("45 minutes"));
    tScaleComboBox->addItem(tr("60 minutes"));
    tScaleComboBox->addItem(tr("90 minutes"));
    tScaleComboBox->addItem(tr("120 minutes"));
    tScaleComboBox->addItem(tr("150 minutes"));
    tScaleComboBox->addItem(tr("180 minutes"));

    tScaleComboBox->setCurrentIndex(0);
    changeTScale(0);

    connect(tScaleComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeTScale(int)));

    //Initialize and connect timer
    tStepTimer = new QTimer(this);

    connect(tStepTimer, SIGNAL(timeout()),
            this, SLOT(updateAnalysisPlot()));



    //Step Size of measurements
    tStepList.append(10);
    tStepList.append(30);
    tStepList.append(60);
    tStepList.append(90);
    tStepList.append(120);
    tStepList.append(240);
    tStepList.append(360);
    tStepList.append(600);

    tStepList.append(1);

    tStepComboBox = new QComboBox();
    tStepComboBox->addItem(tr("10 seconds"));
    tStepComboBox->addItem(tr("30 seconds"));
    tStepComboBox->addItem(tr("60 seconds"));
    tStepComboBox->addItem(tr("90 seconds"));
    tStepComboBox->addItem(tr("120 seconds"));
    tStepComboBox->addItem(tr("240 seconds"));
    tStepComboBox->addItem(tr("360 seconds"));
    tStepComboBox->addItem(tr("600 seconds"));
    tStepComboBox->addItem(tr("1 seconds FOR DEBUG"));

    tStepComboBox->setCurrentIndex(0);
    changeTStepScale(0);

    connect(tStepComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeTStepScale(int)));


    //Start Stop Button Adjustments
    startButton = new QPushButton(tr("Start"));
    stopButton = new QPushButton(tr("Stop"));

    stopButton->setEnabled(false);

    connect(startButton, SIGNAL(clicked()),
            this, SLOT(startPsthAnalysis()));
    connect(stopButton, SIGNAL(clicked()),
            this, SLOT(stopPsthAnalysis()));



    //Errorbar Options
    QCheckBox *errorbarDisplayChackBox = new QCheckBox(tr("Reset PSTH with Acq. Freq."));

    connect(errorbarDisplayChackBox, SIGNAL(clicked(bool)),
            this, SLOT(changePsthReseting(bool)));
    psthReseting = false;


    QHBoxLayout *startStopLayout = new QHBoxLayout;
    startStopLayout->addWidget(startButton);
    startStopLayout->addWidget(stopButton);
    startStopLayout->addStretch(1);

    QGroupBox *plotScalingsBox = new QGroupBox(tr("Plot Scalings"));
    QVBoxLayout *plotScalingsLayout = new  QVBoxLayout;
    plotScalingsLayout->addWidget(yScaleComboBox);
    plotScalingsLayout->addWidget(tScaleComboBox);

    plotScalingsBox->setLayout(plotScalingsLayout);

    /*

    windowForAnalysisComboBox =  new QComboBox;
    windowForAnalysisComboBox->addItem(tr("0 to +50ms"));
    windowForAnalysisComboBox->addItem(tr("0 to +100ms"));
    windowForAnalysisComboBox->addItem(tr("-50 to +50ms"));
    windowForAnalysisComboBox->addItem(tr("-30 to +70ms"));
    windowForAnalysisComboBox->addItem(tr("entire LFP region"));
    windowForAnalysisComboBox->addItem(tr("0 to +10ms"));
    windowForAnalysisComboBox->addItem(tr("0 to +20ms"));
    windowForAnalysisComboBox->addItem(tr("0 to +30ms"));
    windowForAnalysisComboBox->addItem(tr("0 to +40ms"));
    windowForAnalysisComboBox->addItem(tr("0 to +50ms"));


    windowForAnalysisComboBox->setCurrentIndex(0);
    windowForAnalysisComboBox->setEnabled(true);
    connect(windowForAnalysisComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(arrengeSearchForMinWindow(int)));


    analysisTypeComboBox = new QComboBox;
    analysisTypeComboBox->addItem(tr("Min. Point Analysis"));
    analysisTypeComboBox->addItem(tr("Max - Min Difference Analysis"));
    analysisTypeComboBox->setCurrentIndex(0);
    analysisTypeComboBox->setEnabled(true);

    connect(analysisTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setAnalysisType(int)) );

    QGroupBox *analysisSettingsBox = new QGroupBox(tr("Analysis Settings"));
    QVBoxLayout *analysisSettingsLayout = new QVBoxLayout;
    analysisSettingsLayout->addWidget(new QLabel(tr("Analysis Type:")));
    analysisSettingsLayout->addWidget(analysisTypeComboBox);
    analysisSettingsLayout->addWidget(new QLabel(tr("Window for Analysis:")));
    analysisSettingsLayout->addWidget(windowForAnalysisComboBox);
    analysisSettingsBox->setLayout(analysisSettingsLayout);
*/
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(plotScalingsBox);
    leftLayout->addWidget(new QLabel(tr("Acquiring Frequency")));
    leftLayout->addWidget(tStepComboBox);
    leftLayout->addWidget(new QLabel(tr("Start/Stop the Analysis")));
    leftLayout->addLayout(startStopLayout);
    leftLayout->addWidget(new QLabel(tr("Display Settings")));
    leftLayout->addWidget(errorbarDisplayChackBox);
    //leftLayout->addWidget(analysisSettingsBox);
    leftLayout->addStretch(1);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(leftLayout);
    mainLayout->addWidget(analysisPlot);
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);

    setLayout(mainLayout);

}

void PsthAnalysisDialog::changeYScale(int index)
{
    analysisPlot->setYScale(yScaleList[index]);
}

void PsthAnalysisDialog::setYScale(int index)
{
    yScaleComboBox->setCurrentIndex(index);
    analysisPlot->setYScale(yScaleList[index]);
}
/*
void PsthAnalysisDialog::setSampleRate(double newSampleRate)
{
    //analysisPlot->setSampleRate(newSampleRate);
}
*/
void PsthAnalysisDialog:: increaseAnalysisStimulus(){
    if(isRunning){
        analysisPlot->numberStimulus++;
    }

}
void PsthAnalysisDialog::increaseAnalysisSpikes(){
    if(isRunning){
        analysisPlot->numberSpikes++;
    }
}
void PsthAnalysisDialog::updateWaveform()
{
    analysisPlot->updateWaveform();
}

void PsthAnalysisDialog::setNewChannel(SignalChannel *newSignalChannel)
{
    currentChannel = newSignalChannel;

    if(isRunning)
    {
        stopPsthAnalysis();
        startPsthAnalysis();
    }

    analysisPlot->setNewChannel(newSignalChannel);
}

void PsthAnalysisDialog::changePsthReseting(bool state)
{
    psthReseting = state;
    analysisPlot->changePsthReseting(state);
}

void PsthAnalysisDialog::startPsthAnalysis(){

    stopButton->setEnabled(true);
    startButton->setEnabled(false);
    updateAnalysisPlot();
    tStepTimer->start();
    analysisPlot->applyStartProcess();

    //prevent any changes during running
   // windowForAnalysisComboBox->setEnabled(false);
    tStepComboBox->setEnabled(false);
    isRunning = true;
    //analysisTypeComboBox->setEnabled(false);
}

void PsthAnalysisDialog::stopPsthAnalysis(){
    isRunning = false;
    stopButton->setEnabled(false);
    startButton->setEnabled(true);
    tStepTimer->stop();
    analysisPlot->applyStopProcess();
   // windowForAnalysisComboBox->setEnabled(true);
    tStepComboBox->setEnabled(true); //enable modifications in the step size
   // analysisTypeComboBox->setEnabled(true);

}

void PsthAnalysisDialog::changeTStepScale(int index){

    tStepInSeconds = tStepList[index];
    tStepTimer->setInterval(tStepList[index]*1000);
    //Inform AnalysisPlot from this change ....
    analysisPlot->setTStepValue(tStepList[index]);
}

void PsthAnalysisDialog::updateAnalysisPlot(){

     analysisPlot->updateWaveform();
}

void PsthAnalysisDialog::expandYScale(){
    if (yScaleComboBox->currentIndex() > 0) {
        yScaleComboBox->setCurrentIndex(yScaleComboBox->currentIndex() - 1);
        changeYScale(yScaleComboBox->currentIndex());
    }
}

void PsthAnalysisDialog::contractYScale(){
    if (yScaleComboBox->currentIndex() < yScaleList.size() - 1) {
        yScaleComboBox->setCurrentIndex(yScaleComboBox->currentIndex() + 1);
        changeYScale(yScaleComboBox->currentIndex());
    }
}

void PsthAnalysisDialog::changeTScale(int index){

    analysisPlot->setTScale(tScaleList[index]);

}

void PsthAnalysisDialog::arrengeSearchForMinWindow(int index){

    /*
     switch(index){
        case 0:
            analysisPlot->setAnalysisWindow(0.0, 50.0);
            break;
        case 1:
            analysisPlot->setAnalysisWindow(0.0, 100.0);
            break;
        case 2:
            analysisPlot->setAnalysisWindow(-50.0, 50.0);
            break;
        case 3:
            analysisPlot->setAnalysisWindow(-30.0, 70);
            break;
        case 4:
            analysisPlot->setAnalysisWindow(0.0 , 0.0); // equal to 0 input mean entire plot
            break;
        case 5:
            analysisPlot->setAnalysisWindow(0.0,10.0);
            break;
        case 6:
            analysisPlot->setAnalysisWindow(0.0,20.0);
            break;
        case 7:
            analysisPlot->setAnalysisWindow(0.0,30.0);
            break;
        case 8:
            analysisPlot->setAnalysisWindow(0.0,40.0);
            break;
        case 9 :
           analysisPlot->setAnalysisWindow(0.0,50.0);
           break;

     }
     */
 }
/*
void PsthAnalysisDialog::setAnalysisType(int index){
    analysisType = index;
    analysisPlot->changeAnalysisType(analysisType);
}
*/
