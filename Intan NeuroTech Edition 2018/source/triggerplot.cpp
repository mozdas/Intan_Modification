#include "triggerplot.h"

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
#include <qmath.h>
#include <iostream>

#include "globalconstants.h"
#include "signalprocessor.h"
#include "signalchannel.h"
#include "psthdialog.h"


// The RecordPlot widget displays a triggered neural spike plot in the
// Spike Scope dialog.  Multiple spikes are plotted on top of one another
// so users may compare their shapes.  The RMS value of the waveform is
// displayed in the plot.  Users may select a new threshold value by clicking
// on the plot.  Keypresses are used to change the voltage scale of the plot.

TriggerPlot::TriggerPlot(SignalProcessor *inSignalProcessor, SignalChannel *initialChannel,
                     PsthDialog *inPsthDialog, QWidget *parent) :
    QWidget(parent)
{
    signalProcessor = inSignalProcessor;
    psthDialog = inPsthDialog;
    rms=0;


    selectedChannel = initialChannel;

    startingNewChannel = true;



    digitalTriggerChannel = 1;
    digitalEdgePolarity = true;

    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setFocusPolicy(Qt::StrongFocus);





    numberStimulus=0;
    newTrigger=true;



    // Buffers to hold recent history of spike waveform and digital input,
    // used to find trigger events.
    waveformBuffer.resize(10000);
    waveformBuffer.fill(0.0);
    digitalInputBuffer.resize(40000);
    digitalInputBuffer.fill(0);

    // Set up vectors of varying plot colors so that older waveforms
    // are plotted in low-contrast gray and new waveforms are plotted
    // in high-contrast blue.  Older signals fade away, like phosphor
    // traces on old-school CRT oscilloscopes.


    // Default values that may be overwritten.
    yScale = 2;
    setSampleRate(30000.0);
}

// Set voltage scale.
void TriggerPlot::setYScale(double newYScale)
{
    yScale = newYScale;
    initializeDisplay();
}

// Set waveform sample rate.
void TriggerPlot::setSampleRate(double newSampleRate)
{
    // Calculate time step, in msec.
    tStepMsec = 1000.0 / newSampleRate;

    // Calculate number of time steps in 75 msec sample.

    totalTSteps=qCeil(300.0/tStepMsec)+1;

    // Calculate number of time steps in the 25 msec pre-trigger
    // display interval.

    preTriggerTSteps=qCeil(100.0/tStepMsec);




    numBlocksTurn=0;

    startingNewChannel = true;
}

// Draw axis lines on display.
void TriggerPlot::drawAxisLines()
{
    QPainter painter(&pixmap);
    painter.initFrom(this);

    painter.eraseRect(frame);

    painter.setPen(Qt::darkGray);

    // Draw box outline.
    painter.drawRect(frame);

    // Draw horizonal zero voltage line.
    painter.drawLine(frame.left(), frame.center().y(), frame.right(), frame.center().y());

    // Draw vertical lines at 0 ms and 1 ms.
    painter.drawLine(frame.left() + (1.0/3.0) * (frame.right() - frame.left()) + 1, frame.top(),
                      frame.left() + (1.0/3.0) * (frame.right() - frame.left()) + 1, frame.bottom());
    painter.drawLine(frame.left() + (2.0/3.0) * (frame.right() - frame.left()) + 1, frame.top(),
                      frame.left() + (2.0/3.0) * (frame.right() - frame.left()) + 1, frame.bottom());

    update();
}

// Draw text around axes.
void TriggerPlot::drawAxisText()
{
    QPainter painter(&pixmap);
    painter.initFrom(this);
    const int textBoxWidth = painter.fontMetrics().width("+1 LOGIC");
    const int textBoxHeight = painter.fontMetrics().height();

    // Clear entire Widget display area.
    painter.eraseRect(rect());

    // Draw border around Widget display area.
    painter.setPen(Qt::darkGray);
    QRect rect(0, 0, width() - 1, height() - 1);
    painter.drawRect(rect);

    // If the selected channel is an amplifier channel, then write the channel name and number,
    // otherwise remind the user than non-amplifier channels cannot be displayed in Spike Scope.
    if (selectedChannel) {
        if (selectedChannel->signalType == AmplifierSignal) {
            painter.drawText(frame.right() - textBoxWidth - 1, frame.top() - textBoxHeight - 1,
                              textBoxWidth, textBoxHeight,
                              Qt::AlignRight | Qt::AlignBottom, selectedChannel->nativeChannelName);
            painter.drawText(frame.left() + 3, frame.top() - textBoxHeight - 1,
                              textBoxWidth, textBoxHeight,
                              Qt::AlignLeft | Qt::AlignBottom, selectedChannel->customChannelName);
        } else {
            painter.drawText(frame.right() - 2 * textBoxWidth - 1, frame.top() - textBoxHeight - 1,
                              2 * textBoxWidth, textBoxHeight,
                              Qt::AlignRight | Qt::AlignBottom, tr("ONLY AMPLIFIER CHANNELS CAN BE DISPLAYED"));
        }
    }

    // Label the voltage axis.
    painter.drawText(frame.left() - textBoxWidth - 2, frame.top() - 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignRight | Qt::AlignTop,
                      "+1LGC");
    painter.drawText(frame.left() - textBoxWidth - 2, frame.center().y() - textBoxHeight / 2,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignRight | Qt::AlignVCenter, "0");
    painter.drawText(frame.left() - textBoxWidth - 2, frame.bottom() - textBoxHeight + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignRight | Qt::AlignBottom,
                      "-1LGC");

    // Label the time axis.
    painter.drawText(frame.left() - textBoxWidth / 2, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignHCenter | Qt::AlignTop, "-100");
    painter.drawText(frame.left() + (1.0/3.0) * (frame.right() - frame.left()) + 1 - textBoxWidth / 2, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignHCenter | Qt::AlignTop, "0");
    painter.drawText(frame.left() + (2.0/3.0) * (frame.right() - frame.left()) + 1 - textBoxWidth / 2, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignHCenter | Qt::AlignTop, "100");
    painter.drawText(frame.right() - textBoxWidth + 1, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignRight | Qt::AlignTop, "200 ms");

    update();
}

void TriggerPlot::updateWaveform(int numBlocks){
    int i,index,index2;
    bool triggered;
    // Make sure the selected channel is a valid amplifier channel
    if (!selectedChannel) return;
    if (selectedChannel->signalType != AmplifierSignal) return;
    int stream = selectedChannel->boardStream;
    int channel = selectedChannel->chipChannel;
    if(numBlocksTurn==20){
        //trigger
        numBlocksTurn=0;

/*

        int m=0;
        for(int n=0;n<20*SAMPLES_PER_DATA_BLOCK*numBlocks;n++){
            if(m==6000){
                m=0;
            }
            if(m<4000){
                digitalInputBuffer[n]=0;
                m++;
            }
            else if(m<6000){
                digitalInputBuffer[n]=1;
                m++;
            }
        }

*/
        //rms=qSqrt(rms /( 20*(SAMPLES_PER_DATA_BLOCK * numBlocks)));
        //if(thresholdType) setVoltageThreshold(-rmsMultiplier*rms);
       //rms=rmsMultiplier;
        index= startingNewChannel ? (preTriggerTSteps+totalTSteps+1) : preTriggerTSteps+1;
        while (newTrigger &&  index <= 20*SAMPLES_PER_DATA_BLOCK * numBlocks + totalTSteps - 1 - (totalTSteps - preTriggerTSteps)){
            triggered=false;
            if (digitalEdgePolarity) {
                // Digital rising edge trigger
                if (digitalInputBuffer.at(index - 1) == 0 &&
                        digitalInputBuffer.at(index) == 1) {
                    triggered = true;
                    if(newTrigger) newTrigger=false;
                    //numberStimulus++;
                }
            } else {
                // Digital falling edge trigger
                if (digitalInputBuffer.at(index - 1) == 1 &&
                        digitalInputBuffer.at(index) == 0) {
                    triggered = true;
                    if(newTrigger) newTrigger=false;
                    numberStimulus++;
                }
            }
            if(triggered==true){

                index2=-1;
                for(i=index-preTriggerTSteps;i<index+totalTSteps-preTriggerTSteps;i++){
                    index2++;
                    waveformBuffer[index2]=digitalInputBuffer.at(i);

                }
            }
            index++;
        }

        updateTriggerPlot();
        if(startingNewChannel) startingNewChannel=false;
    }


    if(numBlocksTurn==0){
        index=0;
        rms=0;
        for(int j=20*SAMPLES_PER_DATA_BLOCK*numBlocks;j<20*SAMPLES_PER_DATA_BLOCK*numBlocks+totalTSteps;j++){

            digitalInputBuffer[j]=digitalInputBuffer[index++];
        }
        for(i=0;i<SAMPLES_PER_DATA_BLOCK*numBlocks;++i){

            digitalInputBuffer[i + totalTSteps] =  signalProcessor->boardDigIn.at(digitalTriggerChannel).at(i);
        }
        numBlocksTurn++;

    }

    else{
        for(i=0;i<SAMPLES_PER_DATA_BLOCK*numBlocks;++i){

            digitalInputBuffer[numBlocksTurn*SAMPLES_PER_DATA_BLOCK*numBlocks+i + totalTSteps] =  signalProcessor->boardDigIn.at(digitalTriggerChannel).at(i);
        }
        numBlocksTurn++;


    }

}

void TriggerPlot::updateTriggerPlot(){
    int i, j, xOffset, yOffset, index;
    double yAxisLength, tAxisLength;
    QRect adjustedFrame;
    double xScaleFactor, yScaleFactor;
    const double tScale = 300.0;  // time scale = 3.0 ms


    drawAxisLines();

    QPainter painter(&pixmap);
    painter.initFrom(this);

    // Vector for waveform plot points
    QPointF *polyline = new QPointF[totalTSteps];

    yAxisLength = (frame.height() - 2) / 2.0;
    tAxisLength = frame.width() - 1;

    xOffset = frame.left() + 1;

    // Set clipping region for plotting.
    adjustedFrame = frame;
    adjustedFrame.adjust(0, 1, 0, 0);
    painter.setClipRect(adjustedFrame);

    xScaleFactor = tAxisLength * tStepMsec / tScale;

    yScaleFactor = -yAxisLength ;
    yOffset = frame.center().y();



        for (i = 0; i < totalTSteps; ++i) {
            polyline[i] = QPointF(xScaleFactor * i + xOffset, yScaleFactor * waveformBuffer.at(i) + yOffset);
        }

        // Draw waveform
        painter.setPen(Qt::red);
        painter.drawPolyline(polyline, totalTSteps);


    // If using a voltage threshold trigger, plot a line at the threshold level.

    painter.setClipping(false);

    // Don't update the RMS value display every time, or it will change so fast that it
    // will be hard to read.  Only update once every few times we execute this function.


    // Write RMS value to display.


}





// If user spins mouse wheel, change voltage scale.

/*
// Keypresses to change voltage scale.
void TriggerPlot::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Minus:
    case Qt::Key_Underscore:
        psthDialog->contractYScale();
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        psthDialog->expandYScale();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}
*/
QSize TriggerPlot::minimumSizeHint() const
{
    return QSize(SPIKEPLOT_X_SIZE, SPIKEPLOT_Y_SIZE);
}

QSize TriggerPlot::sizeHint() const
{
    return QSize(SPIKEPLOT_X_SIZE, SPIKEPLOT_Y_SIZE);
}

void TriggerPlot::paintEvent(QPaintEvent * /* event */)
{
    QStylePainter stylePainter(this);
    stylePainter.drawPixmap(0, 0, pixmap);
}

void TriggerPlot::closeEvent(QCloseEvent *event)
{
    // Perform any clean-up here before application closes.
    event->accept();
}

// Set the number of spikes that are plotted, superimposed, on the
// display.


// Clear spike display.
void TriggerPlot::clearScope()
{

    drawAxisLines();
}

// Select voltage threshold trigger mode if voltageMode == true, otherwise
// select digital input trigger mode.
/*
void TriggerPlot::setVoltageTriggerMode(bool voltageMode)
{
    voltageTriggerMode = voltageMode;
    if (selectedChannel->signalType == AmplifierSignal) {
        selectedChannel->voltageTriggerMode = voltageMode;
    }
    updateRecordPlot(0.0);
}
*/
// Set voltage threshold trigger level.  We use integer threshold
// levels (in microvolts) since there is no point going to fractional
// microvolt accuracy.


// Select digital input channel for digital input trigger.
void TriggerPlot::setDigitalTriggerChannel(int channel)
{
    digitalTriggerChannel = channel;
    if (selectedChannel->signalType == AmplifierSignal) {
        selectedChannel->digitalTriggerChannel = channel;
    }
    digitalInputBuffer.fill(0);
    waveformBuffer.fill(0.0);
    numBlocksTurn=0;
    newTrigger=true;
}
void TriggerPlot::setNewChannel(SignalChannel *newChannel){
    selectedChannel=newChannel;
    setDigitalEdgePolarity(digitalEdgePolarity);
    setDigitalTriggerChannel(digitalTriggerChannel);
}

// Set digitial trigger edge polarity to rising or falling edge.

void TriggerPlot::setDigitalEdgePolarity(bool risingEdge)
{
    digitalEdgePolarity = risingEdge;
    if (selectedChannel->signalType == AmplifierSignal) {
        selectedChannel->digitalEdgePolarity = risingEdge;
    }
}


void TriggerPlot::resizeEvent(QResizeEvent*) {
    // Pixel map used for double buffering.
    pixmap = QPixmap(size());
    pixmap.fill();
    initializeDisplay();
}

void TriggerPlot::initializeDisplay() {
    const int textBoxWidth = fontMetrics().width("+" + QString::number(yScale) + " " + QSTRING_MU_SYMBOL + "V");
    const int textBoxHeight = fontMetrics().height();
    frame = rect();
    frame.adjust(textBoxWidth + 5, textBoxHeight + 10, -8, -textBoxHeight - 10);

    // Initialize display.
    drawAxisText();
    drawAxisLines();
}


