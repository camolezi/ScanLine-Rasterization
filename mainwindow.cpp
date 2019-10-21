#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QWidget(parent){

    this->setMouseTracking(true);
    this->setMinimumSize(500,500);

    //Set timers for update loop
    QTimer * timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(loop()));
    timer->start(50);

    //Paleta de cores
    cores = new QColorDialog(this);
    corAtual = Qt::blue;

    //Botoês
    QPushButton * apagarBotao = new QPushButton("Apagar",this);
    connect(apagarBotao,SIGNAL(pressed()),this,SLOT(apagarSlot()));
    apagarBotao->move(0,10);

    QPushButton * coresBotao = new QPushButton("Cor",this);
    connect(coresBotao,SIGNAL(pressed()),this,SLOT(coresSlot()));
    coresBotao->move(apagarBotao->size().width(),10);

}

MainWindow::~MainWindow()
{

}

//Update Loop
void MainWindow::loop(){
    this->update();
}

//Escolhe a cor
void MainWindow::coresSlot(){
    corAtual = cores->getColor();
}

//Apaga os pontos
void MainWindow::apagarSlot(){
    pontosPoligono.clear();
}


void MainWindow::mousePressEvent(QMouseEvent *event){
    QPoint ponto = event->pos();
    //Checa se esta dentro do quadrado de seleção de algum ponto
    for(int x = 0; x < pontosPoligono.size(); x++){
        QPoint distancia = pontosPoligono.at(x) - ponto;
        if( (abs(distancia.x()) <= raioDeSelecao/2) && (abs(distancia.y()) <= raioDeSelecao/2 )){
            if(!modificarPonto){
                modificarPonto = true;
                indice = x;
                return;
            }
        }
    }

    if(!modificarPonto){
         pontosPoligono.append(ponto);
    }else{
         QPoint cursorPos = QCursor::pos();
         cursorPos = this->mapFromGlobal(cursorPos);
         pontosPoligono[indice] = cursorPos;
    }

    modificarPonto = false;


}

//Desenha o poligono
void MainWindow::paintEvent(QPaintEvent * event){

    //Desenhar Aqui
    QPainter painter(this);
    painter.setPen(corAtual);
    painter.setFont(QFont("Arial", 30));


    //Se estiver modificando o ponto
    //Atualiza o ponto
    if(modificarPonto){
        QPoint cursorPos = QCursor::pos();
        cursorPos = this->mapFromGlobal(cursorPos);
        pontosPoligono[indice] = cursorPos;
    }


    //Desenha o scan Line
    //Não forma um poligono
    if(pontosPoligono.size() < 3){
        for(int i = 0; i < pontosPoligono.size() -1 ; i++){
            painter.drawLine(pontosPoligono.at(i), pontosPoligono.at(i+1));
        }

    }else{
        //Pontos suficientes para formar um poligono e usar o scan line
        scanLine(painter);
    }

    //Caixa de seleção dos pontos
    painter.setPen(Qt::red);
    for(auto point : pontosPoligono){
        painter.drawRect(point.x() - raioDeSelecao/2 ,point.y() - raioDeSelecao/2,raioDeSelecao,raioDeSelecao);
    }



}



void MainWindow::scanLine(QPainter& paint){

    //Limpa as listas
    edgeTable.clear();
    activeEdgeTable.clear();

    //Gera a ET;
    for(int x = 0; x < pontosPoligono.size() - 1; x++){
        aresta aux;

        //Ymin
        aux.ymin = (pontosPoligono.at(x).y() <= pontosPoligono.at(x+1).y()) ? pontosPoligono.at(x).y() : pontosPoligono.at(x+1).y();
        aux.ymax = (pontosPoligono.at(x).y() > pontosPoligono.at(x+1).y()) ? pontosPoligono.at(x).y() : pontosPoligono.at(x+1).y();
        aux.xmin = (pontosPoligono.at(x).y() <= pontosPoligono.at(x+1).y()) ? pontosPoligono.at(x).x() : pontosPoligono.at(x+1).x();
        aux.inverseSlope = (float)(pontosPoligono.at(x + 1).x() - pontosPoligono.at(x).x() ) / (float) (pontosPoligono.at(x + 1).y() - pontosPoligono.at(x).y());
        aux.dx = (pontosPoligono.at(x + 1).x() - pontosPoligono.at(x).x() );
        aux.dy = (pontosPoligono.at(x + 1).y() - pontosPoligono.at(x).y());
        aux.overFlow = 0;
        //Adiciona aresta na ET
        edgeTable.append(aux);
    }


    //Volta para o primeiro ponto (Pegar a ultima aresta que liga o primeiro com o ultimo ponto)
    //Adiciona ultima aresta com o ponto iniical
    aresta aux;
    int lastSize = pontosPoligono.size() -1;
    aux.ymin = (pontosPoligono.at(0).y() <= pontosPoligono.at(lastSize).y()) ? pontosPoligono.at(0).y() : pontosPoligono.at(lastSize).y();
    aux.ymax = (pontosPoligono.at(0).y() > pontosPoligono.at(lastSize).y()) ? pontosPoligono.at(0).y() : pontosPoligono.at(lastSize).y();
    aux.xmin = (pontosPoligono.at(0).y() <= pontosPoligono.at(lastSize).y()) ? pontosPoligono.at(0).x() : pontosPoligono.at(lastSize).x();
    aux.inverseSlope = (float)(pontosPoligono.at(lastSize).x() - pontosPoligono.at(0).x() ) / (float) (pontosPoligono.at(lastSize).y() - pontosPoligono.at(0).y());
    aux.dx = (pontosPoligono.at(lastSize).x() - pontosPoligono.at(0).x() );
    aux.dy = (pontosPoligono.at(lastSize).y() - pontosPoligono.at(0).y());
    aux.overFlow = 0;
    edgeTable.append(aux);


    //Ordena ET por Ymin, (menor no final da lista)
    for(int x = 0; x < edgeTable.size(); x++){
        for(int y = 0; y < edgeTable.size() - 1 - x; y++){
            if(edgeTable[y].ymin < edgeTable[y+1].ymin){
                aresta aux;
                aux = edgeTable[y];
                edgeTable[y] = edgeTable[y+1];
                edgeTable[y+1] = aux;
            }
        }
    }


    //Inicializa a AET
    int yAtual = edgeTable.last().ymin;
    bool paridade = par;

    while(!edgeTable.isEmpty() || !activeEdgeTable.isEmpty()){

        //Transfera da ET para AET as arestas com ymin == yatual
        while(!edgeTable.isEmpty() && yAtual == edgeTable.last().ymin){
            activeEdgeTable.append(edgeTable.takeLast());
        }

        //Retira da AET a arestas com yatual == ymax
        auto iterator = activeEdgeTable.begin();
        while(iterator != activeEdgeTable.end()){
            if(iterator->ymax == yAtual){
                iterator = activeEdgeTable.erase(iterator);
            }else{
                iterator++;
            }
        }

        //Ordena AET por Xmin
        if(activeEdgeTable.size() >= 2){
            for(int x = 0; x < activeEdgeTable.size(); x++){
                for(int y = 0; y < activeEdgeTable.size() - 1 - x; y++){
                    if(activeEdgeTable.at(y).xmin > activeEdgeTable.at(y+1).xmin){
                        aresta aux;
                        aux = activeEdgeTable[y];
                        activeEdgeTable[y] = activeEdgeTable[y+1];
                        activeEdgeTable[y+1] = aux;
                    }
                }
            }
        }

        //Desenha os pixeis de acordo com a partidae
        if(!activeEdgeTable.isEmpty()){
            paridade = par;
            int auxLista = 0;
            for(int x = activeEdgeTable.first().xmin; x <= activeEdgeTable.last().xmin; x++){

                if(x == activeEdgeTable.at(auxLista).xmin){
                    paridade = !paridade;
                    auxLista++;
                }

                if(paridade == impar){
                    paint.drawPoint(x,yAtual);
                }
            }
        }

        //Aumenta o Yatual
        yAtual++;

        //Atualiza os novos X, baseados no coeficiente angular
        for(int it = 0; it < activeEdgeTable.size(); it++){
            activeEdgeTable[it].overFlow += activeEdgeTable[it].inverseSlope;
            if(activeEdgeTable[it].overFlow >= 1 || activeEdgeTable[it].overFlow <= -1){
                int over = (int)activeEdgeTable[it].overFlow;
                activeEdgeTable[it].xmin += over;
                activeEdgeTable[it].overFlow -= over;
            }
        }

    }

    return;
}



