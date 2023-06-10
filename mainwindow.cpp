#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCamera>
#include <QMediaDevices>
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QDateTime>
#include <iostream>

QPixmap Mat2QImage(Mat src);
float meanPixel(Mat img);
float autoGammaValue(Mat img);
void adaptiveGammaCorrection(Mat img,Mat& dst, float alpha=0);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    image = new QImage();
    connect(timer, SIGNAL(timeout()), this, SLOT(readFrame()));
    const QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();
    if (availableCameras.isEmpty()) {
        ui->pushButton_openCamera->setEnabled(false);
        ui->pushButton_shutdownCamera->setEnabled(false);
        ui->pushButton_takeAPhoto->setEnabled(false);
    }
    else {
        ui->pushButton_openCamera->setEnabled(true);
        ui->pushButton_shutdownCamera->setEnabled(true);
        ui->pushButton_takeAPhoto->setEnabled(true);
    }

    for (const auto& camera : availableCameras) {
        ui->comboBox_cameras->addItem(camera.description());
    }
    ui->comboBox_cameras->setCurrentIndex(0);

    ui->label_showImg1->setScaledContents(true);
    ui->label_showImg2->setScaledContents(true);
}

MainWindow::~MainWindow()
{
    if (bCameraOpen) {
        videoCapture->release();
    }
    videoCapture = nullptr;
    delete image;
    image = nullptr;
    delete timer;
    timer = nullptr;
    delete ui;
    ui = nullptr;
}


void MainWindow::on_pushButton_openPicture_clicked()
{
    QString img_name = QFileDialog::getOpenFileName(this,tr("Open Image"),"D:/CacheImg",tr("Image Files(*.jpg *.png *.bmp)"));
    if (img_name.isEmpty()){
        return;
    }
    matFrame = cv::imread(img_name.toStdString(),IMREAD_COLOR);

    Mat src = matFrame.clone();
    // 划分算法
    src.convertTo(src,CV_32FC3,1.0/255);
    Mat gauss;
    GaussianBlur(src,gauss,Size(101,101),0);
    // 划分
    // 如果混合色与基色相同则结果色为白色
    // 如果混合色为白色则结果色为基色不变
    // 如果混合色为黑色则结果色为白色
    matDst = src/gauss;

    double ratio = static_cast<double>(matFrame.cols)/static_cast<double>(matFrame.rows);
    double labelRatio = static_cast<double>(ui->label_showImg1->width())/static_cast<double>(ui->label_showImg1->height());


    // 展示原图
    QImage img1 = QImage(const_cast<unsigned char*>(matFrame.data),matFrame.cols,matFrame.rows,QImage::Format_BGR888);
    QPixmap scaledPixmap1 = QPixmap::fromImage(img1);


    // 展示新图
    matDst.convertTo(matDst,CV_8UC3,255.0);

    Mat matGamma;
    adaptiveGammaCorrection(matDst,matGamma,1.5);


    Mat gray;
    cvtColor(matGamma,gray,COLOR_BGR2GRAY);
    threshold(gray,gray,0,255,THRESH_BINARY+THRESH_OTSU);
    GaussianBlur(gray,gray,Size(5,5),0);

    QImage img2 = QImage(const_cast<unsigned char*>(gray.data),gray.cols,gray.rows,QImage::Format_Grayscale8);
    QPixmap scaledPixmap2 = QPixmap::fromImage(img2);

    if (ratio>labelRatio){
        scaledPixmap1 = scaledPixmap1.scaledToWidth(ui->label_showImg1->width(),Qt::SmoothTransformation);
        scaledPixmap2 = scaledPixmap2.scaledToWidth(ui->label_showImg1->width(),Qt::SmoothTransformation);
    }else{
        scaledPixmap1 = scaledPixmap1.scaledToHeight(ui->label_showImg1->height(),Qt::SmoothTransformation);
        scaledPixmap2 = scaledPixmap2.scaledToHeight(ui->label_showImg1->height(),Qt::SmoothTransformation);
    }

    ui->label_showImg1->setPixmap(scaledPixmap1);
    ui->label_showImg2->setPixmap(scaledPixmap2);

}


void MainWindow::on_pushButton_algorithm_clicked()
{
    bMethod=true;
    if (timer->isActive()){
        return;
    }
    timer->start(0);
}


void MainWindow::on_pushButton_openCamera_clicked()
{
    const int i = ui->comboBox_cameras->currentIndex();
    if (bCameraOpen){
        videoCapture->release();
    }
    videoCapture = new VideoCapture(i);
    bCameraOpen = true;
    if (timer->isActive()){
        return;
    }
    timer->start(0);
}


void MainWindow::on_pushButton_shutdownCamera_clicked()
{
    timer->stop();
    bCameraOpen = false;
    ui->label_showImg1->clear();
    ui->label_showImg2->clear();
}

void MainWindow::on_pushButton_takeAPhoto_clicked()
{
    QString path = "D:/CacheImg";
    QDir dir(path);
    if (!dir.exists()){
        if (!dir.mkpath(path)){
            QMessageBox::warning(nullptr,"警告","创建文件夹失败");
        }
    }
    const QString name = path + "/" + QDateTime::currentDateTime().toString("yyMMddhhmmss")
                         + ".png";
    imwrite(name.toStdString(),matDst);

}


void MainWindow::on_pushButton_fullScreen_clicked()
{
    this->showFullScreen();
}


void MainWindow::on_pushButton_exit_clicked()
{
    this->showNormal();
}

void MainWindow::readFrame()
{
    if (!bCameraOpen){
        return;
    }

    videoCapture->read(matFrame);
    Mat tmp;

    if (bMethod) {
        cvtColor(matFrame, tmp, COLOR_BGR2GRAY);
        Canny(tmp, tmp, 100, 255, 3);
        cvtColor(tmp, matDst, COLOR_GRAY2BGR);
    }
    else {
        matDst = matFrame.clone();
    }

    QPixmap qpixmap1 = Mat2QImage(matFrame);
    ui->label_showImg1->setPixmap(qpixmap1);
    QPixmap qpixmap2 = Mat2QImage(matDst);
    ui->label_showImg2->setPixmap(qpixmap2);
    double d_fps = 1.0 / ((static_cast<double>(getTickCount()) - lastTime) / getTickFrequency());
    QString str_fps = QString::fromLatin1("FPS: %3").arg(static_cast<int>(qRound(d_fps)));
    ui->label_fps->setText(str_fps);
    d_fps = 1000 * (static_cast<double>(getTickCount()) - lastTime) / getTickFrequency();
    str_fps = QString::fromLatin1("%3 MS").arg(static_cast<double>(qRound(d_fps)));
    ui->label_fps_ps->setText(str_fps);
//    qDebug() << "d_fps: " <<d_fps;
    lastTime = static_cast<double>(getTickCount());
}


QPixmap Mat2QImage(Mat src) {
    QImage img;
    Mat tmp;

    if (src.channels() == 3) {
        cvtColor(src, tmp, COLOR_BGR2RGB);
        img = QImage(const_cast<unsigned char*>(tmp.data), tmp.cols, tmp.rows, QImage::Format_RGB888);
    }
    else {
        img = QImage(const_cast<unsigned char*>(src.data), src.cols, src.rows, QImage::Format_Grayscale8);
    }

    QPixmap qimg = QPixmap::fromImage(img);
    return qimg;
}




// 摘自 https://github.com/GeorgeSeif/Image-Processing-OpenCV/blob/master/Gamma%20Correction%20and%20White%20Balance/enhance.cpp
float meanPixel(Mat img)
{
    if (img.channels() > 2)
    {
        cvtColor(img.clone(), img, COLOR_BGR2GRAY);
        return mean(img)[0];
    }
    else
    {
        return mean(img)[0];
    }
}

float autoGammaValue(Mat img)
{
    float middle_pixel = 128;
    float pixel_range = 256;
    float mean_l = meanPixel(img);

    float gamma = log(middle_pixel/pixel_range)/ log(mean_l/pixel_range); // Formula from ImageJ

    return gamma;

}


void adaptiveGammaCorrection(Mat img,Mat& dst, float alpha)
{

    CV_Assert(img.data);

    // Accept only char type matrices
    CV_Assert(img.depth() != sizeof(uchar));

    // Automatically compute the alpha value
    if (alpha == 0) {alpha = autoGammaValue(img);}

    // Get the image probability density function using the histogram
    Mat gray_img;
    if (img.channels() > 2)
    {
        cvtColor(img.clone(), gray_img, COLOR_BGR2GRAY);
    }


    // Establish the number of bins
    int histSize = 256;

    // Set the range
    float range[] = { 0, 256 };
    const float* histRange = { range };

    bool uniform = true; bool accumulate = false;

    Mat hist;

    // Compute the histogram
    calcHist(&gray_img, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);

    Mat norm_hist = hist / gray_img.total();
    double pdf_min = 0;
    double pdf_max = 0;
    minMaxLoc(norm_hist, &pdf_min, &pdf_max);

    std::vector<float> pdf_weights;
    for (int i = 0; i < 256; i++)
    {
        pdf_weights.push_back(pdf_max * (pow(( norm_hist.at<float>(i, 0) - pdf_min) / (pdf_max - pdf_min), alpha)  ));
    }

    std::vector<float> cdf_weights;
    for (int i = 0; i < 256; i++)
    {
        if (i == 0)
        {
            cdf_weights.push_back((pdf_weights[i]));
        }
        else
        {
            cdf_weights.push_back((pdf_weights[i] + cdf_weights[i-1]) );
        }

    }

    // Build look up table
    unsigned char lut[256];
    for (int i = 0; i < 256; i++)
    {
        float gamma = 1 - cdf_weights[i];
        lut[i] = saturate_cast<uchar>(pow((float)(i / 255.0), gamma) * 255.0f);
    }

    dst = img.clone();
    const int num_channels = dst.channels();
    switch (num_channels)
    {
    case 1:
    {
        MatIterator_<uchar> it, end;
        for (it = dst.begin<uchar>(), end = dst.end<uchar>(); it != end; it++)
            *it = lut[(*it)];

        break;
    }
    case 3:
    {
        MatIterator_<Vec3b> it, end;
        for (it = dst.begin<Vec3b>(), end = dst.end<Vec3b>(); it != end; it++)
        {

            (*it)[0] = lut[((*it)[0])];
            (*it)[1] = lut[((*it)[1])];
            (*it)[2] = lut[((*it)[2])];
        }

        break;

    }
    }
}

