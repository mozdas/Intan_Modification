#include "recordplot.h"
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif
#include <qmath.h>
#include <iostream>
#include <psthplot.h>
#include "globalconstants.h"
#include "signalprocessor.h"
#include "signalchannel.h"
#include "spikescopedialog.h"
#include "psthdialog.h"
#include "recordplot.h"
#include "recorddialog.h"


// The RecordPlot widget displays a triggered neural spike plot in the
// Spike Scope dialog.  Multiple spikes are plotted on top of one another
// so users may compare their shapes.  The RMS value of the waveform is
// displayed in the plot.  Users may select a new threshold value by clicking
// on the plot.  Keypresses are used to change the voltage scale of the plot.

RecordPlot::RecordPlot(RecordDialog *inRecordDialog, QWidget *parent) :
    QWidget(parent)
{
    recordDialog=inRecordDialog;

    waveformIndex = 0;
    numSpikeWaveforms = 0;
    selectedFrame=0;



    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setFocusPolicy(Qt::StrongFocus);

    int i;

    // We can plot up to 30 superimposed spike waveforms on the scope.
    waveformBuffer.resize(20);
    tScale.resize(20);
    for (i = 0; i < waveformBuffer.size(); ++i) {
        // Each waveform is 3 ms in duration.  We need 91 time steps for a 3 ms
        // waveform with the sample rate is set to its maximum value of 30 kS/s.
        waveformBuffer[i].resize(150);
        waveformBuffer[i].fill(0.0);
    }

    // Buffers to hold recent history of spike waveform and digital input,
    // used to find trigger events.


    // Set up vectors of varying plot colors so that older waveforms
    // are plotted in low-contrast gray and new waveforms are plotted
    // in high-contrast blue.  Older signals fade away, like phosphor
    // traces on old-school CRT oscilloscopes.

    // Default values that may be overwritten.
    yScale = 1;

}

// Set voltage scale.
void RecordPlot::setYScale(double newYScale)
{
    yScale = newYScale;
    initializeDisplay();
    updateRecordPlot();
}

// Set waveform sample rate.

// Draw axis lines on display.
// Draw axis lines on display.
void RecordPlot::drawAxisLines()
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
void RecordPlot::drawAxisText()
{
    QPainter painter(&pixmap);
    painter.initFrom(this);
    const int textBoxWidth = painter.fontMetrics().width("+" + QString::number(yScale) + " " + QSTRING_MU_SYMBOL + "VAAA");
    const int textBoxHeight = painter.fontMetrics().height();

    // Clear entire Widget display area.
    painter.eraseRect(rect());

    // Draw border around Widget display area.
    painter.setPen(Qt::darkGray);
    QRect rect(0, 0, width() - 1, height() - 1);
    painter.drawRect(rect);

    // If the selected channel is an amplifier channel, then write the channel name and number,
    // otherwise remind the user than non-amplifier channels cannot be displayed in Spike Scope.


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
                      Qt::AlignHCenter | Qt::AlignTop, "-"+QString::number(tScale[selectedFrame]/3));
    painter.drawText(frame.left() + (1.0/3.0) * (frame.right() - frame.left()) + 1 - textBoxWidth / 2, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignHCenter | Qt::AlignTop, "0");
    painter.drawText(frame.left() + (2.0/3.0) * (frame.right() - frame.left()) + 1 - textBoxWidth / 2, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignHCenter | Qt::AlignTop, "+"+QString::number(tScale[selectedFrame]/3));
    painter.drawText(frame.right() - textBoxWidth + 1, frame.bottom() + 1,
                      textBoxWidth, textBoxHeight,
                      Qt::AlignRight | Qt::AlignTop, "+"+QString::number(2*tScale[selectedFrame]/3)+" ms");

    update();
}

// This function loads waveform data for the selected channel from the signal processor object,
// looks for trigger events, captures 3-ms snippets of the waveform after trigger events,
// measures the rms level of the waveform, and updates the display.
void RecordPlot::updateWaveform(QVector<double> psthWaveForm,double t_Scale )
{
    for(int i=waveformBuffer.size()-1;i>0;i--){
        waveformBuffer[i]=waveformBuffer.at(i-1);
        tScale[i]=tScale.at(i-1);


    }
    tScale[0]=t_Scale;
    waveformBuffer[0]=psthWaveForm;



    // Update plot.
    updateRecordPlot();
}
void RecordPlot::changeSelectedFrame(int frame){
    selectedFrame=frame;
    updateRecordPlot();
}

// Plots spike waveforms and writes RMS value to display.
void RecordPlot::updateRecordPlot()
{







    int  xOffset, yOffset;
    double yAxisLength, tAxisLength;
    QRect adjustedFrame;
    double xScaleFactor, yScaleFactor;
   // const double tScale = 75.0;  // time scale = 75.0 ms


    drawAxisText();
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
    xScaleFactor = tAxisLength  / tScale[selectedFrame];

    yScaleFactor = -yAxisLength / yScale;
    //yScaleFactor=-yAxisLength*yScale;
    yOffset = frame.bottom();


        // Build waveform
    /*
        for (i = 0; i <psthTotalTsteps ; ++i) {
            polyline[i] = QPointF(xScaleFactor * i + xOffset, yScaleFactor * psthArray.at(i) + yOffset);
        }

        // Draw waveform
        painter.setPen(Qt::darkMagenta);
        painter.drawPolyline(polyline, psthTotalTsteps);
*/
    /*
    for (i = 0; i <75 ; ++i) {
        polyline[i] = QPointF(xScaleFactor * i + xOffset, yScaleFactor * adjustedPsthArray.at(i) + yOffset);
    }
*/
    // Draw waveform
    if(tScale[selectedFrame]==75){
        painter.setPen(QPen(Qt::darkMagenta,4));
          }
    else{
        painter.setPen(QPen(Qt::darkMagenta,2));
    }


    for(int m=0;m<tScale[selectedFrame];m++){
        painter.drawLine(xScaleFactor*m+xOffset,yOffset,xScaleFactor*m+xOffset,yScaleFactor * waveformBuffer.at(selectedFrame).at(m) + yOffset);
    }


    painter.setClipping(false);


    update();
}


// If user spins mouse wheel, change voltage scale.
void RecordPlot::wheelEvent(QWheelEvent *event)
{

        if (event->delta() > 0) {
             recordDialog->contractYScale();
        } else {
            recordDialog->expandYScale();
        }

}



QSize RecordPlot::minimumSizeHint() const
{
    return QSize(RECORDPLOT_X_SIZE, RECORDPLOT_Y_SIZE);
}

QSize RecordPlot::sizeHint() const
{
    return QSize(RECORDPLOT_X_SIZE, RECORDPLOT_Y_SIZE);
}

void RecordPlot::paintEvent(QPaintEvent * /* event */)
{
    QStylePainter stylePainter(this);
    stylePainter.drawPixmap(0, 0, pixmap);
}

void RecordPlot::closeEvent(QCloseEvent *event)
{
    // Perform any clean-up here before application closes.
    event->accept();
}

// Set the number of spikes that are plotted, superimposed, on the
// display.

// Clear spike display.
void RecordPlot::clearScope()
{
    drawAxisLines();
}



void RecordPlot::resizeEvent(QResizeEvent*) {
    // Pixel map used for double buffering.
    pixmap = QPixmap(size());
    pixmap.fill();
    initializeDisplay();
    updateRecordPlot();
}

void RecordPlot::initializeDisplay() {
    const int textBoxWidth = fontMetrics().width("+" + QString::number(yScale) + " " + QSTRING_MU_SYMBOL + "V");
    const int textBoxHeight = fontMetrics().height();
    frame = rect();
    frame.adjust(textBoxWidth + 5, textBoxHeight + 10, -8, -textBoxHeight - 10);

    // Initialize display.
    drawAxisText();
    drawAxisLines();
}
