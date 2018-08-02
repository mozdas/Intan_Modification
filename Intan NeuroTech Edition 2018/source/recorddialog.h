#ifndef RECORDDIALOG_H
#define RECORDDIALOG_H


#include <QDialog>

using namespace std;

class QPushButton;
class QComboBox;
class QSpinBox;
class PsthDialog;
class SignalProcessor;
class SignalSources;
class SignalChannel;
class RecordPlot;

class RecordDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RecordDialog( QWidget *parent = 0);
    void setYScale(int index);
    //void setSampleRate(double newSampleRate);
    void updateWaveform(QVector <double> waveform,double tScale_Index);
    //void setVoltageThresholdDisplay(int value);
    //void setNewChannel(SignalChannel* newChannel);
    void expandYScale();
    void contractYScale();
    void updateRecordPlot();

signals:

public slots:

private slots:
    void changeYScale(int index);
    //void setTriggerType(int index);
    //void resetThresholdToZero();
    //void setNumSpikes(int index);
    void clearScope();
    //void setDigitalInput(int index);
    //void setVoltageThreshold(int value);
    //void setEdgePolarity(int index);
    //void applyToAll();
    void changeSelectedFrame(int frame);

private:
    QVector<double> yScaleList;


    //SignalProcessor *signalProcessor;
    //SignalSources *signalSources;
    //SignalChannel *currentChannel;

    //QPushButton *resetToZeroButton;
    QPushButton *clearScopeButton;
    //QPushButton *applyToAllButton;

    //QComboBox *triggerTypeComboBox;
    //QComboBox *numSpikesComboBox;
    //QComboBox *digitalInputComboBox;
    //QComboBox *edgePolarityComboBox;
    QComboBox *yScaleComboBox;
    QComboBox *selectedFrameComboBox;
    RecordPlot *recordPlot;

   // QSpinBox *thresholdSpinBox;

    //RecordPlot *spikePlot;
};
#endif // RECORDDIALOG_H
