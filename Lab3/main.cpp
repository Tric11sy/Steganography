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

#include <QBitArray>
#include <QDebug>
#include <zlib.h> 


//Тестовый текст
QString aliceText() {
    return QStringLiteral("Алиса начала очень скучать: она сидела рядом с сестрой на берегу и ничего не делала. "
                           "Раза два она заглянула в книгу, которую читала ее сестра, но там не было ни картинок, ни разговоров. "
                           "«И что за польза от книги, — подумала она, — в которой нет разговоров или картинок?»\n\n"

                           "Тут она стала размышлять про себя (правда, с трудом, потому что в такой жаркий день чувствовала себя сонной и глупой), "
                           "стоит ли удовольствие плести венок из маргариток беспокойства идти собирать маргаритки, как вдруг совсем близко от нее, "
                           "пробежал Белый Кролик с розовыми глазами.\n\n"

                           "В этом не было ничего очень уж замечательного. Точно так же Алисе не показалось очень необычным, "
                           "когда она услышала, как Кролик говорил сам себе:\n\n"

                           "— О горе, горе! Я опоздаю! (Позже Алиса вспоминала об этом, и ей пришло на ум, что она должна была бы удивиться, "
                           "но в то время все представлялось ей совершенно естественным.)\n\n"

                           "Когда же Кролик вынул часы из жилетного кармана, посмотрел на них и затем помчался еще быстрее, "
                           "Алиса вскочила на ноги, так как в ее голове блеснула мысль, что она никогда прежде не встречала кролика ни с жилетным карманом, "
                           "ни с часами, которые можно было бы вынимать оттуда. Сгорая от любопытства, она бросилась за ним и, к счастью, вовремя, "
                           "чтобы увидеть, как он внезапно нырнул в большую кроличью нору под изгородью.\n\n"

                           "Через мгновение Алиса скользнула туда вслед за Кроликом, не успев и подумать, какие силы в мире помогут ей выбраться обратно.\n\n"

                           "Кроличья нора сначала шла прямо, подобно туннелю, и затем неожиданно обрывалась вниз, так неожиданно, "
                           "что Алиса, не имея ни секунды, чтобы остановиться, упала в глубокий колодец.\n\n"

                           "Или колодец был очень глубок, или она падала очень медленно — во всяком случае, у нее было достаточно времени, "
                           "пока она падала, осматриваться вокруг и гадать, что произойдет дальше. Сначала Алиса попыталась заглянуть вниз, "
                           "стараясь понять, куда она падает, но там было слишком темно, чтобы увидеть хоть что-нибудь.\n\n"

                           "Тогда она принялась разглядывать стены колодца и заметила, что на них были буфетные и книжные полки. "
                           "Здесь и там она видела карты и картины, висящие на колышках. С одной из полок она сняла, летя вниз, "
                           "банку с наклейкой: «АПЕЛЬСИННЫЙ МАРМЕЛАД», но, к ее величайшему разочарованию, банка оказалась пустой. "
                           "Алиса не решилась бросить ее, боясь убить кого-нибудь внизу. И она изловчилась и поставила банку в один из буфетов, "
                           "мимо которого падала.\n\n"

                           "«Ну, — подумала Алиса, — после такого падения для меня просто пустяки слететь с лестницы. Какой храброй будут считать меня дома! "
                           "Да что там! Я не скажу ни словечка, даже если свалюсь с крыши». (Что было очень похоже на правду.)\n\n"

                           "Вниз, вниз, вниз… Кончится ли когда-нибудь падение?");
}

// Функции для сжатия и распаковки данных
QByteArray compressData(const QByteArray& data) {
    if (data.isEmpty()) return QByteArray();
    QByteArray compressedData;
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = (Bytef*)data.data();
    strm.avail_in = data.size();
    strm.next_out = (Bytef*)compressedData.data();
    strm.avail_out = compressedData.size();

    deflateInit(&strm, Z_BEST_COMPRESSION); // Настройка уровня сжатия

    // Рассчитываем предварительный размер буфера
    compressedData.resize(data.size() * 1.1 + 12); // Предполагаемый размер сжатых данных

    strm.next_out = (Bytef*)compressedData.data();
    strm.avail_out = compressedData.size();

    int ret = deflate(&strm, Z_FINISH);
    if (ret == Z_STREAM_ERROR || ret == Z_BUF_ERROR || ret == Z_MEM_ERROR) {
        qWarning() << "Ошибка сжатия: " << ret;
        deflateEnd(&strm);
        return QByteArray();
    }
    deflateEnd(&strm);
    compressedData.resize(strm.total_out); // Обрезаем буфер до фактического размера сжатых данных
    return compressedData;
}

QByteArray decompressData(const QByteArray& compressedData) {
    if (compressedData.isEmpty()) return QByteArray();
    QByteArray decompressedData;
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = (Bytef*)compressedData.data();
    strm.avail_in = compressedData.size();
    strm.next_out = (Bytef*)decompressedData.data();
    strm.avail_out = decompressedData.size();
    inflateInit(&strm);
    // Рассчитываем предварительный размер буфера
    decompressedData.resize(compressedData.size() * 4); // Предполагаемый размер распакованных данных

    strm.next_out = (Bytef*)decompressedData.data();
    strm.avail_out = decompressedData.size();

    int ret = inflate(&strm, Z_FINISH);

    if (ret == Z_STREAM_ERROR || ret == Z_BUF_ERROR || ret == Z_MEM_ERROR) {
        qWarning() << "Ошибка распаковки: " << ret;
        inflateEnd(&strm);
        return QByteArray();
    }
    decompressedData.resize(strm.total_out); // Обрезаем буфер до фактического размера распакованных данных
    inflateEnd(&strm);
    return decompressedData;
}


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
    /*
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
    
        // Параметры алгоритма
        int bitsToEmbed = 1; // Встраиваем 1 бит на пиксель
        int moduloValue = 4; // Используем модуль 2 (LSB)
    
    
        // Встраивание водяного знака
        for (int y = 0; y < watermark.height(); ++y) {
            for (int x = 0; x < watermark.width(); ++x) {
                // Получаем значение пикселя водяного знака (от 0 до 255)
                int watermarkGray = watermark.pixelColor(x, y).red();
    
                // Получаем цвет пикселя исходного изображения
                QColor pixelColor = modifiedImage.pixelColor(x, y);
    
                // Извлекаем компонент, в который будем встраивать (например, синий канал)
                int blue = pixelColor.blue();
    
    
                // 1. Получаем бит водяного знака
                int bit = (watermarkGray > 127) ? 1 : 0;
    
                // 2. Модифицируем выбранный канал
    
                // Вычисляем остаток от деления на moduloValue
                int currentValueModulo = blue % moduloValue;
    
                // Заменяем LSB или применяем операцию модификации в соответствии с битом водяного знака
                if (bit == 1) {
                    if (currentValueModulo != 1) { // Если LSB не равен 1, изменяем его
                        blue = blue + (1 - currentValueModulo);
                    }
                } else { // bit == 0
                    if (currentValueModulo != 0) { // Если LSB не равен 0, изменяем его
                        blue = blue - currentValueModulo;
                    }
    
                }
    
    
    
                // 3. Клиппинг
                blue = std::clamp(blue, 0, 255);
    
                // Устанавливаем новый цвет пикселя
                modifiedImage.setPixelColor(x, y, QColor(pixelColor.red(), pixelColor.green(), blue));
            }
        }
    
        // Обновляем представление модифицированного изображения
        updateModifiedView(modifiedImage);

        double psnr = calculatePSNR(originalImage, modifiedImage);
        std::cout << "PSNR: " << psnr << " dB" << std::endl;
    }

    void extractImage() {
        if (modifiedImage.isNull() || originalImage.isNull()) return;
    
        // Размеры измененного изображения (где может быть водяной знак)
        int modifiedWidth = modifiedImage.width();
        int modifiedHeight = modifiedImage.height();
    
        // Размеры исходного изображения
        int originalWidth = originalImage.width();
        int originalHeight = originalImage.height();
    
        QImage extractedWatermark(originalWidth, originalHeight, QImage::Format_Grayscale8);
        extractedWatermark.fill(Qt::black);  // Заполняем черным по умолчанию
    
        // Параметры алгоритма, которые должны совпадать с параметрами встраивания
        int bitsToEmbed = 1; // Количество бит, встроенных на пиксель
        int moduloValue = 4; // Используемый модуль (LSB)
    
        // Проходим только по области, где был внедрен водяной знак (размер водяного знака равен размеру оригинального изображения)
        for (int y = 0; y < originalHeight; ++y) {
            for (int x = 0; x < originalWidth; ++x) {
                // Получаем цвет пикселя измененного изображения
                QColor pixelColorModified = modifiedImage.pixelColor(x, y);
                int blueModified = pixelColorModified.blue();
    
                // Извлекаем бит из LSB
                int bit = blueModified % moduloValue;
    
                // Преобразуем бит в значение серого цвета (0 или 255)
                int grayValue = (bit == 1) ? 255 : 0;
    
                // Устанавливаем цвет пикселя в извлеченном водяном знаке
                extractedWatermark.setPixelColor(x, y, QColor(grayValue, grayValue, grayValue));
            }
        }
    
        updateModifiedView(extractedWatermark);
    }
    */


    void embedImage() {
        if (originalImage.isNull()) return;
    
        modifiedImage = originalImage.copy();
    
        // 1. Получаем сообщение (например, текст)
        QString message = aliceText();
        QByteArray messageBytes = message.toUtf8();
    
        // 2. Сжимаем сообщение
        QByteArray compressedMessage = compressData(messageBytes);
        if (compressedMessage.isEmpty()) {
            qWarning() << "Не удалось сжать сообщение.";
            return;
        }
    
        // 3. Преобразуем сжатое сообщение в битовую строку
        QBitArray messageBits(compressedMessage.size() * 8);
        for (int i = 0; i < compressedMessage.size(); ++i) {
            for (int j = 0; j < 8; ++j) {
                messageBits.setBit(i * 8 + j, (compressedMessage.at(i) >> j) & 1);
            }
        }
    
        // 4.  Параметры алгоритма
        int bitsToEmbed = 2; // Встраиваем 2 бита на пиксель (увеличиваем, чтобы компенсировать сжатие и вместить больше данных)
        int moduloValue = 4; // Используем модуль 4
        int bitIndex = 0;
    
        // 5.  Встраивание с учетом сжатия и битовой последовательности
        for (int y = 0; y < modifiedImage.height(); ++y) {
            for (int x = 0; x < modifiedImage.width(); ++x) {
                if (bitIndex >= messageBits.size()) {
                    break; // Закончили встраивать сообщение
                }
    
                QColor pixelColor = modifiedImage.pixelColor(x, y);
                int blue = pixelColor.blue();
    
                // Извлекаем биты сообщения для текущего пикселя
                int bit1 = messageBits.testBit(bitIndex++) ? 1 : 0;
                int bit2 = (bitIndex < messageBits.size() )? (messageBits.testBit(bitIndex++) ? 1 : 0): 0;
    
                // Преобразуем биты в значение, которое будем использовать для модификации
                int embedValue = (bit2 << 1) | bit1; //  bit2 * 2 + bit1;  // Создаем значение 0-3 на основе 2 бит
    
                // Применяем Combined LSB substitution and modulo function
                int currentValueModulo = blue % moduloValue;
                int delta = (embedValue - currentValueModulo + moduloValue) % moduloValue; // Гарантирует положительное изменение
    
                blue = blue + delta;
    
                // Клиппинг
                blue = std::clamp(blue, 0, 255);
    
                modifiedImage.setPixelColor(x, y, QColor(pixelColor.red(), pixelColor.green(), blue));
            }
        }
    
        updateModifiedView(modifiedImage);
        double psnr = calculatePSNR(originalImage, modifiedImage);
        qDebug() << "PSNR: " << psnr << " dB";
        qDebug() << "Размер исходного сообщения (байт): " << messageBytes.size();
        qDebug() << "Размер сжатого сообщения (байт): " << compressedMessage.size();
        qDebug() << "Количество внедренных бит: " << bitIndex;
    
    }
    
    void extractImage() {
        if (modifiedImage.isNull() || originalImage.isNull()) return;
    
        int bitsToEmbed = 2; // Должно соответствовать значению в embedImage()
        int moduloValue = 4;
        QBitArray extractedBits;
        extractedBits.resize(modifiedImage.width() * modifiedImage.height() * 2);  // Предварительное выделение памяти
    
        int bitIndex = 0;
    
        // 1. Извлечение битов
        for (int y = 0; y < modifiedImage.height(); ++y) {
            for (int x = 0; x < modifiedImage.width(); ++x) {
                QColor pixelColor = modifiedImage.pixelColor(x, y);
                int blue = pixelColor.blue();
    
                // Извлекаем биты
                int bit1 = (blue % moduloValue) & 1;
                int bit2 = ((blue % moduloValue) >> 1) & 1;
    
                // Используем setBit
                extractedBits.setBit(bitIndex++, bit1);
                extractedBits.setBit(bitIndex++, bit2);
            }
        }
    
        // 2.  Преобразование битов в байты
        QByteArray compressedMessageBytes;
        for (int i = 0; i + 7 < extractedBits.size(); i += 8) {
            char byte = 0;
            for (int j = 0; j < 8; ++j) {
                if (i + j < extractedBits.size()) { // Проверка выхода за границы
                    byte |= (extractedBits.testBit(i + j) << j);
                }
            }
            compressedMessageBytes.append(byte);
        }
    
        // 3. Распаковка сообщения
        QByteArray decompressedMessageBytes = decompressData(compressedMessageBytes);
        if (decompressedMessageBytes.isEmpty()) {
            qWarning() << "Не удалось распаковать сообщение.";
            return;
        }
    
        // 4. Отображаем извлеченное сообщение
        QString extractedMessage = QString::fromUtf8(decompressedMessageBytes);
        qDebug() << "Извлеченное сообщение:" << extractedMessage;
    }


    double calculatePSNR(const QImage& original, const QImage& modified) {
        if (original.isNull() || modified.isNull()) {
            qWarning("Одно или оба изображения не загружены.");
            return -1.0; // Или другое значение ошибки
        }
    
        if (original.size() != modified.size()) {
            qWarning("Размеры изображений не совпадают.");
            return -1.0; // Или другое значение ошибки
        }
    
        double mse = 0.0;
        int width = original.width();
        int height = original.height();
    
        // Вычисляем MSE (Mean Squared Error)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                QColor originalColor = original.pixelColor(x, y);
                QColor modifiedColor = modified.pixelColor(x, y);
    
                // Суммируем квадраты разностей для каждого канала (R, G, B)
                mse += std::pow(originalColor.red() - modifiedColor.red(), 2);
                mse += std::pow(originalColor.green() - modifiedColor.green(), 2);
                mse += std::pow(originalColor.blue() - modifiedColor.blue(), 2);
            }
        }
    
        // Нормируем MSE
        mse /= (double)(width * height * 3); // Учитываем 3 канала (R, G, B)
    
        // Вычисляем PSNR (Peak Signal-to-Noise Ratio)
        if (mse == 0) {
            // В этом случае PSNR стремится к бесконечности (идентичные изображения)
            return 99.0; // Возвращаем очень большое значение
        }
    
        double psnr = 10.0 * std::log10(std::pow(255.0, 2) / mse);
    
        return psnr;
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