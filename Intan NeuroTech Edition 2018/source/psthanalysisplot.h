#ifndef PSTHANALYSISPLOT_H
#define PSTHANALYSISPLOT_H


#define PSTHANALYSISPLOT_X_SIZE 320
#define PSTHANALYSISPLOT_Y_SIZE 346
#define BUFFER_SIZE 25000
#include <QWidget>

using namespace std;

class SignalProcessor;
class PsthAnalysisDialog;
class LfpScopeDialog;
class SignalChannel;
class PsthPlot;


class PsthAnalysisPlot : public QWidget
{
    Q_OBJECT
public:
    explicit PsthAnalysisPlot(PsthPlot *inPsthPlot, SignalProcessor *inSignalProcessor, SignalChannel *initialChannel,
                       PsthAnalysisDialog *inPsthAnalysisDialog, QWidget *parent = 0);

    void setYScale(int newYScale);
    void setSampleRate(double newSampleRate);
    void updateWaveform();
    void setMaxNumLfpWaveforms();
    void clearScope();
    void setNewChannel(SignalChannel* newChannel);

    void setTScale(int t);
    void setTStepValue(int t);
    void changePsthReseting(bool);
    void changeAnalysisType(int);

    void applyStartProcess();
    void applyStopProcess();


    void setAnalysisWindow(double startInMs, double endInMs);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    PsthAnalysisDialog *psthAnalysisDialog;
    double numberStimulus;
    double numberSpikes;
signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent* event);

private:
    void drawAxisLines();
    void drawAxisText();
    void updatePsthAnalysisPlot();
    void initializeDisplay();

    SignalProcessor *signalProcessor;
    PsthPlot *psthPlot;




    //Necessary components for analysis based on overall LFP average
    QVector<double> analysisData;

    int numLfpWaveforms;
    int maxNumLfpWaveforms;
    int totalTSteps;
    double tLfpStepMsec;

    int analysisType;
    //search window descriptions
    int windowStartTimeIndex;
    int windowEndTimeIndex;

    double windowStartTimeInMs;
    double windowEndTimeInMs;

    bool startingNewChannel;
    bool psthReseting;

    int tScaleInSec;
    SignalChannel *selectedChannel;

    QRect frame;

    int tStepSec;
    int yScale;




    QPixmap pixmap;

    QVector<QVector<QColor> > scopeColors;

};


#endif // PSTHANALYSISPLOT_H
