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
#include <QGroupBox>
#include <QSpacerItem>

class BitVisualizer : public QMainWindow {
    Q_OBJECT

public:
    BitVisualizer(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Bit Visualizer");
        auto *widget = new QWidget;
        auto *layout = new QVBoxLayout(widget);

        // Группа для загрузки и сохранения изображений
        QGroupBox *imageGroup = new QGroupBox("Изображение");
        QVBoxLayout *imageLayout = new QVBoxLayout(imageGroup);

        QPushButton *loadButton = new QPushButton("Загрузить изображение");
        connect(loadButton, &QPushButton::clicked, this, &BitVisualizer::loadImage);
        imageLayout->addWidget(loadButton);

        QPushButton *saveButton = new QPushButton("Сохранить результат");
        connect(saveButton, &QPushButton::clicked, this, &BitVisualizer::saveImage);
        imageLayout->addWidget(saveButton);

        layout->addWidget(imageGroup);

        // Группа для выбора бита и преобразования
        QGroupBox *bitGroup = new QGroupBox("Преобразование");
        QVBoxLayout *bitLayout = new QVBoxLayout(bitGroup);

        QLabel *bitLabel = new QLabel("Выберите бит:");
        bitLayout->addWidget(bitLabel);

        bitSelector = new QSpinBox;
        bitSelector->setRange(0, 7);
        bitLayout->addWidget(bitSelector);

        QPushButton *processButton = new QPushButton("Преобразовать");
        connect(processButton, &QPushButton::clicked, this, &BitVisualizer::processImage);
        bitLayout->addWidget(processButton);

        layout->addWidget(bitGroup);

        // Добавляем пространство для отступов
        layout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

        // Отображение изображения
        imageView = new QGraphicsView;
        layout->addWidget(imageView);

        // Кнопка выхода
        QPushButton *exitButton = new QPushButton("Выход");
        connect(exitButton, &QPushButton::clicked, this, &QApplication::quit);
        layout->addWidget(exitButton);

        setCentralWidget(widget);
    }

private slots:
    void loadImage() {
        QString fileName = QFileDialog::getOpenFileName(this, "Выберите изображение", "", "Images (*.bmp *.png *.jpg *.pgm)");
        if (!fileName.isEmpty()) {
            image.load(fileName);
            updateView(image);
        }
    }
    
    void processImage() {
        if (image.isNull()) return;
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
    
    void saveImage() {
        if (resultImage.isNull()) return;
        QString savePath = QFileDialog::getSaveFileName(this, "Сохранить изображение", "", "Images (*.bmp *.png *.jpg *.pgm)");
        if (!savePath.isEmpty()) {
            resultImage.save(savePath);
        }
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
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    BitVisualizer window;
    window.show();
    return app.exec();
}

#include "main.moc"