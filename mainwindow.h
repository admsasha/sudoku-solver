#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QLabel>
#include <QKeyEvent>
#include <QEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private:
        Ui::MainWindow *ui;

        QVector<QLabel*> listSquares;
        QVector<QLabel*> emptySquares;
        QLabel *selectedSquares;
        QVector<QLabel*> listButtons;

        bool isRunSolve;
        bool manuallyStopFindSolution;

        void repaintSquares();
        bool isPossiblePutDigit(int indexSquares,int digit);    
        void trySolve(int startIndex);
        bool isSolveComplete();
        void repaintButtons();

    private slots:
        void clearTable();
        void solve();
        void showFormAbout();

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override;
        void keyPressEvent(QKeyEvent* event) override;

};

#endif // MAINWINDOW_H
