#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPaintDevice>
#include <QPainter>
#include <QPaintEvent>
#include <QList>
#include <QMouseEvent>
#include <iostream>
#include <QTimer>
#include <QColorDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <math.h>
//#include <QCursor.h>

#define par false
#define impar true

#define raioDeSelecao 18


class MainWindow : public QWidget {
    Q_OBJECT


    //Estrutura de dados para o scan Line
    class aresta{
    public:
        int ymin;
        int ymax;
        int xmin;
        float inverseSlope;
        float overFlow;

        int dy;
        int dx;
    };



public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


public slots:

    void loop();
    void apagarSlot();
    void coresSlot();


private:

    //Estruturas de dados para o Scan Line
    //Todos os pontos
    QList<QPoint> pontosPoligono;
    //ET
    QList<aresta> edgeTable;
    //AET
    QList<aresta> activeEdgeTable;

    //Funções Auxiliares
    void scanLine(QPainter& paint);


    //Eventos
    void paintEvent(QPaintEvent * event);
    void mousePressEvent(QMouseEvent *event);

    //Variaveis internas framework
    QColorDialog * cores;
    QColor corAtual;
    bool modificarPonto = false;
    int indice;




};

#endif // MAINWINDOW_H
