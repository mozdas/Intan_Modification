#ifndef PSTHANALYSISDIALOG_H
#define PSTHANALYSISDIALOG_H


#include <QDialog>
#include <QtGui>


using namespace std;

class QPushButton;
class QComboBox;
class QSpinBox;
class LfpPlot;
class DigInPlot;
class LfpScopeDialog;
class SignalProcessor;
class SignalSources;
class SignalChannel;
class PsthPlot;
class PsthAnalysisPlot;
class PsthAnalysisDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PsthAnalysisDialog(PsthPlot *inPsthPlot,
                              SignalProcessor *inSignalProcessor,
                              SignalChannel *initialChannel, QWidget *parent = 0);
    void setYScale(int index);
    void setSampleRate(double newSampleRate);
    void updateWaveform();
    void setNewChannel(SignalChannel* newChannel);
    void expandYScale();
    void contractYScale();
    bool isRunning;
    void increaseAnalysisStimulus();
    void increaseAnalysisSpikes();

signals:

public slots:

private slots:
    void changeYScale(int index);
    void changeTScale(int index);
    void changePsthReseting(bool state);
    void startPsthAnalysis();
    void stopPsthAnalysis();
    void updateAnalysisPlot();
    void changeTStepScale(int index);
    void arrengeSearchForMinWindow(int index);
    //void setAnalysisType(int index);

private:

    int analysisType;

    QVector<int> yScaleList;
    QVector<int> tScaleList;
    QVector<int> tStepList;

    double sampleRate;
    double tStepInSeconds;
    double tScaleInSeconds;



    bool psthReseting;

    QComboBox *yScaleComboBox;
    QComboBox *tScaleComboBox;
    QComboBox *tStepComboBox;
    QComboBox *windowForAnalysisComboBox;
    QComboBox *analysisTypeComboBox;


    QPushButton *startButton;
    QPushButton *stopButton;

    QTimer *tStepTimer; //timer for analysis to run as frequent as defined in tStepComboBox

    SignalProcessor *signalProcessor;
    SignalChannel *currentChannel;
    PsthPlot *psthPlot;
    PsthAnalysisPlot *analysisPlot;



};

#endif // PSTHANALYSISDIALOG_H
