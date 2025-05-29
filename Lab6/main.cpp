#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QSpinBox>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QColor>
#include <QPainter>
#include <QDir>
#include <QMessageBox>
#include <iostream>
#include <cmath>
#include <vector>
#include <map>
#include <algorithm>

class BitVisualizer : public QMainWindow {
    Q_OBJECT

public:
    BitVisualizer(QWidget *parent = nullptr) : QMainWindow(parent) {
        auto *widget = new QWidget;
        auto *layout = new QVBoxLayout(widget);

        QPushButton *loadButton = new QPushButton("Загрузить изображение");
        connect(loadButton, &QPushButton::clicked, this, &BitVisualizer::loadImage);
        layout->addWidget(loadButton);

        QLabel *bitLabel = new QLabel("Выберите бит:");
        layout->addWidget(bitLabel);
        bitSelector = new QSpinBox;
        bitSelector->setRange(0, 7);
        layout->addWidget(bitSelector);

        QPushButton *processButton = new QPushButton("Преобразовать");
        connect(processButton, &QPushButton::clicked, this, &BitVisualizer::processImage);
        layout->addWidget(processButton);

        QPushButton *chiSquareButton = new QPushButton("Хи‑квадрат анализ");
        connect(chiSquareButton, &QPushButton::clicked, this, &BitVisualizer::chiSquareAnalysis);
        layout->addWidget(chiSquareButton);

        QPushButton *rsButton = new QPushButton("RS‑анализ");
        connect(rsButton, &QPushButton::clicked, this, &BitVisualizer::rsAnalysis);
        layout->addWidget(rsButton);

        QPushButton *aumpButton = new QPushButton("AUMP анализ");
        connect(aumpButton, &QPushButton::clicked, this, &BitVisualizer::aumpAnalysis);
        layout->addWidget(aumpButton);

        QPushButton *errRSButton = new QPushButton("RS: Ошибки 1-го/2-го рода");
        connect(errRSButton, &QPushButton::clicked, this, &BitVisualizer::computeErrorRatesRS);
        layout->addWidget(errRSButton);

        QPushButton *errAUMPButton = new QPushButton("AUMP: Ошибки 1-го/2-го рода");
        connect(errAUMPButton, &QPushButton::clicked, this, &BitVisualizer::computeErrorRatesAUMP);
        layout->addWidget(errAUMPButton);
        
        QPushButton *saveButton = new QPushButton("Сохранить результат");
        connect(saveButton, &QPushButton::clicked, this, &BitVisualizer::saveImage);
        layout->addWidget(saveButton);

        imageView = new QGraphicsView;
        layout->addWidget(imageView);

        setCentralWidget(widget);
    }

    virtual ~BitVisualizer() {}

private slots:
    void loadImage() {
        QString fileName = QFileDialog::getOpenFileName(this, "Выберите изображение", "", "Images (*.bmp *.png *.jpg *.pgm)");
        if (!fileName.isEmpty()) {
            image.load(fileName);
            updateView(image);
        }
    }

    void processImage() {
        if (image.isNull())
            return;
        int bit = bitSelector->value();
        QImage processed(image.size(), QImage::Format_Grayscale8);
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                int pixel = qGray(image.pixel(x, y));
                int bitValue = (pixel >> bit) & 1;
                processed.setPixel(x, y, bitValue ? qRgb(255, 255, 255) : qRgb(0, 0, 0));
            }
        }
        resultImage = processed;
        updateView(resultImage);
    }

    void chiSquareAnalysis() {
        if (image.isNull())
            return;
    
        int width = image.width();
        int height = image.height();
        std::vector<double> rowProbabilities(height);
    
        for (int y = 0; y < height; ++y) {
            double avgProb = 0.0;
            for (int channel = 0; channel < 3; ++channel) {
                int histogram[256] = {0};
                for (int x = 0; x < width; ++x) {
                    QRgb pixel = image.pixel(x, y);
                    int value = (channel == 0) ? qRed(pixel) : ((channel == 1) ? qGreen(pixel) : qBlue(pixel));
                    histogram[value]++;
                }
                double chi = 0.0;
                for (int k = 0; k < 128; ++k) {
                    int obs = histogram[2 * k];
                    int obs2 = histogram[2 * k + 1];
                    double expected = (obs + obs2) / 2.0;
                    if (expected == 0)
                        continue;
                    chi += ((obs - expected) * (obs - expected)) / expected;
                }
                double prob = 1.0 / (1.0 + chi);
                avgProb += prob;
            }
            avgProb /= 3.0;
            rowProbabilities[y] = avgProb;
        }
    
        double minProb = *std::min_element(rowProbabilities.begin(), rowProbabilities.end());
        double maxProb = *std::max_element(rowProbabilities.begin(), rowProbabilities.end());
        
        QImage overlay(image.size(), image.format());
        overlay.fill(Qt::black);

        double probRange = maxProb - minProb;
        std::cout << probRange << std::endl;
        //if (probRange < 0.05) {
        //    resultImage = image;
        //    updateView(resultImage);
        //    return;
        //}
    
        for (int y = 0; y < height; ++y) {
            double normProb = 0.0;
            if (maxProb > minProb)
                normProb = (rowProbabilities[y] - minProb) / (maxProb - minProb);
            std::cout << normProb << std::endl;
            QColor rowColor;
            if (normProb < 0.20)
                rowColor = QColor(255, 255, 255); // черный
            else if (normProb < 0.80)
                rowColor = QColor(128, 128, 128); // серый
            else
                rowColor = QColor(0, 0, 0); // белый
    
            for (int x = 0; x < width; ++x)
                overlay.setPixel(x, y, rowColor.rgb());
        }
    
        QImage blended(image.size(), image.format());
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                QColor orig = image.pixelColor(x, y);
                QColor over = overlay.pixelColor(x, y);
                int r = (orig.red() + over.red()) / 2;
                int g = (orig.green() + over.green()) / 2;
                int b = (orig.blue() + over.blue()) / 2;
                blended.setPixelColor(x, y, QColor(r, g, b));
            }
        }
    
        resultImage = blended;
        updateView(resultImage);
    }

    void aumpAnalysis() {
        if (image.isNull())
            return;
    
        
        int rows = image.height();
        int cols = image.width();
        const int totalPixels = rows * cols;
        std::vector<double> Xflat;
        Xflat.reserve(totalPixels);
        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < cols; x++) {
                Xflat.push_back(double(qGray(image.pixel(x, y))));
            }
        }
        
        int m = 8;          
        int d = 1;          
        int q = d + 1;
        int Kn = totalPixels / m;  
    
        std::vector<std::vector<double>> Y(m, std::vector<double>(Kn, 0.0));
        for (int k = 0; k < Kn; k++) {
            for (int i = 0; i < m; i++) {
                Y[i][k] = Xflat[i + k * m];
            }
        }
        
        std::vector<std::vector<double>> H(m, std::vector<double>(q, 0.0));
        for (int i = 0; i < m; i++) {
            double xi = double(i + 1) / m; 
            for (int j = 0; j < q; j++) {
                H[i][j] = pow(xi, j);
            }
        }
        
        std::vector<std::vector<double>> HtH(q, std::vector<double>(q, 0.0));
        for (int i = 0; i < q; i++) {
            for (int j = 0; j < q; j++) {
                double sum = 0.0;
                for (int k = 0; k < m; k++) {
                    sum += H[k][i] * H[k][j];
                }
                HtH[i][j] = sum;
            }
        }
        
        std::vector<std::vector<double>> invHtH(q, std::vector<double>(q, 0.0));
        if (q == 2) {
            double a = HtH[0][0], b = HtH[0][1],
                   c = HtH[1][0], d_val = HtH[1][1];
            double det = a * d_val - b * c;
            if (fabs(det) < 1e-10)
                det = 1e-10;
            invHtH[0][0] = d_val / det;
            invHtH[0][1] = -b / det;
            invHtH[1][0] = -c / det;
            invHtH[1][1] = a / det;
        }
        
        std::vector<std::vector<double>> A(q, std::vector<double>(m, 0.0));
        for (int i = 0; i < q; i++) {
            for (int k = 0; k < m; k++) {
                double sum = 0.0;
                for (int j = 0; j < q; j++) {
                    sum += invHtH[i][j] * H[k][j];
                }
                A[i][k] = sum;
            }
        }
        
        std::vector<std::vector<double>> Ypred(m, std::vector<double>(Kn, 0.0));
        std::vector<double> sigma2(Kn, 0.0);
        for (int k = 0; k < Kn; k++) {
            std::vector<double> p_coeff(q, 0.0);
            for (int i = 0; i < q; i++) {
                double sum = 0.0;
                for (int r = 0; r < m; r++) {
                    sum += A[i][r] * Y[r][k];
                }
                p_coeff[i] = sum;
            }
            for (int i = 0; i < m; i++) {
                double pred_val = 0.0;
                for (int j = 0; j < q; j++) {
                    pred_val += H[i][j] * p_coeff[j];
                }
                Ypred[i][k] = pred_val;
            }
            double sumSq = 0.0;
            for (int i = 0; i < m; i++) {
                double diff = Y[i][k] - Ypred[i][k];
                sumSq += diff * diff;
            }
            sigma2[k] = sumSq / (m - q);
            double sig_th = 1.0;
            if (sigma2[k] < sig_th * sig_th)
                sigma2[k] = sig_th * sig_th;
        }
        
        double sum_inv = 0.0;
        for (int k = 0; k < Kn; k++) {
            sum_inv += 1.0 / sigma2[k];
        }
        double s_n2 = Kn / sum_inv;
        
        std::vector<double> w_blocks(Kn, 0.0);
        for (int k = 0; k < Kn; k++) {
            w_blocks[k] = sqrt(s_n2 / (Kn * (m - q))) / sqrt(sigma2[k]);
        }
        
        double sum_val = 0.0;
        for (int k = 0; k < Kn; k++) {
            for (int i = 0; i < m; i++) {
                double r_val = Y[i][k] - Ypred[i][k];
                int intVal = int(round(Y[i][k]));
                int mod_val = intVal % 2;
                double Xbar_val = (mod_val == 0) ? (Y[i][k] + 1) : (Y[i][k] - 1);
                sum_val += w_blocks[k] * (Y[i][k] - Xbar_val) * r_val;
            }
        }
        double beta = sum_val;  
        lastAUMPValue = beta;
        int dispWidth = 400;
        int dispHeight = 200;
        QImage diagram(dispWidth, dispHeight, QImage::Format_RGB32);
        diagram.fill(Qt::white);
        QPainter painter(&diagram);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::black);
        painter.drawText(QRect(0, 0, dispWidth, dispHeight),
                         Qt::AlignCenter,
                         QString("AUMP beta: %1").arg(beta, 0, 'f', 4));
        painter.end();
    
        updateView(diagram);
    }

    void rsAnalysis() {
        if (image.isNull())
            return;
    
        int bw = 2, bh = 2; 
        std::vector<int> mask = {1, 0, 0, 1};
        std::vector<int> invMask(mask.size());
        for (size_t i = 0; i < mask.size(); ++i) {
            invMask[i] = -mask[i];
        }
    
        auto flip = [](int val) -> int {
            return (val & 1) ? (val - 1) : (val + 1);
        };
    
        auto invert_flip = [&](int val) -> int {
            return (val & 1) ? (val + 1) : (val - 1);
        };
    
        auto smoothness = [&](const std::vector<int>& pix) -> double {
            double sum = 0;
            for (size_t i = 0; i + 1 < pix.size(); ++i) {
                sum += std::abs(pix[i + 1] - pix[i]);
            }
            return sum;
        };
    
        auto lsb_flip = [&](const std::vector<int>& pix) -> std::vector<int> {
            std::vector<int> flipped;
            flipped.reserve(pix.size());
            for (int v : pix) {
                flipped.push_back(v ^ 1);  
            }
            return flipped;
        };
    
        auto get_group = [&](const std::vector<int>& pix, const std::vector<int>& mask) -> std::string {
            
            std::vector<int> flip_pix = pix;
            for (size_t i = 0; i < mask.size() && i < flip_pix.size(); ++i) {
                if (mask[i] == 1)
                    flip_pix[i] = flip(pix[i]);
                else if (mask[i] == -1)
                    flip_pix[i] = invert_flip(pix[i]);
            }
            double d1 = smoothness(pix);
            double d2 = smoothness(flip_pix);
            if (d1 > d2)
                return "S";
            else if (d1 < d2)
                return "R";
            return "U";
        };
    
        auto solve = [&](const std::map<std::string, int>& groups) -> double {
            int d0 = groups.at("R") - groups.at("S");
            int dm0 = groups.at("mR") - groups.at("mS");
            int d1  = groups.at("iR") - groups.at("iS");
            int dm1 = groups.at("imR") - groups.at("imS");
            double a = 2.0 * (d1 + d0);
            double b = dm0 - dm1 - d1 - 3 * d0;
            double c = d0 - dm0;
            double D = b * b - 4 * a * c;
            if (D < 0)
                return 0.0;
            b = -b; 
            if (std::abs(D) < 1e-5) { 
                double x = b / (2 * a);
                return x / (x - 0.5);
            }
            D = sqrt(D);
            double x1 = (b + D) / (2 * a);
            double x2 = (b - D) / (2 * a);
            if (std::abs(x1) < std::abs(x2))
                return x1 / (x1 - 0.5);
            return x2 / (x2 - 0.5);
        };
    
        int width = image.width();
        int height = image.height();
        int blocksInRow = width / bw;
        int blocksInCol = height / bh;
    
        std::map<std::string, int> counterR, counterG, counterB;
        std::vector<std::string> keys = {"R", "S", "U", "mR", "mS", "mU",
                                         "iR", "iS", "iU", "imR", "imS", "imU"};
        for (const auto &key : keys) {
            counterR[key] = 0;
            counterG[key] = 0;
            counterB[key] = 0;
        }
    
        for (int by = 0; by < blocksInCol; ++by) {
            for (int bx = 0; bx < blocksInRow; ++bx) {
                std::vector<int> blockR, blockG, blockB;
                blockR.reserve(bw * bh);
                blockG.reserve(bw * bh);
                blockB.reserve(bw * bh);
                for (int v = 0; v < bh; ++v) {
                    for (int u = 0; u < bw; ++u) {
                        int x = bx * bw + u;
                        int y = by * bh + v;
                        QRgb pixel = image.pixel(x, y);
                        blockR.push_back(qRed(pixel));
                        blockG.push_back(qGreen(pixel));
                        blockB.push_back(qBlue(pixel));
                    }
                }
                std::string gR = get_group(blockR, mask);
                std::string gG = get_group(blockG, mask);
                std::string gB = get_group(blockB, mask);
                counterR[gR] += 1;
                counterG[gG] += 1;
                counterB[gB] += 1;
                
                std::string mgR = get_group(blockR, invMask);
                std::string mgG = get_group(blockG, invMask);
                std::string mgB = get_group(blockB, invMask);
                counterR["m" + mgR] += 1;
                counterG["m" + mgG] += 1;
                counterB["m" + mgB] += 1;
                
                std::vector<int> blockR_flip = lsb_flip(blockR);
                std::vector<int> blockG_flip = lsb_flip(blockG);
                std::vector<int> blockB_flip = lsb_flip(blockB);
                
                std::string igR = get_group(blockR_flip, mask);
                std::string igG = get_group(blockG_flip, mask);
                std::string igB = get_group(blockB_flip, mask);
                counterR["i" + igR] += 1;
                counterG["i" + igG] += 1;
                counterB["i" + igB] += 1;
                
                std::string imagR = get_group(blockR_flip, invMask);
                std::string imagG = get_group(blockG_flip, invMask);
                std::string imagB = get_group(blockB_flip, invMask);
                counterR["im" + imagR] += 1;
                counterG["im" + imagG] += 1;
                counterB["im" + imagB] += 1;
            }
        }
    
        double resR = solve(counterR);
        double resG = solve(counterG);
        double resB = solve(counterB);
        double rsValue = (resR + resG + resB) / 3.0;
        lastRSValue = rsValue;
        int dispWidth = 400;
        int dispHeight = 200;
        QImage diagram(dispWidth, dispHeight, QImage::Format_RGB32);
        diagram.fill(Qt::white);
        QPainter painter(&diagram);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::black);
        painter.drawText(QRect(0, 0, dispWidth, dispHeight),
                         Qt::AlignCenter,
                         QString("RS value: %1").arg(rsValue, 0, 'f', 4));
        painter.end();
    
        updateView(diagram);
    }

    void computeErrorRatesRS() {
        QString paths[3] = {"/home/Steganography/Lab6/clean","/home/Steganography/Lab6/half","/home/Steganography/Lab6/full"};
        double thr = 0.05;
        int TP=0, FP=0, FN=0, TN=0;
        for (int i = 0; i < 3; ++i) {
            QDir dir(paths[i]);
            QStringList files = dir.entryList({"*.png","*.bmp","*.jpg","*.pgm"}, QDir::Files);
            bool stego = (i > 0);
            for (auto &f : files) {
                QImage img(paths[i] + "/" + f);
                image = img;
                rsAnalysis();
                bool det = (lastRSValue >= thr);
                if (stego) det ? ++TP : ++FN;
                else det ? ++FP : ++TN;
            }
        }
        double err1 = double(FP) / (FP + TN);
        double err2 = double(FN) / (FN + TP);
        QMessageBox::information(this, "RS Ошибки", QString("Ошибка 1-го рода: %1\nОшибка 2-го рода: %2").arg(err1).arg(err2));
    }

    void computeErrorRatesAUMP() {
        QString paths[3] = {"/home/Steganography/Lab6/clean","/home/Steganography/Lab6/half","/home/Steganography/Lab6/full"};
        double thr = 0.01;
        int TP=0, FP=0, FN=0, TN=0;
        for (int i = 0; i < 3; ++i) {
            QDir dir(paths[i]);
            QStringList files = dir.entryList({"*.png","*.bmp","*.jpg","*.pgm"}, QDir::Files);
            bool stego = (i > 0);
            for (auto &f : files) {
                QImage img(paths[i] + "/" + f);
                image = img;
                aumpAnalysis();
                bool det = (lastAUMPValue >= thr);
                if (stego) det ? ++TP : ++FN;
                else det ? ++FP : ++TN;
            }
        }
        double err1 = double(FP) / (FP + TN);
        double err2 = double(FN) / (FN + TP);
        QMessageBox::information(this, "AUMP Ошибки", QString("Ошибка 1-го рода: %1\nОшибка 2-го рода: %2").arg(err1).arg(err2));
    }

    void computeErrorRatesCHI() {
        QString paths[3] = {"/home/Steganography/Lab6/clean","/home/Steganography/Lab6/half","/home/Steganography/Lab6/full"};
        double thr = 0.01;
        int TP=0, FP=0, FN=0, TN=0;
        for (int i = 0; i < 3; ++i) {
            QDir dir(paths[i]);
            QStringList files = dir.entryList({"*.png","*.bmp","*.jpg","*.pgm"}, QDir::Files);
            bool stego = (i > 0);
            for (auto &f : files) {
                QImage img(paths[i] + "/" + f);
                image = img;
                chiSquareAnalysis()
                bool det = (lastAUMPValue >= thr);
                if (stego) det ? ++TP : ++FN;
                else det ? ++FP : ++TN;
            }
        }
        double err1 = double(FP) / (FP + TN);
        double err2 = double(FN) / (FN + TP);
        QMessageBox::information(this, "AUMP Ошибки", QString("Ошибка 1-го рода: %1\nОшибка 2-го рода: %2").arg(err1).arg(err2));
    }

    void saveImage() {
        if (resultImage.isNull())
            return;
        QString savePath = QFileDialog::getSaveFileName(this, "Сохранить изображение", "", "Images (*.bmp *.png *.jpg)");
        if (!savePath.isEmpty())
            resultImage.save(savePath);
    }

    void updateView(const QImage &img) {
        QGraphicsScene *scene = new QGraphicsScene;
        scene->addPixmap(QPixmap::fromImage(img));
        imageView->setScene(scene);
    }

private:
    QImage image, resultImage;
    QSpinBox *bitSelector;
    QGraphicsView *imageView;
    double lastRSValue;
    double lastAUMPValue;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    BitVisualizer window;
    window.show();
    return app.exec();
}

#include "main.moc"