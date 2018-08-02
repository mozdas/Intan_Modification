#include <QtGui>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

#include <qmath.h>
#include <iostream>
#include <sstream>

#include "globalconstants.h"
#include "signalprocessor.h"
#include "signalchannel.h"
#include "psthanalysisdialog.h"
#include "psthanalysisplot.h"
#include "psthplot.h"

// The LfpPlot widget displays a triggered neural lfp plot in the
// Lfp Scope dialog.  Multiple lfps are plotted on top of one another
// so users may compare their shapes.  The RMS value of the waveform is
// displayed in the plot.  Users may select a new threshold value by clicking
// on the plot.  Keypresses are used to change the voltage scale of the plot.

PsthAnalysisPlot::PsthAnalysisPlot(PsthPlot *inPsthPlot, SignalProcessor *inSignalProcessor, SignalChannel *initialChannel,
                     PsthAnalysisDialog *inPsthAnalysisDialog, QWidget *parent) :
    QWidget(parent)
{
    psthPlot = inPsthPlot;
    signalProcessor = inSignalProcessor;
    psthAnalysisDialog= inPsthAnalysisDialog;

    selectedChannel = initialChannel;
    startingNewChannel = true;

    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setFocusPolicy(Qt::StrongFocus);

    yScale = 10;
    tScaleInSec = 10*60;
    tStepSec = 1;
    numberSpikes=0;
    numberStimulus=0;
    analysisType = MIN_ANALYSIS;
    tLfpStepMsec = 1;
    //setAnalysisWindow(0 , 50);
}

// Set voltage scale.
void PsthAnalysisPlot::setYScale(int newYScale)
{
    yScale = newYScale;
    initializeDisplay();
}

// Set waveform sample rate.
/*
void PsthAnalysisPlot::setSampleRate(double newSampleRate)
{
    totalTSteps = lfpPlot->totalTSteps;
    tLfpStepMsec = lfpPlot->tStepMsec;

    double initialTimePoint = - (lfpPlot->preTriggerTSteps) * tLfpStepMsec;

    windowStartTimeIndex = qCeil((windowStartTimeInMs - initialTimePoint) / tLfpStepMsec);
    windowEndTimeIndex = qCeil((windowEndTimeInMs - initialTimePoint) /tLfpStepMsec) + 1 ;

    windowStartTimeIndex = (windowStartTimeIndex < 0) ? 0 : windowStartTimeIndex;
    windowEndTimeIndex = (windowEndTimeIndex > totalTSteps) ? totalTSteps : windowEndTimeIndex;

    qDebug() << "index: "<< QString::number( windowStartTimeIndex) << " , " << QString::number(windowEndTimeIndex);

    //Send Message and Stop the Run Reset the stuff
}
*/
// Draw axis lines on display.
void PsthAnalysisPlot::drawAxisLines()
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
    painter.drawLine(frame.left() + (1.0/5.0) * (frame.right() - frame.left()) + 1, frame.top(),
                      frame.left() + (1.0/5.0) * (frame.right() - frame.left()) + 1, frame.bottom());
    painter.drawLine(frame.left() + (2.0/5.0) * (frame.right() - frame.left()) + 1, frame.top(),
                      frame.left() + (2.0/5.0) * (frame.right() - frame.left()) + 1, frame.bottom());
    painter.drawLine(frame.left() + (3.0/5.0) * (frame.right() - frame.left()) + 1, frame.top(),
                      frame.left() + (3.0/5.0) * (frame.right() - frame.left()) + 1, frame.bottom());
    painter.drawLine(frame.left() + (4.0/5.0) * (frame.right() - frame.left()) + 1, frame.top(),
                      frame.left() + (4.0/5.0) * (frame.right() - frame.left()) + 1, frame.bottom());
    update();
}

// Draw text around axes.
void PsthAnalysisPlot::drawAxisText()
{
    QPainter painter(&pixmap);
    painter.initFrom(this);
    const int textBoxWidth = painter.fontMetrics().width("+" + QString::number(1000) + " " + QSTRING_MU_SYMBOL + "V");
    const int textBoxHeight = painter.fontMetrics().height();

    // Clear entire Widget display area.
    painter.eraseRect(rect());

    // Draw border around Widget display area.
    painter.setPen(Qt::darkGray);
    QRect rect(0, 0, width() - 1, height() - 1);
    painter.drawRect(rect);

    // If the selected channel is an amplifier channel, then write the channel name and number,
    // otherwise remind the user than non-amplifier channels cannot be displayed in Lfp Scope.
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
                       QString::number(yScale) );
    painter.drawText(frame.left() - textBoxWidth - 2, frame.center().y() - textBoxHeight / 2,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignRight | Qt::AlignVCenter, QString::number(yScale/2));
    painter.drawText(frame.left() - textBoxWidth - 2, frame.bottom() - textBoxHeight + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignRight | Qt::AlignBottom,
                      "0");

    // Label the time axis.
    painter.drawText(frame.left() - textBoxWidth / 2, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignHCenter | Qt::AlignTop, "0");
    painter.drawText(frame.left() + (1.0/5.0) * (frame.right() - frame.left()) + 1 - textBoxWidth / 2, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignHCenter | Qt::AlignTop, QString::number(tScaleInSec*1/300)+ " mins");
    painter.drawText(frame.left() + (2.0/5.0) * (frame.right() - frame.left()) + 1 - textBoxWidth / 2, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignHCenter | Qt::AlignTop, QString::number(tScaleInSec*2/300)+ " mins");
    painter.drawText(frame.left() + (3.0/5.0) * (frame.right() - frame.left()) + 1 - textBoxWidth / 2, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignHCenter | Qt::AlignTop, QString::number(tScaleInSec*3/300)+ " mins");
    painter.drawText(frame.left() + (4.0/5.0) * (frame.right() - frame.left()) + 1 - textBoxWidth / 2, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignHCenter | Qt::AlignTop, QString::number(tScaleInSec*4/300)+ " mins");
    painter.drawText(frame.right() - textBoxWidth + 1, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignRight | Qt::AlignTop, QString::number(tScaleInSec*5/300)+ " mins");

    update();
}


void PsthAnalysisPlot::updateWaveform()

{
    if(numberStimulus==0){
         analysisData.append(0);

    }
    else{
        analysisData.append(numberSpikes/numberStimulus);
    }

    if(psthReseting){
        psthPlot->resetPsth();
    }




    numberSpikes=0;
    numberStimulus=0;


    updatePsthAnalysisPlot();
}



// Plots lfp waveforms and writes RMS value to display.
void PsthAnalysisPlot::updatePsthAnalysisPlot()
{
    int i, j, xOffset, yOffset, index;
    double yAxisLength, tAxisLength;
    QRect adjustedFrame;
    double xScaleFactor, yScaleFactor;


    drawAxisLines();

    QPainter painter(&pixmap);
    painter.initFrom(this);

    // Vector for waveform plot points


    yAxisLength = (frame.height() - 1);
    tAxisLength = frame.width() - 1;

    xOffset = frame.left() + 1;

    // Set clipping region for plotting.
    adjustedFrame = frame;
    adjustedFrame.adjust(0, 1, 0, 0);
    painter.setClipRect(adjustedFrame);

    xScaleFactor = tAxisLength * tStepSec / tScaleInSec;
    yScaleFactor = -yAxisLength / yScale;
    yOffset = frame.bottom();


    // Error Bar display calculated from Overall average LFP and std from same index
    QPointF *polyline = new QPointF[tScaleInSec];
    for (i = 0; i < analysisData.size(); ++i) {
        polyline[i] = QPointF(xScaleFactor * i + xOffset, yScaleFactor * analysisData.at(i) + yOffset);
/*
        if(errorbarDisplayState){
        painter.setPen(Qt::red);
        painter.drawLine(QPointF(xScaleFactor * i + xOffset,yScaleFactor * (meanLfpAnalysisData.at(i) - stdOfmeanLfpAnalysisData.at(i)) + yOffset),
                         QPointF(xScaleFactor * i + xOffset,yScaleFactor * (meanLfpAnalysisData.at(i) + stdOfmeanLfpAnalysisData.at(i)) + yOffset));
        }
        */
    }
    painter.setPen(Qt::blue);
    painter.drawPolyline(polyline, analysisData.size());
    delete [] polyline;

    //Error Bar display calculated from Each LFP minumum
    /*QPointF *polyline2 = new QPointF[tScaleInSec];
    for (i = 0; i < mLfpAnalysisData.size(); ++i) {
        polyline2[i] = QPointF(xScaleFactor * i + xOffset, yScaleFactor * mLfpAnalysisData.at(i) + yOffset);

        if(errorbarDisplayState){
        painter.setPen(Qt::cyan);
        painter.drawLine(QPointF(xScaleFactor * i + xOffset,yScaleFactor * (mLfpAnalysisData.at(i) - stdLfpAnalysisData.at(i)) + yOffset),
                         QPointF(xScaleFactor * i + xOffset,yScaleFactor * (mLfpAnalysisData.at(i) + stdLfpAnalysisData.at(i)) + yOffset));
        }
    }

    painter.setPen(Qt::blue);
    painter.drawPolyline(polyline2, mLfpAnalysisData.size());
    delete [] polyline2; */

    update();
}

// If user clicks inside display, set voltage threshold to that level.
void PsthAnalysisPlot::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (frame.contains(event->pos())) {
        }
    } else {
        QWidget::mousePressEvent(event);
    }
}

// If user spins mouse wheel, change voltage scale.
void PsthAnalysisPlot::wheelEvent(QWheelEvent *event)
{
    if (event->delta() > 0) {
        psthAnalysisDialog->contractYScale();
    } else {
        psthAnalysisDialog->expandYScale();
    }
}

// Keypresses to change voltage scale.
void PsthAnalysisPlot::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Minus:
    case Qt::Key_Underscore:
        psthAnalysisDialog->contractYScale();
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        psthAnalysisDialog->expandYScale();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

QSize PsthAnalysisPlot::minimumSizeHint() const
{
    return QSize(PSTHPLOT_X_SIZE, PSTHPLOT_Y_SIZE);
}

QSize PsthAnalysisPlot::sizeHint() const
{
    return QSize(PSTHANALYSISPLOT_X_SIZE, PSTHANALYSISPLOT_Y_SIZE);
}

void PsthAnalysisPlot::paintEvent(QPaintEvent * /* event */)
{
    QStylePainter stylePainter(this);
    stylePainter.drawPixmap(0, 0, pixmap);
}

void PsthAnalysisPlot::closeEvent(QCloseEvent *event)
{
    // Perform any clean-up here before application closes.
    event->accept();
}
/*
// Set the number of lfps that are plotted, superimposed, on the
// display.
void PsthAnalysisPlot::setMaxNumLfpWaveforms()
{
    maxNumLfpWaveforms = lfpPlot->maxNumLfpWaveforms;
    numLfpWaveforms = lfpPlot->numLfpWaveforms;
    //Add code for clearing the Buffer used for analysis
}
*/
// Clear lfp display.

void PsthAnalysisPlot::clearScope()
{
    //Fill here with something else
    numLfpWaveforms = 0;
    drawAxisLines();
}

// Change to a new signal channel.
void PsthAnalysisPlot::setNewChannel(SignalChannel* newChannel)
{
    selectedChannel = newChannel;
    startingNewChannel = true;
    initializeDisplay();
}

void PsthAnalysisPlot::resizeEvent(QResizeEvent*) {
    // Pixel map used for double buffering.
    pixmap = QPixmap(size());
    pixmap.fill();
    initializeDisplay();
}

void PsthAnalysisPlot::initializeDisplay() {
    const int textBoxWidth = fontMetrics().width("+" + QString::number(yScale) + " " + QSTRING_MU_SYMBOL + "V");
    const int textBoxHeight = fontMetrics().height();
    frame = rect();
    frame.adjust(textBoxWidth + 5, textBoxHeight + 10, -8, -textBoxHeight - 10);

    // Initialize display.
    drawAxisText();
    drawAxisLines();

    if(analysisData.size() != 0 ){updatePsthAnalysisPlot();} //if the analysis is running redraw the lines
}

void PsthAnalysisPlot::setTScale(int t){

    tScaleInSec = t;
    initializeDisplay();
}

void PsthAnalysisPlot::setTStepValue(int t){

    tStepSec = t;
    initializeDisplay();
}

void PsthAnalysisPlot::changePsthReseting(bool state){
    psthReseting=state;

}


void PsthAnalysisPlot::applyStartProcess(){
    analysisData.resize(0);
    numberStimulus=0;
    numberSpikes=0;


    //stdLfpAnalysisData.resize(0);
    //mLfpAnalysisData.resize(0);

}

void PsthAnalysisPlot::applyStopProcess(){
    analysisData.resize(0);
    numberStimulus=0;
    numberSpikes=0;
    //stdLfpAnalysisData.resize(0);
    //mLfpAnalysisData.resize(0);
}
/*
void PsthAnalysisPlot::setAnalysisWindow(double startInMs, double endInMs){
    if(startInMs == 0 && endInMs == 0)
    {
        windowStartTimeInMs = - (lfpPlot->preTriggerTSteps) * tLfpStepMsec ;
        windowEndTimeInMs = (lfpPlot->totalTSteps - lfpPlot->preTriggerTSteps)*tLfpStepMsec;
    }else{
        windowStartTimeInMs = startInMs;
        windowEndTimeInMs = endInMs;
    }
        setSampleRate(1000/tLfpStepMsec);
}

void PsthAnalysisPlot::changeAnalysisType(int newType){
    analysisType = newType;
}
*/
