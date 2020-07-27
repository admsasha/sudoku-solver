#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <cmath>

#include <QFont>
#include <QBrush>
#include <QFrame>
#include <QMouseEvent>
#include <QMessageBox>
#include <QtDebug>

#include "config_sudoku_solver.h"
#include "FormAbout.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->width(),this->height());
    this->setWindowTitle(QString(tr("Sudoku solver v%1")).arg(APP_VERSION));
    this->setWindowIcon(QIcon(":/images/sudoku-solver.png"));

    QFont font;
    font.setPixelSize(30);

    QFont font2;
    font2.setPixelSize(12);
    font2.setBold(true);

    selectedSquares = nullptr;
    isRunSolve = false;

    // init Squares
    int x=0,y=0;
    for (int i=0;i<81;i++){
        QLabel *squares = new QLabel(ui->label);
        squares->setProperty("index",i);
        squares->setGeometry(x*48,y*48,48,48);
        squares->setText("");
        squares->setAlignment(Qt::AlignCenter);
        squares->setFont(font);
        squares->setFrameShape(QFrame::Box);
        squares->setFrameShadow(QFrame::Plain);
        squares->setAutoFillBackground(true);
        squares->installEventFilter(this);
        listSquares.push_back(squares);

        x++;
        if (x>=9){x=0;y++;}
    }

    // draw line
    for (int i=0;i<4;i++){
        QLabel *line1 = new QLabel(ui->label);
        if (i==0){
            line1->setGeometry(QRect(i*(3*48), 0, 4, ui->label->height()));
        } else if (i==3){
            line1->setGeometry(QRect(i*(3*48)-4, 0, 4, ui->label->height()));
        }else{
            line1->setGeometry(QRect(i*(3*48)-2, 0, 4, ui->label->height()));
        }
        line1->setFrameShape(QFrame::Box);
        line1->setFrameShadow(QFrame::Plain);
        line1->setLineWidth(2);

        QLabel *line2 = new QLabel(ui->label);
        if (i==0){
            line2->setGeometry(QRect(0, i*(3*48), ui->label->width(), 4));
        }else if (i==3){
            line2->setGeometry(QRect(0, i*(3*48)-4, ui->label->width(), 4));
        }else{
            line2->setGeometry(QRect(0, i*(3*48)-2, ui->label->width(), 4));
        }
        line2->setFrameShape(QFrame::Box);
        line2->setFrameShadow(QFrame::Plain);
        line2->setLineWidth(2);
    }

    // init button digital
    x=0;y=0;
    ui->label_11->setFrameShape(QFrame::NoFrame);
    for (int i=1;i<10;i++){
        QLabel *button = new QLabel(ui->label_11);
        button->setProperty("number",i);
        button->setGeometry(x*50,y*50,41,41);
        button->setText(QString::number(i));
        button->setAlignment(Qt::AlignCenter);
        button->setFont(font2);
        button->setFrameShape(QFrame::Box);
        button->setFrameShadow(QFrame::Plain);
        button->setAutoFillBackground(true);
        button->installEventFilter(this);
        listButtons.push_back(button);
        x++;
        if (x>=3){x=0;y++;}
    }

    ui->label_2->setStyleSheet("QLabel {border-style: solid;border-width: 1px;border-color: grey;background-color : #b0b0b0;}");
    ui->label_2->installEventFilter(this);

    ui->pushButton->installEventFilter(this);
    ui->pushButton_2->installEventFilter(this);
    ui->pushButton_4->installEventFilter(this);

    //connects
    connect(ui->pushButton,&QAbstractButton::clicked,this,&QApplication::exit);
    connect(ui->pushButton_2,&QAbstractButton::clicked,this,&MainWindow::solve);
    connect(ui->pushButton_3,&QAbstractButton::clicked,this,&MainWindow::showFormAbout);
    connect(ui->pushButton_4,&QAbstractButton::clicked,this,&MainWindow::clearTable);

    repaintSquares();
    repaintButtons();
}

MainWindow::~MainWindow(){
    delete ui;
}


void MainWindow::repaintSquares(){
    QString border_width="1px";
    QString background_color="#ffffff";



    // clear pallete
    for (QLabel *sq:listSquares){
        if (sq->text().isEmpty() or sq->property("manually").toInt()==0){
            background_color="#ffffff";
        }else{
            background_color="#d0d0d0";
        }

        sq->setStyleSheet("QLabel {"
                             "border-style: solid;border-width: 1px;border-color: grey;"
                             "background-color : "+background_color+";"
                             "}");
    }


    if (selectedSquares!=nullptr){
        if (selectedSquares->text().isEmpty() or selectedSquares->property("manually").toInt()==0){
            background_color="#ffffff";
        }else{
            background_color="#d0d0d0";
        }
        selectedSquares->setStyleSheet("QLabel {"
                             "border-style: solid;"
                             "border-width: 3px;"
                             "border-color: grey; "
                             "background-color : "+background_color+";"
                             "}");
    }


}

// Redraw button options
void MainWindow::repaintButtons(){
    for (QLabel *button:listButtons){
        if (selectedSquares!=nullptr){
            if (isPossiblePutDigit(selectedSquares->property("index").toInt(),button->property("number").toInt())){
                button->setEnabled(true);
                button->setStyleSheet("QLabel {border-style: solid;border-width: 1px;border-color: grey;background-color : #b0b0b0;}");
            }else{
                button->setEnabled(false);
                button->setStyleSheet("QLabel {border-style: solid;border-width: 1px;border-color: grey;background-color : #d0d0d0;}");
            }
        }else{
            button->setEnabled(false);
            button->setStyleSheet("QLabel {border-style: solid;border-width: 1px;border-color: grey;background-color : #d0d0d0;}");
        }
    }
}

void MainWindow::solve(){
    if (isRunSolve){
        manuallyStopFindSolution=true;
        return;
    }

    manuallyStopFindSolution=false;
    isRunSolve=true;
    selectedSquares=nullptr;
    ui->pushButton_4->setEnabled(false);
    ui->pushButton_2->setText(tr("Stop searching"));

    repaintSquares();
    repaintButtons();

    // prepare
    for (QLabel *square:listSquares){
        if (square->text().isEmpty()){
            square->setProperty("static",0);
        }else{
            square->setProperty("static",1);
        }
    }


    // step 1
    bool isChangeSquare=true;
    while(isChangeSquare){
        isChangeSquare=false;

        for (QLabel *square:listSquares){
            if (!square->text().isEmpty()) continue;

            int indexSquares = square->property("index").toInt();
            int absIndex = indexSquares-floor(indexSquares/9)*9;

            // Search in a small square
            int indexSmallSquaresX = floor(absIndex/3);
            int indexSmallSquaresY = floor(indexSquares/27);

            int countRight = (9-absIndex);
            int countLeft = 9-countRight;

            bool PerhapsOtherPlaces=false;

            // ////////////////////
            // method 1: "And who but me?"
            QVector<int> listChoice;
            for (int i = 1; i <= 9; i++) {
                if (isPossiblePutDigit(square->property("index").toInt(),i)){
                    listChoice.push_back(i);
                }
            }
            if (listChoice.size()==1){
                square->setText(QString::number(listChoice.at(0)));
                isChangeSquare=true;
            }


            // ////////////////////
            // method 2: "The last hero"
            for (int digit = 1; digit <= 9; digit++) {
                if (!isPossiblePutDigit(indexSquares,digit)) continue;

                PerhapsOtherPlaces=false;
                for (int x=0;x<3;x++){
                    for (int y=0;y<3;y++){
                        int index = x+(indexSmallSquaresX*3)+y*9+(indexSmallSquaresY*27);
                        if (index==indexSquares) continue;
                        if (!listSquares.at(index)->text().isEmpty()) continue;

                        if (isPossiblePutDigit(index,digit)){
                            PerhapsOtherPlaces=true;
                            break;
                        }
                    }
                    if (PerhapsOtherPlaces) break;
                }
                if (!PerhapsOtherPlaces){
                    square->setText(QString::number(digit));
                    isChangeSquare=true;
                    break;
                }
            }

            // method 3: "The last hero" in the line
            for (int digit = 1; digit <= 9; digit++) {
                if (!isPossiblePutDigit(indexSquares,digit)) continue;
                PerhapsOtherPlaces=false;

                for (int i = indexSquares+1; i < indexSquares+countRight; i++) {
                    if (!listSquares.at(i)->text().isEmpty()) continue;
                    if (isPossiblePutDigit(i,digit)) PerhapsOtherPlaces=true;
                }
                for (int i = indexSquares-countLeft; i < indexSquares; i++) {
                    if (!listSquares.at(i)->text().isEmpty()) continue;
                    if (isPossiblePutDigit(i,digit)) PerhapsOtherPlaces=true;
                }

                if (!PerhapsOtherPlaces){
                    square->setText(QString::number(digit));
                    isChangeSquare=true;
                    break;
                }
            }

            // method 4: "The last hero" in the column
            for (int digit = 1; digit <= 9; digit++) {
                if (!isPossiblePutDigit(indexSquares,digit)) continue;
                PerhapsOtherPlaces=false;

                for (int i = 0; i < 9; i++) {
                    if (absIndex+i*9==indexSquares) continue;
                    if (!listSquares.at(absIndex+i*9)->text().isEmpty()) continue;
                    if (isPossiblePutDigit(absIndex+i*9,digit)){
                        PerhapsOtherPlaces=true;
                    }
                }
                if (!PerhapsOtherPlaces){
                    square->setText(QString::number(digit));
                    isChangeSquare=true;
                    break;
                }
            }

        }

    }

    if (isSolveComplete()==false){
        // Trying brute-force
        emptySquares.clear();
        for (QLabel *square:listSquares){
            if (square->text().isEmpty()){
                emptySquares.push_back(square);
            }
        }
        trySolve(0);
    }


    if (isSolveComplete()==true){
        // If a solution is found
        QMessageBox::information(this,tr("Sudoku solver"),tr("Solving Sudoku is completed!"));
    }else{
        for (QLabel *square:listSquares){
            if (square->property("static").toInt()==0){
                square->setText("");
            }
        }

        if (!manuallyStopFindSolution){
            QMessageBox::information(this,tr("Sudoku solver"),tr("The solution to this Sudoku is not found!"));
        }
    }


    ui->pushButton_4->setEnabled(true);
    ui->pushButton_2->setText(tr("Find a solution"));
    isRunSolve=false;
}

void MainWindow::showFormAbout(){
    FormAbout about(this);
    about.exec();
}

void MainWindow::clearTable(){
    for (QLabel *square:listSquares){
        square->setText("");
        square->setProperty("manually",0);
    }
    repaintButtons();
    repaintSquares();
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event){
    if (!isRunSolve){
        if (event->type() == QEvent::MouseButtonRelease){
            for (QLabel *square:listSquares){
                if (square==obj){
                    selectedSquares=square;
                    repaintButtons();
                    repaintSquares();
                }
            }

            for (QLabel *button:listButtons){
                if (button==obj){
                    if (selectedSquares!=nullptr and button->isEnabled()){
                        selectedSquares->setText(button->property("number").toString());
                        selectedSquares->setProperty("manually",1);
                        repaintSquares();
                    }
                }
            }

            if (obj==ui->label_2){
                if (selectedSquares!=nullptr){
                    selectedSquares->setText("");
                    selectedSquares->setProperty("manually",0);
                    repaintSquares();
                }
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event){
    if (selectedSquares!=nullptr and !isRunSolve){
        if (event->key()>=49 and event->key()<=57){
            if (isPossiblePutDigit(selectedSquares->property("index").toInt(),event->text().toInt())){
                selectedSquares->setText(event->text());
                selectedSquares->setProperty("manually",1);
                repaintSquares();
            }
        }
        if (event->key()==Qt::Key_Backspace or event->key()==Qt::Key_Delete){    // delete digit
            selectedSquares->setText("");
            selectedSquares->setProperty("manually",0);
            repaintSquares();
        }
    }

    if (event->key() == Qt::Key_F7){        // start search solve
        solve();
    }

    // Moving the control cell
    if (selectedSquares!=nullptr and !isRunSolve){
        if (event->key() == Qt::Key_Left){
            if (selectedSquares->property("index").toInt()>0){
                selectedSquares=listSquares[selectedSquares->property("index").toInt()-1];
                repaintButtons();
                repaintSquares();
            }
        }
        if (event->key() == Qt::Key_Right){
            if (selectedSquares->property("index").toInt()<listSquares.size()-1){
                selectedSquares=listSquares[selectedSquares->property("index").toInt()+1];
                repaintButtons();
                repaintSquares();
            }
        }
        if (event->key() == Qt::Key_Up){
            if (selectedSquares->property("index").toInt()>=9){
                selectedSquares=listSquares[selectedSquares->property("index").toInt()-9];
                repaintButtons();
                repaintSquares();
            }
        }
        if (event->key() == Qt::Key_Down){
            if (selectedSquares->property("index").toInt()<listSquares.size()-9){
                selectedSquares=listSquares[selectedSquares->property("index").toInt()+9];
                repaintButtons();
                repaintSquares();
            }
        }
    }
}

// Is it possible to put this number in this field
bool MainWindow::isPossiblePutDigit(int indexSquares,int digit){

    int absIndex = indexSquares-floor(indexSquares/9)*9;

    int countRight = (9-absIndex);
    int countLeft = 9-countRight;

    // Horizontal search
    for (int i = indexSquares+1; i < indexSquares+countRight; i++) {
        if (listSquares.at(i)->text().isEmpty()) continue;
        if (listSquares.at(i)->text().toInt()==digit) return false;
    }
    for (int i = indexSquares-countLeft; i < indexSquares; i++) {
        if (listSquares.at(i)->text().isEmpty()) continue;
        if (listSquares.at(i)->text().toInt()==digit) return false;
    }

    // Vertical search
    for (int i = 0; i < 9; i++) {
        if (absIndex+i*9==indexSquares) continue;
        if (listSquares.at(absIndex+i*9)->text().isEmpty()) continue;
        if (listSquares.at(absIndex+i*9)->text().toInt()==digit) return false;
    }

    // Search in a small square
    int indexSmallSquaresX = floor(absIndex/3);
    int indexSmallSquaresY = floor(indexSquares/27);

    for (int x=0;x<3;x++){
        for (int y=0;y<3;y++){
            int index = x+(indexSmallSquaresX*3)+y*9+(indexSmallSquaresY*27);
            if (index==indexSquares) continue;
            if (listSquares.at(index)->text().toInt()==digit) return false;
        }

    }


    return true;
}

// Trying to find a solution
void MainWindow::trySolve(int startIndex){
    if (startIndex>=emptySquares.size()) return;

    QLabel *square = emptySquares.at(startIndex);

    for (int i=1;i<=9;i++){
        if (isPossiblePutDigit(square->property("index").toInt(),i)){
            square->setText(QString::number(i));
            QCoreApplication::instance()->processEvents();
            trySolve(startIndex+1);
            if (isSolveComplete()==true) return;
            if (!this->isVisible()) return;
            if (manuallyStopFindSolution) return;
        }
    }

    square->setText("");

    return;
}

// this solution is complete
bool MainWindow::isSolveComplete(){
    for (QLabel *square:listSquares){
        if (square->text().isEmpty()) return false;
    }

    return true;
}
