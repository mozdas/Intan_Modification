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
#include "psthplot.h"
#include "recorddialog.h"
#include "psthanalysisdialog.h"
#include "psthanalysisplot.h"

// The RecordPlot widget displays a triggered neural spike plot in the
// Spike Scope dialog.  Multiple spikes are plotted on top of one another
// so users may compare their shapes.  The RMS value of the waveform is
// displayed in the plot.  Users may select a new threshold value by clicking
// on the plot.  Keypresses are used to change the voltage scale of the plot.

PsthPlot::PsthPlot(SignalProcessor *inSignalProcessor, SignalChannel *initialChannel,
                     PsthDialog *inPsthDialog, QWidget *parent) :
    QWidget(parent)
{
    //setting the default values of the variables
    signalProcessor = inSignalProcessor;
    psthDialog = inPsthDialog;
    rms=0;
    thresholdType=true;

    selectedChannel = initialChannel;
    futureChannel=initialChannel;
    startingNewChannel = true;


    voltageThreshold = 0;
    digitalTriggerChannel = 1;
    digitalEdgePolarity = true;
    tScale=75;
    rmsMultiplier=4;

    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setFocusPolicy(Qt::StrongFocus);


    // psthArray is for the values in histogram
    psthArray.resize(4510);
    psthArray.fill(0.0);

    //number stimulus is number of rising or falling edges in digital trigger

    numberStimulus=0;



    // Buffers to hold recent history of spike waveform and digital input,
    // used to find trigger events.
    spikeWaveformBuffer.resize(BUFFER_SIZE);
    spikeWaveformBuffer.fill(0.0);
    digitalInputBuffer.resize(BUFFER_SIZE);
    digitalInputBuffer.fill(0);




    // Default values that may be overwritten.
    yScale = 1;
    setSampleRate(30000.0);
    futureTScale=75;
}

// Set voltage scale.
void PsthPlot::setYScale(double newYScale)
{
    yScale = newYScale;
    initializeDisplay();
}

// Set waveform sample rate.
void PsthPlot::setSampleRate(double newSampleRate)
{
    sampleRate=newSampleRate;
    // Calculate time step, in msec.
    tStepMsec = 1000.0 / newSampleRate;

    // Calculate number of time steps in tScale  sample.

    psthTotalTsteps=qCeil(tScale/tStepMsec)+1;

    // Calculate number of time steps in the tScale/3 msec pre-trigger
    // display interval.

    psthPreTSteps=qCeil(tScale/(3*tStepMsec));




    numBlocksTurn=0;
    psthArray.fill(0.0);
    startingNewChannel = true;

}

// Draw axis lines on display.
void PsthPlot::drawAxisLines()
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
void PsthPlot::drawAxisText()
{
    QPainter painter(&pixmap);
    painter.initFrom(this);
    const int textBoxWidth = painter.fontMetrics().width("+" + QString::number(yScale) + " " + QSTRING_MU_SYMBOL + "V");
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
                        QString::number(yScale) + " "  + "#");
    painter.drawText(frame.left() - textBoxWidth - 2, frame.center().y() - textBoxHeight / 2,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignRight | Qt::AlignVCenter, QString::number(yScale/2));
    painter.drawText(frame.left() - textBoxWidth - 2, frame.bottom() - textBoxHeight + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignRight | Qt::AlignBottom,"0");

    // Label the time axis.
    painter.drawText(frame.left() - textBoxWidth / 2, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignHCenter | Qt::AlignTop, "-"+QString::number(tScale/3));
    painter.drawText(frame.left() + (1.0/3.0) * (frame.right() - frame.left()) + 1 - textBoxWidth / 2, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignHCenter | Qt::AlignTop, "0");
    painter.drawText(frame.left() + (2.0/3.0) * (frame.right() - frame.left()) + 1 - textBoxWidth / 2, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignHCenter | Qt::AlignTop, "+"+QString::number(tScale/3));
    painter.drawText(frame.right() - textBoxWidth + 1, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignRight | Qt::AlignTop, "+"+QString::number(2*tScale/3)+"ms");

    update();
}
//this function is for increasing a variable in psthanalysis plot
void PsthPlot::increaseAnalysisStimulus(){
    psthDialog->increaseAnalysisStimulus();
}
//this function is for increasing a variable in psthanalysis plot
void PsthPlot::increaseAnalysisSpikes(){
    psthDialog->increaseAnalysisSpikes();
}
// generating the psthArray which is going to be plotted.
void PsthPlot::updatePsthArray(int numBlocks){
    int i,index,index2;
    bool triggered;
    // Make sure the selected channel is a valid amplifier channel
    if (!selectedChannel) return;
    if (selectedChannel->signalType != AmplifierSignal) return;
    int stream = selectedChannel->boardStream;
    int channel = selectedChannel->chipChannel;

    // for first 20 turns, data is placed to the buffers, now they are processed
    if(numBlocksTurn==20){
        //trigger
        numBlocksTurn=0;

// this part is for test digital data, when data is not connected to the board,
        //do not uncomment if you wanna test it.
/*
        int m=0;
        for(int n=0;n<20*SAMPLES_PER_DATA_BLOCK*numBlocks;n++){
            if(m==500){
                m=0;
            }
            if(m<400){
                digitalInputBuffer[n]=1;
                m++;
            }
            else if(m<500){
                digitalInputBuffer[n]=0;
                m++;
            }
        }
*/
//generating rms value to be used in the threshold.

        rms=qSqrt(rms /( 20*(SAMPLES_PER_DATA_BLOCK * numBlocks)));
        if(thresholdType) setVoltageThreshold(-rmsMultiplier*rms);

        index= startingNewChannel ? (psthPreTSteps+psthTotalTsteps) : psthPreTSteps;
        while (index <= 20*SAMPLES_PER_DATA_BLOCK * numBlocks + psthTotalTsteps - 1 - (psthTotalTsteps - psthPreTSteps)){
            triggered=false;

            if (digitalEdgePolarity) {
                // Digital rising edge trigger
                if (digitalInputBuffer.at(index - 1) == 0 &&
                        digitalInputBuffer.at(index) == 1) {
                    triggered = true;
                    numberStimulus++;

                    increaseAnalysisStimulus();


                }
            } else {
                // Digital falling edge trigger
                if (digitalInputBuffer.at(index - 1) == 1 &&
                        digitalInputBuffer.at(index) == 0) {
                    triggered = true;
                    numberStimulus++;
                    increaseAnalysisStimulus();


                }
            }
            if(triggered==true){
                //when trigger event comes, enter to another loop

                index2=-1;
                for(i=index-psthPreTSteps+1;i<index+psthTotalTsteps-psthPreTSteps;i++){
                    index2++;
                    if (spikeWaveformBuffer.at(i - 1) > voltageThreshold &&
                            spikeWaveformBuffer.at(i) <= voltageThreshold){
                        psthArray[index2]+=1;
                        increaseAnalysisSpikes();
                    }
                }
            }
            index++;
        }
        if(startingNewChannel) startingNewChannel=false;
        updatePsthPlot();
    }
    if(numBlocksTurn==0){

        index=0;
        rms=0;
        //for the first recording, copy the tail of the end of the last buffer to the beginning of the new one
        for(int j=20*SAMPLES_PER_DATA_BLOCK*numBlocks;j<20*SAMPLES_PER_DATA_BLOCK*numBlocks+psthTotalTsteps;j++){
            spikeWaveformBuffer[index]=spikeWaveformBuffer[j];
            digitalInputBuffer[index++]=digitalInputBuffer[j];
        }
        for(i=0;i<SAMPLES_PER_DATA_BLOCK*numBlocks;++i){
            spikeWaveformBuffer[i + psthTotalTsteps] = signalProcessor->amplifierPostFilter.at(stream).at(channel).at(i);
            rms += (signalProcessor->amplifierPostFilter.at(stream).at(channel).at(i) *
                    signalProcessor->amplifierPostFilter.at(stream).at(channel).at(i));
            digitalInputBuffer[i + psthTotalTsteps] =  signalProcessor->boardDigIn.at(digitalTriggerChannel).at(i);
        }
        numBlocksTurn++;
    }

    else{
        for(i=0;i<SAMPLES_PER_DATA_BLOCK*numBlocks;++i){
            spikeWaveformBuffer[numBlocksTurn*SAMPLES_PER_DATA_BLOCK*numBlocks + i + psthTotalTsteps] = signalProcessor->amplifierPostFilter.at(stream).at(channel).at(i);
            rms += (signalProcessor->amplifierPostFilter.at(stream).at(channel).at(i) *
                    signalProcessor->amplifierPostFilter.at(stream).at(channel).at(i));
            digitalInputBuffer[numBlocksTurn*SAMPLES_PER_DATA_BLOCK*numBlocks+i + psthTotalTsteps] =  signalProcessor->boardDigIn.at(digitalTriggerChannel).at(i);
        }
        numBlocksTurn++;
    }
}

void PsthPlot::updatePsthPlot(){
    // creating a new array for the 1ms time steps in plot
    adjustedPsthArray.resize(150);
    adjustedPsthArray.fill(0.0);

    for(int m=0;m<tScale;m++ ){
        for(int n=0;n<1/tStepMsec;n++){
            adjustedPsthArray[m]+=psthArray[(m*(1/tStepMsec))+n];
        }
        if(numberStimulus==0) numberStimulus++;
        adjustedPsthArray[m]=adjustedPsthArray[m]/numberStimulus;
    }




    int  xOffset, yOffset;
    double yAxisLength, tAxisLength;
    QRect adjustedFrame;
    double xScaleFactor, yScaleFactor;
   // const double tScale = 75.0;  // time scale = 75.0 ms



    drawAxisLines();

    QPainter painter(&pixmap);
    painter.initFrom(this);


    // Vector for waveform plot points
    //QPointF *polyline = new QPointF[psthTotalTsteps];



    yAxisLength = (frame.height() -1);
    tAxisLength = frame.width() - 1;

    xOffset = frame.left() + 1;

    // Set clipping region for plotting.
    adjustedFrame = frame;
    adjustedFrame.adjust(0, 1, 0, 0);
    painter.setClipRect(adjustedFrame);

    //xScaleFactor = tAxisLength * tStepMsec / tScale;
    xScaleFactor = tAxisLength  / tScale;

    yScaleFactor = -yAxisLength / yScale;
    //yScaleFactor=-yAxisLength*yScale;
    yOffset = frame.bottom();


    if(tScale==75){
        painter.setPen(QPen(Qt::darkMagenta,3));
          }
    else{
        painter.setPen(QPen(Qt::darkMagenta,1));
    }


    for(int m=0;m<tScale;m++){
        painter.drawLine(xScaleFactor*m+xOffset,yOffset,xScaleFactor*m+xOffset,yScaleFactor * adjustedPsthArray.at(m) + yOffset);
    }

    // If using a voltage threshold trigger, plot a line at the threshold level.

/*

        painter.setPen(Qt::red);

        painter.drawLine(xOffset, yScaleFactor * voltageThreshold + yOffset,
                          xScaleFactor * (psthTotalTsteps - 1) +  xOffset, yScaleFactor * voltageThreshold + yOffset);

*/
    painter.setClipping(false);



    // Write number of stimulus value to display.
    const int textBoxWidth = 180;
    const int textBoxHeight = painter.fontMetrics().height();
    painter.setPen(Qt::darkGreen);
    painter.drawText(frame.left() + 6, frame.top() + 5,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignLeft | Qt::AlignTop,
                     // "RMS:" + QString::number(rms, 'f', (rms < 10.0) ? 1 : 0) +
                     // " " + QSTRING_MU_SYMBOL + "V");
                      "#Stimuli:" + QString::number(numberStimulus));
                     //QString::number(rms)+" "+QString::number(rmsMultiplier)+" "+QString::number(voltageThreshold));


    update();


}
void PsthPlot::zeroAnalysisData(){
    //this is for the psth analysis class, reseting the values
    numberAnalysisStimulus=0;
    psthAnalysisData=0;
}
void PsthPlot:: changeTScale(double newTScale){
    futureTScale=newTScale;


}

void PsthPlot::setRmsMultiplier(int value){
    rmsMultiplier=value;
}




// If user spins mouse wheel, change voltage scale.
void PsthPlot::wheelEvent(QWheelEvent *event)
{
    if (event->delta() > 0) {
        psthDialog->contractpsthYScale();
    } else {
        psthDialog->expandpsthYScale();
    }
}
/*
// Keypresses to change voltage scale.
void PsthPlot::keyPressEvent(QKeyEvent *event)
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
QSize PsthPlot::minimumSizeHint() const
{
    return QSize(PSTHPLOT_X_SIZE, PSTHPLOT_Y_SIZE);
}

QSize PsthPlot::sizeHint() const
{
    return QSize(PSTHPLOT_X_SIZE, PSTHPLOT_Y_SIZE);
}

void PsthPlot::paintEvent(QPaintEvent * /* event */)
{
    QStylePainter stylePainter(this);
    stylePainter.drawPixmap(0, 0, pixmap);
}

void PsthPlot::closeEvent(QCloseEvent *event)
{
    // Perform any clean-up here before application closes.
    event->accept();
}

// Set the number of spikes that are plotted, superimposed, on the
// display.


// Clear spike display.
void PsthPlot::clearScope()
{

    drawAxisLines();
}

// Select voltage threshold trigger mode if voltageMode == true, otherwise
// select digital input trigger mode.
/*
void PsthPlot::setVoltageTriggerMode(bool voltageMode)
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
void PsthPlot::setVoltageThreshold(int threshold)
{
    voltageThreshold = threshold;
    if (selectedChannel->signalType == AmplifierSignal) {
        selectedChannel->voltageThreshold = threshold;
    }
}

// Select digital input channel for digital input trigger.
void PsthPlot::setDigitalTriggerChannel(int channel)
{
    digitalTriggerChannel = channel;
    if (selectedChannel->signalType == AmplifierSignal) {
        selectedChannel->digitalTriggerChannel = channel;
    }
}

// Set digitial trigger edge polarity to rising or falling edge.

void PsthPlot::setDigitalEdgePolarity(bool risingEdge)
{
    digitalEdgePolarity = risingEdge;
    if (selectedChannel->signalType == AmplifierSignal) {
        selectedChannel->digitalEdgePolarity = risingEdge;
    }
}
void PsthPlot::resetPsth(){
    //resets psth data and sends data to the psthDialog

    selectedChannel=futureChannel;
    if(psthDialog->recordButton->isChecked()){
        psthDialog->recordDialog->updateWaveform(adjustedPsthArray,tScale);
        psthDialog->recordDialog->updateRecordPlot();
    }
    psthArray.resize(4510);
    psthArray.fill(0);
    numberStimulus=0;
    startingNewChannel = true;
    spikeWaveformBuffer.resize(BUFFER_SIZE);
    spikeWaveformBuffer.fill(0.0);
    digitalInputBuffer.resize(BUFFER_SIZE);
    digitalInputBuffer.fill(0);
    tScale=futureTScale;
    setSampleRate(sampleRate);
    setDigitalTriggerChannel(digitalTriggerChannel);
    setDigitalEdgePolarity(digitalEdgePolarity);






    initializeDisplay();


}
// Change to a new signal channel.
void PsthPlot::setNewChannel(SignalChannel* newChannel)
{

    futureChannel = newChannel;

}

void PsthPlot::resizeEvent(QResizeEvent*) {
    // Pixel map used for double buffering.
    pixmap = QPixmap(size());
    pixmap.fill();
    initializeDisplay();
}

void PsthPlot::initializeDisplay() {
    const int textBoxWidth = fontMetrics().width("+" + QString::number(yScale) + " " + QSTRING_MU_SYMBOL + "V");
    const int textBoxHeight = fontMetrics().height();
    frame = rect();
    frame.adjust(textBoxWidth + 5, textBoxHeight + 10, -8, -textBoxHeight - 10);

    // Initialize display.
    drawAxisText();
    drawAxisLines();
}
void PsthPlot::setThresholdType(bool value){
    thresholdType=value;
    //true means rms times multiplier

    //false means chosen from spikeplot

}
