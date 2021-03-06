#pragma once

#ifdef QT_NO_DEBUG
    #ifndef QT_NO_DEBUG_OUTPUT
        #define QT_NO_DEBUG_OUTPUT
    #endif
#endif


#include "parameters/default_parameters_widget.hpp"
#include "parameters/constrained_parameters_widget.hpp"
#include "system/montecarlohost.hpp"
#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QtConcurrent/QtConcurrent>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QtDebug>
#include <QEvent>
#include <QTimer>
#include <iostream>
#include <atomic>


class BaseMCWidget : public QWidget
{
    Q_OBJECT
    
public:
    ~BaseMCWidget();
    
    bool getRunning();
    
    virtual void equilibrateAction() = 0;
    virtual void productionAction() = 0;
    virtual void pauseAction() = 0;
    virtual void abortAction() = 0;
    void saveAction();

    void setParameters(BaseParametersWidget*);
    
public slots:
    void setRunning(bool);
    void makeSystemNew();
    void makeRecordsNew();
    void makeSystemRandom();
    
signals:
    void resetSignal();
    void resetChartSignal();
    void runningSignal(bool);
    void drawRequest(const MonteCarloHost&, const unsigned long);
    void drawCorrelationRequest(const Histogram<double>&);
    void finishedSteps(const unsigned long);
    
protected:
    explicit BaseMCWidget(QWidget* parent = Q_NULLPTR);
    BaseMCWidget(const BaseMCWidget&) = delete;
    void operator=(const BaseMCWidget&) = delete;

    void server();
    
    BaseParametersWidget* prmsWidget = Q_NULLPTR;
    QPushButton* equilBtn = new QPushButton("Equilibration Run",this);
    QPushButton* prodBtn = new QPushButton("Production Run",this);
    QPushButton* pauseBtn = new QPushButton("Pause",this);
    QPushButton* abortBtn = new QPushButton("Abort",this);
    QPushButton* saveBtn = new QPushButton("Save sample data",this);
    
    QTimer* drawRequestTimer;
    
    std::atomic<bool> equilibration_mode {false};
    std::atomic<bool> simulation_running {false};
    std::atomic<bool> parameters_linked {false};
    
    MonteCarloHost MC {};
    
    std::atomic<unsigned long> steps_done {0};
    std::atomic<unsigned int>  drawRequestTime {100};  

private: 

};

