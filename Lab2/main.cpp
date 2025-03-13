#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>
#include <QImage>
#include <QGraphicsView>
#include <QGraphicsScene>

#include <random>
#include <iostream>

class ImageSteganography : public QMainWindow {
    Q_OBJECT

public:
    ImageSteganography(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("ЦВЗ Внедрение и Извлечение");
        auto *widget = new QWidget;
        auto *layout = new QVBoxLayout(widget);

        // Создаем графические представления для исходного и модифицированного изображений
        originalImageView = new QGraphicsView;
        modifiedImageView = new QGraphicsView;

        layout->addWidget(new QLabel("Исходное изображение:"));
        layout->addWidget(originalImageView);
        layout->addWidget(new QLabel("Изображение с внедренным знаком:"));
        layout->addWidget(modifiedImageView);

        QPushButton *loadButton = new QPushButton("Загрузить изображение");
        connect(loadButton, &QPushButton::clicked, this, &ImageSteganography::loadImage);
        layout->addWidget(loadButton);

        QPushButton *embedButton = new QPushButton("Внедрить ЦВЗ");
        connect(embedButton, &QPushButton::clicked, this, &ImageSteganography::embedImage);
        layout->addWidget(embedButton);

        QPushButton *extractButton = new QPushButton("Извлечь ЦВЗ");
        connect(extractButton, &QPushButton::clicked, this, &ImageSteganography::extractImage);
        layout->addWidget(extractButton);

        QPushButton *saveButton = new QPushButton("Сохранить изображение");
        connect(saveButton, &QPushButton::clicked, this, &ImageSteganography::saveImage);
        layout->addWidget(saveButton);

        setCentralWidget(widget);
    }

private slots:
    void loadImage() {
        QString fileName = QFileDialog::getOpenFileName(this, "Выберите изображение", "", "Images (*.bmp *.png *.jpg *.pgm *.jpeg)");
        if (!fileName.isEmpty()) {
            originalImage.load(fileName);
            updateOriginalView(originalImage);
            modifiedImage = QImage(); // Сбрасываем модифицированное изображение
            updateModifiedView(modifiedImage);
        }
    }

    ///*
    void embedImage() {
        if (originalImage.isNull()) return;

        // Создаем новое изображение для модификации
        modifiedImage = originalImage.copy(); // Копируем оригинальное изображение

        // Водяной знак (например, черно-белое изображение)
        QImage watermark = QImage("1.pgm").convertToFormat(QImage::Format_Grayscale8);

        if (watermark.isNull()) {
            qWarning("Не удалось загрузить водяной знак.");
            return;
        }

        // Убедимся, что водяной знак не больше исходного изображения
        if (watermark.width() > modifiedImage.width() || watermark.height() > modifiedImage.height()) {
            qWarning("Водяной знак больше, чем исходное изображение.");
            return;
        }

        // Параметры метода Куттера-Джордана-Боссена
        double alpha = 0.8; // Коэффициент масштабирования для интенсивности встраивания. Подбирается экспериментально.

        // Встраивание водяного знака методом Куттера-Джордана-Боссена
        for (int y = 0; y < watermark.height(); ++y) {
            for (int x = 0; x < watermark.width(); ++x) {
                // Получаем цвет пикселя водяного знака (от 0 до 255, так как Grayscale8)
                int watermarkGray = watermark.pixelColor(x, y).red(); // или .green(), так как grayscale

                // Получаем цвет пикселя исходного изображения
                QColor pixelColor = modifiedImage.pixelColor(x, y);

                // Вычисляем Luma (яркость) исходного пикселя
                double luma = 0.299 * pixelColor.red() + 0.587 * pixelColor.green() + 0.114 * pixelColor.blue();

                // Встраиваем бит водяного знака в синий канал
                int blue = pixelColor.blue();

                // Если watermarkGray > 127, внедряем 1, иначе 0
                int bit = (watermarkGray > 127) ? 1 : 0;
                blue += (int)(alpha * (bit * 255 - 128) * (luma / 255.0)); // Корректируем яркость

                // Клиппинг, чтобы значения не выходили за пределы [0, 255]
                blue = qBound(0, blue, 255);

                // Устанавливаем новый цвет пикселя
                modifiedImage.setPixelColor(x, y, QColor(pixelColor.red(), pixelColor.green(), blue));
            }
        }

        // Обновляем представление модифицированного изображения
        updateModifiedView(modifiedImage);
    }

    void extractImage() {
        if (modifiedImage.isNull() || originalImage.isNull()) return;
    
        // Размеры исходного изображения (и водяного знака)
        int originalWidth = originalImage.width();
        int originalHeight = originalImage.height();
    
        QImage extractedWatermark(originalWidth, originalHeight, QImage::Format_Grayscale8);
        extractedWatermark.fill(Qt::black);
    
        int neighborhoodSize = 1; // Размер окрестности (креста). 1 - это окрестность 3x3 пикселя
    
        for (int y = 0; y < originalHeight; ++y) {
            for (int x = 0; x < originalWidth; ++x) {
                double deltaBlueSum = 0.0;
                int validPixelCount = 0;
    
                for (int dy = -neighborhoodSize; dy <= neighborhoodSize; ++dy) {
                    for (int dx = -neighborhoodSize; dx <= neighborhoodSize; ++dx) {
                        if (abs(dx) + abs(dy) <= neighborhoodSize) {
                            int neighborX = x + dx;
                            int neighborY = y + dy;
    
                            if (neighborX >= 0 && neighborX < originalWidth && neighborY >= 0 && neighborY < originalHeight) {
                                double deltaBlue = modifiedImage.pixelColor(neighborX, neighborY).blue() -
                                                   originalImage.pixelColor(neighborX, neighborY).blue();
    
                                deltaBlueSum += deltaBlue;
                                validPixelCount++;
                            }
                        }
                    }
                }
    
                double avgDeltaBlue = (validPixelCount > 0) ? deltaBlueSum / validPixelCount : 0.0;
                int bit = (avgDeltaBlue > 0) ? 255 : 0;  // Еще больше упрощаем, убираем alpha
                extractedWatermark.setPixelColor(x, y, QColor(bit, bit, bit));
            }
        }
    
        updateModifiedView(extractedWatermark);
    }


void applyMedianFilter(QImage& image) {
    QImage filteredImage = image.copy();
    for (int y = 1; y < image.height() - 1; ++y) {
        for (int x = 1; x < image.width() - 1; ++x) {
            QList<int> pixels;
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    pixels.append(qGray(image.pixelColor(x + j, y + i).rgb()));
                }
            }
            std::sort(pixels.begin(), pixels.end());
            filteredImage.setPixelColor(x, y, QColor(pixels[4], pixels[4], pixels[4]));
        }
    }
    image = filteredImage;
}

    void saveImage() {
        if (modifiedImage.isNull()) return;

        QString fileName = QFileDialog::getSaveFileName(this, "Сохранить изображение", "", "Images (*.bmp *.png *.jpg *.pgm)");
        if (!fileName.isEmpty()) {
            modifiedImage.save(fileName);
        }
    }

    void updateOriginalView(const QImage &img) {
        QGraphicsScene *scene = new QGraphicsScene;
        scene->addPixmap(QPixmap::fromImage(img));
        originalImageView->setScene(scene);
    }

    void updateModifiedView(const QImage &img) {
        QGraphicsScene *scene = new QGraphicsScene;
        scene->addPixmap(QPixmap::fromImage(img));
        modifiedImageView->setScene(scene);
    }

private:
    QImage originalImage, modifiedImage;
    QGraphicsView *originalImageView;
    QGraphicsView *modifiedImageView;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ImageSteganography window;
    window.show();
    return app.exec();
}

#include "main.moc"

