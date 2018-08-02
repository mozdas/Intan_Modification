#include "magnifyplot.h"

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
#include "magnifydialog.h"


// The MagnifyPlot widget displays a triggered neural spike plot in the
// Spike Scope dialog.  Multiple spikes are plotted on top of one another
// so users may compare their shapes.  The RMS value of the waveform is
// displayed in the plot.  Users may select a new threshold value by clicking
// on the plot.  Keypresses are used to change the voltage scale of the plot.

MagnifyPlot::MagnifyPlot(SignalProcessor *inSignalProcessor, SignalChannel *initialChannel,
                     MagnifyDialog *inMagnifyDialog, QWidget *parent) :
    QWidget(parent)
{
    signalProcessor = inSignalProcessor;
    magnifyDialog = inMagnifyDialog;

    // if true for spikescope dialog , if false for psth Dialog;

    selectedChannel = initialChannel;



    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setFocusPolicy(Qt::StrongFocus);



    // We can plot up to 30 superimposed spike waveforms on the scope.


    // Buffers to hold recent history of spike waveform and digital input,
    // used to find trigger events.
    spikeWaveformBuffer.resize(68000);
    spikeWaveformBuffer.fill(0.0);




    // Default values that may be overwritten.
    yScale = 500;
    tScale=200;
    setSampleRate(30000.0);
}
void MagnifyPlot::setTScale(int newTScale){
    tScale=newTScale;
    totalTSteps = qCeil(tScale / tStepMsec) + 1;
    initializeDisplay();
}
// Set voltage scale.
void MagnifyPlot::setYScale(int newYScale)
{
    yScale = newYScale;
    initializeDisplay();
}

// Set waveform sample rate.
void MagnifyPlot::setSampleRate(double newSampleRate)
{

    // Calculate time step, in msec.
    sampleRate=newSampleRate;
    tStepMsec = 1000.0 / newSampleRate;

    // Calculate number of time steps in 3 msec sample.

    totalTSteps = qCeil(tScale / tStepMsec) + 1;



}

// Draw axis lines on display.
void MagnifyPlot::drawAxisLines()
{
    QPainter painter(&pixmap);
    painter.initFrom(this);

    painter.eraseRect(frame);

    painter.setPen(Qt::darkGray);

    // Draw box outline.
    painter.drawRect(frame);

    // Draw horizonal zero voltage line.
    painter.drawLine(frame.left(), frame.center().y(), frame.right(), frame.center().y());


    update();
}

// Draw text around axes.
void MagnifyPlot::drawAxisText()
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
                      "+" + QString::number(yScale) + " " + QSTRING_MU_SYMBOL + "V");
    painter.drawText(frame.left() - textBoxWidth - 2, frame.center().y() - textBoxHeight / 2,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignRight | Qt::AlignVCenter, "0");
    painter.drawText(frame.left() - textBoxWidth - 2, frame.bottom() - textBoxHeight + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignRight | Qt::AlignBottom,
                      "-" + QString::number(yScale) + " " + QSTRING_MU_SYMBOL + "V");


    painter.drawText(frame.right() - textBoxWidth + 1, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignRight | Qt::AlignTop, QString::number(tScale)+"ms");

    update();
}

// This function loads waveform data for the selected channel from the signal processor object,
// looks for trigger events, captures 3-ms snippets of the waveform after trigger events,
// measures the rms level of the waveform, and updates the display.
void MagnifyPlot::updateWaveform(int numBlocks)
{
    int i, index;

    // Make sure the selected channel is a valid amplifier channel
    if (!selectedChannel) return;
    if (selectedChannel->signalType != AmplifierSignal) return;

    int stream = selectedChannel->boardStream;
    int channel = selectedChannel->chipChannel;

    // Load recent waveform data and digital input data into our buffers.  Also, calculate
    // waveform RMS value.

    for(i=totalTSteps;i>=SAMPLES_PER_DATA_BLOCK*numBlocks;i--){
        spikeWaveformBuffer[i] = spikeWaveformBuffer[i-SAMPLES_PER_DATA_BLOCK*numBlocks];

    }
    for (i = 0; i < SAMPLES_PER_DATA_BLOCK * numBlocks; ++i) {
        spikeWaveformBuffer[i] = signalProcessor->amplifierPostFilter.at(stream).at(channel).at(i);

    }


    // Find trigger events, and then copy waveform snippets to spikeWaveform vector.


    // Copy tail end of waveform to beginning of spike waveform buffer, in case there is a spike
    // at the seam between two data blocks.


    // Update plot.
    updateMagnifyPlot();
}

// Plots spike waveforms and writes RMS value to display.
void MagnifyPlot::updateMagnifyPlot()
{
    int i, j, xOffset, yOffset, index;
    double yAxisLength, tAxisLength;
    QRect adjustedFrame;
    double xScaleFactor, yScaleFactor;






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
    yScaleFactor = -yAxisLength / yScale;
    yOffset = frame.center().y();



        for (i = 0; i < totalTSteps; ++i) {
            polyline[i] = QPointF(xScaleFactor * i + xOffset, yScaleFactor * spikeWaveformBuffer.at(i) + yOffset);
        }

        // Draw waveform
        painter.setPen(Qt::blue);
        painter.drawPolyline(polyline, totalTSteps);




    painter.setClipping(false);


    delete [] polyline;
    update();
}




// If user clicks inside display, set voltage threshold to that level.


// If user spins mouse wheel, change voltage scale.
void MagnifyPlot::wheelEvent(QWheelEvent *event)
{


        if (event->delta() > 0) {
             magnifyDialog->contractYScale();
        } else {
            magnifyDialog->expandYScale();
        }

}



QSize MagnifyPlot::minimumSizeHint() const
{
    return QSize(MAGNIFYPLOT_X_SIZE, MAGNIFYPLOT_Y_SIZE);
}

QSize MagnifyPlot::sizeHint() const
{
    return QSize(MAGNIFYPLOT_X_SIZE, MAGNIFYPLOT_Y_SIZE);
}

void MagnifyPlot::paintEvent(QPaintEvent * /* event */)
{
    QStylePainter stylePainter(this);
    stylePainter.drawPixmap(0, 0, pixmap);
}

void MagnifyPlot::closeEvent(QCloseEvent *event)
{
    // Perform any clean-up here before application closes.
    event->accept();
}

// Set the number of spikes that are plotted, superimposed, on the
// display.


// Clear spike display.
void MagnifyPlot::clearScope()
{
    spikeWaveformBuffer.fill(0);
    drawAxisLines();
}



// Set voltage threshold trigger level.  We use integer threshold
// levels (in microvolts) since there is no point going to fractional
// microvolt accuracy.





// Change to a new signal channel.
void MagnifyPlot::setNewChannel(SignalChannel* newChannel)
{
    selectedChannel = newChannel;

    initializeDisplay();
}

void MagnifyPlot::resizeEvent(QResizeEvent*) {
    // Pixel map used for double buffering.
    pixmap = QPixmap(size());
    pixmap.fill();
    initializeDisplay();
}



void MagnifyPlot::initializeDisplay() {
    const int textBoxWidth = fontMetrics().width("+" + QString::number(yScale) + " " + QSTRING_MU_SYMBOL + "V");
    const int textBoxHeight = fontMetrics().height();
    frame = rect();
    frame.adjust(textBoxWidth + 5, textBoxHeight + 10, -8, -textBoxHeight - 10);

    // Initialize display.
    drawAxisText();
    drawAxisLines();
}
