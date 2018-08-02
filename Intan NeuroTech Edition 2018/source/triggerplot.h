#ifndef TRIGGERPLOT_H
#define TRIGGERPLOT_H


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



#define SPIKEPLOT_X_SIZE 320
#define SPIKEPLOT_Y_SIZE 346
#define BUFFER_SIZE 25000

#include <QWidget>

using namespace std;

class SignalProcessor;
class PsthDialog;
class SignalChannel;

class TriggerPlot : public QWidget
{
    Q_OBJECT
public:
    explicit TriggerPlot(SignalProcessor *inSignalProcessor, SignalChannel *initialChannel,
                       PsthDialog *inPsthDialog, QWidget *parent = 0);
    void setYScale(double newYScale);
    void setSampleRate(double newSampleRate);
    //void updateWaveform(int numBlocks);
    void updateWaveform(int numBlocks);
    //void setMaxNumSpikeWaveforms(int num);
    void clearScope();
    //void setVoltageTrggerMode(bool voltageMode);
    //void setVoltageThreshold(int threshold);
    void setDigitalTriggerChannel(int channel);
    void setDigitalEdgePolarity(bool risingEdge);
    void setNewChannel(SignalChannel* newChannel);
    //void setThresholdType(bool index);
    //void setRmsMultiplier(int value);
    //void resetPsth();

    bool thresholdType;

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);
   // void mousePressEvent(QMouseEvent *event);
    //void wheelEvent(QWheelEvent *event);
    //void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent* event);


private:
    void drawAxisLines();
    void drawAxisText();
    //void updateRecordPlot(double rms);
    void initializeDisplay();

    SignalProcessor *signalProcessor;
    PsthDialog *psthDialog;

    //QVector<QVector<double> > spikeWaveform;



    QVector<int> digitalInputBuffer;
    QVector<int> waveformBuffer;
    //int spikeWaveformIndex;
    //int numSpikeWaveforms;
    //int maxNumSpikeWaveforms;
    //bool voltageTriggerMode;
    //int voltageThreshold;
    int digitalTriggerChannel;
    bool digitalEdgePolarity;

    int preTriggerTSteps;
    int totalTSteps;
    bool startingNewChannel;
    //int rmsDisplayPeriod;


    SignalChannel *selectedChannel;
    SignalChannel *futureChannel;

    int numBlocksTurn;
    int numberStimulus;

    void updateTriggerPlot();


    QRect frame;
    int rmsMultiplier;
    double rms;

    double tStepMsec;

    double yScale;
    //double savedRms;
    bool newTrigger;
    bool triggerChange;
    int futureTriggerChannel;
    bool futureEdgePolarity;


    QPixmap pixmap;

    //QVector<QVector<QColor> > scopeColors;

};

#endif // TRIGGERPLOT_H
