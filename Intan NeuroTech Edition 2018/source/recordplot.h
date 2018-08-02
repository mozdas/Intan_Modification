#ifndef RECORDPLOT_H
#define RECORDPLOT_H


#define RECORDPLOT_X_SIZE 600
#define RECORDPLOT_Y_SIZE 400

#include <QWidget>

using namespace std;

class SignalProcessor;
class SpikeScopeDialog;
class SignalChannel;
class RecordDialog;

class RecordPlot : public QWidget
{
    Q_OBJECT
public:
    explicit RecordPlot(RecordDialog *inRecordDialog, QWidget *parent = 0);
    void setYScale(double newYScale);
    //void setSampleRate(double newSampleRate);
    void updateWaveform(QVector<double> psthWaveform,double tScale_Index);
    //void setMaxNumSpikeWaveforms(int num);
    void clearScope();
    void changeSelectedFrame(int frame);
    //void setVoltageTriggerMode(bool voltageMode);
    //void setVoltageThreshold(int threshold);
    //void setDigitalTriggerChannel(int channel);
    //void setDigitalEdgePolarity(bool risingEdge);
    //void setNewChannel(SignalChannel* newChannel);
    void updateRecordPlot();

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

    void initializeDisplay();

    SignalProcessor *signalProcessor;
    RecordDialog *recordDialog;


    QVector<QVector<double> > waveformBuffer;
    QVector<double> tScale;

    int waveformIndex;
    int numSpikeWaveforms;
    int selectedFrame;


    QRect frame;

    double tStepMsec;
    double yScale;




    QPixmap pixmap;

};

#endif // RECORDPLOT_H
