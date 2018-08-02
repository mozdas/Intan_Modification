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

#ifndef PSTHPLOT_H
#define PSTHPLOT_H

#define PSTHPLOT_X_SIZE 450
#define PSTHPLOT_Y_SIZE 346
#define BUFFER_SIZE 35000

#include <QWidget>

using namespace std;

class SignalProcessor;
class PsthDialog;
class SignalChannel;

class PsthPlot : public QWidget
{
    Q_OBJECT
public:
    explicit PsthPlot(SignalProcessor *inSignalProcessor, SignalChannel *initialChannel,
                       PsthDialog *inPsthDialog, QWidget *parent = 0);
    void setYScale(double newYScale);
    void setSampleRate(double newSampleRate);
    //void updateWaveform(int numBlocks);
    void updatePsthArray(int numBlocks);
    //void setMaxNumSpikeWaveforms(int num);
    void clearScope();
    //void setVoltageTriggerMode(bool voltageMode);
    void setVoltageThreshold(int threshold);
    void setDigitalTriggerChannel(int channel);
    void setDigitalEdgePolarity(bool risingEdge);
    void setNewChannel(SignalChannel* newChannel);
    void setThresholdType(bool index);
    void setRmsMultiplier(int value);
    void resetPsth();
    void changeTScale(double newTScale);

    void zeroAnalysisData();
    void increaseAnalysisStimulus();

    void increaseAnalysisSpikes();


    bool thresholdType;
    SignalChannel *selectedChannel;



    QSize minimumSizeHint() const;
    QSize sizeHint() const;

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);
   // void mousePressEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    //void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent* event);


private:
     QVector<double> adjustedPsthArray;
    void drawAxisLines();
    void drawAxisText();
    //void updateRecordPlot(double rms);
    void initializeDisplay();

    SignalProcessor *signalProcessor;
    PsthDialog *psthDialog;

    //QVector<QVector<double> > spikeWaveform;

    QVector<double> spikeWaveformBuffer;
    QVector<double> psthArray;
    QVector<int> digitalInputBuffer;
    //int spikeWaveformIndex;
    //int numSpikeWaveforms;
    //int maxNumSpikeWaveforms;
    //bool voltageTriggerMode;
    int voltageThreshold;
    int digitalTriggerChannel;
    bool digitalEdgePolarity;

    //int preTriggerTSteps;
    //int totalTSteps;
    bool startingNewChannel;
    //int rmsDisplayPeriod;



    SignalChannel *futureChannel;

    int numBlocksTurn;
    int numberStimulus;

    void updatePsthPlot();


    QRect frame;
    int rmsMultiplier;
    double rms;

    double tStepMsec;
    int psthTotalTsteps;
    int psthPreTSteps;
    double yScale;
    double tScale;
    double futureTScale;
    double sampleRate;

    int psthAnalysisData;
    int numberAnalysisStimulus;

    //double savedRms;


    QPixmap pixmap;

    //QVector<QVector<QColor> > scopeColors;

};

#endif // PSTHPLOT_H
