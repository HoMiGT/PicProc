#pragma once

#include <QMainWindow>
#include <opencv2/opencv.hpp>
#include <QTimer>

using namespace cv;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_openPicture_clicked();

    void on_pushButton_algorithm_clicked();

    void on_pushButton_openCamera_clicked();

    void on_pushButton_shutdownCamera_clicked();

    void on_pushButton_takeAPhoto_clicked();

    void on_pushButton_fullScreen_clicked();

    void on_pushButton_exit_clicked();

    void readFrame();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QImage *image;
    VideoCapture *videoCapture;
    Mat matFrame;
    Mat matDst;
    bool bMethod{false};
    bool bCameraOpen{false};
    double lastTime{0};
    int screenWidth{0};
    int screenHeight{0};
};
