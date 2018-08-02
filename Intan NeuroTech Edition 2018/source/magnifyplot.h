#ifndef MAGNIFYPLOT_H
#define MAGNIFYPLOT_H


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



#define MAGNIFYPLOT_X_SIZE 650
#define MAGNIFYPLOT_Y_SIZE 346

#include <QWidget>

using namespace std;

class SignalProcessor;
class MagnifyDialog;
class SignalChannel;


class MagnifyPlot : public QWidget
{
    Q_OBJECT
public:
    explicit MagnifyPlot(SignalProcessor *inSignalProcessor, SignalChannel *initialChannel,
                       MagnifyDialog *inMagnifyDialog, QWidget *parent = 0);
    void setYScale(int newYScale);
    void setTScale(int newTScale);
    void setSampleRate(double newSampleRate);
    void updateWaveform(int numBlocks);
    //void setMaxNumSpikeWaveforms(int num);
    void clearScope();
    //void setVoltageTriggerMode(bool voltageMode);
    //void setVoltageThreshold(int threshold);
    //void setDigitalTriggerChannel(int channel);
    //void setDigitalEdgePolarity(bool risingEdge);
    void setNewChannel(SignalChannel* newChannel);
    void drawWaveForms(int numBlocks);


    QSize minimumSizeHint() const;
    QSize sizeHint() const;


signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);
    //void mousePressEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    //void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent* event);

private:
    void drawAxisLines();
    void drawAxisText();
    void updateMagnifyPlot();
    void initializeDisplay();

    SignalProcessor *signalProcessor;
    MagnifyDialog *magnifyDialog;

    QVector<QVector<double> > spikeWaveform;

    QVector<double> spikeWaveformBuffer;
   // QVector<int> digitalInputBuffer;
   // int spikeWaveformIndex;
   // int numSpikeWaveforms;
   // int maxNumSpikeWaveforms;
   // bool voltageTriggerMode;
   // int voltageThreshold;
   // int digitalTriggerChannel;
   // bool digitalEdgePolarity;
    double sampleRate;
    double tScale;



    //int preTriggerTSteps;
    int totalTSteps;
    //bool startingNewChannel;
    //int rmsDisplayPeriod;


    SignalChannel *selectedChannel;

    QRect frame;

    double tStepMsec;
    int yScale;
    //double savedRms;

    QPixmap pixmap;

    //QVector<QVector<QColor> > scopeColors;

};


#endif // MAGNIFYPLOT_H
