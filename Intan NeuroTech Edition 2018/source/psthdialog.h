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

#ifndef PSTHDIALOG_H
#define PSTHDIALOG_H

#include <QDialog>

using namespace std;

class QPushButton;
class QComboBox;
class QSpinBox;
class PsthPlot;
class SignalProcessor;
class SignalSources;
class SignalChannel;
class SpikePlot;
class TriggerPlot;
class QRadioButton;
class RecordDialog;
class PsthAnalysisDialog;
class PsthDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PsthDialog(SignalProcessor *inSignalProcessor, SignalSources *inSignalSources,
                              SignalChannel *initialChannel, QWidget *parent = 0);
    void setYScale(int index);
    void psthSetYScale(int index);
    void setSampleRate(double newSampleRate);
    void updateWaveform(int numBlocks);
    void updatePsthArray(int numBlocks);
    void setVoltageThresholdDisplay(int value);
    void setNewChannel(SignalChannel* newChannel);
    void expandYScale();
    void contractYScale();
    void contractpsthYScale();
    void expandpsthYScale();
    bool getThresholdType();
    void increaseAnalysisStimulus();
    void increaseAnalysisSpikes();
    QRadioButton *recordButton;
    RecordDialog *recordDialog;




signals:

public slots:
    void setVoltageThreshold(int value);

private slots:
    void changeYScale(int index);
    void psthChangeYScale(int index);
    void changePsthTScale(int index);
    void setTriggerType(int index);
    void resetThresholdToZero();
    void setNumSpikes(int index);
    void clearScope();
    void setDigitalInput(int index);
    void setMultiplier(int value);
    void raiseRecordedWindow();
    void psthAnalysis();


    void setEdgePolarity(int index);
    void applyToAll();
    void setThresholdType(int index);
    void resetPsth();


private:
    QVector<int> yScaleList;

    QVector<double>psthYScaleList;
    QVector <int> psthTScaleList;

    SignalProcessor *signalProcessor;
    SignalSources *signalSources;
    SignalChannel *currentChannel;



    QPushButton *resetToZeroButton;
    QPushButton *clearScopeButton;
    QPushButton *applyToAllButton;
    QPushButton *resetPsthButton;
    QPushButton *recordedDataButton;
    QPushButton *psthAnalysisButton;


    QComboBox *triggerTypeComboBox;
    QComboBox *thresholdTypeComboBox;
    QComboBox *numSpikesComboBox;
    QComboBox *digitalInputComboBox;
    QComboBox *edgePolarityComboBox;
    QComboBox *yScaleComboBox;
    QComboBox *psthYScaleComboBox;
    QComboBox *psthTScaleComboBox;



    QSpinBox *thresholdSpinBox;
    QSpinBox *multiplierSpinBox;

    SpikePlot *spikePlot;

    PsthPlot *psthPlot;
    TriggerPlot *triggerPlot;
    PsthAnalysisDialog *analysisDialog;




};

#endif // SPIKESCOPEDIALOG_H
