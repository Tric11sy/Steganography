#include <QApplication>
#include <QFileDialog>
#include <QInputDialog>
#include <QImage>
#include <QLabel>
#include <QHBoxLayout>
#include <QDialogButtonBox>

#include <QRandomGenerator>
#include <QTextEdit>
#include <QWidget>
#include <QDebug>
#include <QString>
#include <QVector>
#include <QPixmap>
#include <cmath>
#include <algorithm>
#include <iostream>

const QChar END_SYMBOL = QChar(0x0003);

QString getMultilineInput(QWidget *parent, bool &ok) {
    QDialog dialog(parent);
    dialog.setWindowTitle("Введите данные для скрытия");
    dialog.setMinimumSize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QTextEdit *textEdit = new QTextEdit(&dialog);
    layout->addWidget(textEdit);

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, &dialog);
    layout->addWidget(buttons);

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    ok = (dialog.exec() == QDialog::Accepted);
    return ok ? textEdit->toPlainText() : QString();
}

double computeMSE(const QImage &original, const QImage &stego) {
    if (original.size() != stego.size()) return -1;

    double mse = 0.0;
    int width = original.width();
    int height = original.height();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int origPixel = qGray(original.pixel(x, y));
            int stegoPixel = qGray(stego.pixel(x, y));
            mse += (origPixel - stegoPixel) * (origPixel - stegoPixel);
        }
    }
    mse /= (width * height);
    return mse;
}

double computePSNR(const QImage &original, const QImage &stego) {
    double mse = computeMSE(original, stego);
    if (mse == 0) return 100.0; 
    return 10 * log10((255 * 255) / mse);
}

double computeEmbeddingCapacity(const QImage &cover, const QImage &stego) {
    int w = stego.width();
    int h = stego.height();
    int blocksX = (w / 2) - 1;
    int blocksY = (h / 2) - 1;

    int embeddedBits = 0;

    for (int by = 0; by < blocksY; ++by) {
        for (int bx = 0; bx < blocksX; ++bx) {
            int X = 2 * bx;
            int Y = 2 * by;

            int p1 = qGray(cover.pixel(X, Y));
            int p2 = qGray(cover.pixel(X + 2, Y));
            int p3 = qGray(cover.pixel(X, Y + 2));
            int p4 = qGray(cover.pixel(X + 2, Y + 2));

            int omin = std::min({p1, p2, p3, p4});
            int omax = std::max({p1, p2, p3, p4});

            int v1 = qGray(stego.pixel(X, Y + 1)) - omin;
            int v2 = qGray(stego.pixel(X + 1, Y)) - omin;
            int v3 = qGray(stego.pixel(X + 1, Y + 1)) - omin;

            int a1 = (v1 == 0) ? 0 : int(std::log2(v1));
            int a2 = (v2 == 0) ? 0 : int(std::log2(v2));
            int a3 = (v3 == 0) ? 0 : int(std::log2(v3));

            embeddedBits += (a1 + a2 + a3);
        }
    }

    return double(embeddedBits) / (w * h);
}
void analyzeStegoImage(const QImage &original, const QImage &stego) {
    double psnr = computePSNR(original, stego);
    double capacity = computeEmbeddingCapacity(original, stego);

    std::cout << "Оценка ёмкости встраивания: " << capacity << " bpp" << std::endl;
    std::cout << "PSNR: " << psnr << " dB" << std::endl;
}

QVector<int> toBits(const QString &s) {
    QVector<int> result;
    for (int i = 0; i < s.size(); ++i) {
        int value = s.at(i).toLatin1();
        for (int b = 7; b >= 0; --b) {
            result.append((value >> b) & 1);
        }
    }
    return result;
}

QString fromBits(const QVector<int> &bits) {
    QString result;
    int nBytes = bits.size() / 8;
    for (int b = 0; b < nBytes; ++b) {
        int value = 0;
        for (int i = 0; i < 8; ++i) {
            value = (value << 1) | bits[b * 8 + i];
        }
        result.append(QChar(value));
    }
    return result;
}

QImage coverImage(const QImage &img) {
    int w = img.width();
    int h = img.height();
    QImage imgC(w * 2, h * 2, QImage::Format_Grayscale8);
    imgC.fill(0);

    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            int X = 2 * x;
            int Y = 2 * y;
            imgC.setPixelColor(X, Y, img.pixelColor(x, y));
        }
    }

    for (int x = 0; x < w - 1; ++x) {
        for (int y = 0; y < h - 1; ++y) {
            int X = 2 * x;
            int Y = 2 * y;
            int p1 = qGray(imgC.pixelColor(X, Y).rgb());
            int p2 = qGray(imgC.pixelColor(X + 2, Y).rgb());
            int p3 = qGray(imgC.pixelColor(X, Y + 2).rgb());
            int p4 = qGray(imgC.pixelColor(X + 2, Y + 2).rgb());
            int omax = std::max({ p1, p2, p3, p4 });
            int newVal1 = static_cast<int>((omax + (p1 + p3) / 2.0) / 2.0);
            int newVal2 = static_cast<int>((omax + (p1 + p2) / 2.0) / 2.0);
            int newVal3 = static_cast<int>((newVal1 + newVal2) / 2.0);
            imgC.setPixelColor(X, Y + 1, QColor(newVal1, newVal1, newVal1));
            imgC.setPixelColor(X + 1, Y, QColor(newVal2, newVal2, newVal2));
            imgC.setPixelColor(X + 1, Y + 1, QColor(newVal3, newVal3, newVal3));
        }
    }
    return imgC;
}
QImage img_orig;

QImage stegImage(QImage img, const QString &data) {
    int w = img.width();
    int h = img.height();
    img_orig = img;
    QVector<int> bdata = toBits(data);
    int dpointer = 0;

    for (int xBlock = 0; xBlock < (w / 2) - 1; ++xBlock) {
        for (int yBlock = 0; yBlock < (h / 2) - 1; ++yBlock) {
            int X = 2 * xBlock;
            int Y = 2 * yBlock;
            int p1 = qGray(img.pixelColor(X, Y).rgb());
            int p2 = qGray(img.pixelColor(X + 2, Y).rgb());
            int p3 = qGray(img.pixelColor(X, Y + 2).rgb());
            int p4 = qGray(img.pixelColor(X + 2, Y + 2).rgb());
            int omin = std::min({ p1, p2, p3, p4 });

            // Группа 1 (v1)
            int v1 = qGray(img.pixelColor(X, Y + 1).rgb()) - omin;
            int a1 = (v1 == 0) ? 0 : static_cast<int>(std::log2(v1));
            if(a1 > 0) {
                if(dpointer + a1 < bdata.size()){
                    dpointer += a1;
                    int sum = 0;
                    int m = 1;
                    for (int i = 0; i < a1; ++i) {
                        sum += m * bdata[dpointer - (i + 1)];
                        m *= 2;
                    }
                    int maxVal = std::max(qGray(img.pixelColor(X, Y).rgb()),
                                          qGray(img.pixelColor(X, Y + 2).rgb()));
                    int newPixel = std::max(0, maxVal - sum);
                    img.setPixelColor(X, Y + 1, QColor(newPixel, newPixel, newPixel));
                } else {
                    return img;
                }
            }

            // Группа 2 (v2)
            int v2 = qGray(img.pixelColor(X + 1, Y).rgb()) - omin;
            int a2 = (v2 == 0) ? 0 : static_cast<int>(std::log2(v2));
            if(a2 > 0) {
                if(dpointer + a2 < bdata.size()){
                    dpointer += a2;
                    int sum = 0;
                    int m = 1;
                    for (int i = 0; i < a2; ++i) {
                        sum += m * bdata[dpointer - (i + 1)];
                        m *= 2;
                    }
                    int maxVal = std::max(qGray(img.pixelColor(X, Y).rgb()),
                                          qGray(img.pixelColor(X + 2, Y).rgb()));
                    int newPixel = std::max(0, maxVal - sum);
                    img.setPixelColor(X + 1, Y, QColor(newPixel, newPixel, newPixel));
                } else {
                    return img;
                }
            }

            // Группа 3 (v3)
            int v3 = qGray(img.pixelColor(X + 1, Y + 1).rgb()) - omin;
            int a3 = (v3 == 0) ? 0 : static_cast<int>(std::log2(v3));
            if(a3 > 0) {
                if(dpointer + a3 < bdata.size()){
                    dpointer += a3;
                    int sum = 0;
                    int m = 1;
                    for (int i = 0; i < a3; ++i) {
                        sum += m * bdata[dpointer - (i + 1)];
                        m *= 2;
                    }
                    int maxVal = std::max(qGray(img.pixelColor(X + 1, Y).rgb()),
                                          qGray(img.pixelColor(X, Y + 1).rgb()));
                    int newPixel = std::max(0, maxVal - sum);
                    img.setPixelColor(X + 1, Y + 1, QColor(newPixel, newPixel, newPixel));
                } else {
                    return img;
                }
            }
        }
    }
    
    return img;
}

QVector<int> generateKey(int nBits, quint32 seed = 0xDEADBEEF) {
    QVector<int> key;
    key.reserve(nBits);
    QRandomGenerator rng(seed);
    for (int i = 0; i < nBits; ++i)
        key.append(rng.bounded(2)); // 0 или 1
    return key;
}

QString decode(const QImage &img) {
    analyzeStegoImage(img_orig, img);
    int w = img.width();
    int h = img.height();
    QVector<int> messageBits;
    for (int xBlock = 0; xBlock < (w / 2) - 1; ++xBlock) {
        for (int yBlock = 0; yBlock < (h / 2) - 1; ++yBlock) {
            int X = 2 * xBlock;
            int Y = 2 * yBlock;
            int p1 = qGray(img.pixelColor(X, Y).rgb());
            int p2 = qGray(img.pixelColor(X + 2, Y).rgb());
            int p3 = qGray(img.pixelColor(X, Y + 2).rgb());
            int p4 = qGray(img.pixelColor(X + 2, Y + 2).rgb());
            int omin = std::min({ p1, p2, p3, p4 });
            int omax = std::max({ p1, p2, p3, p4 });
            
            int v1 = static_cast<int>((omax + (p1 + p3) / 2.0) / 2.0) - omin;
            int v2 = static_cast<int>((omax + (p1 + p2) / 2.0) / 2.0) - omin;
            int v3 = static_cast<int>(((static_cast<int>((omax + (p1 + p3) / 2.0) / 2.0) +
                                        static_cast<int>((omax + (p1 + p2) / 2.0) / 2.0)) / 2.0)) - omin;
            
            // Группа 1
            int a1 = (v1 == 0) ? 0 : static_cast<int>(std::log2(v1));
            if(a1 > 0) {
                int sum = std::max(p1, p3) - qGray(img.pixelColor(X, Y + 1).rgb());
                QVector<int> temp(8, 0);
                for (int i = 0; i < a1; ++i) {
                    temp[a1 - (i + 1)] = sum % 2;
                    sum /= 2;
                }
                for (int i = 0; i < a1; ++i)
                    messageBits.append(temp[i]);
            }
            // Группа 2
            int a2 = (v2 == 0) ? 0 : static_cast<int>(std::log2(v2));
            if(a2 > 0) {
                int sum = std::max(p1, p2) - qGray(img.pixelColor(X + 1, Y).rgb());
                QVector<int> temp(8, 0);
                for (int i = 0; i < a2; ++i) {
                    temp[a2 - (i + 1)] = sum % 2;
                    sum /= 2;
                }
                for (int i = 0; i < a2; ++i)
                    messageBits.append(temp[i]);
            }
            // Группа 3
            int a3 = (v3 == 0) ? 0 : static_cast<int>(std::log2(v3));
            if(a3 > 0) {
                int sum = std::max(qGray(img.pixelColor(X + 1, Y).rgb()),
                                   qGray(img.pixelColor(X, Y + 1).rgb())) - qGray(img.pixelColor(X + 1, Y + 1).rgb());
                QVector<int> temp(8, 0);
                for (int i = 0; i < a3; ++i) {
                    temp[a3 - (i + 1)] = sum % 2;
                    sum /= 2;
                }
                for (int i = 0; i < a3; ++i)
                    messageBits.append(temp[i]);
            }
        }
    }
    QString decoded = fromBits(messageBits);
    //qDebug() << "DECODED1:" << decoded;
    QVector<int> key = generateKey(messageBits.size(), /*тот же seed*/ 12345);
    for (int i = 0; i < messageBits.size(); ++i)
        messageBits[i] ^= key[i];
    
    decoded = fromBits(messageBits);
    decoded.remove(QChar(0));
    //qDebug() << "DECODED2:" << decoded;
    int termIndex = decoded.indexOf(END_SYMBOL);
    if(termIndex != -1) {
        decoded = decoded.left(termIndex);
    }
    return decoded;
}



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString fileName = QFileDialog::getOpenFileName(nullptr, "Выберите изображение", "",
                                                    "Images (*.png *.jpg *.bmp *.pgm)");
    if (fileName.isEmpty()) {
        qDebug() << "Файл не выбран.";
        return 0;
    }
    QImage imgInput(fileName);
    if (imgInput.isNull()) {
        qDebug() << "Не удалось загрузить изображение.";
        return 0;
    }
    QImage imgGray = imgInput.convertToFormat(QImage::Format_Grayscale8);
    int w = imgGray.width();
    int h = imgGray.height();
    QImage imgOriginal = imgGray.scaled(w / 2, h / 2);
    QImage imgCover = coverImage(imgOriginal);

    // Replace small input dialog with large multiline
    bool ok;
    QString data = getMultilineInput(nullptr, ok);
    if (!ok || data.isEmpty()) {
        qDebug() << "Сообщение не введено.";
        return 0;
    }
    // Append special end symbol
    data.append(END_SYMBOL);
    data.append(END_SYMBOL);
    QVector<int> bits = toBits(data);
    QVector<int> key  = generateKey(bits.size(), /*можно передавать seed из пароля*/ 12345);
    for (int i = 0; i < bits.size(); ++i)
        bits[i] ^= key[i];
    data = fromBits(bits);

    
    qDebug() << "DATA:" << data;
    QImage imgStego = stegImage(imgCover, data);

    QWidget *window = new QWidget;
    window->setWindowTitle("Cover image (слева) и Stego image (справа)");
    QHBoxLayout *layout = new QHBoxLayout(window);

    QLabel *labelCover = new QLabel;
    labelCover->setPixmap(QPixmap::fromImage(imgCover));
    labelCover->setAlignment(Qt::AlignCenter);
    QLabel *labelStego = new QLabel;
    labelStego->setPixmap(QPixmap::fromImage(imgStego));
    labelStego->setAlignment(Qt::AlignCenter);

    layout->addWidget(labelCover);
    layout->addWidget(labelStego);
    window->setLayout(layout);
    window->show();

    // Decode and print strictly up to the end symbol
    QString extracted = decode(imgStego);
    int termIndex = extracted.indexOf(END_SYMBOL);
    if (termIndex != -1) {
        extracted = extracted.left(termIndex);
    }
    qDebug() << "Извлечённое сообщение:" << extracted;

    return app.exec();
}
